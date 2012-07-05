/* arch/arm/mach-msm/board-tsunagi-battery.c
*
* Copyright (C) 2010 Markinus
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


#include <linux/module.h>
#include <linux/param.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/android_alarm.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/wakelock.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>

#include <mach/msm_iomap.h>

////////////////////////////////////////////////////////////////////////////////////////
extern int msm_chg_usb_i_is_not_available(void);
extern int msm_chg_usb_charger_connected(void);
extern int msm_chg_usb_charger_disconnected(void);
extern int msm_chg_usb_chg_is_charging_proc(void);
extern int msm_chg_usb_chg_is_charger_valid_proc(void);
extern int msm_chg_usb_chg_is_battery_valid_proc(void);
// from board-tsunagi-power.c
extern int is_ac_power_supplied(void);


struct battery_status
{
	int timestamp;
	u8 percentage;		/* battery percentage */
	u8 charge_source;
	u8 charge_mode;
	u8 charger_valid;
	u8 battery_full;
} __attribute__((packed));


#define SOURCE_NONE	0
#define SOURCE_USB	1
#define SOURCE_AC	2

#define CHARGE_OFF	0
#define CHARGE_ON	1

/* When we're awake or running on wall power, sample the battery
* gauge every FAST_POLL seconds.  If we're asleep and on battery
* power, sample every SLOW_POLL seconds
*/
#define FAST_POLL	(60)
#define SLOW_POLL	(60)

struct tsunagi_device_info
{
	struct device *dev;

	struct battery_status status;

	struct power_supply bat;
	struct workqueue_struct *monitor_wqueue;
	struct work_struct monitor_work;
	struct alarm alarm;
	struct wake_lock work_wake_lock;

	u8 slow_poll;
	u8 last_charge_mode;

	ktime_t last_poll;
	ktime_t last_charge_seen;
};


#define psy_to_dev_info(x) container_of((x), struct tsunagi_device_info, bat)

static enum power_supply_property battery_properties[] =
{
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
};


extern void notify_cable_status(int);
static int battery_get_property(struct power_supply *psy, enum power_supply_property psp, union power_supply_propval *val);
static void tsunagi_program_alarm(struct tsunagi_device_info *di, int seconds);
static void battery_ext_power_changed(struct power_supply *psy);

#define to_tsunagi_device_info(x) container_of((x), struct tsunagi_device_info, bat);

