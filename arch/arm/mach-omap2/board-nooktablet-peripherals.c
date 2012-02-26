/*
 * Board support file for OMAP4430 ACCLAIM.
 *
 * Copyright (C) 2009 Texas Instruments
 * Intrinsyc Software International, Inc. on behalf of Barnes & Noble, Inc.
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
#include <linux/gpio_keys.h>
#include <linux/usb/otg.h>
#include <linux/spi/spi.h>
#include <linux/i2c/twl.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/tps6130x.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/wl12xx.h>

#include <mach/hardware.h>
#include <mach/omap4-common.h>
#include <mach/emif.h>
#include <mach/lpddr2-elpida.h>
#include <mach/dmm.h>
//#include <mach/lpddr2-samsung.h>
#include <mach/board-nooktablet.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/control.h>
#include "timer-gp.h"
#include <plat/display.h>
#include <plat/usb.h>
#include <plat/omap_device.h>
#include <plat/omap_hwmod.h>
#include <plat/omap-serial.h>
#include <plat/serial.h>
#include <linux/wakelock.h>
#include <plat/opp_twl_tps.h>
#include <plat/mmc.h>
#include <plat/omap4-keypad.h>
#include <plat/voltage.h>
#include "omap_ram_console.h"
#include "common-board-devices.h"
#include "mux.h"
#include "hsmmc.h"
#include <plat/omap-pm.h>
#include "pm.h"
#include "board-44xx-tablet.h"
#include "omap4_ion.h"
//#include "smartreflex-class3.h"
//#include "board-4430sdp-wifi.h"

#include <linux/skbuff.h>
#include <linux/ti_wilink_st.h>
#include <linux/err.h>
#include <linux/hwspinlock.h>

#include <linux/input/cyttsp.h>
#include <linux/input/ft5x06.h>
#include <linux/input/kxtf9.h>
#include <linux/power/max17042.h>
#include <linux/power/max8903.h>

#include <plat/mcspi.h>

#define WILINK_UART_DEV_NAME "/dev/ttyO1"
#define KXTF9_DEVICE_ID                 "kxtf9"
#define KXTF9_I2C_SLAVE_ADDRESS         0x0F
#define KXTF9_GPIO_FOR_PWR              34

#define CONFIG_SERIAL_OMAP_IDLE_TIMEOUT 5

#define GPIO_WIFI_PMENA		54
#define GPIO_WIFI_IRQ		53

#define CYTTSP_I2C_SLAVEADDRESS 	34
#define OMAP_CYTTSP_GPIO        	37 /*99*/
#define OMAP_CYTTSP_RESET_GPIO 		39 /*46*/

#define FT5x06_I2C_SLAVEADDRESS  (0x70 >> 1)
#define OMAP_FT5x06_GPIO         37 /*99*/
#define OMAP_FT5x06_RESET_GPIO   39 /*46*/

#define TWL6030_RTC_GPIO 6
#define BLUETOOTH_UART UART2
#define CONSOLE_UART UART1

#define  SAMSUNG_SDRAM 0x1
#define  ELPIDA_SDRAM  0x3
#define  HYNIX_SDRAM   0x6

#define TPS62361_GPIO   7

static int max17042_gpio_for_irq = 0;
static int kxtf9_gpio_for_irq = 0;

void __init nooktablet_panel_init(void);

#ifdef CONFIG_BATTERY_MAX17042
static void max17042_dev_init(void)
{
	printk("board-4430sdp.c: max17042_dev_init ...\n");

	if (gpio_request(max17042_gpio_for_irq, "max17042_irq") < 0) {
		printk(KERN_ERR "Can't get GPIO for max17042 IRQ\n");
		return;
	}

	printk("board-4430sdp.c: max17042_dev_init > Init max17042 irq pin %d !\n", max17042_gpio_for_irq);
	gpio_direction_input(max17042_gpio_for_irq);
	printk("max17042 GPIO pin read %d\n", gpio_get_value(max17042_gpio_for_irq));
}
#endif

