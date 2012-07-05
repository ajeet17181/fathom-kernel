#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-msm.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/bootmem.h>
#include <linux/platform_device.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_event.h>
#include <mach/board_lge.h>
#include <mach/pmic.h>
#include "gpio_chip.h"
#include <linux/leds.h>

//////////////////////////////////////////////////////////////////////
// KEYPAD
//////////////////////////////////////////////////////////////////////

#define PP2106_KEYPAD_ROW       8
#define PP2106_KEYPAD_COL       8
#define KEY_FOLDER_HOME         243
#define KEY_FOLDER_MENU         244

/* pp2106 qwerty keypad device */
static unsigned short pp2106_keycode[PP2106_KEYPAD_ROW][PP2106_KEYPAD_COL] = {
{KEY_UNKNOWN,      KEY_1,       KEY_2,         KEY_3,      KEY_4,        KEY_5,      KEY_6,          KEY_7},
{KEY_8,            KEY_9,       KEY_0,         KEY_UNKNOWN,KEY_Q,        KEY_W,      KEY_E,          KEY_R},
{KEY_T,            KEY_Y,       KEY_U,         KEY_I,      KEY_O,        KEY_P,      KEY_LEFTSHIFT,  KEY_A},
{KEY_S,            KEY_D,       KEY_F,         KEY_G,      KEY_H,        KEY_J,      KEY_K,          KEY_L},
{KEY_BACKSPACE,    KEY_LEFTALT, KEY_Z,         KEY_X,      KEY_C,        KEY_V,      KEY_SPACE,      KEY_BACK},
{KEY_UNKNOWN,      KEY_B,       KEY_N,         KEY_M,      KEY_UNKNOWN,  KEY_RIGHT,  KEY_LEFT,       KEY_BACK},
{KEY_DOWN,         KEY_UP,      KEY_ENTER,     KEY_K,      KEY_UNKNOWN,  KEY_HOME,   KEY_MENU,       KEY_MENU},
{KEY_ENTER,        KEY_VOLUMEUP,KEY_VOLUMEDOWN,KEY_END,    KEY_UNKNOWN,  KEY_BACK,   KEY_MENU,       KEY_END},};

static struct pp2106_platform_data pp2106_pdata = {
        .keypad_row = PP2106_KEYPAD_ROW,
        .keypad_col = PP2106_KEYPAD_COL,
        .keycode = (unsigned char *)pp2106_keycode,
        .reset_pin = 82,//GPIO_PP2106_RESET,
        .irq_pin = 42,//GPIO_PP2106_IRQ,
        .sda_pin = 41,//GPIO_PP2106_SDA,
        .scl_pin = 40,//GPIO_PP2106_SCL,
};

static struct platform_device qwerty_device = {
        .name = "kbd_pp2106",
        .id = -1,
        .dev = {
                .platform_data = &pp2106_pdata,
        },
};

int init_gpio_i2c_pin(struct i2c_gpio_platform_data *i2c_adap_pdata,
                struct gpio_i2c_pin gpio_i2c_pin,
                struct i2c_board_info *i2c_board_info_data)
{
        i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
        i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

