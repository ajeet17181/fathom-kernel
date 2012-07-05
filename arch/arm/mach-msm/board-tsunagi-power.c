/* arch/arm/mach-msm/board-tsunagi-power.c
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <mach/msm_fast_timer.h>
#include <mach/msm_rpcrouter.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_rpcrouter.h>

#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/switch.h>

/* rpc related */
#define RPC_TIMEOUT (5*HZ)

static DEFINE_MUTEX(chg_mutex);
static struct work_struct vbus_work;
static int inited = 0;
static int usb_status;
static int vbus_present;

extern int msm_hsusb_cable_notifi_disconnect(void);
extern int msm_hsusb_rpc_connect(void);
extern int msm_chg_rpc_connect(void);
extern int msm_chg_usb_chg_is_battery_valid_proc(void);
extern int msm_rapp_rpc_connect(void);
extern int msm_rapc_enable_ds_channel(void);


#ifdef CONFIG_USB_ANDROID
extern void notify_usb_connected(int status);
static struct t_usb_status_notifier usb_status_notifier = {
	.name = "htc_battery",
	.func = notify_usb_connected,
};
#endif

static char *supply_list[] =
{
	"battery",
};

static int power_get_property(struct power_supply *psy,
                              enum power_supply_property psp,
                              union power_supply_propval *val)
{
	if (psp != POWER_SUPPLY_PROP_ONLINE)
	return -EINVAL;

	if (psy->type == POWER_SUPPLY_TYPE_MAINS)
	{
		val->intval = (usb_status == 2);
	}
	else
	{
		val->intval = vbus_present;
	}
	return 0;
}

static enum power_supply_property power_properties[] =
{
	POWER_SUPPLY_PROP_ONLINE,
};

static struct power_supply ac_supply =
{
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.supplied_to = supply_list,
	.num_supplicants = ARRAY_SIZE(supply_list),
	.properties = power_properties,
	.num_properties = ARRAY_SIZE(power_properties),
	.get_property = power_get_property,
};

static struct power_supply usb_supply =
{
	.name = "usb",
	.type = POWER_SUPPLY_TYPE_USB,
	.supplied_to = supply_list,
	.num_supplicants = ARRAY_SIZE(supply_list),
	.properties = power_properties,
	.num_properties = ARRAY_SIZE(power_properties),
	.get_property = power_get_property,
};

// called from RPC Callback
void notify_cable_status(int status)
{
	pr_info("%s: %d\n", __func__, status);
	vbus_present = status;
	msm_hsusb_set_vbus_state(vbus_present);
	power_supply_changed(&ac_supply);
	power_supply_changed(&usb_supply);
}
EXPORT_SYMBOL(notify_cable_status);

// called from USB driver
void notify_usb_connected(int status)
{
	if (!inited)    return;
	pr_info("%s: %d\n", __func__, status);
	usb_status = status;
	power_supply_changed(&ac_supply);
	power_supply_changed(&usb_supply);
}

// used by battery driver
int is_ac_power_supplied(void)
{
	return (usb_status == 2);
}

static void vbus_work_func(struct work_struct *work)
{
	int vbus = msm_chg_usb_chg_is_battery_valid_proc();
	printk("%s:  new vbus = %d\n", __func__, vbus);
	notify_cable_status(vbus);
}

//VBUS
void msm_otg_set_vbus_state(int online)
{
	printk("VBUS: %d\n", online);
}

static int tsunagi_power_probe(struct platform_device *pdev)
{
	int ret=0;
	pr_info("%s\n", __func__);

	INIT_WORK(&vbus_work, vbus_work_func);

	power_supply_register(&pdev->dev, &ac_supply);
	power_supply_register(&pdev->dev, &usb_supply);

	// Enabling DS0 channel, for first here, later put to other place
	ret = msm_rapp_rpc_connect();
	printk("RET RAPP CONNECT: %d\n", ret);
//	ret = msm_rapc_enable_ds_channel();
//	printk("RET DS ENABLE CONNECT: %d\n", ret);
	ret = msm_chg_rpc_connect();
	printk("RET CHG CONNECT: %d\n", ret);

	inited = 1;
	return 0;
}

static struct platform_driver tsunagi_power_driver =
{
	.probe	= tsunagi_power_probe,
	.driver	=
	{
		.name	= "tsunagi_power",
		.owner	= THIS_MODULE,
	},
};

static int __init tsunagi_power_init(void)
{
	pr_info("%s\n", __func__);
	platform_driver_register(&tsunagi_power_driver);
#ifdef CONFIG_USB_ANDROID
	usb_register_notifier(&usb_status_notifier);
#endif
	return 0;
}

//module_init(tsunagi_power_init);
late_initcall(tsunagi_power_init);
MODULE_AUTHOR("Markinus");
MODULE_DESCRIPTION("Tsunagi Power Driver");
MODULE_LICENSE("GPL");
