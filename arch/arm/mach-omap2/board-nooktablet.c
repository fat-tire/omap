
/*
 * Board support file for OMAP4430 SDP.
 *
 * Copyright (C) 2009 Texas Instruments
 *
 * Author: Santosh Shilimkar <santosh.shilimkar@ti.com>
 *
 * Based on mach-omap2/board-3430sdp.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/moduleparam.h>
#include <linux/usb/otg.h>
#include <linux/spi/spi.h>
#include <linux/hwspinlock.h>
#include <linux/bootmem.h>
#include <linux/i2c/twl.h>
#include <linux/i2c/bq2415x.h>
#include <linux/gpio_keys.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/tps6130x.h>
#include <linux/leds.h>
#include <linux/leds_pwm.h>
#include <linux/omapfb.h>
#include <linux/reboot.h>
#include <linux/twl6040-vib.h>
#include <linux/wl12xx.h>
#include <linux/memblock.h>
#include <linux/mfd/twl6040-codec.h>

#include <linux/input/cyttsp.h>
#include <linux/input/ft5x06.h>
#include <linux/input/kxtf9.h>
#include <linux/power/max17042.h>
#include <linux/power/max8903.h>

#include <mach/board-nooktablet.h>
#include <mach/hardware.h>
#include <mach/omap4-common.h>
#include <mach/emif.h>
#include <mach/lpddr2-elpida.h>
#include <mach/dmm.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/usb.h>
#include <plat/mmc.h>
#include <plat/omap4-keypad.h>
#include <plat/omap_apps_brd_id.h>
#include <plat/omap-serial.h>
#include <plat/remoteproc.h>
#include <video/omapdss.h>
#include <video/omap-panel-nokia-dsi.h>
#include <plat/vram.h>
#include <plat/omap-pm.h>
#include <linux/wakelock.h>
//#include "board-blaze.h"
#include "omap4_ion.h"
#include "omap_ram_console.h"
#include "mux.h"
#include "hsmmc.h"
#include "timer-gp.h"
#include "control.h"
#include "common-board-devices.h"
#include "pm.h"
#include "prm-regbits-44xx.h"
#include "prm44xx.h"
/* for TI WiLink devices */
#include <linux/skbuff.h>
#include <linux/ti_wilink_st.h>
#include <plat/omap-serial.h>
#define WILINK_UART_DEV_NAME "/dev/ttyO1"

#define WILINK_UART_DEV_NAME "/dev/ttyO1"

#define KXTF9_DEVICE_ID                 "kxtf9"
#define KXTF9_I2C_SLAVE_ADDRESS         0x0F
#define KXTF9_GPIO_FOR_PWR              34

#define CONFIG_SERIAL_OMAP_IDLE_TIMEOUT 5

#define GPIO_WIFI_PMENA			54
#define GPIO_WIFI_IRQ			53

// #define CYTTSP_I2C_SLAVEADDRESS 	34
// #define OMAP_CYTTSP_GPIO        	37 /*99*/
// #define OMAP_CYTTSP_RESET_GPIO 		39 /*46*/

#define FT5x06_I2C_SLAVEADDRESS  	(0x70 >> 1)
#define OMAP_FT5x06_GPIO         	37 /*99*/
#define OMAP_FT5x06_RESET_GPIO   	39 /*46*/

#define TWL6030_RTC_GPIO 		6
#define BLUETOOTH_UART			UART2
#define CONSOLE_UART			UART1

// #define OMAP4_TOUCH_IRQ_1		35
// #define OMAP4_TOUCH_IRQ_2		36
// //#define HDMI_GPIO_CT_CP_HPD		60
// //#define HDMI_GPIO_HPD			63  /* Hot plug pin for HDMI */
// //#define HDMI_GPIO_LS_OE 41 /* Level shifter for HDMI */
// #define LCD_BL_GPIO		27	/* LCD Backlight GPIO */
// /* PWM2 and TOGGLE3 register offsets */
// #define LED_PWM2ON		0x03
// #define LED_PWM2OFF		0x04
// #define TWL6030_TOGGLE3		0x92

// #define TPS62361_GPIO   7

#define MAX17042_GPIO_FOR_IRQ  65
#define KXTF9_GPIO_FOR_IRQ  66

void __init acclaim_panel_init(void);

#ifdef CONFIG_BATTERY_MAX17042
static void max17042_dev_init(void)
{
	printk("board-4430sdp.c: max17042_dev_init ...\n");

	if (gpio_request(MAX17042_GPIO_FOR_IRQ, "max17042_irq") < 0) {
		printk(KERN_ERR "Can't get GPIO for max17042 IRQ\n");
		return;
	}

	printk("board-4430sdp.c: max17042_dev_init > Init max17042 irq pin %d !\n", MAX17042_GPIO_FOR_IRQ);
	gpio_direction_input(MAX17042_GPIO_FOR_IRQ);
	printk("max17042 GPIO pin read %d\n", gpio_get_value(MAX17042_GPIO_FOR_IRQ));
}
#endif

static void kxtf9_dev_init(void)
{
	printk("board-4430sdp.c: kxtf9_dev_init ...\n");

	if (gpio_request(KXTF9_GPIO_FOR_IRQ, "kxtf9_irq") < 0)
	{
		printk("Can't get GPIO for kxtf9 IRQ\n");
		return;
	}

	printk("board-4430sdp.c: kxtf9_dev_init > Init kxtf9 irq pin %d !\n",
			KXTF9_GPIO_FOR_IRQ);
	gpio_direction_input(KXTF9_GPIO_FOR_IRQ);
}


struct kxtf9_platform_data kxtf9_platform_data_here = {
	.min_interval   = 1,
	.poll_interval  = 1000,

	.g_range        = KXTF9_G_8G,
	.shift_adj      = SHIFT_ADJ_2G,

	/* Map the axes from the sensor to the device */
	/* SETTINGS FOR acclaim */
	.axis_map_x = 1,
	.axis_map_y = 0,
	.axis_map_z = 2,
	.axis_map_x     = 1,
	.axis_map_y     = 0,
	.axis_map_z     = 2,
	.negate_x       = 1,
	.negate_y       = 0,
	.negate_z       = 0,
	.data_odr_init          = ODR12_5F,
	.ctrl_reg1_init         = KXTF9_G_8G | RES_12BIT | TDTE | WUFE | TPE,
	.int_ctrl_init          = KXTF9_IEN | KXTF9_IEA | KXTF9_IEL,
	.int_ctrl_init          = KXTF9_IEN,
	.tilt_timer_init        = 0x03,
	.engine_odr_init        = OTP12_5 | OWUF50 | OTDT400,
	.wuf_timer_init         = 0x16,
	.wuf_thresh_init        = 0x28,
	.tdt_timer_init         = 0x78,
	.tdt_h_thresh_init      = 0xFF,
	.tdt_l_thresh_init      = 0x14,
	.tdt_tap_timer_init     = 0x53,
	.tdt_total_timer_init   = 0x24,
	.tdt_latency_timer_init = 0x10,
	.tdt_window_timer_init  = 0xA0,

	.gpio = KXTF9_GPIO_FOR_IRQ,
};

int ft5x06_dev_init(int resource)
{
	if (resource){
		omap_mux_init_signal("gpmc_ad13.gpio_37", OMAP_PIN_INPUT | OMAP_PIN_OFF_WAKEUPENABLE);
		omap_mux_init_signal("gpmc_ad15.gpio_39", OMAP_PIN_OUTPUT );

		if (gpio_request(OMAP_FT5x06_RESET_GPIO, "ft5x06_reset") < 0){
			printk(KERN_ERR "can't get ft5x06 xreset GPIO\n");
			return -1;
		}

		if (gpio_request(OMAP_FT5x06_GPIO, "ft5x06_touch") < 0) {
			printk(KERN_ERR "can't get ft5x06 interrupt GPIO\n");
			return -1;
		}

		gpio_direction_input(OMAP_FT5x06_GPIO);
	} else {
		gpio_free(OMAP_FT5x06_GPIO);
		gpio_free(OMAP_FT5x06_RESET_GPIO);
	}

	return 0;
}

static void ft5x06_platform_suspend(void)
{
	omap_mux_init_signal("gpmc_ad13.gpio_37", OMAP_PIN_INPUT );
}

static void ft5x06_platform_resume(void)
{
	omap_mux_init_signal("gpmc_ad13.gpio_37", OMAP_PIN_INPUT | OMAP_PIN_OFF_WAKEUPENABLE);
}

static struct ft5x06_platform_data ft5x06_platform_data = {
	.maxx = 600,
	.maxy = 1024,
	.flags = 0,
	.reset_gpio = OMAP_FT5x06_RESET_GPIO,
	.use_st = FT_USE_ST,
	.use_mt = FT_USE_MT,
	.use_trk_id = FT_USE_TRACKING_ID,
	.use_sleep = FT_USE_SLEEP,
	.use_gestures = 1,
	.platform_suspend = ft5x06_platform_suspend,
	.platform_resume = ft5x06_platform_resume,
};