static int battery_get_property(struct power_supply *psy, enum power_supply_property psp, union power_supply_propval *val)
{
	struct tsunagi_device_info *di = psy_to_dev_info(psy);

	switch (psp)
	{
	case POWER_SUPPLY_PROP_STATUS:
        switch (di->status.charge_source)
        {
        case CHARGE_OFF:
            val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
            break;
        case CHARGE_ON:
            if (di->status.battery_full)
                val->intval = POWER_SUPPLY_STATUS_FULL;
            else
                val->intval = POWER_SUPPLY_STATUS_CHARGING;
            break;
        default:
            val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
            break;
        }
        break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = di->status.charger_valid;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = di->status.percentage;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
extern int report_battery_percent(void);
static void tsunagi_battery_update_status(struct tsunagi_device_info *di)
{
	u8 last_level;
//	pr_info("%s\n", __func__);
	last_level = di->status.percentage;
//	di->status.percentage = (readl(MSM_SHARED_RAM_BASE + 0x43e88) & 0xff) * 10;
//	pr_info("BATTERY NE PERC: %d\n", di->status.percentage);
	di->status.percentage =	report_battery_percent();
//	di->status.percentage =	100;
	if (last_level != di->status.percentage)
	{
		  power_supply_changed(&di->bat);
	}
}

static int battery_adjust_charge_state(struct tsunagi_device_info *di)
{
	int old_charge_source = di->status.charge_source;
	if(msm_chg_usb_chg_is_charger_valid_proc()) {
		di->status.charger_valid = 1;
		if(msm_chg_usb_chg_is_charging_proc()) {
			di->status.charge_source = CHARGE_ON;
			di->status.battery_full = 0;
		}
		else {
			di->status.charge_source = CHARGE_OFF;
			di->status.battery_full = 1;
		}
	}
	else {
		di->status.charger_valid = 0;
		di->status.charge_source = CHARGE_OFF;
		di->status.battery_full = 0;
	}
	if(old_charge_source != di->status.charge_source)
		return 1;
	else
		return 0;
}

static void tsunagi_battery_work(struct work_struct *work)
{
	struct tsunagi_device_info *di = container_of(work, struct tsunagi_device_info, monitor_work);
	struct timespec ts;
	unsigned long flags;

	tsunagi_battery_update_status(di);

	di->last_poll = alarm_get_elapsed_realtime();

	if (battery_adjust_charge_state(di))
	{
		if(di->status.charger_valid) {
			msm_chg_usb_charger_connected();
		}
		else {
			msm_chg_usb_charger_disconnected();
			msm_chg_usb_i_is_not_available();
		}
		notify_cable_status(di->status.charger_valid);
		power_supply_changed(&di->bat);
	}

	ts = ktime_to_timespec(di->last_poll);
	di->status.timestamp = ts.tv_sec;

	/* prevent suspend before starting the alarm */
	local_irq_save(flags);
	wake_unlock(&di->work_wake_lock);
	tsunagi_program_alarm(di, FAST_POLL);
	local_irq_restore(flags);
}

//////////////////////////////////////////////////////////////////////////

static void tsunagi_program_alarm(struct tsunagi_device_info *di, int seconds)
{
	ktime_t low_interval = ktime_set(seconds - 10, 0);
	ktime_t slack = ktime_set(20, 0);
	ktime_t next;

	next = ktime_add(di->last_poll, low_interval);

	alarm_start_range(&di->alarm, next, ktime_add(next, slack));
}

static void tsunagi_battery_alarm(struct alarm *alarm)
{
	struct tsunagi_device_info *di = container_of(alarm, struct tsunagi_device_info, alarm);
	wake_lock(&di->work_wake_lock);
	queue_work(di->monitor_wqueue, &di->monitor_work);
}

static void battery_ext_power_changed(struct power_supply *psy)
{
	struct tsunagi_device_info *di;
	int got_power;

	di = psy_to_dev_info(psy);
	got_power = power_supply_am_i_supplied(psy);

	if (got_power)
	{
		if (is_ac_power_supplied())
		    di->status.charge_source = SOURCE_AC;
		else
		    di->status.charge_source = SOURCE_USB;
	}
	else
	{
		di->status.charge_source = SOURCE_NONE;
	}
	battery_adjust_charge_state(di);
	power_supply_changed(psy);
}

static int tsunagi_battery_probe(struct platform_device *pdev)
{
	int rc;
	struct tsunagi_device_info *di;

	pr_info("%s\n", __func__);
	
	if(!msm_chg_usb_chg_is_battery_valid_proc()) return -1;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
	{
	    return -ENOMEM;
	}

	di->bat.name = "battery";
	di->bat.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bat.properties = battery_properties;
	di->bat.num_properties = ARRAY_SIZE(battery_properties);
	di->bat.external_power_changed = battery_ext_power_changed;
	di->bat.get_property = battery_get_property;
	di->last_charge_mode = 0xff;

	rc = power_supply_register(&pdev->dev, &di->bat);
	if (rc)
	{
	    goto fail_register;
	}

	INIT_WORK(&di->monitor_work, tsunagi_battery_work);
	di->monitor_wqueue = create_freezeable_workqueue(dev_name(&pdev->dev));

	/* init to something sane */
	di->last_poll = alarm_get_elapsed_realtime();

	if (!di->monitor_wqueue)
	{
	    rc = -ESRCH;
	    goto fail_workqueue;
	}
	wake_lock_init(&di->work_wake_lock, WAKE_LOCK_SUSPEND, "tsunagi-battery");
	printk("alarm init\n");
	alarm_init(&di->alarm, ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP, tsunagi_battery_alarm);
	wake_lock(&di->work_wake_lock);
	queue_work(di->monitor_wqueue, &di->monitor_work);
	pr_info("%s done!\n", __func__);
	return 0;

fail_workqueue:
	power_supply_unregister(&di->bat);
fail_register:

	kfree(di);
	return rc;
}

static int tsunagi_suspend(struct device *dev)
{
	struct tsunagi_device_info *di = dev_get_drvdata(dev);

	/* If we are on battery, reduce our update rate until
	* we next resume.
	*/
	if (di->status.charge_source == SOURCE_NONE)
	{
		tsunagi_program_alarm(di, SLOW_POLL);
		di->slow_poll = 1;
	}
	return 0;
}

static int tsunagi_resume(struct device *dev)
{
	struct tsunagi_device_info *di = dev_get_drvdata(dev);

	/* We might be on a slow sample cycle.  If we're
	* resuming we should resample the battery state
	* if it's been over a minute since we last did
	* so, and move back to sampling every minute until
	* we suspend again.
	*/
	if (di->slow_poll)
	{
		tsunagi_program_alarm(di, FAST_POLL);
		di->slow_poll = 0;
	}
	return 0;
}

void notify_vbus_change_intr(void)
{
printk("notifiy vbs state change\n");
}

static struct dev_pm_ops tsunagi_pm_ops =
{
	.suspend	= tsunagi_suspend,
	.resume	= tsunagi_resume,
};

static struct platform_driver tsunagi_battery_driver =
{
	.probe	= tsunagi_battery_probe,
	.driver	=
	{
		.name	= "tsunagi_battery",
		.owner	= THIS_MODULE,
	//	.pm = &tsunagi_pm_ops,
      },
};

static int __init tsunagi_battery_init(void)
{
	pr_info("%s\n", __func__);
	return platform_driver_register(&tsunagi_battery_driver); 
}

late_initcall(tsunagi_battery_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Markinus");
MODULE_DESCRIPTION("tsunagi battery driver");
// END OF FILE
