-- Copyright 2020 Ivan Belokobylskiy <belokobylskij@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.jntools", package.seeall)

function index()
	entry({"admin", "system", "jntools"}, template("jntools"), _("Zigbee Tools"), 30)
	entry({"admin", "system", "jntools", "flash"}, post("action_flash")).leaf = true
	entry({"admin", "system", "jntools", "exec"}, post("action_exec")).leaf = true
end

function action_flash(command)
	local sys = require "luci.sys"
	local cmd = { "/usr/bin/jnflash" }
	local firmware = luci.http.formvalue("firmware")

	if command == 'do-flash-url' then
	    sys.httpget(firmware, false, "/tmp/firmware.bin")
	    firmware = "/tmp/firmware.bin"
	end

	if firmware then
		cmd[#cmd + 1] = firmware
		cmd[#cmd + 1] = "-q"
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json(sys.process.exec(cmd, true, true))
end

function action_exec(command)
	local sys = require "luci.sys"
	local nixio  = require "nixio"
	local cmd = { "/usr/bin/jntool" }
	local baudrate = luci.http.formvalue("baudrate")
	if not baudrate then
	    baudrate = "115200"
        end

	cmd[#cmd + 1] = command

	luci.http.prepare_content("application/json")
	nixio.setenv("BAUDRATE", baudrate)
	luci.http.write_json(sys.process.exec(cmd, true, true))
end