// int cyttsp_dev_init(int resource)
// {
// 	if (resource) {
// 		omap_mux_init_signal("gpmc_ad13.gpio_37", OMAP_PIN_INPUT | OMAP_PIN_OFF_WAKEUPENABLE);
// 		omap_mux_init_signal("gpmc_ad15.gpio_39", OMAP_PIN_OUTPUT );
// 
// 
// 		if (gpio_request(OMAP_CYTTSP_RESET_GPIO, "tma340_reset") < 0) {
// 			printk(KERN_ERR "can't get tma340 xreset GPIO\n");
// 			return -1;
// 		}
// 
// 		if (gpio_request(OMAP_CYTTSP_GPIO, "cyttsp_touch") < 0) {
// 			printk(KERN_ERR "can't get cyttsp interrupt GPIO\n");
// 			return -1;
// 		}
// 
// 		gpio_direction_input(OMAP_CYTTSP_GPIO);
// 		/* omap_set_gpio_debounce(OMAP_CYTTSP_GPIO, 0); */
// 	} else {
// 		printk ("\n%s: Free resources",__FUNCTION__);
// 		gpio_free(OMAP_CYTTSP_GPIO);
// 		gpio_free(OMAP_CYTTSP_RESET_GPIO);
// 	}
// 	return 0;
// }

// static struct cyttsp_platform_data cyttsp_platform_data = {
// 	.maxx = 480,
// 	.maxy = 800,
// 	.flags = 0,
// 	.gen = CY_GEN3,
// 	.use_st = CY_USE_ST,
// 	.use_mt = CY_USE_MT,
// 	.use_hndshk = CY_SEND_HNDSHK,
// 	.use_trk_id = CY_USE_TRACKING_ID,
// 	.use_sleep = CY_USE_SLEEP,
// 	.use_gestures = CY_USE_GESTURES,
// 	/* activate up to 4 groups
// 	 * and set active distance
// 	 */
// 	.gest_set = CY_GEST_GRP1 | CY_GEST_GRP2 | CY_GEST_GRP3 | CY_GEST_GRP4 | CY_ACT_DIST,
// 	/* change act_intrvl to customize the Active power state.
// 	 * scanning/processing refresh interval for Operating mode
// 	 */
// 	.act_intrvl = CY_ACT_INTRVL_DFLT,
// 	/* change tch_tmout to customize the touch timeout for the
// 	 * Active power state for Operating mode
// 	 */
// 	.tch_tmout = CY_TCH_TMOUT_DFLT,
// 	/* change lp_intrvl to customize the Low Power power state.
// 	 * scanning/processing refresh interval for Operating mode
// 	 */
// 	.lp_intrvl = CY_LP_INTRVL_DFLT,
// };

#ifdef CONFIG_CHARGER_MAX8903

static struct resource max8903_gpio_resources_evt1a[] = {
	{	.name	= MAX8903_TOKEN_GPIO_CHG_EN,
		.start	= MAX8903_GPIO_CHG_EN,
		.end	= MAX8903_GPIO_CHG_EN,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_FLT,
		.start	= MAX8903_GPIO_CHG_FLT,
		.end	= MAX8903_GPIO_CHG_FLT,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_IUSB,
		.start	= MAX8903_GPIO_CHG_IUSB,
		.end	= MAX8903_GPIO_CHG_IUSB,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_USUS,
		.start	= MAX8903_GPIO_CHG_USUS_EVT1A,
		.end	= MAX8903_GPIO_CHG_USUS_EVT1A,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_ILM,
		.start	= MAX8903_GPIO_CHG_ILM_EVT1A,
		.end	= MAX8903_GPIO_CHG_ILM_EVT1A,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_UOK,
		.start	= MAX8903_UOK_GPIO_FOR_IRQ,
		.end	= MAX8903_UOK_GPIO_FOR_IRQ,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_DOK,
		.start	= MAX8903_DOK_GPIO_FOR_IRQ,
		.end	= MAX8903_DOK_GPIO_FOR_IRQ,
		.flags	= IORESOURCE_IO,
	}
};

static struct resource max8903_gpio_resources_evt1b[] = {
	{	.name	= MAX8903_TOKEN_GPIO_CHG_EN,
		.start	= MAX8903_GPIO_CHG_EN,
		.end	= MAX8903_GPIO_CHG_EN,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_FLT,
		.start	= MAX8903_GPIO_CHG_FLT,
		.end	= MAX8903_GPIO_CHG_FLT,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_IUSB,
		.start	= MAX8903_GPIO_CHG_IUSB,
		.end	= MAX8903_GPIO_CHG_IUSB,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_USUS,
		.start	= MAX8903_GPIO_CHG_USUS_EVT1B,
		.end	= MAX8903_GPIO_CHG_USUS_EVT1B,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_ILM,
		.start	= MAX8903_GPIO_CHG_ILM_EVT1B,
		.end	= MAX8903_GPIO_CHG_ILM_EVT1B,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_UOK,
		.start	= MAX8903_UOK_GPIO_FOR_IRQ,
		.end	= MAX8903_UOK_GPIO_FOR_IRQ,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_DOK,
		.start	= MAX8903_DOK_GPIO_FOR_IRQ,
		.end	= MAX8903_DOK_GPIO_FOR_IRQ,
		.flags	= IORESOURCE_IO,
	}
};

static struct resource max8903_gpio_resources_dvt[] = {
	{	.name	= MAX8903_TOKEN_GPIO_CHG_EN,
		.start	= MAX8903_GPIO_CHG_EN,
		.end	= MAX8903_GPIO_CHG_EN,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_FLT,
		.start	= MAX8903_GPIO_CHG_FLT,
		.end	= MAX8903_GPIO_CHG_FLT,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_IUSB,
		.start	= MAX8903_GPIO_CHG_IUSB,
		.end	= MAX8903_GPIO_CHG_IUSB,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_USUS,
		.start	= MAX8903_GPIO_CHG_USUS_DVT,
		.end	= MAX8903_GPIO_CHG_USUS_DVT,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_ILM,
		.start	= MAX8903_GPIO_CHG_ILM_DVT,
		.end	= MAX8903_GPIO_CHG_ILM_DVT,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_UOK,
		.start	= MAX8903_UOK_GPIO_FOR_IRQ,
		.end	= MAX8903_UOK_GPIO_FOR_IRQ,
		.flags	= IORESOURCE_IO,
	}, {
		.name	= MAX8903_TOKEN_GPIO_CHG_DOK,
		.start	= MAX8903_DOK_GPIO_FOR_IRQ,
		.end	= MAX8903_DOK_GPIO_FOR_IRQ,
		.flags	= IORESOURCE_IO,
	}
};

static struct platform_device max8903_charger_device = {
	.name           = "max8903_charger",
	.id             = -1,
};

static inline void acclaim_init_charger(void)
{
	const int board_type = acclaim_board_type();

	pr_info("Acclaim init charger.\n");
	if (board_type >= DVT) {
		max8903_charger_device.resource = max8903_gpio_resources_dvt;
		max8903_charger_device.num_resources = ARRAY_SIZE(max8903_gpio_resources_dvt);
	} else if (board_type >= EVT1B) {
		max8903_charger_device.resource = max8903_gpio_resources_evt1b;
		max8903_charger_device.num_resources = ARRAY_SIZE(max8903_gpio_resources_evt1b);
	} else if (board_type == EVT1A) {
		max8903_charger_device.resource = max8903_gpio_resources_evt1a;
		max8903_charger_device.num_resources = ARRAY_SIZE(max8903_gpio_resources_evt1a);
	} else {
		pr_err("%s: Acclaim board %d not supported\n", __func__, board_type);
		return;
	}
	platform_device_register(&max8903_charger_device);
}

#endif

#ifdef CONFIG_BATTERY_MAX17042
struct max17042_platform_data max17042_platform_data_here = {

	.gpio = MAX17042_GPIO_FOR_IRQ,

};
#endif

static const int sdp4430_keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(1, 0, KEY_VOLUMEDOWN),
};

static struct matrix_keymap_data sdp4430_keymap_data = {
	.keymap			= sdp4430_keymap,
	.keymap_size		= ARRAY_SIZE(sdp4430_keymap),
};

void keypad_pad_wkup(int enable)
{
 	int (*set_wkup_fcn)(const char *muxname);
 
 	/* PAD wakup for keyboard is needed for off mode
 	 * due to IO isolation.
 	 */
 	if (!off_mode_enabled)
 		return;
 
 	if (enable)
 		set_wkup_fcn = omap_mux_enable_wkup;
 	else
 		set_wkup_fcn = omap_mux_disable_wkup;
 
 	set_wkup_fcn("kpd_col0.kpd_col0");
// 	set_wkup_fcn("kpd_col1.kpd_col1");
// 	set_wkup_fcn("kpd_col2.kpd_col2");
// 	set_wkup_fcn("kpd_col0.kpd_col0");
// 	set_wkup_fcn("kpd_col1.kpd_col1");
// 	set_wkup_fcn("kpd_col2.kpd_col2");
// 	set_wkup_fcn("kpd_col3.kpd_col3");
// 	set_wkup_fcn("kpd_col4.kpd_col4");
// 	set_wkup_fcn("kpd_col5.kpd_col5");
// 	set_wkup_fcn("gpmc_a23.kpd_col7");
// 	set_wkup_fcn("gpmc_a22.kpd_col6");
 	set_wkup_fcn("kpd_row0.kpd_row0");
 	set_wkup_fcn("kpd_row1.kpd_row1");
// 	set_wkup_fcn("kpd_row2.kpd_row2");
// 	set_wkup_fcn("kpd_row3.kpd_row3");
// 	set_wkup_fcn("kpd_row4.kpd_row4");
// 	set_wkup_fcn("kpd_row5.kpd_row5");
// 	set_wkup_fcn("gpmc_a18.kpd_row6");
// 	set_wkup_fcn("gpmc_a19.kpd_row7");

}

