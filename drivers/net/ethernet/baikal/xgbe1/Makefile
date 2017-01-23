#
# MAkefile for Baikal Electronics 10Gb Ethernet driver
#

obj-$(CONFIG_BAIKAL_XGBE) += baikal-xgbe.o

baikal-xgbe-objs := xgbe-main.o xgbe-drv.o xgbe-dev.o \
		 xgbe-desc.o xgbe-ethtool.o xgbe-mdio.o \
		 xgbe-ptp.o

baikal-xgbe-$(CONFIG_DEBUG_FS) += xgbe-debugfs.o