static void kxtf9_dev_init(void)
{
	printk("board-4430sdp.c: kxtf9_dev_init ...\n");

	if (gpio_request(kxtf9_gpio_for_irq, "kxtf9_irq") < 0)
	{
		printk("Can't get GPIO for kxtf9 IRQ\n");
		return;
	}

	printk("board-4430sdp.c: kxtf9_dev_init > Init kxtf9 irq pin %d !\n",
			kxtf9_gpio_for_irq);
	gpio_direction_input(kxtf9_gpio_for_irq);
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

	.gpio = 0,
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

int cyttsp_dev_init(int resource)
{
	if (resource) {
		omap_mux_init_signal("gpmc_ad13.gpio_37", OMAP_PIN_INPUT | OMAP_PIN_OFF_WAKEUPENABLE);
		omap_mux_init_signal("gpmc_ad15.gpio_39", OMAP_PIN_OUTPUT );


		if (gpio_request(OMAP_CYTTSP_RESET_GPIO, "tma340_reset") < 0) {
			printk(KERN_ERR "can't get tma340 xreset GPIO\n");
			return -1;
		}

		if (gpio_request(OMAP_CYTTSP_GPIO, "cyttsp_touch") < 0) {
			printk(KERN_ERR "can't get cyttsp interrupt GPIO\n");
			return -1;
		}

		gpio_direction_input(OMAP_CYTTSP_GPIO);
		/* omap_set_gpio_debounce(OMAP_CYTTSP_GPIO, 0); */
	} else {
		printk ("\n%s: Free resources",__FUNCTION__);
		gpio_free(OMAP_CYTTSP_GPIO);
		gpio_free(OMAP_CYTTSP_RESET_GPIO);
	}
	return 0;
}

static struct cyttsp_platform_data cyttsp_platform_data = {
	.maxx = 480,
	.maxy = 800,
	.flags = 0,
	.gen = CY_GEN3,
	.use_st = CY_USE_ST,
	.use_mt = CY_USE_MT,
	.use_hndshk = CY_SEND_HNDSHK,
	.use_trk_id = CY_USE_TRACKING_ID,
	.use_sleep = CY_USE_SLEEP,
	.use_gestures = CY_USE_GESTURES,
	/* activate up to 4 groups
	 * and set active distance
	 */
	.gest_set = CY_GEST_GRP1 | CY_GEST_GRP2 | CY_GEST_GRP3 | CY_GEST_GRP4 | CY_ACT_DIST,
	/* change act_intrvl to customize the Active power state.
	 * scanning/processing refresh interval for Operating mode
	 */
	.act_intrvl = CY_ACT_INTRVL_DFLT,
	/* change tch_tmout to customize the touch timeout for the
	 * Active power state for Operating mode
	 */
	.tch_tmout = CY_TCH_TMOUT_DFLT,
	/* change lp_intrvl to customize the Low Power power state.
	 * scanning/processing refresh interval for Operating mode
	 */
	.lp_intrvl = CY_LP_INTRVL_DFLT,
};

static struct wake_lock uart_lock;

static int sdp4430_keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(1, 0, KEY_VOLUMEDOWN),
};

static struct matrix_keymap_data sdp4430_keymap_data = {
	.keymap			= sdp4430_keymap,
	.keymap_size		= ARRAY_SIZE(sdp4430_keymap),
};

static struct omap4_keypad_platform_data sdp4430_keypad_data = {
	.keymap_data		= &sdp4430_keymap_data,
	.rows			= 2,
	.cols			= 1,
//	.rep			= 0, //FIXME need or not need?
};

void keyboard_mux_init(void)
{
	// Column mode
	omap_mux_init_signal("kpd_col0.kpd_col0",
			OMAP_WAKEUP_EN | OMAP_MUX_MODE0);
	// Row mode
	omap_mux_init_signal("kpd_row0.kpd_row0",
			OMAP_PULL_ENA | OMAP_PULL_UP |
			OMAP_WAKEUP_EN | OMAP_MUX_MODE0 |
			OMAP_INPUT_EN);
	omap_mux_init_signal("kpd_row1.kpd_row1",
			OMAP_PULL_ENA | OMAP_PULL_UP |
			OMAP_WAKEUP_EN | OMAP_MUX_MODE0 |
			OMAP_INPUT_EN);
}

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

	.gpio = 0,

};
#endif
static struct omap2_mcspi_device_config boxer_mcspi_config = {
		.turbo_mode= 1,
		.single_channel= 1,  /* 0: slave, 1: master */
};
static struct spi_board_info sdp4430_spi_board_info[] __initdata = {
	{
		.modalias		= "boxer_disp_spi",
		.bus_num		= 4,	/* 4: McSPI4 */
		.chip_select		= 0,
		.max_speed_hz		= 375000,
		.controller_data	= &boxer_mcspi_config
	},
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
.baud_rate = 3686400,
.suspend = plat_wlink_kim_suspend,
.resume = plat_wlink_kim_resume,
.chip_asleep = plat_uart_disable,
.chip_awake  = plat_uart_enable,
.chip_enable = plat_uart_enable,
.chip_disable = plat_uart_disable,
};

static struct platform_device wl128x_device = {
.name= "kim",
.id= -1,
.dev.platform_data = &wilink_pdata,
};

static struct platform_device btwilink_device = {
.name = "btwilink",
.id = -1,
};
#ifdef CONFIG_TI_ST
static bool is_bt_active(void)
{
	struct platform_device  *pdev;
	struct kim_data_s       *kim_gdata;

	pdev = &wl128x_device;
	kim_gdata = dev_get_drvdata(&pdev->dev);
	if (st_ll_getstate(kim_gdata->core_data) != ST_LL_ASLEEP &&
			st_ll_getstate(kim_gdata->core_data) != ST_LL_INVALID)
		return true;
	else
		return false;
}
#else
#define is_bt_active NULL
#endif

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

static struct platform_device *nooktablet_devices[] __initdata = {
	&acclaim_keys_gpio,
	&acclaim_lcd_touch_regulator_device,
	&wl128x_device,
	&btwilink_device,
};

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

