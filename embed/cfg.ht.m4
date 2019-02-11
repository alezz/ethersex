<html>
<head>
<title>Ethersex - Setup</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function fillFields() {
	getCmd('version', writeVal, $('version'));
	getCmd('mac', writeVal, $('mac'));
	getCmd('dhcp', writeCheckbox, $('dhcp'));
	getCmd('ip', writeVal, $('ip'));
	getCmd('netmask', writeVal, $('netmask'));
	getCmd('gw', writeVal, $('gateway'));
	getCmd('ntp+server', writeVal, $('ntp'));

}

function getCmd(cmd, handler, data) {
	ArrAjax.ecmd(cmd, handler, 'GET', data);
}

function setCmd(cmd, value) {
	ArrAjax.ecmd(cmd + ' ' + value);
}

function writeVal(request, data) {
	data.value = request.responseText;
}

function writeCheckbox(request, data) {
	data.checked = (request.responseText == '0\n') ? false : true;
}
	
function changeState(request, data) {
	data.style.backgroundColor = (request.responseText == "OK\n") ? "green" : "red";
}

function updateValues() {
	$('valdiv').style.visibility = "hidden";
	$('waitdiv').style.visibility = "visible";
	setCmd('reset', '');
}
</script>
</head><body onLoad='fillFields()'>
<h1>Ethersex Setup</h1>
<div id="valdiv">
<center><table>
	<tr>
	<td>Version</td>
	<td><input type="text" id="version" readonly> </td>
	</tr>
	<tr>
	<td>MAC</td>
	<td><input type="text" id="mac" onChange='getCmd("mac " + this.value, changeState, this);'> </td>
	</tr>
	<tr>
	<td>DHCP</td>
	<td><input type="checkbox" id="dhcp" onChange='getCmd("dhcp " + (this.checked ? "1" : "0"), changeState, this);'></td>
	</tr>
	<tr>
	<td>IP</td>
	<td><input  type="text" id="ip" onChange='getCmd("ip " + this.value, changeState, this);' > </td>
	</tr>
	<tr>
	<td>Netmask</td>
	<td><input type="text" id="netmask" onChange='getCmd("netmask " + this.value, changeState, this);'></td>
	</tr>
	<tr>
	<td>Gateway</td>
	<td><input  type="text" id="gateway" onChange='getCmd("gw " + this.value, changeState, this);' ></td>
	</tr>
	<tr>
	<td>NTP Server</td>
	<td><input type="text" id="ntp" onChange='getCmd("ntp server " + this.value, changeState, this);'></td>
	</tr>
	<tr>
	<td></td>
	<td><input type="button" value="RESTART" onClick='updateValues();'></td>
	</tr>
</table></center>

<a href="idx.ht"> Back </a>
</div>
<div id="waitdiv" style="visibility:hidden">
<center> Please wait while Server restarts</center>
</div>
<div id="logconsole"></div>
</body>
</html>
