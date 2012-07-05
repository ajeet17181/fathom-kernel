/* linux/arch/arm/mach-msm/board-tsunagi-panel.c
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
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>

#include <linux/types.h>
#include <asm/io.h>
#include <asm/mach-types.h>

#include <mach/msm_fb.h>
#include <mach/msm_iomap.h>
#include <mach/vreg.h>

#include "proc_comm.h"
#include "board-htcleo.h"
#include "devices.h"

#define MODULE_NAME "lge"
#define dprintk(fmt, args...)   printk(KERN_INFO "%s:%s: " fmt, MODULE_NAME, __func__, ## args)
#define lgit_writew(reg, val)   serigo(reg, val)

/* LGE_CHANGES_S [cis@lge.com] 2010-02-17, [VS740] for sleep current */
#if 1//defined (CONFIG_MACH_MSM7X27_ALOHAV)
#include <mach/vreg.h>

#define MSM_VREG_OP(name, op, level) \
        do { \
                vreg = vreg_get(0, name); \
                vreg_set_level(vreg, level); \
                if (vreg_##op(vreg)) \
                        printk(KERN_ERR "%s: %s vreg operation failed \n", \
                                (vreg_##op == vreg_enable) ? "vreg_enable" \
                                        : "vreg_disable", name); \
        } while (0)
#endif
/* LGE_CHANGES_E [cis@lge.com] 2010-02-17, [VS740] for sleep current */

static int spi_cs;
static int spi_sclk;
static int spi_sdi;
static int spi_sdo;

struct lgit_state_type{
        int disp_initialized;
        int display_on;
        int disp_powered_up;
};

static struct lgit_state_type lgit_state = { 0 };
static struct msm_panel_lgit_pdata *lcdc_lgit_pdata;
static void lcdc_lgit_config_gpios(int enable);

/* Tunning data */
static unsigned int h_back_porch = 23;//0x51;//0x39;

static void lcdc_spi_pin_assign(void)
{
        spi_sclk = 150;
        spi_cs   = 151;
        spi_sdi  = 148;
        spi_sdo  = 149;
}

static void lgit_spi_init(void)
{
        /* Set the output so that we don't disturb the slave device */
        gpio_set_value(spi_sclk, 0);
        gpio_set_value(spi_sdi, 0);

        /* Set the Chip Select deasserted (active low) */
        gpio_set_value(spi_cs, 1);
}

static void lgit_spi_write_word(u16 val)
{
        int i;

        for (i = 0; i < 16; i++) {
                /* #1: Drive the Clk Low */
                gpio_set_value(spi_sclk, 0);

                /* #2: Drive the Data (High or Low) */
                if (val & 0x8000)
                        gpio_set_value(spi_sdi, 1);
                else
                        gpio_set_value(spi_sdi, 0);

                /* #3: Drive the Clk High */
                gpio_set_value(spi_sclk, 1);

                /* #4: Next bit */
                val <<= 1;
        }
}

static void lgit_spi_write_byte(u8 val)
{
        int i;

        /* Clock should be Low before entering */
        for (i = 0; i < 8; i++) {
                /* #1: Drive the Clk Low */
                gpio_set_value(spi_sclk, 0);

                /* #2: Drive the Data (High or Low) */
                if (val & 0x80)
                        gpio_set_value(spi_sdi, 1);
                else
                        gpio_set_value(spi_sdi, 0);

                /* #3: Drive the Clk High */
                gpio_set_value(spi_sclk, 1);

                /* #4: Next bit */
                val <<= 1;
        }
}

static void serigo(u16 reg, u16 data)
{
        /* Transmit register address first, then data */
        if(reg != 0xFFFF) {
                gpio_set_value(spi_cs, 0);
                udelay(1);

                /* Set index register */
                lgit_spi_write_byte(0x7C);
                lgit_spi_write_word(reg);

                gpio_set_value(spi_cs, 1);
                udelay(1);
        }
        gpio_set_value(spi_cs, 0);
        gpio_set_value(spi_sdi, 0);
        udelay(1);

        /* Transmit data */
        lgit_spi_write_byte(0x7E);
        lgit_spi_write_word(data);

        gpio_set_value(spi_sdi, 0);
        gpio_set_value(spi_cs, 1);
}

static void lgit_init(void)
{
        lgit_writew(0x0020, 0x0001);    //RGB Interface Select
        lgit_writew(0x003A, 0x0060);    //Interface Pixel Format                60 --> 18bit 70--> 24bit

        lgit_writew(0x00B1, 0x0016);    //RGB Interface Setting
        lgit_writew(0xFFFF, h_back_porch);      //H back Porch
        lgit_writew(0xFFFF, 0x0005);    //V back Porch

        lgit_writew(0x00B2, 0x0000);    //Panel Characteristics Setting
        lgit_writew(0xFFFF, 0x00C8);

        lgit_writew(0x00B3, 0x0011);    //Entry Mode Setting
        lgit_writew(0xFFFF, 0x00FF);

        lgit_writew(0x00B4, 0x0010);    //Display Mode Control  //0.2 : 0 --> 0.4 : 0x10

        lgit_writew(0x00B6, h_back_porch-1);    //Display Control 2, SDT
        lgit_writew(0xFFFF, 0x0018);
        lgit_writew(0xFFFF, 0x003A);
        lgit_writew(0xFFFF, 0x0040);
        lgit_writew(0xFFFF, 0x0010);

        lgit_writew(0x00B7, 0x0000);    //Display Control 3
        lgit_writew(0xFFFF, 0x0010);
        lgit_writew(0xFFFF, 0x0001);

        lgit_writew(0x00C3, 0x0001);    //Power Control 3       //0x0005
        lgit_writew(0xFFFF, 0x0005);    //Step up freq
        lgit_writew(0xFFFF, 0x0000);
        lgit_writew(0xFFFF, 0x0007);
        lgit_writew(0xFFFF, 0x0001);

        lgit_writew(0x00C4, 0x0033);    //Power Control 4
        lgit_writew(0xFFFF, 0x0003);
        lgit_writew(0xFFFF, 0x0000);
        lgit_writew(0xFFFF, 0x001B);
        lgit_writew(0xFFFF, 0x001B);
        lgit_writew(0xFFFF, 0x0000);
        lgit_writew(0xFFFF, 0x0000);
        lgit_writew(0xFFFF, 0x0004);

        lgit_writew(0x00C5, 0x006B);    //Power Control 5
        lgit_writew(0xFFFF, 0x0007);

	      lgit_writew(0x00C6, 0x0023);    //Power Control 6 //0x0023
        lgit_writew(0xFFFF, 0x0000);
        lgit_writew(0xFFFF, 0x0000);

        lgit_writew(0x00C8, 0x0001);    //Backlight Control
        lgit_writew(0xFFFF, 0x0001);
        lgit_writew(0xFFFF, 0x0003);
        lgit_writew(0xFFFF, 0x00FF);
        mdelay(70);

        lgit_writew(0x00D0, 0x0000);    //Set Positive Gamma R.G
        lgit_writew(0xFFFF, 0x0025);
        lgit_writew(0xFFFF, 0x0072);
        lgit_writew(0xFFFF, 0x0005);
        lgit_writew(0xFFFF, 0x0008);
        lgit_writew(0xFFFF, 0x0007);
        lgit_writew(0xFFFF, 0x0063);
        lgit_writew(0xFFFF, 0x0024);
        lgit_writew(0xFFFF, 0x0003);

        lgit_writew(0x00D1, 0x0040);    //Set Negative Gamma R.G
        lgit_writew(0xFFFF, 0x0015);
        lgit_writew(0xFFFF, 0x0074);
        lgit_writew(0xFFFF, 0x0027);
        lgit_writew(0xFFFF, 0x001C);
        lgit_writew(0xFFFF, 0x0000);
        lgit_writew(0xFFFF, 0x0055);
        lgit_writew(0xFFFF, 0x0043);
        lgit_writew(0xFFFF, 0x0004);

        lgit_writew(0x00D2, 0x0000);    //Set Positive Gamma Blue
        lgit_writew(0xFFFF, 0x0025);
        lgit_writew(0xFFFF, 0x0072);
        lgit_writew(0xFFFF, 0x0005);
        lgit_writew(0xFFFF, 0x0008);
        lgit_writew(0xFFFF, 0x0007);
        lgit_writew(0xFFFF, 0x0063);
        lgit_writew(0xFFFF, 0x0024);
        lgit_writew(0xFFFF, 0x0003);

        lgit_writew(0x00D3, 0x0040);    //Set Positive Gamma Blue
        lgit_writew(0xFFFF, 0x0015);
        lgit_writew(0xFFFF, 0x0074);
        lgit_writew(0xFFFF, 0x0027);
	  lgit_writew(0xFFFF, 0x001C);
        lgit_writew(0xFFFF, 0x0000);
        lgit_writew(0xFFFF, 0x0055);
        lgit_writew(0xFFFF, 0x0043);
        lgit_writew(0xFFFF, 0x0004);

        lgit_writew(0x00C2, 0x0008);    //Power Control 2
        mdelay(20);
        lgit_writew(0x00C2, 0x0018);    //Power Control 2
        mdelay(20);
        lgit_writew(0x00C2, 0x00B8);    //Power Control 2
        mdelay(20);
        lgit_writew(0x00B5, 0x0001);    //Display Control 1
        mdelay(20);

        lgit_writew(0x0029, 0x0001);    //Display ON
}

static void lgit_sleep(void)
{
        lgit_writew(0x0028, 0x0000);    //Display off
        mdelay(20);
        lgit_writew(0x00B5, 0x0000);    //Display control 1
        mdelay(20);
        lgit_writew(0x00C2, 0x0018);    //Power Control2
        mdelay(20);
        lgit_writew(0x00C2, 0x0000);    //Power Control2
        mdelay(20);

        lgit_writew(0x00C4, 0x0000);    //Power Control4
        lgit_writew(0xffff, 0x0000);
        lgit_writew(0xffff, 0x0000);
        lgit_writew(0xffff, 0x001B);
        lgit_writew(0xffff, 0x001B);
        lgit_writew(0xffff, 0x0000);
        lgit_writew(0xffff, 0x0000);
        lgit_writew(0xffff, 0x0004);
        mdelay(20);
}

static void lgit_disp_on(void)
{
        dprintk("lgit disp on\n");
        dprintk("lgit disp on initialize data go out\n");
                lgit_init();
        
}

static void lcdc_lgit_panel_reset(void)
{
                gpio_set_value(156, 0);
                mdelay(10);
                gpio_set_value(156, 1);
                mdelay(10);
}

static int lcdc_lgit_panel_on()
{
        	dprintk("lgit panel on\n");
	        lcdc_lgit_panel_reset();
                lcdc_lgit_config_gpios(1);
		lgit_spi_init();
                lgit_disp_on();
        	return 0;
}

static int lcdc_lgit_panel_off()
{
        	dprintk("lgit disp off\n");
                lgit_sleep();
                lcdc_lgit_config_gpios(0);
        	return 0;
}


typedef uint16_t UINT;

struct spi_cmd
{
	uint16_t reg;
	uint16_t val;
	uint16_t delay;
} __attribute__((packed));


#define PANELTYPE_UNKNOWN	0

int tsunagi_panel_type = -1;
int board_id = -1;
static struct vreg *vreg_lcd;

#define GPIO_CFG PCOM_GPIO_CFG
#define LCDC_GPIO_SPI_SCL 150
#define LCDC_GPIO_SPI_CS  151
#define LCDC_GPIO_SPI_SDI 148
#define LCDC_GPIO_SPI_SDO 149
#define LCD_GPIO_RESET_N 156 
#define LCD_GPIO_MAKER_ID 155
 
static void __init lcdc_lgit_gpio_init(void)
{
        if (gpio_request(LCDC_GPIO_SPI_SCL, "spi_clk"))
                pr_err("failed to request gpio spi_clk\n");
        if (gpio_request(LCDC_GPIO_SPI_CS, "spi_cs"))
                pr_err("failed to request gpio spi_cs\n");
        if (gpio_request(LCDC_GPIO_SPI_SDI, "spi_sdi"))
                pr_err("failed to request gpio spi_sdi\n");
        if (gpio_request(LCDC_GPIO_SPI_SDO, "spi_sdoi"))
                pr_err("failed to request gpio spi_sdoi\n");
        if (gpio_request(LCD_GPIO_RESET_N, "gpio_dac"))
                pr_err("failed to request gpio_dac\n");
        if(gpio_request(LCD_GPIO_MAKER_ID,"lcd_maker_id"))
        pr_err("failed to request lcd_maker_id\n");
}

static uint32_t lcdc_lgit_gpio_table[] = {
        GPIO_CFG(LCDC_GPIO_SPI_SCL, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
        GPIO_CFG(LCDC_GPIO_SPI_CS, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
        GPIO_CFG(LCDC_GPIO_SPI_SDI, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
        GPIO_CFG(LCDC_GPIO_SPI_SDO, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
        GPIO_CFG(LCD_GPIO_RESET_N,  0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
        GPIO_CFG(LCD_GPIO_MAKER_ID, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
};

static void config_lcdc_gpio_table(uint32_t *table, int len, unsigned enable)
{
        int n, rc;
        for (n = 0; n < len; n++) {
                rc = gpio_tlmm_config(table[n],
                        enable ? GPIO_ENABLE : GPIO_DISABLE);
                if (rc) {
                        printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
                                __func__, table[n], rc);
                        break;
                }
        }
}

static void lcdc_lgit_config_gpios(int enable)
{
        config_lcdc_gpio_table(lcdc_lgit_gpio_table,
                ARRAY_SIZE(lcdc_lgit_gpio_table), enable);
}


static char *msm_fb_lcdc_vreg[] = {
        "gp1",
        "wlan"
};

#define MSM_FB_LCDC_VREG_OP(name, op, level) \
do { \
        vreg = vreg_get(0, name); \
        vreg_set_level(vreg, level); \
        if (vreg_##op(vreg)) \
                printk(KERN_ERR "%s: %s vreg operation failed \n", \
                        (vreg_##op == vreg_enable) ? "vreg_enable" \
                                : "vreg_disable", name); \
} while (0)


#define LCM_DELAY(a)		msleep(a)

static DEFINE_MUTEX(panel_lock);

static int unk_panel_init(struct msm_lcdc_panel_ops *ops)
{
	return 0;
}
static int unk_panel_unblank(struct msm_lcdc_panel_ops *ops)
{
	struct vreg *vreg;
	MSM_FB_LCDC_VREG_OP(msm_fb_lcdc_vreg[0], enable, 1800);
        MSM_FB_LCDC_VREG_OP(msm_fb_lcdc_vreg[1], enable, 2800);
	lcdc_lgit_panel_on();
	return 0;
}

static int unk_panel_blank(struct msm_lcdc_panel_ops *ops)
{
       		 struct vreg *vreg;
	 	lcdc_lgit_panel_off();
		MSM_FB_LCDC_VREG_OP(msm_fb_lcdc_vreg[0], disable, 0);
                MSM_FB_LCDC_VREG_OP(msm_fb_lcdc_vreg[1], disable, 0);
	return 0;
}


//////////////////////////////////////////////////////////////////////////

static struct resource resources_msm_fb[] =
{
	{
		.start = MSM_FB_BASE,
		.end = MSM_FB_BASE + MSM_FB_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct msm_lcdc_panel_ops unk_lcdc_panel_ops =
{
//	.init		= unk_panel_unblank,
//	.uninit		= unk_panel_blank,
	.blank		= unk_panel_blank,
	.unblank	= unk_panel_unblank,
};

static struct msm_lcdc_timing tsunagi_lcdc_timing =
{
// This are in the moment only dummy values  
	.clk_rate		= 24576000,
	.hsync_pulse_width	= 2,
	.hsync_back_porch	= 23,
	.hsync_front_porch	= 2,
	.hsync_skew		= 0,
	.vsync_pulse_width	= 2,
	.vsync_back_porch	= 5,
	.vsync_front_porch	= 2,
	.vsync_act_low		= 1,
	.hsync_act_low		= 1,
	.den_act_low		= 0,
};

static struct msm_fb_data tsunagi_lcdc_fb_data =
{
	.xres		= 480,
	.yres		= 800,
	.width		= 48,
	.height		= 80,
	.output_format	= 1,
};

static struct msm_lcdc_platform_data tsunagi_lcdc_platform_data =
{
	.panel_ops	= &unk_lcdc_panel_ops,
	.timing		= &tsunagi_lcdc_timing,
	.fb_id		= 0,
	.fb_data	= &tsunagi_lcdc_fb_data,
	.fb_resource= &resources_msm_fb[0],
};


static struct platform_device tsunagi_lcdc_device =
{
	.name	= "msm_mdp_lcdc",
	.id	= -1,
	.dev	=
	{
		.platform_data = &tsunagi_lcdc_platform_data,
	},
};

static struct msm_mdp_platform_data mdp_pdata = {
        .color_format = MSM_MDP_OUT_IF_FMT_RGB666,
};


int __init tsunagi_init_panel(void)
{
	int ret=0;
	pr_info("Init Panel\n");
	 
         lcdc_lgit_gpio_init();
	 lcdc_spi_pin_assign();	
	


	msm_device_mdp.dev.platform_data = &mdp_pdata;

	ret = platform_device_register(&msm_device_mdp);
	if (ret != 0)
		return ret;


	ret = platform_device_register(&tsunagi_lcdc_device);
	if (ret != 0)
		return ret;

	return 0;
}
//device_initcall(tsunagi_init_panel);

