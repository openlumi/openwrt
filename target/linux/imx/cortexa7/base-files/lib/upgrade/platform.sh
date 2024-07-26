. /lib/imx.sh

RAMFS_COPY_BIN='blkid jffs2reset'

enable_image_metadata_check() {
	case "$(board_name)" in
	technexion,imx7d-pico-pi)
		REQUIRE_IMAGE_METADATA=1
		;;
	esac
}
enable_image_metadata_check

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	aqara,zhwg11lm |\
	xiaomi,dgnwg05lm )
		local platform_dir_name=$(echo $board | sed 's/,/_/g')
		nand_do_platform_check $platform_dir_name $1
		return $?;
		;;
	geniatech,gtw360 )
		local platform_dir_name=$(echo $board | sed 's/,/_/g')
		nand_do_platform_check $platform_dir_name $1
		return $?;
		;;
	technexion,imx7d-pico-pi)
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	aqara,zhwg11lm |\
	xiaomi,dgnwg05lm )
		nand_do_upgrade "$1"
		;;
	geniatech,gtw360 )
	  platform_do_flash_fit "$1" "$board" "fitImageA" "$(platform_get_rootfs)"
		;;
	technexion,imx7d-pico-pi)
		imx_sdcard_do_upgrade "$1"
		;;
	esac
}

platform_get_rootfs() {
	local rootfsdev

	if read cmdline < /proc/cmdline; then
		case "$cmdline" in
			*root=*)
				rootfsdev="${cmdline##*root=}"
				rootfsdev="${rootfsdev%% *}"
			;;
		esac

		echo "${rootfsdev}"
	fi
}

platform_do_flash_fit() {
	local tar_file=$1
	local board=$2
	local kernel=$3
	local rootfs=$4

	local board_dir=$(tar tf "$tar_file" | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}
	[ -n "$board_dir" ] || return 1

	mkdir -p /boot

	if [ $board = "geniatech,gtw360" ]; then
		# mmcblk1p1 (fat) contains all FIT images for the Gateway
		mount -t vfat /dev/mmcblk1p1 /boot

		echo "flashing Kernel to /boot/$kernel (/dev/mmblk1p1)"
		tar -Oxf $tar_file "$board_dir/kernel" > /boot/$kernel
	fi

	echo "flashing rootfs to ${rootfs}"
	tar xf $tar_file $board_dir/root -O | dd of="${rootfs}" bs=4096

	sync
	umount /boot
}

platform_copy_config() {
	local board=$(board_name)

	case "$board" in
	technexion,imx7d-pico-pi)
		imx_sdcard_copy_config
		;;
	esac
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	technexion,imx7d-pico-pi)
		imx_sdcard_pre_upgrade
		;;
	esac
}