static struct omap_device_pad keypad_pads[] = {
	{	.name   = "kpd_col0.kpd_col0",
		.enable = OMAP_WAKEUP_EN | OMAP_MUX_MODE0,
	},
	{	.name   = "kpd_row0.kpd_row0",
		.enable = OMAP_WAKEUP_EN | OMAP_MUX_MODE0,
	},
	{	.name   = "kpd_row1.kpd_row1",
		.enable = OMAP_PULL_ENA | OMAP_PULL_UP | OMAP_WAKEUP_EN | OMAP_MUX_MODE0 | OMAP_INPUT_EN,
	},
};
static struct omap_board_data keypad_data = {
	.id	    		= 1,
	.pads	 		= keypad_pads,
	.pads_cnt       	= ARRAY_SIZE(keypad_pads),
};

static struct omap4_keypad_platform_data sdp4430_keypad_data = {
	.keymap_data		= &sdp4430_keymap_data,
	.rows			= 2,
	.cols			= 1,
	.keypad_pad_wkup        = keypad_pad_wkup,
};

static struct gpio_keys_button acclaim_gpio_buttons[] = {
	{
		.code 		= KEY_POWER,
		.gpio 		= 29,
		.desc 		= "POWER",
		.active_low 	= 0,
		.wakeup 	= 1,
	},
	{

		.code 		= KEY_HOME,
		.gpio 		= 32,
		.desc 		= "HOME",
		.active_low 	= 1,
		.wakeup 	= 1,
	}
};

static struct gpio_keys_platform_data acclaim_gpio_key_info ={
	.buttons	= acclaim_gpio_buttons,
	.nbuttons	= ARRAY_SIZE ( acclaim_gpio_buttons ),
};

static struct platform_device acclaim_keys_gpio = {
	.name		= "gpio-keys",
	.id		= -1,
	.dev		= {
				.platform_data = &acclaim_gpio_key_info,
	},
};

// static struct gpio_led sdp4430_gpio_leds[] = {
// 	{
// 		.name	= "omap4:green:debug0",
// 		.gpio	= 61,
// 	},
// 	{
// 		.name	= "omap4:green:debug1",
// 		.gpio	= 30,
// 	},
// 	{
// 		.name	= "omap4:green:debug2",
// 		.gpio	= 7,
// 	},
// 	{
// 		.name	= "omap4:green:debug3",
// 		.gpio	= 8,
// 	},
// 	{
// 		.name	= "omap4:green:debug4",
// 		.gpio	= 50,
// 	},
// 	{
// 		.name	= "omap4:blue:user",
// 		.gpio	= 169,
// 	},
// 	{
// 		.name	= "omap4:red:user",
// 		.gpio	= 170,
// 	},
// 	{
// 		.name	= "omap4:green:user",
// 		.gpio	= 139,
// 	},
// 
// };

// static struct gpio_led_platform_data sdp4430_led_data = {
// 	.leds	= sdp4430_gpio_leds,
// 	.num_leds	= ARRAY_SIZE(sdp4430_gpio_leds),
// };

// static struct led_pwm sdp4430_pwm_leds[] = {
// 	{
// 		.name		= "omap4:green:chrg",
// 		.pwm_id		= 1,
// 		.max_brightness	= 255,
// 		.pwm_period_ns	= 7812500,
// 	},
// };

// static struct led_pwm_platform_data sdp4430_pwm_data = {
// 	.num_leds	= ARRAY_SIZE(sdp4430_pwm_leds),
// 	.leds		= sdp4430_pwm_leds,
// };

// static struct platform_device sdp4430_leds_pwm = {
// 	.name	= "leds_pwm",
// 	.id	= -1,
// 	.dev	= {
// 		.platform_data = &sdp4430_pwm_data,
// 	},
// };

// static struct platform_device sdp4430_leds_gpio = {
// 	.name	= "leds-gpio",
// 	.id	= -1,
// 	.dev	= {
// 		.platform_data = &sdp4430_led_data,
// 	},
// };
// static struct spi_board_info sdp4430_spi_board_info[] __initdata = {
// 	{
// 		.modalias               = "ks8851",
// 		.bus_num                = 1,
// 		.chip_select            = 0,
// 		.max_speed_hz           = 24000000,
// 		.irq                    = ETH_KS8851_IRQ,
// 	},
// };
static struct spi_board_info sdp4430_spi_board_info[] __initdata = {
	{
		.modalias		= "boxer_disp_spi",
		.bus_num		= 4,	/* 4: McSPI4 */
		.chip_select		= 0,
		.max_speed_hz		= 375000,
	},
};
// static struct gpio sdp4430_eth_gpios[] __initdata = {
// 	{ ETH_KS8851_POWER_ON,	GPIOF_OUT_INIT_HIGH,	"eth_power"	},
// 	{ ETH_KS8851_QUART,	GPIOF_OUT_INIT_HIGH,	"quart"		},
// 	{ ETH_KS8851_IRQ,	GPIOF_IN,		"eth_irq"	},
// };

// static int __init omap_ethernet_init(void)
// {
// 	int status;
// 
// 	/* Request of GPIO lines */
// 	status = gpio_request_array(sdp4430_eth_gpios,
// 				    ARRAY_SIZE(sdp4430_eth_gpios));
// 	if (status)
// 		pr_err("Cannot request ETH GPIOs\n");
// 
// 	return status;
// }

/* TODO: handle suspend/resume here.
 * Upon every suspend, make sure the wilink chip is capable enough to wake-up the
 * OMAP host.
 */
static int plat_wlink_kim_suspend(struct platform_device *pdev, pm_message_t
		state)
{
	return 0;
}

static int plat_wlink_kim_resume(struct platform_device *pdev)
{
	return 0;
}

static bool uart_req;
static struct wake_lock st_wk_lock;
/* Call the uart disable of serial driver */
static int plat_uart_disable(void)
{
	int port_id = 0;
	int err = 0;
	if (uart_req) {
		sscanf(WILINK_UART_DEV_NAME, "/dev/ttyO%d", &port_id);
		err = omap_serial_ext_uart_disable(port_id);
		if (!err)
			uart_req = false;
	}
	wake_unlock(&st_wk_lock);
	return err;
}

/* Call the uart enable of serial driver */
static int plat_uart_enable(void)
{
	int port_id = 0;
	int err = 0;
	if (!uart_req) {
		sscanf(WILINK_UART_DEV_NAME, "/dev/ttyO%d", &port_id);
		err = omap_serial_ext_uart_enable(port_id);
		if (!err)
			uart_req = true;
	}
	wake_lock(&st_wk_lock);
	return err;
}

/* wl128x BT, FM, GPS connectivity chip */
static struct ti_st_plat_data wilink_pdata = {
	.nshutdown_gpio = 55,
	.dev_name = WILINK_UART_DEV_NAME,
	.flow_cntrl = 1,
	.baud_rate = 3000000,//3686400,
	.suspend = plat_wlink_kim_suspend,
	.resume = plat_wlink_kim_resume,
	.chip_asleep = plat_uart_disable,
	.chip_awake  = plat_uart_enable,
	.chip_enable = plat_uart_enable,
	.chip_disable = plat_uart_disable,
};

static struct platform_device wl128x_device = {
	.name		= "kim",
	.id		= -1,
	.dev.platform_data = &wilink_pdata,
};

static struct platform_device btwilink_device = {
	.name = "btwilink",
	.id = -1,
};

// static struct twl4030_madc_platform_data twl6030_gpadc = {
// 	.irq_line = -1,
// };

static struct regulator_consumer_supply acclaim_lcd_tp_supply[] = {
	{ .supply = "vtp" },
	{ .supply = "vlcd" },
};

static struct regulator_init_data acclaim_lcd_tp_vinit = {
	.constraints = {
		.min_uV = 3300000,
		.max_uV = 3300000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = 2,
	.consumer_supplies = acclaim_lcd_tp_supply,
};

static struct fixed_voltage_config acclaim_lcd_touch_reg_data = {
	.supply_name = "vdd_lcdtp",
	.microvolts = 3300000,
	.gpio = 36,
	.enable_high = 1,
	.enabled_at_boot = 1,
	.init_data = &acclaim_lcd_tp_vinit,
};

static struct platform_device acclaim_lcd_touch_regulator_device = {
	.name   = "reg-fixed-voltage",
	.id     = -1,
	.dev    = {
		.platform_data = &acclaim_lcd_touch_reg_data,
	},
};

static struct platform_device *sdp4430_devices[] __initdata = {
	//&sdp4430_leds_gpio,
	//&sdp4430_leds_pwm,
	&acclaim_keys_gpio,
	&wl128x_device,
	&btwilink_device,
	&acclaim_lcd_touch_regulator_device,
};

static struct omap_board_config_kernel sdp4430_config[] __initdata = {
};

static void __init omap_4430sdp_init_early(void)
{
	omap2_init_common_infrastructure();
	omap2_init_common_devices(NULL, NULL);
#ifdef CONFIG_OMAP_32K_TIMER
	omap2_gp_clockevent_set_gptimer(1);
#endif
}

// static struct omap_musb_board_data musb_board_data = {
// 	.interface_type		= MUSB_INTERFACE_UTMI,
// #ifdef CONFIG_USB_MUSB_OTG
// 	.mode			= MUSB_OTG,
// #else
// 	.mode			= MUSB_PERIPHERAL,
// #endif
// 	.power			= 200,
// };
static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_UTMI,
#ifdef CONFIG_USB_MUSB_OTG
	.mode			= MUSB_OTG,
#elif defined(CONFIG_USB_MUSB_HDRC_HCD)
	.mode			= MUSB_HOST,
#elif defined(CONFIG_USB_GADGET_MUSB_HDRC)
	.mode			= MUSB_PERIPHERAL,
#endif
	.power			= 100,
};

