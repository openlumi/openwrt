
function handleFlashUrl(ev)
{
	var name_or_url = document.querySelector('input[name="install"]').value,
	    install = E('div', {
			'class': 'btn cbi-button-action',
			'data-command': 'do-flash-url',
			'data-firmware': name_or_url,
			'click': function(ev) {
				document.querySelector('input[name="install"]').value = '';
				handleZigbeeFw(ev);
			}
		}, _('Download and flash')), warning;

	if (!name_or_url.length) {
		return;
	}

	warning = E('p', {}, _('Are you sure to install zigbee firmware from <em>%h</em>?').format(name_or_url));
	L.showModal(_('Download and write zigbee firmware'), [
		warning,
		E('div', { 'class': 'right' }, [
			E('div', {
				'click': L.hideModal,
				'class': 'btn cbi-button-neutral'
			}, _('Cancel')),
			' ', install
		])
	]);
}

function handleZigbeeFw(ev)
{
	return new Promise(function(resolveFn, rejectFn) {
		var cmd = ev.target.getAttribute('data-command'),
		    fw = ev.target.getAttribute('data-firmware'),
		    url = 'admin/system/jntools/flash/' + encodeURIComponent(cmd);

		var dlg = L.showModal(_('Executing zigbee firmware updater'), [
			E('p', { 'class': 'spinning' },
				_('Waiting for the <em>jnflash %h</em> command to complete…').format(cmd))
		]);

		L.post(url, { firmware: fw }, function(xhr, res) {
			dlg.removeChild(dlg.lastChild);

			if (res.stdout)
				dlg.appendChild(E('pre', [ res.stdout ]));

			if (res.stderr) {
				dlg.appendChild(E('h5', _('Errors')));
				dlg.appendChild(E('pre', { 'class': 'errors' }, [ res.stderr ]));
			}

			if (res.code !== 0)
				dlg.appendChild(E('p', _('The <em>jnflash %h</em> command failed with code <code>%d</code>.').format(cmd, (res.code & 0xff) || -1)));

			dlg.appendChild(E('div', { 'class': 'right' },
				E('div', {
					'class': 'btn',
					'click': L.bind(function(res) {
						if (L.ui.menu && L.ui.menu.flushCache)
							L.ui.menu.flushCache();

						L.hideModal();

						if (res.code !== 0)
							rejectFn(new Error(res.stderr || 'jnflash error %d'.format(res.code)));
						else
							resolveFn(res);
					}, this, res)
				}, _('Dismiss'))));
		});
	});
}

function handleUpload(ev)
{
	var path = '/tmp/firmware.bin';
	return L.ui.uploadFile(path).then(L.bind(function(btn, res) {
		L.showModal(_('Manually install firmware'), [
			E('p', {}, _('Installing firmwares from untrusted sources is a potential security risk! Really attempt to write <em>%h</em>?').format(res.name)),
			E('ul', {}, [
				res.size ? E('li', {}, '%s: %1024.2mB'.format(_('Size'), res.size)) : '',
				res.checksum ? E('li', {}, '%s: %s'.format(_('MD5'), res.checksum)) : '',
				res.sha256sum ? E('li', {}, '%s: %s'.format(_('SHA256'), res.sha256sum)) : ''
			]),
			E('div', { 'class': 'right' }, [
				E('div', {
					'click': function(ev) {
						L.hideModal();
						L.fs.remove(path);
					},
					'class': 'btn cbi-button-neutral'
				}, _('Cancel')), ' ',
				E('div', {
					'class': 'btn cbi-button-action',
					'data-command': 'do-flash',
					'data-firmware': path,
					'click': function(ev) {
						handleZigbeeFw(ev).finally(function() {
							L.fs.remove(path)
						});
					}
				}, _('Flash'))
			])
		]);
	}, this, ev.target));
}

function handleErasePdm(ev) {
        if (!confirm(_('Do you really want to erase all previous devices?')))
                return;
	return handleExec(ev);
}

function handleExec(ev)
{
        var cmd = ev.target.getAttribute('data-command'),
            baudrate = document.getElementById('baudrate').value;
            url = 'admin/system/jntools/exec/' + encodeURIComponent(cmd);

        var dlg = L.showModal(_('Executing Zigate command'), [
                E('p', { 'class': 'spinning' },
                        _('Waiting for the <em>jntool %h</em> command to complete…').format(cmd))
        ]);

        L.post(url, { baudrate: baudrate }, function(xhr, res) {
                dlg.removeChild(dlg.lastChild);

                if (res.stdout)
                        dlg.appendChild(E('pre', [ res.stdout ]));

                if (res.stderr) {
                        dlg.appendChild(E('h5', _('Errors')));
                        dlg.appendChild(E('pre', { 'class': 'errors' }, [ res.stderr ]));
                }

                if (res.code !== 0)
                        dlg.appendChild(E('p', _('The <em>jntool %h</em> command failed with code <code>%d</code>.').format(cmd, (res.code & 0xff) || -1)));

                dlg.appendChild(E('div', { 'class': 'right' },
                        E('div', {
                                'class': 'btn',
                                'click': L.bind(function(res) {
                                        if (L.ui.menu && L.ui.menu.flushCache)
                                                L.ui.menu.flushCache();

                                        L.hideModal();

                                        if (res.code !== 0)
                                                rejectFn(new Error(res.stderr || 'jntool error %d'.format(res.code)));
                                        else
                                                resolveFn(res);
                                }, this, res)
                        }, _('Dismiss'))));
        });
}
