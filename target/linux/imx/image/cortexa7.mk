define Device/Default
  PROFILES := Default
  FILESYSTEMS := squashfs ext4
  KERNEL_INSTALL := 1
  KERNEL_SUFFIX := -uImage
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | uImage none
  KERNEL_LOADADDR := 0x80008000
  IMAGES :=
endef

define Device/xiaomi_gateway-lumi
  DEVICE_PACKAGES := \
	kmod-button-hotplug kmod-input-gpio-keys \
	kmod-ledtrig-activity kmod-ledtrig-oneshot \
	kmod-ledtrig-transient kmod-ledtrig-gpio \
	kmod-i2c-core kmod-iio-core kmod-iio-vf610 \
	kmod-sound-core kmod-sound-soc-imx \
	kmod-sound-soc-tfa9882 alsa-utils \
	wpa-supplicant ca-certificates hostapd \
	nand-utils kobs-ng

  KERNEL_INSTALL := 0
  KERNEL_NAME := zImage
  KERNEL_SUFFIX := -zImage
  KERNEL := kernel-bin
  IMAGES := sysupgrade.bin dtb rootfs.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/rootfs.bin := append-rootfs
  IMAGE/dtb := install-dtb
endef

define Device/xiaomi_dgnwg05lm
  $(Device/xiaomi_gateway-lumi)
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := DGNWG05LM
  DEVICE_PACKAGES += kmod-rtl8723bs-ol \
	kmod-bluetooth bluez-daemon bluez-libs bluez-utils
  DEVICE_DTS := \
	imx6ull-xiaomi-dgnwg05lm
endef
TARGET_DEVICES += xiaomi_dgnwg05lm