static struct twl4030_usb_data omap4_usbphy_data = {
	.phy_init	= omap4430_phy_init,
	.phy_exit	= omap4430_phy_exit,
	.phy_power	= omap4430_phy_power,
	.phy_set_clock	= omap4430_phy_set_clk,
	.phy_suspend	= omap4430_phy_suspend,
};

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
			MMC_CAP_1_8V_DDR,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.ocr_mask	= MMC_VDD_165_195,
		.nonremovable   = true,
#ifdef CONFIG_PM_RUNTIME
		.power_saving	= true,
#endif
	},
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
			MMC_CAP_1_8V_DDR,
		.gpio_wp	= -EINVAL,
#ifdef CONFIG_PM_RUNTIME
		.power_saving	= true,
#endif
	},
#ifdef CONFIG_TIWLAN_SDIO
	{
		.mmc		= 3,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp        = 4,
		.ocr_mask	= MMC_VDD_165_195,
	},
#else
	{
		.mmc		= 5,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
		.ocr_mask	= MMC_VDD_165_195,
		.nonremovable	= true,
	},
#endif
  // 	{
// 		.mmc		= 2,
// 		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
// 					MMC_CAP_1_8V_DDR,
// 		.gpio_cd	= -EINVAL,
// 		.gpio_wp	= -EINVAL,
// 		.nonremovable   = true,
// 		.ocr_mask	= MMC_VDD_29_30,
// 		.no_off_init	= true,
// 	},
// 	{
// 		.mmc		= 1,
// 		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
// 					MMC_CAP_1_8V_DDR,
// 		.gpio_wp	= -EINVAL,
// 	},
// 	{
// 		.mmc		= 5,
// 		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD,
// 		.gpio_cd	= -EINVAL,
// 		.gpio_wp	= -EINVAL,
// 		.ocr_mask	= MMC_VDD_165_195,
// 		.nonremovable	= true,
// 	},
	{}	/* Terminator */
};


static struct regulator_consumer_supply sdp4430_vmmc_supply[] = {
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.0"),
};

static struct regulator_consumer_supply sdp4430_vemmc_supply[] = {
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.1"),
};

// static struct regulator_consumer_supply sdp4430_vaux_supply[] = {
// 	{
// 		.supply = "vmmc",
// 		.dev_name = "omap_hsmmc.1",
// 	},
// };
// static struct regulator_consumer_supply sdp4430_vmmc_supply[] = {
// 	{
// 		.supply = "vmmc",
// 		.dev_name = "omap_hsmmc.0",
// 	},
// };
// static struct regulator_consumer_supply sdp4430_vcxio_supply[] = {
// 	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dss"),
// 	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dsi1"),
// };
static struct regulator_consumer_supply omap4_sdp4430_vmmc5_supply = {
	.supply = "vmmc",
	.dev_name = "omap_hsmmc.4",
};
static struct regulator_init_data sdp4430_vmmc5 = {
	.constraints = {
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &omap4_sdp4430_vmmc5_supply,
};
static struct fixed_voltage_config sdp4430_vwlan = {
	.supply_name = "vwl1271",
	.microvolts = 1800000, /* 1.8V */
	.gpio = GPIO_WIFI_PMENA,
	.startup_delay = 70000, /* 70msec */
	.enable_high = 1,
	.enabled_at_boot = 0,
	.init_data = &sdp4430_vmmc5,
};
static struct platform_device omap_vwlan_device = {
	.name		= "reg-fixed-voltage",
	.id		= 1,
	.dev = {
		.platform_data = &sdp4430_vwlan,
               }
};
// static struct regulator_consumer_supply sdp4430_cam2_supply[] = {
// 	{
// 		.supply = "cam2pwr",
// 	},
// };

static int omap4_twl6030_hsmmc_late_init(struct device *dev)
{
	int ret = 0;
	struct platform_device *pdev = container_of(dev,
				struct platform_device, dev);
	struct omap_mmc_platform_data *pdata = dev->platform_data;

	pr_info("Setting MMC1 Card detect Irq\n");
	/* Setting MMC1 Card detect Irq */
	if (pdev->id == 0) {
		ret = twl6030_mmc_card_detect_config();
		if (ret)
			pr_err("Failed configuring MMC1 card detect\n");
		pdata->slots[0].card_detect_irq = TWL6030_IRQ_BASE +
						MMCDETECT_INTR_OFFSET;
		pdata->slots[0].card_detect = twl6030_mmc_card_detect;
	}
#ifndef CONFIG_TIWLAN_SDIO
	/* Set the MMC5 (wlan) power function */
	if (pdev->id == 4)
		pdata->slots[0].set_power = wifi_set_power;
#endif
	return ret;
}

static __init void omap4_twl6030_hsmmc_set_late_init(struct device *dev)
{
	struct omap_mmc_platform_data *pdata;

	/* dev can be null if CONFIG_MMC_OMAP_HS is not set */
	if (!dev) {
		pr_err("Failed %s\n", __func__);
		return;
	}
	pdata = dev->platform_data;
	pdata->init =	omap4_twl6030_hsmmc_late_init;
}

static int __init omap4_twl6030_hsmmc_init(struct omap2_hsmmc_info *controllers)
{
	struct omap2_hsmmc_info *c;

	omap2_hsmmc_init(controllers);
	for (c = controllers; c->mmc; c++)
		omap4_twl6030_hsmmc_set_late_init(c->dev);

	return 0;
}

// static struct regulator_init_data sdp4430_vaux1 = {
// 	.constraints = {
// 		.min_uV			= 1000000,
// 		.max_uV			= 3000000,
// 		.apply_uV		= true,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 					| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
// 					| REGULATOR_CHANGE_MODE
// 					| REGULATOR_CHANGE_STATUS,
// 	},
// 	.num_consumer_supplies  = 1,
// 	.consumer_supplies      = sdp4430_vaux_supply,
// };
// 
// static struct regulator_consumer_supply sdp4430_vaux2_supply[] = {
// 	REGULATOR_SUPPLY("av-switch", "soc-audio"),
// };
// 
// static struct regulator_init_data sdp4430_vaux2 = {
// 	.constraints = {
// 		.min_uV			= 1200000,
// 		.max_uV			= 2800000,
// 		.apply_uV		= true,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 					| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
// 					| REGULATOR_CHANGE_MODE
// 					| REGULATOR_CHANGE_STATUS,
// 	},
// 	.num_consumer_supplies	= 1,
// 	.consumer_supplies	= sdp4430_vaux2_supply,
// };
// 
// // static struct regulator_init_data sdp4430_vaux3 = {
// // 	.constraints = {
// // 		.min_uV			= 1000000,
// // 		.max_uV			= 3000000,
// // 		.apply_uV		= true,
// // 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// // 					| REGULATOR_MODE_STANDBY,
// // 		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
// // 					| REGULATOR_CHANGE_MODE
// // 					| REGULATOR_CHANGE_STATUS,
// // 	},
// // 	.num_consumer_supplies = 1,
// // 	.consumer_supplies = sdp4430_cam2_supply,
// // };
// 
// /* VMMC1 for MMC1 card */
// static struct regulator_init_data sdp4430_vmmc = {
// 	.constraints = {
// 		.min_uV			= 1200000,
// 		.max_uV			= 3000000,
// 		.apply_uV		= true,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 					| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
// 					| REGULATOR_CHANGE_MODE
// 					| REGULATOR_CHANGE_STATUS,
// 	},
// 	.num_consumer_supplies  = 1,
// 	.consumer_supplies      = sdp4430_vmmc_supply,
// };
// 
// static struct regulator_init_data sdp4430_vpp = {
// 	.constraints = {
// 		.min_uV			= 1800000,
// 		.max_uV			= 2500000,
// 		.apply_uV		= true,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 					| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
// 					| REGULATOR_CHANGE_MODE
// 					| REGULATOR_CHANGE_STATUS,
// 	},
// };
// 
// static struct regulator_init_data sdp4430_vusim = {
// 	.constraints = {
// 		.min_uV			= 1200000,
// 		.max_uV			= 2900000,
// 		.apply_uV		= true,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 					| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
// 					| REGULATOR_CHANGE_MODE
// 					| REGULATOR_CHANGE_STATUS,
// 	},
// };
// 
// static struct regulator_init_data sdp4430_vana = {
// 	.constraints = {
// 		.min_uV			= 2100000,
// 		.max_uV			= 2100000,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 					| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
// 					| REGULATOR_CHANGE_STATUS,
// 		.always_on	= true,
// 	},
// };
// 
// // static struct regulator_init_data sdp4430_vcxio = {
// // 	.constraints = {
// // 		.min_uV			= 1800000,
// // 		.max_uV			= 1800000,
// // 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// // 					| REGULATOR_MODE_STANDBY,
// // 		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
// // 					| REGULATOR_CHANGE_STATUS,
// // 		.always_on	= true,
// // 	},
// // 	.num_consumer_supplies	= ARRAY_SIZE(sdp4430_vcxio_supply),
// // 	.consumer_supplies	= sdp4430_vcxio_supply,
// // };
// 
// // static struct regulator_consumer_supply sdp4430_vdac_supply[] = {
// // 	{
// // 		.supply = "hdmi_vref",
// // 	},
// // };
// 
// // static struct regulator_init_data sdp4430_vdac = {
// // 	.constraints = {
// // 		.min_uV			= 1800000,
// // 		.max_uV			= 1800000,
// // 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// // 					| REGULATOR_MODE_STANDBY,
// // 		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
// // 					| REGULATOR_CHANGE_STATUS,
// // 		.always_on	= true,
// // 	},
// // 	.num_consumer_supplies  = ARRAY_SIZE(sdp4430_vdac_supply),
// // 	.consumer_supplies      = sdp4430_vdac_supply,
// // };
// 
// static struct regulator_init_data sdp4430_vusb = {
// 	.constraints = {
// 		.min_uV			= 3300000,
// 		.max_uV			= 3300000,
// 		.apply_uV		= true,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 					| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask	 =	REGULATOR_CHANGE_MODE
// 					| REGULATOR_CHANGE_STATUS,
// 	},
// };
// 
// static struct regulator_init_data sdp4430_clk32kg = {
// 	.constraints = {
// 		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
// 		.always_on		= true,
// 	},
// };

static struct regulator_init_data sdp4430_vaux1 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3000000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
			| REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
		.always_on	= true,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= sdp4430_vemmc_supply,
};

