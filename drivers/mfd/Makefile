#
# Makefile for multifunction miscellaneous devices
#

88pm860x-objs			:= 88pm860x-core.o 88pm860x-i2c.o
obj-$(CONFIG_MFD_88PM860X)	+= 88pm860x.o
obj-$(CONFIG_MFD_SM501)		+= sm501.o
obj-$(CONFIG_MFD_ASIC3)		+= asic3.o tmio_core.o
obj-$(CONFIG_MFD_SH_MOBILE_SDHI)		+= sh_mobile_sdhi.o

obj-$(CONFIG_HTC_EGPIO)		+= htc-egpio.o
obj-$(CONFIG_HTC_PASIC3)	+= htc-pasic3.o
obj-$(CONFIG_HTC_I2CPLD)	+= htc-i2cpld.o

obj-$(CONFIG_MFD_DAVINCI_VOICECODEC)	+= davinci_voicecodec.o
obj-$(CONFIG_MFD_DM355EVM_MSP)	+= dm355evm_msp.o

obj-$(CONFIG_MFD_TC35892)	+= tc35892.o
obj-$(CONFIG_MFD_T7L66XB)	+= t7l66xb.o tmio_core.o
obj-$(CONFIG_MFD_TC6387XB)	+= tc6387xb.o tmio_core.o
obj-$(CONFIG_MFD_TC6393XB)	+= tc6393xb.o tmio_core.o

obj-$(CONFIG_MFD_WM8400)	+= wm8400-core.o
wm831x-objs			:= wm831x-core.o wm831x-irq.o wm831x-otp.o
obj-$(CONFIG_MFD_WM831X)	+= wm831x.o
wm8350-objs			:= wm8350-core.o wm8350-regmap.o wm8350-gpio.o
wm8350-objs			+= wm8350-irq.o
obj-$(CONFIG_MFD_WM8350)	+= wm8350.o
obj-$(CONFIG_MFD_WM8350_I2C)	+= wm8350-i2c.o
obj-$(CONFIG_MFD_WM8994)	+= wm8994-core.o wm8994-irq.o

obj-$(CONFIG_TPS65010)		+= tps65010.o
obj-$(CONFIG_TPS6507X)		+= tps6507x.o
obj-$(CONFIG_MENELAUS)		+= menelaus.o

obj-$(CONFIG_TWL4030_CORE)	+= twl-core.o twl4030-irq.o twl6030-irq.o
obj-$(CONFIG_TWL4030_POWER)    += twl4030-power.o
obj-$(CONFIG_TWL4030_CODEC)	+= twl4030-codec.o

obj-$(CONFIG_MFD_MC13783)	+= mc13783-core.o

ifdef CONFIG_ARCH_MSM8X60
obj-$(CONFIG_MARIMBA_CORE)	+= marimba-core-8x60.o
obj-$(CONFIG_MARIMBA_CODEC)     += marimba-codec-8x60.o
obj-$(CONFIG_MARIMBA_CODEC)     += msm-adie-codec.o
else
obj-$(CONFIG_MARIMBA_CORE)	+= marimba-core.o
obj-$(CONFIG_MARIMBA_CODEC)     += marimba-codec.o
endif

obj-$(CONFIG_MARIMBA_TSADC)	+= marimba-tsadc.o
obj-$(CONFIG_BAHAMA_CORE)	+= bahama-core.o
obj-$(CONFIG_TPS65023)		+= tps65023.o
obj-$(CONFIG_MFD_MC13783)	+= mc13783-core.o

obj-$(CONFIG_MFD_CORE)		+= mfd-core.o

obj-$(CONFIG_EZX_PCAP)		+= ezx-pcap.o

obj-$(CONFIG_MCP)		+= mcp-core.o
obj-$(CONFIG_MCP_SA11X0)	+= mcp-sa11x0.o
obj-$(CONFIG_MCP_UCB1200)	+= ucb1x00-core.o
obj-$(CONFIG_MCP_UCB1200_TS)	+= ucb1x00-ts.o

obj-$(CONFIG_TIMPANI_CODEC)	+= timpani-codec.o
obj-$(CONFIG_TIMPANI_CODEC)	+= msm-adie-codec.o

ifeq ($(CONFIG_SA1100_ASSABET),y)
obj-$(CONFIG_MCP_UCB1200)	+= ucb1x00-assabet.o
endif
obj-$(CONFIG_UCB1400_CORE)	+= ucb1400_core.o

obj-$(CONFIG_PMIC_DA903X)	+= da903x.o
max8925-objs			:= max8925-core.o max8925-i2c.o
obj-$(CONFIG_MFD_MAX8925)	+= max8925.o

pcf50633-objs			:= pcf50633-core.o pcf50633-irq.o
obj-$(CONFIG_MFD_PCF50633)	+= pcf50633.o
obj-$(CONFIG_PCF50633_ADC)	+= pcf50633-adc.o
obj-$(CONFIG_PCF50633_GPIO)	+= pcf50633-gpio.o
obj-$(CONFIG_ABX500_CORE)	+= abx500-core.o
obj-$(CONFIG_AB3100_CORE)	+= ab3100-core.o
obj-$(CONFIG_AB3100_OTP)	+= ab3100-otp.o
obj-$(CONFIG_AB3550_CORE)	+= ab3550-core.o
obj-$(CONFIG_AB8500_CORE)	+= ab8500-core.o ab8500-spi.o
obj-$(CONFIG_MFD_TIMBERDALE)    += timberdale.o
obj-$(CONFIG_PMIC_ADP5520)	+= adp5520.o
obj-$(CONFIG_LPC_SCH)		+= lpc_sch.o
obj-$(CONFIG_MFD_RDC321X)	+= rdc321x-southbridge.o
obj-$(CONFIG_MFD_JANZ_CMODIO)	+= janz-cmodio.o
ifeq ($(CONFIG_MSM_SSBI),y)
obj-$(CONFIG_PMIC8058)		+= pm8058-core.o
else
ifeq ($(CONFIG_ARCH_MSM8X60),y)
obj-$(CONFIG_PMIC8058)		+= pmic8058.o
obj-$(CONFIG_PMIC8901)          += pmic8901.o
endif
endif
#obj-y				+= tps65023.o
