-- Copyright 2020 Ivan Belokobylskiy <belokobylskij@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.jnflasher", package.seeall)

function index()
	entry({"admin", "system", "jnflasher"}, template("jnflasher"), _("Zigbee Firmware"), 30)
	entry({"admin", "system", "jnflasher", "exec"}, post("action_exec")).leaf = true
end

function action_exec(command)
        local util = require "luci.util"
	local sys = require "luci.sys"
	local cmd = { "/root/flash.sh" }
	local firmware = luci.http.formvalue("firmware")

	if command == 'flash-url' then
	    sys.httpget(firmware, false, "/tmp/firmware.bin")
	    firmware = "/tmp/firmware.bin"
	end

	if firmware then
		cmd[#cmd + 1] = firmware
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json(sys.process.exec(cmd, true, true))
end
