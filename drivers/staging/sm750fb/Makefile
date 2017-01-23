obj-$(CONFIG_FB_SM750)	+= sm750fb.o

sm750fb-objs		:= drv.o hw.o accel.o cursor.o chip.o power.o mode.o
sm750fb-objs		+= display.o help.o swi2c.o sii164.o dvi.o hwi2c.o

ccflags-y += -DUSE_DVICHIP -DDVI_CTRL_SII164
