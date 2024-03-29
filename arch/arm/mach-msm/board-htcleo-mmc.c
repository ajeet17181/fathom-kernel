/* linux/arch/arm/mach-msm/board-htcleo-mmc.c
 *
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2009 HTC Corporation
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

#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/platform_device.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>

#include <mach/vreg.h>
#include <mach/gpio.h>

#include "board-htcleo.h"
#include "devices.h"
#include "proc_comm.h"
#include "pmic.h"

#undef HTCLEO_DEBUG_MMC
#define CONFIG_BCM4329_GPIO_WL_RESET 26

static bool opt_disable_sdcard;
static int __init htcleo_disablesdcard_setup(char *str)
{
	opt_disable_sdcard = (bool)simple_strtol(str, NULL, 0);
	return 1;
}

__setup("board_htcleo.disable_sdcard=", htcleo_disablesdcard_setup);

static uint32_t htcleo_sdslot_switchvdd(struct device *dev, unsigned int vdd)
{
	return 0;
}

static unsigned int htcleo_sdslot_status(struct device *dev)
{
	return 1;
}

#define HTCLEO_MMC_VDD	(MMC_VDD_165_195 | MMC_VDD_20_21 | \
				 MMC_VDD_21_22  | MMC_VDD_22_23 | \
				 MMC_VDD_23_24 | MMC_VDD_24_25 | \
				 MMC_VDD_25_26 | MMC_VDD_26_27 | \
				 MMC_VDD_27_28 | MMC_VDD_28_29 | \
				 MMC_VDD_29_30)

static unsigned int htcleo_sdslot_type = MMC_TYPE_SD;

static struct mmc_platform_data htcleo_sdslot_data =
{
	.ocr_mask		= HTCLEO_MMC_VDD,
	.status			= htcleo_sdslot_status,
	.register_status_notify	= NULL,
	.translate_vdd		= htcleo_sdslot_switchvdd,
	.slot_type		= &htcleo_sdslot_type,
};

static uint32_t wifi_on_gpio_table[] =
{
        PCOM_GPIO_CFG(63, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT3 */
        PCOM_GPIO_CFG(64, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT2 */
        PCOM_GPIO_CFG(65, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT1 */
        PCOM_GPIO_CFG(66, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT0 */
        PCOM_GPIO_CFG(67, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), /* CMD */
        PCOM_GPIO_CFG(62, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* CLK */
        PCOM_GPIO_CFG(28, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),  /* WLAN IRQ */
};

static uint32_t wifi_off_gpio_table[] =
{
        PCOM_GPIO_CFG(63, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* DAT3 */
        PCOM_GPIO_CFG(64, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* DAT2 */
        PCOM_GPIO_CFG(65, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* DAT1 */
        PCOM_GPIO_CFG(66, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* DAT0 */
        PCOM_GPIO_CFG(67, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* CMD */
        PCOM_GPIO_CFG(62, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* CLK */
        PCOM_GPIO_CFG(28, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),  /* WLAN IRQ */
};

static int htcleo_wifi_cd = 0; /* WIFI virtual 'card detect' status */
static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;

static int htcleo_wifi_status_register(
			void (*callback)(int card_present, void *dev_id),
			void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static unsigned int htcleo_wifi_status(struct device *dev)
{
	return htcleo_wifi_cd;
}

static struct mmc_platform_data htcleo_wifi_data = {
	.ocr_mask		= MMC_VDD_30_31,
	.status			= htcleo_wifi_status,
	.register_status_notify	= htcleo_wifi_status_register,
//	.embedded_sdio		= &htcleo_wifi_emb_data,
};

int htcleo_wifi_set_carddetect(int val)
{
	pr_info("%s: %d\n", __func__, val);
	htcleo_wifi_cd = val;
	if (wifi_status_cb) {
		wifi_status_cb(val, wifi_status_cb_devid);
	} else
		pr_warning("%s: Nobody to notify\n", __func__);
	return 0;
}

static int htcleo_wifi_power_state;

int htcleo_wifi_power(int on)
{
	printk("%s: %d\n", __func__, on);

	if (on) {
		config_gpio_table(wifi_on_gpio_table,ARRAY_SIZE(wifi_on_gpio_table));
		gpio_tlmm_config(PCOM_GPIO_CFG(CONFIG_BCM4329_GPIO_WL_RESET, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	        gpio_set_value(CONFIG_BCM4329_GPIO_WL_RESET, 1);
		mdelay(50);
	} else {
		config_gpio_table(wifi_off_gpio_table, ARRAY_SIZE(wifi_off_gpio_table));
            gpio_tlmm_config(PCOM_GPIO_CFG(CONFIG_BCM4329_GPIO_WL_RESET, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_DISABLE);
        	gpio_set_value(CONFIG_BCM4329_GPIO_WL_RESET, 0);
		mdelay(100);
	}
	
	mdelay(100);

	htcleo_wifi_power_state = on;
	return 0;
}

static int htcleo_wifi_reset_state;

int htcleo_wifi_reset(int on)
{
	printk("%s: do nothing\n", __func__);
	htcleo_wifi_reset_state = on;
	return 0;
}

int msm_add_sdcc(unsigned int controller, struct mmc_platform_data *plat,
		 unsigned int stat_irq, unsigned long stat_irq_flags);

int __init htcleo_init_mmc(unsigned debug_uart)
{
	printk("%s()+\n", __func__);

	msm_add_sdcc(1, &htcleo_sdslot_data,0,0);
        msm_add_sdcc(2, &htcleo_wifi_data,0,0);

	return 0;
}

#if defined(HTCLEO_DEBUG_MMC) && defined(CONFIG_DEBUG_FS)

static int htcleommc_dbg_wifi_reset_set(void *data, u64 val)
{
	htcleo_wifi_reset((int) val);
	return 0;
}

static int htcleommc_dbg_wifi_reset_get(void *data, u64 *val)
{
	*val = htcleo_wifi_reset_state;
	return 0;
}

static int htcleommc_dbg_wifi_cd_set(void *data, u64 val)
{
	htcleo_wifi_set_carddetect((int) val);
	return 0;
}

static int htcleommc_dbg_wifi_cd_get(void *data, u64 *val)
{
	*val = htcleo_wifi_cd;
	return 0;
}

static int htcleommc_dbg_wifi_pwr_set(void *data, u64 val)
{
	htcleo_wifi_power((int) val);
	return 0;
}

static int htcleommc_dbg_wifi_pwr_get(void *data, u64 *val)
{
	*val = htcleo_wifi_power_state;
	return 0;
}

static int htcleommc_dbg_sd_pwr_set(void *data, u64 val)
{
	htcleo_sdslot_switchvdd(NULL, (unsigned int) val);
	return 0;
}

static int htcleommc_dbg_sd_pwr_get(void *data, u64 *val)
{
	*val = sdslot_vdd;
	return 0;
}

static int htcleommc_dbg_sd_cd_set(void *data, u64 val)
{
	return -ENOSYS;
}

static int htcleommc_dbg_sd_cd_get(void *data, u64 *val)
{
	*val = htcleo_sdslot_data.status(NULL);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(htcleommc_dbg_wifi_reset_fops,
			htcleommc_dbg_wifi_reset_get,
			htcleommc_dbg_wifi_reset_set, "%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(htcleommc_dbg_wifi_cd_fops,
			htcleommc_dbg_wifi_cd_get,
			htcleommc_dbg_wifi_cd_set, "%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(htcleommc_dbg_wifi_pwr_fops,
			htcleommc_dbg_wifi_pwr_get,
			htcleommc_dbg_wifi_pwr_set, "%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(htcleommc_dbg_sd_pwr_fops,
			htcleommc_dbg_sd_pwr_get,
			htcleommc_dbg_sd_pwr_set, "%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(htcleommc_dbg_sd_cd_fops,
			htcleommc_dbg_sd_cd_get,
			htcleommc_dbg_sd_cd_set, "%llu\n");

static int __init htcleommc_dbg_init(void)
{
	struct dentry *dent;

	if (!machine_is_htcleo() && !machine_is_htcleo())
		return 0;

	dent = debugfs_create_dir("htcleo_mmc_dbg", 0);
	if (IS_ERR(dent))
		return PTR_ERR(dent);

	debugfs_create_file("wifi_reset", 0644, dent, NULL,
			    &htcleommc_dbg_wifi_reset_fops);
	debugfs_create_file("wifi_cd", 0644, dent, NULL,
			    &htcleommc_dbg_wifi_cd_fops);
	debugfs_create_file("wifi_pwr", 0644, dent, NULL,
			    &htcleommc_dbg_wifi_pwr_fops);
	debugfs_create_file("sd_pwr", 0644, dent, NULL,
			    &htcleommc_dbg_sd_pwr_fops);
	debugfs_create_file("sd_cd", 0644, dent, NULL,
			    &htcleommc_dbg_sd_cd_fops);
	return 0;
}

device_initcall(htcleommc_dbg_init);
#endif