#ifndef CONFIG_TIWLAN_SDIO
static int wifi_set_power(struct device *dev, int slot, int power_on, int vdd)
{
	static int power_state;

	pr_debug("Powering %s wifi", (power_on ? "on" : "off"));

	if (power_on == power_state)
		return 0;
	power_state = power_on;

	if (power_on) {
		gpio_set_value(GPIO_WIFI_PMENA, 1);
		mdelay(15);
		gpio_set_value(GPIO_WIFI_PMENA, 0);
		mdelay(1);
		gpio_set_value(GPIO_WIFI_PMENA, 1);
		mdelay(70);
	} else {
		gpio_set_value(GPIO_WIFI_PMENA, 0);
	}

	return 0;
}
#endif

static struct twl4030_usb_data omap4_usbphy_data = {
	.phy_init	= omap4430_phy_init,
	.phy_exit	= omap4430_phy_exit,
	.phy_power	= omap4430_phy_power,
	.phy_set_clock	= omap4430_phy_set_clk,
	.phy_suspend= omap4430_phy_suspend,
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
		.power_saving	= false,//true,
#endif
	},
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
			MMC_CAP_1_8V_DDR,
		.gpio_wp	= -EINVAL,
#ifdef CONFIG_PM_RUNTIME
		.power_saving	= false,//true,
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
	{}	/* Terminator */
};

#ifndef CONFIG_TIWLAN_SDIO
static struct wl12xx_platform_data omap4_panda_wlan_data __initdata = {
	.irq = OMAP_GPIO_IRQ(GPIO_WIFI_IRQ),
	.board_ref_clock = WL12XX_REFCLOCK_26,
	.board_tcxo_clock = 1,
};
#endif

static struct regulator_consumer_supply sdp4430_vmmc_supply[] = {
//		.supply = "vmmc",
//		.dev_name = "omap_hsmmc.0",
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.0"),
};

static struct regulator_consumer_supply sdp4430_vemmc_supply[] = {
//		.supply = "vmmc",
//		.dev_name = "omap_hsmmc.1",
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.1"),
};

static struct regulator_consumer_supply sdp4430_vcxio_supply[] = {
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dss"),
	REGULATOR_SUPPLY("vdds_dsi", "omapdss_dsi1"),
};

static struct regulator_consumer_supply sdp4430_vwlan_supply[] = {
	{
		.supply = "vwlan",
	},
};

static int omap4_twl6030_hsmmc_late_init(struct device *dev)
{
	int ret = 0;
	struct platform_device *pdev = container_of(dev,
			struct platform_device, dev);
	struct omap_mmc_platform_data *pdata = dev->platform_data;

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
	if (!dev)
		return;

	pdata = dev->platform_data;
	pdata->init = omap4_twl6030_hsmmc_late_init;
}

static int __init omap4_twl6030_hsmmc_init(struct omap2_hsmmc_info *controllers)
{
	struct omap2_hsmmc_info *c;

	omap2_hsmmc_init(controllers);
	for (c = controllers; c->mmc; c++)
		omap4_twl6030_hsmmc_set_late_init(c->dev);

	return 0;
}

static struct regulator_init_data sdp4430_vaux1 = {
	.constraints = {
// 		.min_uV			= 1000000,
// 		.max_uV			= 3000000,
// 		.apply_uV		= true,
// 		.valid_modes_mask	= REGULATOR_MODE_NORMAL
// 			| REGULATOR_MODE_STANDBY,
// 		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
// 			| REGULATOR_CHANGE_MODE
// 			| REGULATOR_CHANGE_STATUS,
// 		.state_mem = {
// 			.enabled	= false,
// 			.disabled	= true,
// 		},
// 		.always_on	= true,
// 	},
// 	//.num_consumer_supplies	= 1,
// 	//.consumer_supplies	= sdp4430_vemmc_supply,

		.min_uV			= 1000000,
		.max_uV			= 3000000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = sdp4430_vemmc_supply,
};

static struct regulator_consumer_supply sdp4430_vaux2_supply[] = {
	REGULATOR_SUPPLY("av-switch", "soc-audio"),
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
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= sdp4430_vaux2_supply,
};

static struct regulator_init_data sdp4430_vaux3 = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
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
	.num_consumer_supplies = 1,
	.consumer_supplies = sdp4430_vwlan_supply,
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
	},
};

static struct regulator_init_data sdp4430_vana = {
	.constraints = {
		.min_uV			= 2100000,
		.max_uV			= 2100000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
		.always_on	= true,
	},
};

static struct regulator_init_data sdp4430_vcxio = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
		.always_on	= true,
	},
	.num_consumer_supplies	= ARRAY_SIZE(sdp4430_vcxio_supply),
	.consumer_supplies	= sdp4430_vcxio_supply,
};

static struct regulator_consumer_supply sdp4430_vdac_supply[] = {
	{
		.supply = "hdmi_vref",
	},
};

static struct regulator_init_data sdp4430_vdac = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
		.always_on	= true,
	},
	.num_consumer_supplies  = ARRAY_SIZE(sdp4430_vdac_supply),
	.consumer_supplies      = sdp4430_vdac_supply,
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
	},
};

