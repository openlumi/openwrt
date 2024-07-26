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

	[ -b "$rootfs" ] || {
		echo "rootfs device ${rootfs} is not a block device"
		return 1
	}

	mkdir -p /boot

	if [ $board = "geniatech,gtw360" ]; then
		# mmcblk1p1 (fat) contains all FIT images for the Gateway
		mount -t vfat /dev/mmcblk1p1 /boot || {
			echo "failed to mount /dev/mmcblk1p1"
			return 1
		}

		echo "flashing Kernel to /boot/$kernel (/dev/mmblk1p1)"
		tar -Oxf $tar_file "$board_dir/kernel" > /boot/$kernel || {
			echo "failed to write kernel to /boot/$kernel"
			umount /boot
			return 1
		}
	fi

	echo "flashing rootfs to ${rootfs}"
	local blocks=$(tar xf $tar_file $board_dir/root -O | dd of="${rootfs}" bs=512 2>&1 | grep "records out" | cut -d' ' -f1)
	blocks=$((${blocks:-0}))
	[ "$blocks" -gt 0 ] || {
		echo "failed to write rootfs to ${rootfs}"
		umount /boot
		return 1
	}

	# Invalidate the old f2fs overlay that follows the squashfs.
	# Account for the 64KiB overlay alignment in libfstools.
	blocks=$(((blocks + 127) & ~127))
	dd if=/dev/zero of="${rootfs}" bs=512 seek=$blocks count=8

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
