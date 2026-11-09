include $(TOPDIR)/rules.mk

PKG_NAME:=rtw88-ol
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/igorlistopad/rtw88.git
PKG_MIRROR_HASH:=3740f17fc6706b1cbe72dee2618ed90a79c54527c4a2ddf7ae3ac728230df9ae
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2026-11-05
PKG_SOURCE_VERSION:=b22577b5905e2b1ad92760b4b7b98e4c13b943a5

PKG_MAINTAINER:=Ivan Belokobylskiy <belokobylskij@gmail.com>
PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtw88-ol
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek rtw88 core and SDIO glue (out-of-tree fork)
  DEPENDS:=+kmod-mac80211 +kmod-cfg80211 +kmod-mmc +@DRIVER_11N_SUPPORT
  CONFLICTS:=kmod-rtw88 kmod-rtw88-sdio
  FILES:=\
	$(PKG_BUILD_DIR)/rtw_core.ko \
	$(PKG_BUILD_DIR)/rtw_sdio.ko
  AUTOLOAD:=$(call AutoProbe,rtw_core rtw_sdio)
endef

define KernelPackage/rtw88-ol/description
 Core and SDIO bus modules of the Realtek rtw88 driver
 (https://github.com/igorlistopad/rtw88).
endef

define KernelPackage/rtw88-8189es
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8189ES SDIO WiFi support (rtw88 fork)
  DEPENDS:=+kmod-rtw88-ol +rtw8188e-firmware
  FILES:=\
	$(PKG_BUILD_DIR)/rtw_8188e.ko \
	$(PKG_BUILD_DIR)/rtw_8189es.ko
  AUTOLOAD:=$(call AutoProbe,rtw_8188e rtw_8189es)
endef

define KernelPackage/rtw88-8723bs
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8723BS SDIO WiFi support (rtw88 fork)
  DEPENDS:=+kmod-rtw88-ol +rtw8723b-firmware
  FILES:=\
	$(PKG_BUILD_DIR)/rtw_8723x.ko \
	$(PKG_BUILD_DIR)/rtw_8723b.ko \
	$(PKG_BUILD_DIR)/rtw_8723bs.ko
  AUTOLOAD:=$(call AutoProbe,rtw_8723x rtw_8723b rtw_8723bs)
endef

define Package/rtw8188e-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=RealTek RTL8188E (RTL8189ES) firmware for the rtw88 fork
  DEPENDS:=
endef

define Package/rtw8188e-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/rtw88
	$(INSTALL_DATA) ./files/rtw8188e_fw.bin $(1)/lib/firmware/rtw88/
endef

define Package/rtw8723b-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=RealTek RTL8723B (RTL8723BS) firmware for the rtw88 fork
  DEPENDS:=
endef

define Package/rtw8723b-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/rtw88
	$(INSTALL_DATA) ./files/rtw8723b_fw.bin $(1)/lib/firmware/rtw88/
endef

NOSTDINC_FLAGS := \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/backport.h \
	-Wno-error=missing-prototypes \
	-Wno-error=missing-declarations \
	-Wno-error=empty-body

# The fork selects mac80211 API variants by LINUX_VERSION_CODE, but we build
# against mac80211 backports (6.1 API) on a 5.15 kernel. Make all the checks
# see the backports version; this also selects the right generic kernel API
# (e.g. from_timer() in tx.c) since 6.1 and 5.15 agree on those.
define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) ./files/Kbuild ./files/rtw_compat_guard.h $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR) -maxdepth 1 \( -name '*.c' -o -name '*.h' \) \
		-exec $(SED) 's/\<LINUX_VERSION_CODE\>/RTW_KERNEL_VERSION_CODE/g' {} +
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

$(eval $(call KernelPackage,rtw88-ol))
$(eval $(call KernelPackage,rtw88-8189es))
$(eval $(call KernelPackage,rtw88-8723bs))
$(eval $(call BuildPackage,rtw8188e-firmware))
$(eval $(call BuildPackage,rtw8723b-firmware))