static struct regulator_init_data sdp4430_clk32kg = {
	.constraints = {
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
		.always_on		= true,
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


static struct twl4030_platform_data sdp4430_twldata = {
	.irq_base	= TWL6030_IRQ_BASE,
	.irq_end	= TWL6030_IRQ_END,

	/* Regulators */
	.vmmc		= &sdp4430_vmmc,
	.vpp		= &sdp4430_vpp,
//	.vusim		= &sdp4430_vusim,  //not connected on the board
	.vana		= &sdp4430_vana,
	.vcxio		= &sdp4430_vcxio,
//	.vdac		= &sdp4430_vdac,	//not used
	.vusb		= &sdp4430_vusb,
	.vaux1		= &sdp4430_vaux1,
//	.vaux2		= &sdp4430_vaux2,	//proximity sensor not functional switching off vaux2.
	.vaux3		= &sdp4430_vaux3,
	.usb		= &omap4_usbphy_data,
	.clk32kg        = &sdp4430_clk32kg,      // always ON for WiFi
 	.madc           = &sdp4430_gpadc_data,
};

static struct i2c_board_info __initdata sdp4430_i2c_1_boardinfo[] = {
	{
		I2C_BOARD_INFO("twl6030", 0x48),
		.flags = I2C_CLIENT_WAKE,
		.irq = OMAP44XX_IRQ_SYS_1N,
		.platform_data = &sdp4430_twldata,
	},
	{
		I2C_BOARD_INFO(KXTF9_DEVICE_ID, KXTF9_I2C_SLAVE_ADDRESS),
		.platform_data = &kxtf9_platform_data_here,
		.irq = 0,
	},
	{
		I2C_BOARD_INFO(MAX17042_DEVICE_ID, MAX17042_I2C_SLAVE_ADDRESS),
		.platform_data = &max17042_platform_data_here,
		.irq = 0,
	},
};

void __init acclaim_board_init(void)
{
	const int board_type = acclaim_board_type();
	show_acclaim_board_revision(board_type);

	if ( board_type == EVT1A ){
		max17042_gpio_for_irq = 98;
		kxtf9_gpio_for_irq = 99;
	} else if ( board_type >= EVT1B ) {
		max17042_gpio_for_irq = 65;
		kxtf9_gpio_for_irq = 66;
	}

	max17042_platform_data_here.gpio = max17042_gpio_for_irq;
	sdp4430_i2c_1_boardinfo[2].irq = OMAP_GPIO_IRQ(max17042_gpio_for_irq);
	kxtf9_platform_data_here.gpio = kxtf9_gpio_for_irq;
	sdp4430_i2c_1_boardinfo[1].irq = OMAP_GPIO_IRQ(kxtf9_gpio_for_irq);
	omap_mux_init_signal("sys_pwron_reset_out", OMAP_MUX_MODE3);
	omap_mux_init_signal("fref_clk3_req", OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN);
}

static struct i2c_board_info __initdata sdp4430_i2c_2_boardinfo[] = {
	{
		I2C_BOARD_INFO(CY_I2C_NAME, CYTTSP_I2C_SLAVEADDRESS),
		.platform_data = &cyttsp_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_CYTTSP_GPIO),
	},
	{
		I2C_BOARD_INFO(FT_I2C_NAME, FT5x06_I2C_SLAVEADDRESS),
		.platform_data = &ft5x06_platform_data,
		.irq = OMAP_GPIO_IRQ(OMAP_FT5x06_GPIO),
	},
	{
		I2C_BOARD_INFO("tlv320aic3100", 0x18),
	},
};

static struct i2c_board_info __initdata sdp4430_i2c_3_boardinfo[] = {
};

static struct i2c_board_info __initdata sdp4430_i2c_4_boardinfo[] = {
};

static struct usbhs_omap_board_data usbhs_pdata __initconst = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_OHCI_PORT_MODE_PHY_6PIN_DATSE0,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
	.phy_reset  = false,
	.reset_gpio_port[0]  = -EINVAL,
	.reset_gpio_port[1]  = -EINVAL,
	.reset_gpio_port[2]  = -EINVAL
};

static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_1_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_2_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_3_bus_pdata;
static struct omap_i2c_bus_board_data __initdata sdp4430_i2c_4_bus_pdata;

static void blaze_set_osc_timings(void)
{
	/* Device Oscilator
	 * tstart = 2ms + 2ms = 4ms.
	 * tshut = Not defined in oscillator data sheet so setting to 1us
	 */
	omap_pm_set_osc_lp_time(4000, 1);
}
/*
 * LPDDR2 Configuration Data:
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
//	omap_serial_init_port_pads(2, blaze_uart3_pads,
//		ARRAY_SIZE(blaze_uart3_pads), &blaze_uart_info);
//	omap_serial_init_port_pads(3, blaze_uart4_pads,
//		ARRAY_SIZE(blaze_uart4_pads), &blaze_uart_info_uncon);
}
/*static void __init omap_i2c_hwspin_lock_init(int bus_id, unsigned int
		spinlock_id, struct omap_i2c_bus_board_data *pdata)
{
	pdata->handle = hwspin_lock_request_specific(spinlock_id);
	if (pdata->handle != NULL) {
		pdata->hwspin_lock_lock = hwspin_lock_lock;
		pdata->hwspin_lock_unlock = hwspin_lock_unlock;
	} else {
		pr_err("I2C hwspinlock request failed for bus %d\n", bus_id);
	}
}*/

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

