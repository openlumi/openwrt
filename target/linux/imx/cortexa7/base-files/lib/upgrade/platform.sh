PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	xiaomi,dgnwg05lm )
		local platform_dir_name=$(echo $board | sed 's/,/_/g')
		nand_do_platform_check $platform_dir_name $1
		return $?;
		;;
	esac
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	xiaomi,dgnwg05lm )
		nand_do_upgrade "$1"
		;;
  *)
		default_do_upgrade "$1"
		;;
	esac
}