static struct regulator_init_data sdp4430_vaux2 = {
	.constraints = {
		.min_uV			= 1200000,
		.max_uV			= 2800000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
			| REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
	},
};

 static struct regulator_init_data sdp4430_vaux3 = {
 	.constraints = {
 		.min_uV			= 1800000,
 		.max_uV			= 1800000,
//  		.apply_uV		= true,
 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
 			| REGULATOR_MODE_STANDBY,
 		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
 			| REGULATOR_CHANGE_MODE
 			| REGULATOR_CHANGE_STATUS,
 		.state_mem = {
 			.enabled	= false,
 			.disabled	= true,
 		},
 		.always_on	= true,
 	},
//  	.num_consumer_supplies = 1,
//  	.consumer_supplies = sdp4430_vwlan_supply,
 };

static struct regulator_init_data sdp4430_vmmc = {
	.constraints = {
		.min_uV			= 1200000,
		.max_uV			= 3000000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
			| REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = sdp4430_vmmc_supply,
};

static struct regulator_init_data sdp4430_vpp = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 2500000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
			| REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
	},
};

static struct regulator_init_data sdp4430_vusim = {
	.constraints = {
		.min_uV			= 1200000,
		.max_uV			= 2900000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
			| REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
	},
};

static struct regulator_init_data sdp4430_vana = {
	.constraints = {
		.min_uV			= 2100000,
		.max_uV			= 2100000,
		//.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
	},
};

static struct regulator_init_data sdp4430_vcxio = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		//.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
		.always_on	= true,
	},
};

static struct regulator_init_data sdp4430_vdac = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
	},
};


static struct regulator_init_data sdp4430_vusb = {
	.constraints = {
		.min_uV			= 3300000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
			| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 =	REGULATOR_CHANGE_MODE
			| REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= false,
			.disabled	= true,
		},
	},
};

static struct regulator_init_data sdp4430_clk32kg = {
	.constraints = {
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.valid_ops_mask	 = REGULATOR_CHANGE_STATUS,
		.always_on	= true,
	},
};

static struct twl4030_madc_platform_data sdp4430_gpadc_data = {
	.irq_line	= 1,
};

static int sdp4430_batt_table[] = {
	/* adc code for temperature in degree C */
	929, 925, /* -2 ,-1 */
	920, 917, 912, 908, 904, 899, 895, 890, 885, 880, /* 00 - 09 */
	875, 869, 864, 858, 853, 847, 841, 835, 829, 823, /* 10 - 19 */
	816, 810, 804, 797, 790, 783, 776, 769, 762, 755, /* 20 - 29 */
	748, 740, 732, 725, 718, 710, 703, 695, 687, 679, /* 30 - 39 */
	671, 663, 655, 647, 639, 631, 623, 615, 607, 599, /* 40 - 49 */
	591, 583, 575, 567, 559, 551, 543, 535, 527, 519, /* 50 - 59 */
	511, 504, 496 /* 60 - 62 */
};

static struct twl4030_bci_platform_data sdp4430_bci_data = {
	.monitoring_interval		= 10,
	.max_charger_currentmA		= 1500,
	.max_charger_voltagemV		= 4560,
	.max_bat_voltagemV		= 4200,
	.low_bat_voltagemV		= 3300,
	.battery_tmp_tbl		= sdp4430_batt_table,
	.tblsize			= ARRAY_SIZE(sdp4430_batt_table),
};
// static void omap4_audio_conf(void)
// {
// 	/* twl6040 naudint */
// 	omap_mux_init_signal("sys_nirq2.sys_nirq2", OMAP_PIN_INPUT_PULLUP);
// }

// static int tps6130x_enable(int on)
// {
// 	u8 val = 0;
// 	int ret;
// 
// 	ret = twl_i2c_read_u8(TWL_MODULE_AUDIO_VOICE, &val, TWL6040_REG_GPOCTL);
// 	if (ret < 0) {
// 		pr_err("%s: failed to read GPOCTL %d\n", __func__, ret);
// 		return ret;
// 	}
// 
// 	/* TWL6040 GPO2 connected to TPS6130X NRESET */
// 	if (on)
// 		val |= TWL6040_GPO2;
// 	else
// 		val &= ~TWL6040_GPO2;
// 
// 	ret = twl_i2c_write_u8(TWL_MODULE_AUDIO_VOICE, val, TWL6040_REG_GPOCTL);
// 	if (ret < 0)
// 		pr_err("%s: failed to write GPOCTL %d\n", __func__, ret);
// 
// 	return ret;
// }

// static struct tps6130x_platform_data tps6130x_pdata = {
// 	.chip_enable	= tps6130x_enable,
// };
// 
// static struct regulator_consumer_supply twl6040_vddhf_supply[] = {
// 	REGULATOR_SUPPLY("vddhf", "twl6040-codec"),
// };
// 
// static struct regulator_init_data twl6040_vddhf = {
// 	.constraints = {
// 		.min_uV			= 4075000,
// 		.max_uV			= 4950000,
// 		.apply_uV		= true,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 					| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
// 					| REGULATOR_CHANGE_MODE
// 					| REGULATOR_CHANGE_STATUS,
// 	},
// 	.num_consumer_supplies	= ARRAY_SIZE(twl6040_vddhf_supply),
// 	.consumer_supplies	= twl6040_vddhf_supply,
// 	.driver_data		= &tps6130x_pdata,
// };

// static int twl6040_init(void)
// {
// 	u8 rev = 0;
// 	int ret;
// 
// 	ret = twl_i2c_read_u8(TWL_MODULE_AUDIO_VOICE,
// 				&rev, TWL6040_REG_ASICREV);
// 	if (ret)
// 		return ret;
// 
// 	/*
// 	 * ERRATA: Reset value of PDM_UL buffer logic is 1 (VDDVIO)
// 	 * when AUDPWRON = 0, which causes current drain on this pin's
// 	 * pull-down on OMAP side. The workaround consists of disabling
// 	 * pull-down resistor of ABE_PDM_UL_DATA pin
// 	 * Impacted revisions: ES1.1 and ES1.2 (both share same ASICREV value)
// 	 */
// 	if (rev == TWL6040_REV_1_1)
// 		omap_mux_init_signal("abe_pdm_ul_data.abe_pdm_ul_data",
// 			OMAP_PIN_INPUT);
// 
// 	return 0;
// }

// static struct twl4030_codec_audio_data twl6040_audio = {
// 	/* single-step ramp for headset and handsfree */
// 	.hs_left_step	= 0x0f,
// 	.hs_right_step	= 0x0f,
// 	.hf_left_step	= 0x1d,
// 	.hf_right_step	= 0x1d,
// 	.vddhf_uV	= 4075000,
// };
// 
// static struct twl4030_codec_vibra_data twl6040_vibra = {
// 	.max_timeout	= 15000,
// 	.initial_vibrate = 0,
// 	.voltage_raise_speed = 0x26,
// };
// 
// static struct twl4030_codec_data twl6040_codec = {
// 	.audio		= &twl6040_audio,
// 	.vibra		= &twl6040_vibra,
// 	.audpwron_gpio	= 127,
// 	.naudint_irq	= OMAP44XX_IRQ_SYS_2N,
// 	.irq_base	= TWL6040_CODEC_IRQ_BASE,
// 	.init		= twl6040_init,
// };


static struct twl4030_platform_data sdp4430_twldata = {
	.irq_base	= TWL6030_IRQ_BASE,
	.irq_end	= TWL6030_IRQ_END,

	/* Regulators */
	.vmmc		= &sdp4430_vmmc,
	.vpp		= &sdp4430_vpp,
//	.vusim		= &sdp4430_vusim,
	.vana		= &sdp4430_vana,
	.vcxio		= &sdp4430_vcxio,
//	.vdac		= &sdp4430_vdac,
	.vusb		= &sdp4430_vusb,
	.vaux1		= &sdp4430_vaux1,
//	.vaux2		= &sdp4430_vaux2,
	.vaux3		= &sdp4430_vaux3,
	.clk32kg	= &sdp4430_clk32kg,
	.usb		= &omap4_usbphy_data,
	.bci		= &sdp4430_bci_data,
	/* children */
//	.codec		= &twl6040_codec,
	.madc           = &sdp4430_gpadc_data,

};



static struct i2c_board_info __initdata sdp4430_i2c_1_boardinfo[] = {
// 	{
// /*		I2C_BOARD_INFO("twl6030", 0x48),
// 		.flags = I2C_CLIENT_WAKE,
// 		.irq = OMAP44XX_IRQ_SYS_1N,
// 		.platform_data = &sdp4430_twldata,*/
// 	},
	{
		I2C_BOARD_INFO(KXTF9_DEVICE_ID, KXTF9_I2C_SLAVE_ADDRESS),
		.platform_data = &kxtf9_platform_data_here,
		.irq = OMAP_GPIO_IRQ(66),
	},
	{
		I2C_BOARD_INFO(MAX17042_DEVICE_ID, MAX17042_I2C_SLAVE_ADDRESS),
		.platform_data = &max17042_platform_data_here,
		.irq = OMAP_GPIO_IRQ(65),
	},
};
// static struct bq2415x_platform_data sdp4430_bqdata = {
// 	.max_charger_voltagemV = 4200,
// 	.max_charger_currentmA = 1550,
// };