static int __init omap4_i2c_init(void)
{
	omap_i2c_hwspinlock_init(1, 0, &sdp4430_i2c_1_bus_pdata);
	omap_i2c_hwspinlock_init(2, 1, &sdp4430_i2c_2_bus_pdata);
	omap_i2c_hwspinlock_init(3, 2, &sdp4430_i2c_3_bus_pdata);
	omap_i2c_hwspinlock_init(4, 3, &sdp4430_i2c_4_bus_pdata);

	regulator_has_full_constraints();

	/*
	 * Phoenix Audio IC needs I2C1 to
	 * start with 400 KHz or less
	 */
	omap_register_i2c_bus_board_data(1, &sdp4430_i2c_1_bus_pdata);
	omap_register_i2c_bus_board_data(2, &sdp4430_i2c_2_bus_pdata);
	omap_register_i2c_bus_board_data(3, &sdp4430_i2c_3_bus_pdata);
	omap_register_i2c_bus_board_data(4, &sdp4430_i2c_4_bus_pdata);

	omap_register_i2c_bus(1, 400, sdp4430_i2c_1_boardinfo, ARRAY_SIZE(sdp4430_i2c_1_boardinfo));
	omap_register_i2c_bus(2, 400, sdp4430_i2c_2_boardinfo, ARRAY_SIZE(sdp4430_i2c_2_boardinfo));
	omap_register_i2c_bus(3, 400, sdp4430_i2c_3_boardinfo, ARRAY_SIZE(sdp4430_i2c_3_boardinfo));
	omap_register_i2c_bus(4, 400, sdp4430_i2c_4_boardinfo, ARRAY_SIZE(sdp4430_i2c_4_boardinfo));
	return 0;
}
static bool enable_suspend_off = true;
module_param(enable_suspend_off, bool, S_IRUSR | S_IRGRP | S_IROTH);

//static void enable_board_wakeup_source(void)
//{
//	int gpio_val;
//	/* Android does not have touchscreen as wakeup source */
//#if !defined(CONFIG_ANDROID)
//	gpio_val = omap_mux_get_gpio(OMAP4_TOUCH_IRQ_1);
//	if ((gpio_val & OMAP44XX_PADCONF_WAKEUPENABLE0) == 0) {
//		gpio_val |= OMAP44XX_PADCONF_WAKEUPENABLE0;
//		omap_mux_set_gpio(gpio_val, OMAP4_TOUCH_IRQ_1);
//	}

//#endif
//	gpio_val = omap_mux_get_gpio(32);

//	if ((gpio_val & OMAP44XX_PADCONF_WAKEUPENABLE0) == 0) {
//		gpio_val |= OMAP44XX_PADCONF_WAKEUPENABLE0;
//		omap_mux_set_gpio(gpio_val, 32);
//	}

//	gpio_val = omap_mux_get_gpio(29);

//	if ((gpio_val & OMAP44XX_PADCONF_WAKEUPENABLE0) == 0) {
//		gpio_val |= OMAP44XX_PADCONF_WAKEUPENABLE0;
//		omap_mux_set_gpio(gpio_val, 29);
//	}

	/*
	 * Enable IO daisy for sys_nirq1/2, to be able to
	 * wakeup from interrupts from PMIC/Audio IC.
	 * Needed only in Device OFF mode.
	 */
//	omap_mux_set_wakeupenable("sys_nirq1");//FIXME new interface added in this kernel
//}

static struct omap_volt_pmic_info omap_pmic_core = {
	.name = "twl",
	.slew_rate = 4000,
	.step_size = 12500,
	.i2c_addr = 0x12,
	.i2c_vreg = 0x61,
	.i2c_cmdreg = 0x62,
	.vsel_to_uv = omap_twl_vsel_to_uv,
	.uv_to_vsel = omap_twl_uv_to_vsel,
	.onforce_cmd = omap_twl_onforce_cmd,
	.on_cmd = omap_twl_on_cmd,
	.sleepforce_cmd = omap_twl_sleepforce_cmd,
	.sleep_cmd = omap_twl_sleep_cmd,
	.vp_config_erroroffset = 0,
	.vp_vstepmin_vstepmin = 0x01,
	.vp_vstepmax_vstepmax = 0x04,
	.vp_vlimitto_timeout_us = 0x200,
	.vp_vlimitto_vddmin = 0xA,
	.vp_vlimitto_vddmax = 0x28,
};

static struct omap_volt_pmic_info omap_pmic_mpu = {
	.name = "twl",
	.slew_rate = 4000,
	.step_size = 12500,
	.i2c_addr = 0x12,
	.i2c_vreg = 0x55,
	.i2c_cmdreg = 0x56,
	.vsel_to_uv = omap_twl_vsel_to_uv,
	.uv_to_vsel = omap_twl_uv_to_vsel,
	.onforce_cmd = omap_twl_onforce_cmd,
	.on_cmd = omap_twl_on_cmd,
	.sleepforce_cmd = omap_twl_sleepforce_cmd,
	.sleep_cmd = omap_twl_sleep_cmd,
	.vp_config_erroroffset = 0,
	.vp_vstepmin_vstepmin = 0x01,
	.vp_vstepmax_vstepmax = 0x04,
	.vp_vlimitto_timeout_us = 0x200,
	.vp_vlimitto_vddmin = 0xA,
	.vp_vlimitto_vddmax = 0x39,
};

