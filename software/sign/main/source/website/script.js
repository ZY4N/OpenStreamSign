function encodeString(str, dst) {
	for (let i = 0; i < str.length; i++) {
		const code = str.charCodeAt(i);
		if (code < 32 || code >= 127) {
			return false;
		}
		dst.push(code);
	}
	return true;
}

function encodeUnsignedShort(str, dst) {
	const num = parseInt(str);
	if (isNaN(num) || num < 0 || num > 65535)
		return false;
	const buffer = new ArrayBuffer(2);
	const integers = new Uint16Array(buffer);
	integers[0] = num;
	const bytes = new Uint8Array(buffer);
	dst.concat(Array.from(bytes));
	return true;
}

function encodeIPv4(str, dst) {
	const pattern = '^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))$';
	if (str.match(pattern) == null)
		return false; 
	str.split('.').forEach(num =>
		dst.push(parseInt(num))
	);
	return true;
}

window.addEventListener('load', () => {
	const andChar = 254;
	const equalChar = 255;

	const encoders = {
		txt: encodeString,
		ipv4: encodeIPv4,
		u16: encodeUnsignedShort,
		ign: (src, dst) => true
	};

	for (const form of document.forms) {

		const secretField = form.elements.namedItem('secret');
		const copyButton = form.elements.namedItem('copy');
		if (secretField && copyButton) {
			copyButton.addEventListener('click', () => {
				secretField.select();
				secretField.setSelectionRange(0, 99999);
				navigator.clipboard.writeText(secretField.value);
				alert("Copied secret to clipboard!");
			});
		}

		form.addEventListener('submit', (event) => {
			event.preventDefault();
			let buffer = [];
			try {
				for (const input of form.elements) {
					const encoder = encoders[input.dataset.t];
					if (!encoder)
						continue;
					if (!encodeString(input.name, buffer))
						throw `Cannot encode name '${input.name}' of field '${input.name}'`;
					buffer.push(equalChar);
					if (!encoder(input.value, buffer))
						throw `Invalid value '${input.value}' in field '${input.name}'`;
					buffer.push(andChar);
				};
			} catch (e) {
				alert(e);
				return;
			}

			buffer.pop();

			const xhr = new XMLHttpRequest();
			xhr.open('POST', (form.action && form.action.length) ? form.action : window.location.href);
			xhr.setRequestHeader('Content-Type', 'application/octet-stream');
			xhr.onload = () => {
				if (xhr.readyState === xhr.DONE) {
					const redirectTo = form.getAttribute("redirect");
					if (xhr.status !== 200) {
						alert('The Backend died, try again!');
					} else if (redirectTo && redirectTo.length) {
						window.location.href = redirectTo;
					}
				}
			};
			xhr.send(new Uint8Array(buffer));
		});
	}
});
