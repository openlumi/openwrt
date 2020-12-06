include $(TOPDIR)/rules.mk

PKG_NAME:=lumi-8189es-wifi
PKG_RELEASE=5.3.3.1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Ben Greear <greearb@candelatech.com>
PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/lumi-8189es-wifi
  SUBMENU:=Lumi
  TITLE:=Wireless Drivers for Xiaomi Lumi Gw Realtek rtl8189es wifi module
  DEPENDS:=+kmod-cfg80211 +@DRIVER_11N_SUPPORT
  FILES:=\
	$(PKG_BUILD_DIR)/rtl8189es.ko
  AUTOLOAD:=$(call AutoProbe,rtl8189es)
  PROVIDES:=kmod-rtl8189es
endef

NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/backport.h

NOSTDINC_FLAGS+=-DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONCURRENT_MODE -DBUILD_OPENWRT

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

$(eval $(call KernelPackage,lumi-8189es-wifi))