static struct omap_volt_pmic_info omap_pmic_iva = {
	.name = "twl",
	.slew_rate = 4000,
	.step_size = 12500,
	.i2c_addr = 0x12,
	.i2c_vreg = 0x5b,
	.i2c_cmdreg = 0x5c,
	.vsel_to_uv = omap_twl_vsel_to_uv,
	.uv_to_vsel = omap_twl_uv_to_vsel,
	.onforce_cmd = omap_twl_onforce_cmd,
	.on_cmd = omap_twl_on_cmd,
	.sleepforce_cmd = omap_twl_sleepforce_cmd,
	.sleep_cmd = omap_twl_sleep_cmd,
	.vp_config_erroroffset = 0,
	.vp_vstepmin_vstepmin = 0x01,
	.vp_vstepmax_vstepmax = 0x04,
	.vp_vlimitto_timeout_us = 0x200,
	.vp_vlimitto_vddmin = 0xA,
	.vp_vlimitto_vddmax = 0x2D,
};

static struct omap_volt_vc_data vc_config = {
	.vdd0_on = 1375000,        /* 1.375v */
	.vdd0_onlp = 1375000,      /* 1.375v */
	.vdd0_ret = 837500,       /* 0.8375v */
	.vdd0_off = 0,		/* 0 v */
	.vdd1_on = 1300000,        /* 1.3v */
	.vdd1_onlp = 1300000,      /* 1.3v */
	.vdd1_ret = 837500,       /* 0.8375v */
	.vdd1_off = 0,		/* 0 v */
	.vdd2_on = 1200000,        /* 1.2v */
	.vdd2_onlp = 1200000,      /* 1.2v */
	.vdd2_ret = 837500,       /* .8375v */
	.vdd2_off = 0,		/* 0 v */
};

void plat_hold_wakelock(void *up, int flag)
{
	struct uart_omap_port *up2 = (struct uart_omap_port *)up;
	/*Specific wakelock for bluetooth usecases*/
	if ((up2->pdev->id == BLUETOOTH_UART)
			&& ((flag == WAKELK_TX) || (flag == WAKELK_RX)))
		wake_lock_timeout(&uart_lock, 2*HZ);

	/*Specific wakelock for console usecases*/
	if ((up2->pdev->id == CONSOLE_UART)
			&& ((flag == WAKELK_IRQ) || (flag == WAKELK_RESUME)))
		wake_lock_timeout(&uart_lock, 5*HZ);
	return;
}