static struct i2c_board_info __initdata sdp4430_i2c_2_boardinfo[] = {
// 	{
// 		I2C_BOARD_INFO(CY_I2C_NAME, CYTTSP_I2C_SLAVEADDRESS),
// 		.platform_data = &cyttsp_platform_data,
// 		.irq = OMAP_GPIO_IRQ(OMAP_CYTTSP_GPIO),
// 	},
	{
		I2C_BOARD_INFO(FT_I2C_NAME, FT5x06_I2C_SLAVEADDRESS),
		.platform_data = &ft5x06_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_FT5x06_GPIO),
	},
// 	{
// 		I2C_BOARD_INFO("tlv320aic3100", 0x18),
// 	},
};

static struct i2c_board_info __initdata sdp4430_i2c_3_boardinfo[] = {

};

static struct i2c_board_info __initdata sdp4430_i2c_4_boardinfo[] = {

};
static void __init show_acclaim_board_revision(int revision)
{
	switch (revision) {
		case EVT1A:
			printk(KERN_INFO "Board revision %s\n", "EVT1A");
			break;
		case EVT1B:
			printk(KERN_INFO "Board revision %s\n", "EVT1B");
			break;
		case EVT2:
			printk(KERN_INFO "Board revision %s\n", "EVT2");
			break;
		case DVT:
			printk(KERN_INFO "Board revision %s\n", "DVT");
			break;
		case PVT:
			printk(KERN_INFO "Board revision %s\n", "PVT");
			break;
		default:
			printk(KERN_ERR "Board revision UNKNOWN (0x%x)\n", revision);
			break;
	}
}
void __init acclaim_board_init(void)
{
	const int board_type = acclaim_board_type();
	show_acclaim_board_revision(board_type);

// 	if ( board_type == EVT1A ){
// 		max17042_gpio_for_irq = 98;
// 		kxtf9_gpio_for_irq = 99;
// 	} else if ( board_type >= EVT1B ) {
// 		max17042_gpio_for_irq = 65;
// 		kxtf9_gpio_for_irq = 66;
// 	}
// 
// 	max17042_platform_data_here.gpio = max17042_gpio_for_irq;
// 	sdp4430_i2c_1_boardinfo[1].irq = OMAP_GPIO_IRQ(max17042_gpio_for_irq);
// 	kxtf9_platform_data_here.gpio = kxtf9_gpio_for_irq;
// 	sdp4430_i2c_1_boardinfo[0].irq = OMAP_GPIO_IRQ(kxtf9_gpio_for_irq);
	omap_mux_init_signal("sys_pwron_reset_out", OMAP_MUX_MODE3);
	omap_mux_init_signal("fref_clk3_req", OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN);
}

static void __init blaze_pmic_mux_init(void)
{

	omap_mux_init_signal("sys_nirq1", OMAP_PIN_INPUT_PULLUP |
						OMAP_WAKEUP_EN);
}

static void __init omap_i2c_hwspinlock_init(int bus_id, int spinlock_id,
				struct omap_i2c_bus_board_data *pdata)
{
	/* spinlock_id should be -1 for a generic lock request */
	if (spinlock_id < 0)
		pdata->handle = hwspin_lock_request();
	else
		pdata->handle = hwspin_lock_request_specific(spinlock_id);

	if (pdata->handle != NULL) {
		pdata->hwspin_lock_timeout = hwspin_lock_timeout;
		pdata->hwspin_unlock = hwspin_unlock;
	} else {
		pr_err("I2C hwspinlock request failed for bus %d\n", \
								bus_id);
	}
}

static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_1_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_2_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_3_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_4_bus_pdata;

static int __init omap4_i2c_init(void)
{
	int err;

	omap_i2c_hwspinlock_init(1, 0, &sdp4430_i2c_1_bus_pdata);
	omap_i2c_hwspinlock_init(2, 1, &sdp4430_i2c_2_bus_pdata);
	omap_i2c_hwspinlock_init(3, 2, &sdp4430_i2c_3_bus_pdata);
	omap_i2c_hwspinlock_init(4, 3, &sdp4430_i2c_4_bus_pdata);

	omap_register_i2c_bus_board_data(1, &sdp4430_i2c_1_bus_pdata);
	omap_register_i2c_bus_board_data(2, &sdp4430_i2c_2_bus_pdata);
	omap_register_i2c_bus_board_data(3, &sdp4430_i2c_3_bus_pdata);
	omap_register_i2c_bus_board_data(4, &sdp4430_i2c_4_bus_pdata);

	omap4_pmic_init("twl6030", &sdp4430_twldata);
	
	err=i2c_register_board_info(1,sdp4430_i2c_1_boardinfo, ARRAY_SIZE(sdp4430_i2c_1_boardinfo));
	if (err)
	  return err;
// 	i2c_register_board_info(1, sdp4430_i2c_boardinfo,
// 				ARRAY_SIZE(sdp4430_i2c_boardinfo));
	omap_register_i2c_bus(2, 400, sdp4430_i2c_2_boardinfo,
				ARRAY_SIZE(sdp4430_i2c_2_boardinfo));
	omap_register_i2c_bus(3, 400, NULL, 0);// sdp4430_i2c_3_boardinfo,
				//ARRAY_SIZE(sdp4430_i2c_3_boardinfo));
	omap_register_i2c_bus(4, 400, NULL, 0); // sdp4430_i2c_4_boardinfo,
				//ARRAY_SIZE(sdp4430_i2c_4_boardinfo));

	/*
	 * This will allow unused regulator to be shutdown. This flag
	 * should be set in the board file. Before regulators are registered.
	 */
	regulator_has_full_constraints();

	gpio_request(TWL6030_RTC_GPIO, "h_SYS_DRM_MSEC");
	gpio_direction_output(TWL6030_RTC_GPIO, 1);

	omap_mux_init_signal("fref_clk0_out.gpio_wk6", OMAP_PIN_OUTPUT| OMAP_PIN_OFF_NONE);
// 	gpio_request(6, "msecure");
// 	gpio_direction_output(6, 1);

	return 0;
}

static bool enable_suspend_off = true;
module_param(enable_suspend_off, bool, S_IRUSR | S_IRGRP | S_IROTH);

// static int dsi1_panel_set_backlight(struct omap_dss_device *dssdev, int level)
// {
// 	int r;
// 
// 	r = twl_i2c_write_u8(TWL_MODULE_PWM, 0x7F, LED_PWM2OFF);
// 	if (r)
// 		return r;
// 
// 	if (level > 1) {
// 		if (level == 255)
// 			level = 0x7F;
// 		else
// 			level = (~(level/2)) & 0x7F;
// 
// 		r = twl_i2c_write_u8(TWL_MODULE_PWM, level, LED_PWM2ON);
// 		if (r)
// 			return r;
// 		r = twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x30, TWL6030_TOGGLE3);
// 		if (r)
// 			return r;
// 	} else if (level <= 1) {
// 		r = twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x08, TWL6030_TOGGLE3);
// 		if (r)
// 			return r;
// 		r = twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x28, TWL6030_TOGGLE3);
// 		if (r)
// 			return r;
// 		r = twl_i2c_write_u8(TWL6030_MODULE_ID1, 0x00, TWL6030_TOGGLE3);
// 		if (r)
// 			return r;
// 	}

// 	return 0;
// }


//static struct nokia_dsi_panel_data dsi1_panel;

// static void sdp4430_lcd_init(void)
// {
// 	u32 reg;
// 	int status;
// 
// 	/* Enable 3 lanes in DSI1 module, disable pull down */
// 	reg = omap4_ctrl_pad_readl(OMAP4_CTRL_MODULE_PAD_CORE_CONTROL_DSIPHY);
// 	reg &= ~OMAP4_DSI1_LANEENABLE_MASK;
// 	reg |= 0x7 << OMAP4_DSI1_LANEENABLE_SHIFT;
// 	reg &= ~OMAP4_DSI1_PIPD_MASK;
// 	reg |= 0x7 << OMAP4_DSI1_PIPD_SHIFT;
// 	omap4_ctrl_pad_writel(reg, OMAP4_CTRL_MODULE_PAD_CORE_CONTROL_DSIPHY);
// 
// 	/* Panel Taal reset and backlight GPIO init */
// 	status = gpio_request_one(dsi1_panel.reset_gpio, GPIOF_DIR_OUT,
// 		"lcd_reset_gpio");
// 	if (status)
// 		pr_err("%s: Could not get lcd_reset_gpio\n", __func__);
// 
// 	if (dsi1_panel.use_ext_te) {
// 		status = omap_mux_init_signal("gpmc_ncs4.gpio_101",
// 				OMAP_PIN_INPUT_PULLUP);
// 		if (status)
// 			pr_err("%s: Could not get ext_te gpio\n", __func__);
// 	}
// 
// 	status = gpio_request_one(LCD_BL_GPIO, GPIOF_DIR_OUT, "lcd_bl_gpio");
// 	if (status)
// 		pr_err("%s: Could not get lcd_bl_gpio\n", __func__);
// 
// 	gpio_set_value(LCD_BL_GPIO, 0);
// }