        gpio_tlmm_config(PCOM_GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_OUTPUT,
                                GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
        gpio_tlmm_config(PCOM_GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_OUTPUT,
                                GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
        gpio_set_value(gpio_i2c_pin.sda_pin, 1);
        gpio_set_value(gpio_i2c_pin.scl_pin, 1);

        if (gpio_i2c_pin.reset_pin) {
                gpio_tlmm_config(PCOM_GPIO_CFG(gpio_i2c_pin.reset_pin, 0, GPIO_OUTPUT,
                                        GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
                gpio_set_value(gpio_i2c_pin.reset_pin, 1);
        }

        if (gpio_i2c_pin.irq_pin) {
                gpio_tlmm_config(PCOM_GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_INPUT,
                                        GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE);
                i2c_board_info_data->irq =
                        MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
        }

        return 0;
}



#define GPIO_HALLIC_IRQ 35

static struct gpio_event_direct_entry alohav_slide_switch_map[] = {
          { GPIO_HALLIC_IRQ,          SW_LID          },
};

static int alohav_gpio_slide_input_func(struct input_dev *input_dev,
                struct gpio_event_info *info, void **data, int func)
{
        if (func == GPIO_EVENT_FUNC_INIT)
                gpio_tlmm_config(PCOM_GPIO_CFG(GPIO_HALLIC_IRQ, 0, GPIO_INPUT, GPIO_PULL_UP,
                                        GPIO_2MA), GPIO_ENABLE);

        return gpio_event_input_func(input_dev, info, data, func);
}
static int alohav_gpio_slide_power(
                const struct gpio_event_platform_data *pdata, bool on)
{
        return 0;
}

static struct gpio_event_input_info alohav_slide_switch_info = {
        .info.func = alohav_gpio_slide_input_func,
        .info.no_suspend = true,
        .debounce_time.tv64 = 0,
        .flags = 0,
        .type = EV_SW,
        .keymap = alohav_slide_switch_map,
        .keymap_size = ARRAY_SIZE(alohav_slide_switch_map)
};

static struct gpio_event_info *alohav_gpio_slide_info[] = {
        &alohav_slide_switch_info.info,
};

static struct gpio_event_platform_data alohav_gpio_slide_data = {
        .name = "gpio-slide-detect",
        .info = alohav_gpio_slide_info,
        .info_count = ARRAY_SIZE(alohav_gpio_slide_info),
        .power = alohav_gpio_slide_power,
};


static struct platform_device alohav_gpio_slide_device = {
       .name = GPIO_EVENT_DEV_NAME,
        .id = 0,
        .dev        = {
                .platform_data  = &alohav_gpio_slide_data,
        },
};



#define I2C_BUS_NUM_AMP                 1
#define GPIO_AMP_I2C_SCL                109
#define GPIO_AMP_I2C_SDA                108

static struct i2c_gpio_platform_data amp_i2c_pdata = {
        .sda_pin = GPIO_AMP_I2C_SDA,
        .sda_is_open_drain = 0,
        .scl_pin = GPIO_AMP_I2C_SCL,
        .scl_is_open_drain = 0,
        .udelay = 2
};

static struct platform_device eve_amp_i2c_device = {
        .name = "i2c-gpio",
        .id = I2C_BUS_NUM_AMP,
        .dev.platform_data = &amp_i2c_pdata,
};

static struct i2c_board_info amp_i2c_bdinfo = {
        I2C_BOARD_INFO("amp_max9877", 0x7C),
};


void __init eve_init_i2c_amp(void)
{

        gpio_tlmm_config(PCOM_GPIO_CFG(108, 0, GPIO_OUTPUT,
                                GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
        gpio_tlmm_config(PCOM_GPIO_CFG(109, 0, GPIO_OUTPUT,
                                GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);

        gpio_configure(GPIO_AMP_I2C_SDA, GPIOF_DRIVE_OUTPUT);
        gpio_configure(GPIO_AMP_I2C_SCL, GPIOF_DRIVE_OUTPUT);

        i2c_register_board_info(I2C_BUS_NUM_AMP, &amp_i2c_bdinfo, 1);
        platform_device_register(&eve_amp_i2c_device);
}


#define EL_EN_GPIO              106

static void pmic_mpp_isink_set(struct led_classdev *led_cdev,
        enum led_brightness value)
{
        int mpp_number;
        int on_off;

        if (!strcmp(led_cdev->name ,"red"))
                mpp_number = (int)PM_MPP_13;
        else if (!strcmp(led_cdev->name, "green"))
                mpp_number = (int)PM_MPP_14;
        else if (!strcmp(led_cdev->name, "blue"))
                mpp_number = (int)PM_MPP_15;
        else
                return;

        if(value == 0)
                on_off = (int)PM_MPP__I_SINK__SWITCH_DIS;
        else
                on_off = (int)PM_MPP__I_SINK__SWITCH_ENA;

        pmic_secure_mpp_config_i_sink((enum mpp_which)mpp_number,
                PM_MPP__I_SINK__LEVEL_20mA, (enum mpp_i_sink_switch)on_off);
}


static void button_backlight_set(struct led_classdev *led_cdev,
        enum led_brightness value)
{
        if (value == 0)
                gpio_set_value(EL_EN_GPIO, 0);
         else
                gpio_set_value(EL_EN_GPIO, 1);
}


struct led_classdev alohag_custom_leds[] = {
        {
                .name = "red",
                .brightness_set = pmic_mpp_isink_set,
                .brightness = LED_OFF,
        },
        {
                .name = "green",

             .brightness_set = pmic_mpp_isink_set,
                .brightness = LED_OFF,
        },
        {
                .name = "blue",
                .brightness_set = pmic_mpp_isink_set,
                .brightness = LED_OFF,
        },
        {
                .name = "button-backlight",
                .brightness_set = button_backlight_set,
                .brightness = LED_OFF,
        },

};

static int register_leds(struct platform_device *pdev)
{
        int rc;
        int i;

    gpio_tlmm_config(PCOM_GPIO_CFG(EL_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP,
                                                GPIO_2MA), GPIO_ENABLE);
        for(i = 0 ; i < ARRAY_SIZE(alohag_custom_leds) ; i++) {
                rc = led_classdev_register(&pdev->dev, &alohag_custom_leds[i]);
                if (rc) {
                dev_err(&pdev->dev, "unable to register led class driver : aloha_custom_leds \n");
                return rc;
            }
            pmic_mpp_isink_set(&alohag_custom_leds[i], LED_OFF);
        }

        return rc;
}

static void unregister_leds (void)
{
        int i;
        for (i = 0; i< ARRAY_SIZE(alohag_custom_leds); ++i)
                led_classdev_unregister(&alohag_custom_leds[i]);
}

static void suspend_leds (void)
{
        int i;
        for (i = 0; i< ARRAY_SIZE(alohag_custom_leds); ++i)
                led_classdev_suspend(&alohag_custom_leds[i]);
}

static void resume_leds (void)
{
        int i;
        for (i = 0; i< ARRAY_SIZE(alohag_custom_leds); ++i)
                led_classdev_resume(&alohag_custom_leds[i]);
}

int keypad_led_set(unsigned char value)
{
        int ret;
        printk("Keypad led set %d\n",value);
        ret = pmic_set_led_intensity(LED_KEYPAD, value);
        printk("keypad_led_set returns %d\n",ret);
        return ret;
}

static struct msm_pmic_leds_pdata leds_pdata = {
        .custom_leds                    = alohag_custom_leds,
        .register_custom_leds   = register_leds,
        .unregister_custom_leds = unregister_leds,
        .suspend_custom_leds    = suspend_leds,
        .resume_custom_leds             = resume_leds,
        .msm_keypad_led_set             = keypad_led_set,
};

static struct platform_device msm_device_pmic_leds = {
        .name                           = "pmic-leds",
        .id                                     = -1,
        .dev.platform_data      = &leds_pdata,
};


#define I2C_BUS_NUM_BACKLIGHT 2

static struct i2c_gpio_platform_data bl_i2c_pdata = {
        .sda_pin = 100,
        .scl_pin = 101,
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .udelay                 = 2,
};


static struct platform_device bl_i2c_device = {
        .name   = "i2c-gpio",
        .id     = I2C_BUS_NUM_BACKLIGHT,
        .dev.platform_data = &bl_i2c_pdata,
};


static struct backlight_platform_data bd6084gu_data = {
        .gpio = 98,
        .init_on_boot = 0,
};

static struct i2c_board_info bl_i2c_bdinfo[] = {
        [0] = {
                I2C_BOARD_INFO("bd6084gu", 0xEC >> 1),
                .type = "bd6084gu",
                .platform_data = &bd6084gu_data,
        },
};




void /*__init*/ lge_init_i2c_backlight()
{

       i2c_register_board_info(I2C_BUS_NUM_BACKLIGHT, &bl_i2c_bdinfo[0], 1);
       platform_device_register(&bl_i2c_device);
}


#define I2C_BUS_NUM_FUEL        3
#define MAX17040_GPIO_I2C_SDA   93
#define MAX17040_GPIO_I2C_SCL   161


static struct i2c_gpio_platform_data fuel_i2c_pdata = {
        .sda_pin    = MAX17040_GPIO_I2C_SDA,
        .scl_pin    = MAX17040_GPIO_I2C_SCL,
        .sda_is_open_drain  = 0,
        .scl_is_open_drain  = 0,
        .udelay             = 2,
};


static struct platform_device fuel_i2c_device = {
        .name   = "i2c-gpio",
        .id     = I2C_BUS_NUM_FUEL,
        .dev.platform_data = &fuel_i2c_pdata,
};

static struct i2c_board_info fuel_i2c_bdinfo[] = {
        [0] = {
                I2C_BOARD_INFO("max17040", MAX17040_I2C_SLAVE_ADDR),
                .type = "max17040",
                .platform_data = NULL,
        },
};

static void __init lge_init_i2c_fuel_gauge(){
        gpio_tlmm_config(PCOM_GPIO_CFG(MAX17040_GPIO_I2C_SDA, 0, GPIO_OUTPUT, GPIO_PULL_UP,
                                GPIO_2MA), GPIO_ENABLE);
        gpio_tlmm_config(PCOM_GPIO_CFG(MAX17040_GPIO_I2C_SCL, 0, GPIO_OUTPUT, GPIO_PULL_UP,
                                GPIO_2MA), GPIO_ENABLE);

        gpio_set_value(MAX17040_GPIO_I2C_SDA,1);
        gpio_set_value(MAX17040_GPIO_I2C_SCL,1);
        i2c_register_board_info(I2C_BUS_NUM_FUEL, &fuel_i2c_bdinfo[0], 1);
        platform_device_register(&fuel_i2c_device);
}


#define ACCEL_GPIO_I2C_SDA 142
#define ACCEL_GPIO_I2C_SCL 143
#define ACCEL_GPIO_INT 92 /*MOTION_INT*/
static struct gpio_i2c_pin accel_i2c_pin[] = {
        [0] = {
                .sda_pin        = ACCEL_GPIO_I2C_SDA,
                .scl_pin        = ACCEL_GPIO_I2C_SCL,
                .reset_pin      = 0,
                .irq_pin        = ACCEL_GPIO_INT,
        },
};

static struct i2c_gpio_platform_data accel_i2c_pdata = {
        .sda_is_open_drain = 0,
        .scl_is_open_drain = 0,
        .udelay = 2,
};

static struct platform_device accel_i2c_device = {
        .name = "i2c-gpio",
        .dev.platform_data = &accel_i2c_pdata,
};

static int accel_power_set(unsigned char onoff)
{
        int ret = 0;
        return ret;
}

static struct acceleration_platform_data accel_pdata = {
        .power          = accel_power_set,
};
#define ACCEL_I2C_ADDRESS 0x38
static struct i2c_board_info accel_i2c_bdinfo[] = {
        [0] = {
                I2C_BOARD_INFO("bma150", ACCEL_I2C_ADDRESS),
                .type = "bma150",
                .platform_data = &accel_pdata,
        }
};

void __init lge_init_accel(void)
{
  accel_i2c_device.id = 4;
        init_gpio_i2c_pin(&accel_i2c_pdata, accel_i2c_pin[0], &accel_i2c_bdinfo[0]);

        i2c_register_board_info(4, &accel_i2c_bdinfo[0], 1);
        platform_device_register(&accel_i2c_device);

}
void lge_init(){

	platform_device_register(&qwerty_device);
	platform_device_register(&alohav_gpio_slide_device);
	platform_device_register(&msm_device_pmic_leds);
	
        eve_init_i2c_amp();
        lge_init_i2c_backlight();
        lge_init_i2c_fuel_gauge();
        lge_init_accel();
}
