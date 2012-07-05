/* pp2106 qwerty platform data */
struct pp2106_platform_data {
        unsigned int reset_pin;
        unsigned int irq_pin;
        unsigned int sda_pin;
        unsigned int scl_pin;
        unsigned int keypad_row;
        unsigned int keypad_col;
        unsigned char *keycode;
};

struct msm_i2ckbd_platform_data {
        uint8_t hwrepeat;
        uint8_t scanset1;
        int  gpioreset;
        int  gpioirq;
        int  (*gpio_setup) (void);
        void (*gpio_shutdown)(void);
        void (*hw_reset) (int);
};

struct proximity_platform_data {
        int irq_num;
        int (*power)(unsigned char onoff);
        int methods;
        int operation_mode;
        int debounce;
        u8 cycle;
};



#define MAX17040_I2C_SLAVE_ADDR (0x6C >> 1)


struct backlight_platform_data {
        void (*platform_init)(void);
        int gpio;
        unsigned int mode;                   /* initial mode */
        int max_current;                         /* led max current(0-7F) */
        int init_on_boot;                        /* flag which initialize on system boot */
};

struct gpio_i2c_pin {
        unsigned int sda_pin;
        unsigned int scl_pin;
        unsigned int reset_pin;
        unsigned int irq_pin;
};

struct acceleration_platform_data {
        int irq_num;
        int (*power)(unsigned char onoff);
};

struct msm_pmic_leds_pdata {
        struct led_classdev *custom_leds;
        int (*register_custom_leds)(struct platform_device *pdev);
        void (*unregister_custom_leds)(void);
        void (*suspend_custom_leds)(void);
        void (*resume_custom_leds)(void);
        int (*msm_keypad_led_set)(unsigned char value);
};

struct bu52031_platform_data {
        unsigned int irq_pin;
        unsigned int prohibit_time;
};

struct gpio_h2w_platform_data {
        int gpio_detect;
        int gpio_button_detect;
};

struct msm_panel_lgit_pdata {
        int gpio;
        int (*backlight_level)(int level, int max, int min);
        int (*pmic_backlight)(int level);
        int (*panel_num)(void);
        void (*panel_config_gpio)(int);
        int *gpio_num;
        int initialized;
};

/* Define new structure named 'msm_panel_hitachi_pdata' */
struct msm_panel_hitachi_pdata {
        int gpio;
        int (*backlight_level)(int level, int max, int min);
        int (*pmic_backlight)(int level);
        int (*panel_num)(void);
        void (*panel_config_gpio)(int);
        int *gpio_num;
        int initialized;
};