// static struct gpio sdp4430_hdmi_gpios[] = {
// 	{HDMI_GPIO_CT_CP_HPD,  GPIOF_OUT_INIT_HIGH,    "hdmi_gpio_hpd"   },
// 	{HDMI_GPIO_LS_OE,      GPIOF_OUT_INIT_HIGH,    "hdmi_gpio_ls_oe" },
// };


// static void sdp4430_hdmi_mux_init(void)
// {
// 	u32 r;
// 	int status;
// 	/* PAD0_HDMI_HPD_PAD1_HDMI_CEC */
// 	omap_mux_init_signal("hdmi_hpd.hdmi_hpd",
// 				OMAP_PIN_INPUT_PULLDOWN);
// 	omap_mux_init_signal("gpmc_wait2.gpio_100",
// 			OMAP_PIN_INPUT_PULLDOWN);
// 	omap_mux_init_signal("hdmi_cec.hdmi_cec",
// 			OMAP_PIN_INPUT_PULLUP);
// 	/* PAD0_HDMI_DDC_SCL_PAD1_HDMI_DDC_SDA */
// 	omap_mux_init_signal("hdmi_ddc_scl.hdmi_ddc_scl",
// 			OMAP_PIN_INPUT_PULLUP);
// 	omap_mux_init_signal("hdmi_ddc_sda.hdmi_ddc_sda",
// 			OMAP_PIN_INPUT_PULLUP);
// 
// 	/* strong pullup on DDC lines using unpublished register */
// 	r = ((1 << 24) | (1 << 28)) ;
// 	omap4_ctrl_pad_writel(r, OMAP4_CTRL_MODULE_PAD_CORE_CONTROL_I2C_1);
// 
// 	gpio_request(HDMI_GPIO_HPD, NULL);
// 	omap_mux_init_gpio(HDMI_GPIO_HPD, OMAP_PIN_INPUT | OMAP_PULL_ENA);
// 	gpio_direction_input(HDMI_GPIO_HPD);
// 
// 	status = gpio_request_array(sdp4430_hdmi_gpios,
// 			ARRAY_SIZE(sdp4430_hdmi_gpios));
// 	if (status)
// 		pr_err("%s:Cannot request HDMI GPIOs %x \n", __func__, status);
// }



// static struct nokia_dsi_panel_data dsi1_panel = {
// 		.name		= "taal",
// 		.reset_gpio	= 102,
// 		.use_ext_te	= false,
// 		.ext_te_gpio	= 101,
// 		.esd_interval	= 0,
// 		.set_backlight	= dsi1_panel_set_backlight,
// };
// 
// static struct omap_dss_device sdp4430_lcd_device = {
// 	.name			= "lcd",
// 	.driver_name		= "taal",
// 	.type			= OMAP_DISPLAY_TYPE_DSI,
// 	.data			= &dsi1_panel,
// 	.phy.dsi		= {
// 		.clk_lane	= 1,
// 		.clk_pol	= 0,
// 		.data1_lane	= 2,
// 		.data1_pol	= 0,
// 		.data2_lane	= 3,
// 		.data2_pol	= 0,
// 	},
// 
// 	.clocks = {
// 		.dispc = {
// 			.channel = {
// 				.lck_div	= 1,	/* Logic Clock = 172.8 MHz */
// 				.pck_div	= 5,	/* Pixel Clock = 34.56 MHz */
// 				.lcd_clk_src	= OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DISPC,
// 			},
// 			.dispc_fclk_src	= OMAP_DSS_CLK_SRC_FCK,
// 		},
// 
// 		.dsi = {
// 			.regn		= 16,	/* Fint = 2.4 MHz */
// 			.regm		= 180,	/* DDR Clock = 216 MHz */
// 			.regm_dispc	= 5,	/* PLL1_CLK1 = 172.8 MHz */
// 			.regm_dsi	= 5,	/* PLL1_CLK2 = 172.8 MHz */
// 
// 			.lp_clk_div	= 10,	/* LP Clock = 8.64 MHz */
// 			.dsi_fclk_src	= OMAP_DSS_CLK_SRC_DSI_PLL_HSDIV_DSI,
// 		},
// 	},
// 	.channel = OMAP_DSS_CHANNEL_LCD,
// 	.skip_init = false,
// };

// static struct omap_dss_device sdp4430_hdmi_device = {
// 	.name = "hdmi",
// 	.driver_name = "hdmi_panel",
// 	.type = OMAP_DISPLAY_TYPE_HDMI,
// 	.clocks	= {
// 		.dispc	= {
// 			.dispc_fclk_src	= OMAP_DSS_CLK_SRC_FCK,
// 		},
// 		.hdmi	= {
// 			.regn	= 15,
// 			.regm2	= 1,
// 		},
// 	},
// 	.hpd_gpio = HDMI_GPIO_HPD,
// 	.channel = OMAP_DSS_CHANNEL_DIGIT,
// };

// static struct omap_dss_device *sdp4430_dss_devices[] = {
// 	&sdp4430_lcd_device,
// //	&sdp4430_hdmi_device,
// };
// 
// static struct omap_dss_board_info sdp4430_dss_data = {
// 	.num_devices	= ARRAY_SIZE(sdp4430_dss_devices),
// 	.devices	= sdp4430_dss_devices,
// 	.default_device	= &sdp4430_lcd_device,
// };
// 
// #define BLAZE_FB_RAM_SIZE                SZ_16M /* 1920×1080*4 * 2 */
// static struct omapfb_platform_data blaze_fb_pdata = {
// 	.mem_desc = {
// 		.region_cnt = 1,
// 		.region = {
// 			[0] = {
// 				.size = BLAZE_FB_RAM_SIZE,
// 			},
// 		},
// 	},
// };

// static void omap_4430sdp_display_init(void)
// {
// 	sdp4430_lcd_init();
// 	sdp4430_hdmi_mux_init();
// 	omap_vram_set_sdram_vram(BLAZE_FB_RAM_SIZE, 0);
// 	omapfb_set_platform_data(&blaze_fb_pdata);
// 	omap_display_init(&sdp4430_dss_data);
// }

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	OMAP4_MUX(USBB1_ULPITLL_CLK, OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT),
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};

#else
#define board_mux	NULL
#define board_wkup_mux NULL
#endif

/*
 * LPDDR2 Configeration Data:
 * The memory organisation is as below :
 *	EMIF1 - CS0 -	2 Gb
 *		CS1 -	2 Gb
 *	EMIF2 - CS0 -	2 Gb
 *		CS1 -	2 Gb
 *	--------------------
 *	TOTAL -		8 Gb
 *
 * Same devices installed on EMIF1 and EMIF2
 */
static __initdata struct emif_device_details emif_devices = {
	.cs0_device = &lpddr2_elpida_2G_S4_dev,
	.cs1_device = &lpddr2_elpida_2G_S4_dev
};

static struct omap_device_pad blaze_uart1_pads[] __initdata = {
	{
		.name	= "uart1_cts.uart1_cts",
		.enable	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart1_rts.uart1_rts",
		.enable	= OMAP_PIN_OUTPUT | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart1_tx.uart1_tx",
		.enable	= OMAP_PIN_OUTPUT | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart1_rx.uart1_rx",
		.flags	= OMAP_DEVICE_PAD_REMUX | OMAP_DEVICE_PAD_WAKEUP,
		.enable	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE0,
		.idle	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE0,
	},
};


static struct omap_device_pad blaze_uart2_pads[] __initdata = {
	{
		.name	= "uart2_cts.uart2_cts",
		.enable	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE0,
		.flags  = OMAP_DEVICE_PAD_REMUX,
		.idle   = OMAP_WAKEUP_EN | OMAP_PIN_OFF_INPUT_PULLUP |
			  OMAP_MUX_MODE0,
	},
	{
		.name	= "uart2_rts.uart2_rts",
		.flags  = OMAP_DEVICE_PAD_REMUX,
		.enable	= OMAP_PIN_OUTPUT | OMAP_MUX_MODE0,
		.idle   = OMAP_PIN_OFF_INPUT_PULLUP | OMAP_MUX_MODE7,
	},
	{
		.name	= "uart2_tx.uart2_tx",
		.enable	= OMAP_PIN_OUTPUT | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart2_rx.uart2_rx",
		.enable	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE0,
	},
};

static struct omap_device_pad blaze_uart3_pads[] __initdata = {
	{
		.name	= "uart3_cts_rctx.uart3_cts_rctx",
		.enable	= OMAP_PIN_INPUT_PULLUP | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart3_rts_sd.uart3_rts_sd",
		.enable	= OMAP_PIN_OUTPUT | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart3_tx_irtx.uart3_tx_irtx",
		.enable	= OMAP_PIN_OUTPUT | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart3_rx_irrx.uart3_rx_irrx",
		.flags	= OMAP_DEVICE_PAD_REMUX | OMAP_DEVICE_PAD_WAKEUP,
		.enable	= OMAP_PIN_INPUT | OMAP_MUX_MODE0,
		.idle	= OMAP_PIN_INPUT | OMAP_MUX_MODE0,
	},
};

static struct omap_device_pad blaze_uart4_pads[] __initdata = {
	{
		.name	= "uart4_tx.uart4_tx",
		.enable	= OMAP_PIN_OUTPUT | OMAP_MUX_MODE0,
	},
	{
		.name	= "uart4_rx.uart4_rx",
		.flags	= OMAP_DEVICE_PAD_REMUX | OMAP_DEVICE_PAD_WAKEUP,
		.enable	= OMAP_PIN_INPUT | OMAP_MUX_MODE0,
		.idle	= OMAP_PIN_INPUT | OMAP_MUX_MODE0,
	},
};

