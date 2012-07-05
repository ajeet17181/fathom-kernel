/*
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2009 HTC Corporation.
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

/* Control bluetooth power for htcleo platform */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/rfkill.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>

#include "gpio_chip.h"
#include "proc_comm.h"
#include "board-htcleo.h"

#define HTC_RFKILL_DBG
#define MAX_BT_SIZE 0x6U

static struct rfkill *bt_rfk;
static const char bt_name[] = "bcm4329";
static int pre_state;


/* Fix BT address for Leo */
static unsigned char bt_bd_ram[MAX_BT_SIZE] = {0x50,0xC3,0x00,0x00,0x00,0x00};
unsigned char *get_bt_bd_ram(void)
{
	return (bt_bd_ram);
}

#define GPIO_CFG PCOM_GPIO_CFG
static unsigned bt_config_power_on[] = {
        GPIO_CFG(22, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),   /* WAKE */
        GPIO_CFG(43, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),   /* RFR */
        GPIO_CFG(44, 2, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),   /* CTS */
        GPIO_CFG(45, 2, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),   /* Rx */
        GPIO_CFG(46, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),   /* Tx */
        GPIO_CFG(68, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),   /* PCM_DOUT */
        GPIO_CFG(69, 1, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),   /* PCM_DIN */
        GPIO_CFG(70, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),   /* PCM_SYNC */
        GPIO_CFG(71, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),   /* PCM_CLK */
        GPIO_CFG(21, 0, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),   /* HOST_WAKE */
};
static unsigned bt_config_power_off[] = {
        GPIO_CFG(22, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* WAKE */
        GPIO_CFG(43, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* RFR */
        GPIO_CFG(44, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* CTS */
        GPIO_CFG(45, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* Rx */
        GPIO_CFG(46, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* Tx */
        GPIO_CFG(68, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* PCM_DOUT */
        GPIO_CFG(69, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* PCM_DIN */
        GPIO_CFG(70, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* PCM_SYNC */
        GPIO_CFG(71, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* PCM_CLK */
        GPIO_CFG(21, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),  /* HOST_WAKE */
};




static void htcleo_config_bt_init(void)
{
}
#define GPIO_BT_RESET_N 23

static void htcleo_config_bt_on(void)
{
	int pin,rc;
	#ifdef HTC_RFKILL_DBG
	printk(KERN_INFO "-- RK ON --\n");
	#endif
	
	 for (pin = 0; pin < ARRAY_SIZE(bt_config_power_on); pin++) {
                        rc = gpio_tlmm_config(bt_config_power_on[pin],
                                        GPIO_ENABLE);
                        if (rc) {
                                printk(KERN_ERR
                                                "%s: gpio_tlmm_config(%#x)=%d\n",
                                                __func__, bt_config_power_on[pin], rc);
                        }
                }

                printk(KERN_DEBUG       "*****wait for regulator turned-on, 150ms*****\n");
	gpio_tlmm_config(GPIO_CFG(GPIO_BT_RESET_N, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), GPIO_ENABLE);  	
	     gpio_set_value(GPIO_BT_RESET_N, 0);
                /*reset assert for 120ms*/
                msleep (200);

                /*Power On Reset BCM4325D0*/
                gpio_set_value(GPIO_BT_RESET_N, 1);
                /*wait for 50ms for POR BCM2045*/
                msleep (200);

	 	
}

static void htcleo_config_bt_off(void)
{
	int pin,rc;
	#ifdef HTC_RFKILL_DBG
	printk(KERN_INFO "-- RK OFF --\n");
	#endif
	
	    for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off); pin++) {
                        rc = gpio_tlmm_config(bt_config_power_off[pin],
                                        GPIO_ENABLE);
                        if (rc) {
                                printk(KERN_ERR
                                                "%s: gpio_tlmm_config(%#x)=%d\n",
                                                __func__, bt_config_power_off[pin], rc);
                        }
                }
                gpio_set_value(GPIO_BT_RESET_N, 0);

}

static int bluetooth_set_power(void *data, bool blocked)
{
	if (pre_state == blocked) {
		#ifdef HTC_RFKILL_DBG
		printk(KERN_INFO "-- SAME ST --\n");
		#endif
		return 0;
	} else
		pre_state = blocked;

	if (!blocked)
		htcleo_config_bt_on();
	else
		htcleo_config_bt_off();

	return 0;
}

static struct rfkill_ops htcleo_rfkill_ops = {
	.set_block = bluetooth_set_power,
};

static int htcleo_rfkill_probe(struct platform_device *pdev)
{
	int rc = 0;
	bool default_state = true; /* off */

	htcleo_config_bt_init();	/* bt gpio initial config */

	bluetooth_set_power(NULL, default_state);

	bt_rfk = rfkill_alloc(bt_name, &pdev->dev, RFKILL_TYPE_BLUETOOTH,
						 &htcleo_rfkill_ops, NULL);
	if (!bt_rfk) {
		rc = -ENOMEM;
		goto err_rfkill_reset;
	}

	rfkill_set_states(bt_rfk, default_state, false);

	/* userspace cannot take exclusive control */
	rc = rfkill_register(bt_rfk);
	if (rc)
		goto err_rfkill_reg;

	return 0;

err_rfkill_reg:
	rfkill_destroy(bt_rfk);
err_rfkill_reset:
	return rc;
}

static int htcleo_rfkill_remove(struct platform_device *dev)
{
	rfkill_unregister(bt_rfk);
	rfkill_destroy(bt_rfk);

	return 0;
}

static struct platform_driver htcleo_rfkill_driver = {
	.probe = htcleo_rfkill_probe,
	.remove = htcleo_rfkill_remove,
	.driver = {
		.name = "htcleo_rfkill",
		.owner = THIS_MODULE,
	},
};

static int __init htcleo_rfkill_init(void)
{
	pre_state = -1;
//	if (!machine_is_htcleo())
//		return 0;

	return platform_driver_register(&htcleo_rfkill_driver);
}

static void __exit htcleo_rfkill_exit(void)
{
	platform_driver_unregister(&htcleo_rfkill_driver);
}

module_init(htcleo_rfkill_init);
module_exit(htcleo_rfkill_exit);
MODULE_DESCRIPTION("htcleo rfkill");
MODULE_AUTHOR("Markinus");
MODULE_LICENSE("GPL");
