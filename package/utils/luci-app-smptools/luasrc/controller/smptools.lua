-- Copyright 2020 Ivan Belokobylskiy <belokobylskij@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.smptools", package.seeall)

function index()
	entry({"admin", "system", "smptools"}, template("smptools"), _("Zigbee Tools"), 65).acl_depends={ "luci-mod-system-config" }
	entry({"admin", "system", "smptools", "flash"}, post("action_flash")).leaf = true
	entry({"admin", "system", "smptools", "exec"}, post("action_exec")).leaf = true
end

function action_flash(command)
	local sys = require "luci.sys"
	local cmd = { "/bin/sh" }
    cmd[#cmd + 1] = "/usr/bin/smpflash"
	local firmware = luci.http.formvalue("firmware")
	local baudrate = luci.http.formvalue("baudrate")
	if not baudrate then
		baudrate = "1000000"
	end

	if command == 'do-flash-url' then
	    sys.httpget(firmware, false, "/tmp/firmware.bin")
	    firmware = "/tmp/firmware.bin"
	end

	if firmware then
		cmd[#cmd + 1] = firmware
	end

	luci.http.prepare_content("application/json")
	nixio.setenv("BAUDRATE", baudrate)
	luci.http.write_json(sys.process.exec(cmd, true, true))
end
