/* arch/arm/mach-msm/board-htcleo-audio.c
 *
 * Copyright (C) 2010 Cotulla
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

#include <linux/gpio.h>
#include <linux/delay.h>
#include <mach/msm_qdsp6_audio.h>
#include <mach/htc_acoustic_qsd.h>
#include <asm/gpio.h>
#include <mach/gpio.h>
#include <mach/htc_headset_mgr.h>

#include "board-htcleo.h"
#include "devices.h"
#include "dex_comm.h"
#include "proc_comm.h"
#include "pmic.h"


#if 1
#define D(fmt, args...) printk(KERN_INFO "Audio: "fmt, ##args)
#else
#define D(fmt, args...) do {} while (0)
#endif

static struct mutex mic_lock;
static struct mutex bt_sco_lock;

#if 1

// LEO
static struct q6_hw_info q6_audio_hw[Q6_HW_COUNT] =
{
    [Q6_HW_HANDSET] =
	{
        .min_gain = -1500,
        .max_gain = 500,
    },
    [Q6_HW_HEADSET] =
	{
        .min_gain = -1500,
        .max_gain = 500,
    },
    [Q6_HW_SPEAKER] =
	{
        .min_gain = -1000,
        .max_gain = 800,
    },
    [Q6_HW_TTY] =
	{
        .min_gain = 0,
        .max_gain = 0,
    },
    [Q6_HW_BT_SCO] =
	{
        .min_gain = -1100,
        .max_gain = 500,
    },
    [Q6_HW_BT_A2DP] =
	{
        .min_gain = -1100,
        .max_gain = 500,
    },
};

#else

// old desire one
static struct q6_hw_info q6_audio_hw[Q6_HW_COUNT] = 
{
    [Q6_HW_HANDSET] = {
        .min_gain = -2000,
        .max_gain = 0,
    },
    [Q6_HW_HEADSET] = {
        .min_gain = -2000,
        .max_gain = 0,
    },
    [Q6_HW_SPEAKER] = {
        .min_gain = -1500,
        .max_gain = 0,
    },
    [Q6_HW_TTY] = {
        .min_gain = -2000,
        .max_gain = 0,
    },
    [Q6_HW_BT_SCO] = {
        .min_gain = -2000,
        .max_gain = 0,
    },
    [Q6_HW_BT_A2DP] = {
        .min_gain = -2000,
        .max_gain = 0,
    },
};

#endif

extern int max9877_amp_write_register(char reg, char val);

void htcleo_headset_enable(int en)
{
    if (en)	
		{
		max9877_amp_write_register(0,1);
		max9877_amp_write_register(1,0x2c);
		}
    else
		max9877_amp_write_register(0,0);
}

void htcleo_speaker_enable(int en)
{
    if (en) 	
		{
		max9877_amp_write_register(0,1);
		max9877_amp_write_register(1,0x23);
		}
    else 
		max9877_amp_write_register(0,0);
}

void htcleo_receiver_enable(int en)
{
        if (en) 
        {
        } 
        else 
        {
	}
}
           
static uint32_t bt_sco_enable[] = 
{
    PCOM_GPIO_CFG(HTCLEO_BT_PCM_OUT, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
    PCOM_GPIO_CFG(HTCLEO_BT_PCM_IN, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
    PCOM_GPIO_CFG(HTCLEO_BT_PCM_SYNC, 2, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
    PCOM_GPIO_CFG(HTCLEO_BT_PCM_CLK, 2, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA)
};

static uint32_t bt_sco_disable[] = 
{
    PCOM_GPIO_CFG(HTCLEO_BT_PCM_OUT, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
    PCOM_GPIO_CFG(HTCLEO_BT_PCM_IN, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
    PCOM_GPIO_CFG(HTCLEO_BT_PCM_SYNC, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
    PCOM_GPIO_CFG(HTCLEO_BT_PCM_CLK, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA)
};

void htcleo_bt_sco_enable(int en)
{
    static int bt_sco_refcount;
    D("%s %d\n", __func__, en);

    mutex_lock(&bt_sco_lock);
    if (en) 
    {
        if (++bt_sco_refcount == 1)
        {
            config_gpio_table(bt_sco_enable, ARRAY_SIZE(bt_sco_enable));
        }        
    } 
    else 
    {
        if (--bt_sco_refcount == 0) 
        {
            config_gpio_table(bt_sco_disable, ARRAY_SIZE(bt_sco_disable));
            gpio_set_value(HTCLEO_BT_PCM_OUT, 0);
        }
    }
    mutex_unlock(&bt_sco_lock);
}

void htcleo_mic_enable(int en)
{
	 pmic_mic_en(en);
}


void htcleo_analog_init(void)
{
}

int htcleo_get_rx_vol(uint8_t hw, int level)
{
    int vol;

    if (level > 100)
        level = 100;
    else if (level < 0)
        level = 0;

    // TODO: is it correct?
    struct q6_hw_info *info;
    info = &q6_audio_hw[hw];
    vol = info->min_gain + ((info->max_gain - info->min_gain) * level) / 100;

    D("%s %d\n", __func__, vol);
    return vol;
}

static struct qsd_acoustic_ops acoustic = 
{
    .enable_mic_bias = htcleo_mic_enable,
};

static struct q6audio_analog_ops ops = 
{
    .init = htcleo_analog_init,
    .speaker_enable = htcleo_speaker_enable,
    .headset_enable = htcleo_headset_enable,
    .receiver_enable = htcleo_receiver_enable,
    .bt_sco_enable = htcleo_bt_sco_enable,
    .int_mic_enable = htcleo_mic_enable,
    .ext_mic_enable = htcleo_mic_enable,
    .get_rx_vol = htcleo_get_rx_vol,
};


void __init htcleo_audio_init(void)
{
    mutex_init(&mic_lock);
    mutex_init(&bt_sco_lock);
    q6audio_register_analog_ops(&ops);
    acoustic_register_ops(&acoustic);
}