/*static struct omap_uart_port_info omap_serial_platform_data[] = {
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.plat_hold_wakelock = plat_hold_wakelock,
		.plat_omap_bt_active = NULL,
		.rts_padconf	= 0,
		.rts_override	= 0,
		.cts_padconf	= 0,
		.padconf	= OMAP4_CTRL_MODULE_PAD_MCSPI1_CS1_OFFSET,
		.padconf_wake_ev =
			OMAP4_CTRL_MODULE_PAD_CORE_PADCONF_WAKEUPEVENT_3,
		.wk_mask	=
			OMAP4_MCSPI1_CS1_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_MCSPI1_CS2_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_MCSPI1_CS3_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_UART3_CTS_RCTX_DUPLICATEWAKEUPEVENT_MASK,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.plat_hold_wakelock = plat_hold_wakelock,
		.plat_omap_bt_active = is_bt_active,
		.rts_padconf	= OMAP4_CTRL_MODULE_PAD_UART2_RTS_OFFSET,
		.rts_override	= 0,
		.cts_padconf	= OMAP4_CTRL_MODULE_PAD_UART2_CTS_OFFSET,
		.padconf	= OMAP4_CTRL_MODULE_PAD_UART2_RX_OFFSET,
		.padconf_wake_ev =
			OMAP4_CTRL_MODULE_PAD_CORE_PADCONF_WAKEUPEVENT_3,
		.wk_mask	=
			OMAP4_UART2_TX_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_UART2_RX_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_UART2_RTS_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_UART2_CTS_DUPLICATEWAKEUPEVENT_MASK,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.plat_hold_wakelock = plat_hold_wakelock,
		.plat_omap_bt_active = NULL,
		.rts_padconf	= 0,
		.rts_override	= 0,
		.cts_padconf	= 0,
		.padconf	= OMAP4_CTRL_MODULE_PAD_UART3_RX_IRRX_OFFSET,
		.padconf_wake_ev =
			OMAP4_CTRL_MODULE_PAD_CORE_PADCONF_WAKEUPEVENT_4,
		.wk_mask	=
			OMAP4_UART3_TX_IRTX_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_UART3_RX_IRRX_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_UART3_RTS_SD_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_UART3_CTS_RCTX_DUPLICATEWAKEUPEVENT_MASK,
	},
	{
		.use_dma	= 0,
		.dma_rx_buf_size = DEFAULT_RXDMA_BUFSIZE,
		.dma_rx_poll_rate = DEFAULT_RXDMA_POLLRATE,
		.dma_rx_timeout = DEFAULT_RXDMA_TIMEOUT,
		.idle_timeout	= DEFAULT_IDLE_TIMEOUT,
		.flags		= 1,
		.plat_hold_wakelock = NULL,
		.plat_omap_bt_active = NULL,
		.rts_padconf	= 0,
		.rts_override	= 0,
		.cts_padconf	= 0,
		.padconf	= OMAP4_CTRL_MODULE_PAD_UART4_RX_OFFSET,
		.padconf_wake_ev =
			OMAP4_CTRL_MODULE_PAD_CORE_PADCONF_WAKEUPEVENT_4,
		.wk_mask	=
			OMAP4_UART4_TX_DUPLICATEWAKEUPEVENT_MASK |
			OMAP4_UART4_RX_DUPLICATEWAKEUPEVENT_MASK,
	},
	{
		.flags		= 0
	}
};*/

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
#ifndef CONFIG_TIWLAN_SDIO
	/* WLAN IRQ - GPIO 53 */
	OMAP4_MUX(GPMC_NCS3, OMAP_MUX_MODE3 | OMAP_PIN_INPUT),
	/* WLAN_EN - GPIO 54 */
	OMAP4_MUX(GPMC_NWP, OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT),
	/* WLAN SDIO: MMC5 CMD */
	OMAP4_MUX(SDMMC5_CMD, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	/* WLAN SDIO: MMC5 CLK */
	OMAP4_MUX(SDMMC5_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	/* WLAN SDIO: MMC5 DAT[0-3] */
	OMAP4_MUX(SDMMC5_DAT0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP4_MUX(SDMMC5_DAT1, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP4_MUX(SDMMC5_DAT2, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP4_MUX(SDMMC5_DAT3, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
#endif
	OMAP4_MUX(USBB1_ULPITLL_CLK, OMAP_MUX_MODE3 | OMAP_PIN_INPUT),
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux	NULL
#endif

static void enable_rtc_gpio(void){
	/* To access twl registers we enable gpio6
	 * we need this so the RTC driver can work.
	 */
	gpio_request(TWL6030_RTC_GPIO, "h_SYS_DRM_MSEC");
	gpio_direction_output(TWL6030_RTC_GPIO, 1);

	omap_mux_init_signal("fref_clk0_out.gpio_wk6", \
			OMAP_PIN_OUTPUT | OMAP_PIN_OFF_NONE);
	return;
}

#ifndef CONFIG_TIWLAN_SDIO
static void omap4_4430sdp_wifi_init(void)
{
	if (gpio_request(GPIO_WIFI_PMENA, "wl12xx") ||
			gpio_direction_output(GPIO_WIFI_PMENA, 0))
		pr_err("Error initializing up WLAN_EN\n");
	if (wl12xx_set_platform_data(&omap4_panda_wlan_data))
		pr_err("Error setting wl12xx data\n");
}
#endif

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct resource ram_console_resource = {
	.start  = ACCLAIM_RAM_CONSOLE_START,
	.end    = (ACCLAIM_RAM_CONSOLE_START + (1 << CONFIG_LOG_BUF_SHIFT) - 1),
	.flags  = IORESOURCE_MEM,
};

static struct platform_device ram_console_device = {
	.name           = "ram_console",
	.id             = 0,
	.num_resources  = 1,
	.resource       = &ram_console_resource,
};

static inline void ramconsole_init(void)
{
	platform_device_register(&ram_console_device);
}
#else
static inline void ramconsole_init(void) {}
#endif /* CONFIG_ANDROID_RAM_CONSOLE */

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

//	omap_mux_init_signal("usbb2_ulpitll_clk.gpio_157", \
		OMAP_PIN_OUTPUT | \
		OMAP_PIN_OFF_NONE);

	/* Power on the ULPI PHY */
//	if (gpio_is_valid(BLAZE_MDM_PWR_EN_GPIO)) {
//		gpio_request(BLAZE_MDM_PWR_EN_GPIO, "USBB1 PHY VMDM_3V3");
//		gpio_direction_output(BLAZE_MDM_PWR_EN_GPIO, 1);
//	}

//	usbhs_init(&usbhs_bdata);

	return;

}
#else
static void __init omap4_ehci_ohci_init(void){}
#endif

static struct omap_board_config_kernel sdp4430_config[] __initdata = {
};

void __init acclaim_peripherals_init(void)
{
//	int status;
//	int package = OMAP_PACKAGE_CBS;

//	ramconsole_init();

//	if (omap_rev() == OMAP4430_REV_ES1_0)
//		package = OMAP_PACKAGE_CBL;
//	omap4_mux_init(board_mux, NULL, package);
//	acclaim_board_init();

//	if (sdram_vendor() == SAMSUNG_SDRAM) {
//		omap_emif_setup_device_details(&emif_devices, &emif_devices);
//		printk(KERN_INFO"Samsung DDR Memory \n");
//	} else if (sdram_vendor() == ELPIDA_SDRAM) {
//		omap_emif_setup_device_details(&emif_devices_elpida, &emif_devices_elpida);
//		printk(KERN_INFO"Elpida DDR Memory \n");
//	} else if (sdram_vendor() == HYNIX_SDRAM) {
//		/* Re-use ELPIDA timings as they are absolutely the same */
//		omap_emif_setup_device_details(&emif_devices_elpida, &emif_devices_elpida);
//		printk(KERN_INFO"Hynix DDR Memory \n");
//	} else
//		pr_err("Memory type does not exist\n");

//	omap_init_emif_timings();

//	enable_rtc_gpio();
//	omap4_i2c_init();
//	platform_add_devices(nooktablet_devices, ARRAY_SIZE(nooktablet_devices));
//	acclaim_init_charger();

//	wake_lock_init(&uart_lock, WAKE_LOCK_SUSPEND, "uart_wake_lock");
//	omap_serial_init(/*omap_serial_platform_data*/);
//	omap4_twl6030_hsmmc_init(mmc);

//#ifdef CONFIG_TIWLAN_SDIO
//	config_wlan_mux();
//#else
//	//omap4_4430sdp_wifi_init();//FIXME
//#endif

//	kxtf9_dev_init();
//#ifdef CONFIG_BATTERY_MAX17042
//	max17042_dev_init();
//#endif

//	usbhs_init(&usbhs_pdata);
//	usb_musb_init(&musb_board_data);

//	status = omap4_keyboard_init(&sdp4430_keypad_data);
//	if (status)
//		pr_err("Keypad initialization failed: %d\n", status);
//
//	spi_register_board_info(sdp4430_spi_board_info,
//			ARRAY_SIZE(sdp4430_spi_board_info));

//	nooktablet_panel_init();
//	enable_board_wakeup_source();
// 	omap_voltage_register_data(&omap_pmic_core, "core");
// 	omap_voltage_register_data(&omap_pmic_mpu, "mpu");
// 	omap_voltage_register_data(&omap_pmic_iva, "iva");
// 	omap_voltage_init_vc(&vc_config);
	
//		if (cpu_is_omap446x()) {
//		/* Vsel0 = gpio, vsel1 = gnd */
//		status = omap_tps6236x_board_setup(true, TPS62361_GPIO, -1,
//					OMAP_PIN_OFF_OUTPUT_HIGH, -1);
//		if (status)
//			pr_err("TPS62361 initialization failed: %d\n", status);
//	}
//
//	omap_enable_smartreflex_on_init();
//        if (enable_suspend_off)
//                omap_pm_enable_off_mode();
		
			int status;
	int package = OMAP_PACKAGE_CBS;

	if (omap_rev() == OMAP4430_REV_ES1_0)
		package = OMAP_PACKAGE_CBL;
	omap4_mux_init(board_mux, NULL, package);
	acclaim_board_init();
	omap_emif_setup_device_details(&emif_devices, &emif_devices);

	omap_board_config = sdp4430_config;
	omap_board_config_size = ARRAY_SIZE(sdp4430_config);

	//omap_init_board_version(0);

	//omap4_audio_conf();
	omap4_create_board_props();
	blaze_pmic_mux_init();
	blaze_set_osc_timings();
	omap4_i2c_init();
	//blaze_sensor_init();
	//blaze_touch_init();
	omap4_register_ion();
	platform_add_devices(nooktablet_devices, ARRAY_SIZE(nooktablet_devices));
	wake_lock_init(&st_wk_lock, WAKE_LOCK_SUSPEND, "st_wake_lock");
	board_serial_init();
//	omap4_sdp4430_wifi_init();
	kxtf9_dev_init();
#ifdef CONFIG_BATTERY_MAX17042
	max17042_dev_init();
#endif
	omap4_twl6030_hsmmc_init(mmc);

	/* blaze_modem_init shall be called before omap4_ehci_ohci_init */
//	if (!strcmp(modem_ipc, "hsi"))
//		blaze_modem_init(true);
//	else
//		blaze_modem_init(false);

	omap4_ehci_ohci_init();

	usb_musb_init(&musb_board_data);

	status = omap4_keyboard_init(&sdp4430_keypad_data);
	if (status)
		pr_err("Keypad initialization failed: %d\n", status);

	omap_dmm_init();
	spi_register_board_info(sdp4430_spi_board_info,	ARRAY_SIZE(sdp4430_spi_board_info));
	nooktablet_panel_init();
//	blaze_panel_init();
//	blaze_keypad_init();

	if (cpu_is_omap446x()) {
		/* Vsel0 = gpio, vsel1 = gnd */
		status = omap_tps6236x_board_setup(true, TPS62361_GPIO, -1,
					OMAP_PIN_OFF_OUTPUT_HIGH, -1);
		if (status)
			pr_err("TPS62361 initialization failed: %d\n", status);
	}

	omap_enable_smartreflex_on_init();
        if (enable_suspend_off)
                omap_pm_enable_off_mode();

}