static struct omap_uart_port_info blaze_uart_info_uncon __initdata = {
	.use_dma	= 0,
	.auto_sus_timeout = DEFAULT_AUTOSUSPEND_DELAY,
        .wer = 0,
};

static struct omap_uart_port_info blaze_uart_info __initdata = {
	.use_dma	= 0,
	.auto_sus_timeout = DEFAULT_AUTOSUSPEND_DELAY,
        .wer = (OMAP_UART_WER_TX | OMAP_UART_WER_RX | OMAP_UART_WER_CTS),
};

static inline void __init board_serial_init(void)
{
	omap_serial_init_port_pads(0, blaze_uart1_pads,
		ARRAY_SIZE(blaze_uart1_pads), &blaze_uart_info_uncon);
	omap_serial_init_port_pads(1, blaze_uart2_pads,
		ARRAY_SIZE(blaze_uart2_pads), &blaze_uart_info);
	omap_serial_init_port_pads(2, blaze_uart3_pads,
		ARRAY_SIZE(blaze_uart3_pads), &blaze_uart_info);
	omap_serial_init_port_pads(3, blaze_uart4_pads,
		ARRAY_SIZE(blaze_uart4_pads), &blaze_uart_info_uncon);
}

static void omap4_sdp4430_wifi_mux_init(void)
{
	omap_mux_init_gpio(GPIO_WIFI_IRQ, OMAP_PIN_INPUT |
				OMAP_PIN_OFF_WAKEUPENABLE);
	omap_mux_init_gpio(GPIO_WIFI_PMENA, OMAP_PIN_OUTPUT);

	omap_mux_init_signal("sdmmc5_cmd.sdmmc5_cmd",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_clk.sdmmc5_clk",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_dat0.sdmmc5_dat0",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_dat1.sdmmc5_dat1",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_dat2.sdmmc5_dat2",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("sdmmc5_dat3.sdmmc5_dat3",
				OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
}

static struct wl12xx_platform_data omap4_sdp4430_wlan_data __initdata = {
	.irq = OMAP_GPIO_IRQ(GPIO_WIFI_IRQ),
	.board_ref_clock = WL12XX_REFCLOCK_26,
	.board_tcxo_clock = WL12XX_TCXOCLOCK_26,
};

static void omap4_sdp4430_wifi_init(void)
{
	omap4_sdp4430_wifi_mux_init();
	if (wl12xx_set_platform_data(&omap4_sdp4430_wlan_data))
		pr_err("Error setting wl12xx data\n");
	platform_device_register(&omap_vwlan_device);
}

#if defined(CONFIG_USB_EHCI_HCD_OMAP) || defined(CONFIG_USB_OHCI_HCD_OMAP3)
struct usbhs_omap_board_data usbhs_bdata __initdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_OHCI_PORT_MODE_PHY_6PIN_DATSE0,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
	.phy_reset  = false,
	.reset_gpio_port[0]  = -EINVAL,
	.reset_gpio_port[1]  = -EINVAL,
	.reset_gpio_port[2]  = -EINVAL
};

static void __init omap4_ehci_ohci_init(void)
{

// 	omap_mux_init_signal("usbb2_ulpitll_clk.gpio_157", \
// 		OMAP_PIN_OUTPUT | \
// 		OMAP_PIN_OFF_NONE);
// 
// 	/* Power on the ULPI PHY */
// 	if (gpio_is_valid(BLAZE_MDM_PWR_EN_GPIO)) {
// 		gpio_request(BLAZE_MDM_PWR_EN_GPIO, "USBB1 PHY VMDM_3V3");
// 		gpio_direction_output(BLAZE_MDM_PWR_EN_GPIO, 1);
// 	}

	usbhs_init(&usbhs_bdata);

	return;

}
#else
static void __init omap4_ehci_ohci_init(void){}
#endif

static void blaze_set_osc_timings(void)
{
	/* Device Oscilator
	 * tstart = 2ms + 2ms = 4ms.
	 * tshut = Not defined in oscillator data sheet so setting to 1us
	 */
	omap_pm_set_osc_lp_time(4000, 1);
}


/*
 * As OMAP4430 mux HSI and USB signals, when HSI is used (for instance HSI
 * modem is plugged) we should configure HSI pad conf and disable some USB
 * configurations.
 * HSI usage is declared using bootargs variable:
 * board-4430sdp.modem_ipc=hsi
 * Any other or missing value will not setup HSI pad conf, and port_mode[0]
 * will be used by USB.
 * Variable modem_ipc is used to catch bootargs parameter value.
 */
// static char *modem_ipc = "n/a";
// module_param(modem_ipc, charp, 0);
// MODULE_PARM_DESC(modem_ipc, "Modem IPC setting");

static void __init omap_4430sdp_init(void)
{
	int status;
	int package = OMAP_PACKAGE_CBS;

	if (omap_rev() == OMAP4430_REV_ES1_0)
		package = OMAP_PACKAGE_CBL;
	omap4_mux_init(board_mux, NULL, package);

	acclaim_board_init();
	
	omap_emif_setup_device_details(&emif_devices, &emif_devices);

	omap_board_config = sdp4430_config;
	omap_board_config_size = ARRAY_SIZE(sdp4430_config);

	omap_init_board_version(0);

//	omap4_audio_conf();
	omap4_create_board_props();
	blaze_pmic_mux_init();
	blaze_set_osc_timings();
	omap4_i2c_init();
//	blaze_sensor_init();
//	blaze_touch_init();
	omap4_register_ion();
	platform_add_devices(sdp4430_devices, ARRAY_SIZE(sdp4430_devices));
	
#ifdef CONFIG_CHARGER_MAX8903	
	acclaim_init_charger();
#endif
	
	wake_lock_init(&st_wk_lock, WAKE_LOCK_SUSPEND, "st_wake_lock");
	board_serial_init();
	omap4_sdp4430_wifi_init();
	omap4_twl6030_hsmmc_init(mmc);

// 	/* blaze_modem_init shall be called before omap4_ehci_ohci_init */
// 	if (!strcmp(modem_ipc, "hsi"))
// 		blaze_modem_init(true);
// 	else
// 		blaze_modem_init(false);
// #ifdef CONFIG_TIWLAN_SDIO
// 	config_wlan_mux();
// #else
// 	omap4_4430sdp_wifi_init();
// #endif

	kxtf9_dev_init();
#ifdef CONFIG_BATTERY_MAX17042
	max17042_dev_init();
#endif
	omap4_ehci_ohci_init();

	usb_musb_init(&musb_board_data);

// 	status = omap_ethernet_init();
// 	if (status) {
// 		pr_err("Ethernet initialization failed: %d\n", status);
// 	} else {
// 		sdp4430_spi_board_info[0].irq = gpio_to_irq(ETH_KS8851_IRQ);
// 		spi_register_board_info(sdp4430_spi_board_info,
// 				ARRAY_SIZE(sdp4430_spi_board_info));
// 	}

	status = omap4_keyboard_init(&sdp4430_keypad_data);
	if (status)
		pr_err("Keypad initialization failed: %d\n", status);

	spi_register_board_info(sdp4430_spi_board_info,
			ARRAY_SIZE(sdp4430_spi_board_info));
	
	omap_dmm_init();
	acclaim_panel_init();
//	blaze_panel_init();
//	blaze_keypad_init();

// 	if (cpu_is_omap446x()) {
// 		/* Vsel0 = gpio, vsel1 = gnd */
// 		status = omap_tps6236x_board_setup(true, TPS62361_GPIO, -1,
// 					OMAP_PIN_OFF_OUTPUT_HIGH, -1);
// 		if (status)
// 			pr_err("TPS62361 initialization failed: %d\n", status);
// 	}

	omap_enable_smartreflex_on_init();
        if (enable_suspend_off)
                omap_pm_enable_off_mode();

}

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static inline void ramconsole_reserve_sdram(void)
{
	// make the ram console the size of the printk log buffer
    reserve_bootmem(ACCLAIM_RAM_CONSOLE_START, (1 << CONFIG_LOG_BUF_SHIFT), 0);
}
#else
static inline void ramconsole_reserve_sdram(void) {}
#endif /* CONFIG_ANDROID_RAM_CONSOLE */

static void __init omap_4430sdp_map_io(void)
{
//	ramconsole_reserve_sdram();
	omap2_set_globals_443x();
	omap44xx_map_common_io();
}
static void __init omap_4430sdp_reserve(void)
{
	omap_ram_console_init(OMAP_RAM_CONSOLE_START_DEFAULT,
			OMAP_RAM_CONSOLE_SIZE_DEFAULT);

	/* do the static reservations first */
	memblock_remove(PHYS_ADDR_SMC_MEM, PHYS_ADDR_SMC_SIZE);
	memblock_remove(PHYS_ADDR_DUCATI_MEM, PHYS_ADDR_DUCATI_SIZE);
	/* ipu needs to recognize secure input buffer area as well */
	omap_ipu_set_static_mempool(PHYS_ADDR_DUCATI_MEM, PHYS_ADDR_DUCATI_SIZE +
					OMAP4_ION_HEAP_SECURE_INPUT_SIZE);
#ifdef CONFIG_ION_OMAP
	omap_ion_init();
#endif

	omap_reserve();
}

MACHINE_START(OMAP4_NOOKTABLET, "B@N NOOK TABLET")
	.boot_params	= 0x80000100,
	.reserve	= omap_4430sdp_reserve,
	.map_io		= omap_4430sdp_map_io,
	.init_early	= omap_4430sdp_init_early,
	.init_irq	= gic_init_irq,
	.init_machine	= omap_4430sdp_init,
	.timer		= &omap_timer,
MACHINE_END
