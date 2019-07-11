var connect = false;
var firstrun = false;
var counter = 10;
var savecounter = 0;

var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

setInterval(function () { if ( connect == true ) getStatus();}, 1000);

connection.onopen = function () {
	connect = true; 	
	console.log('STA');
	connection.send('STA');
};

connection.onerror = function (error) {
	connect = false; 
	console.log('WebSocket Error ', error);
	connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']); 
};

connection.onmessage = function (e) {
	console.log('Server: ', e.data);
	partsarry = e.data.split('\\');
	if (firstrun == false) { firstrun = true; }
	if (partsarry[0] == 'OScopeProbe') {
		GotOScope(e.data );
	}
	else if (partsarry[0] == 'status') {
		if ( partsarry[1] == 'Save' ) {
			document.getElementById('fixedfooter').style.background = "#008000";
			document.getElementById('status').firstChild.nodeValue = "Einstellungen gespeichert";
			savecounter = 2;
		}
		else {
			if ( savecounter < 0 ) document.getElementById('status').firstChild.nodeValue = partsarry[1];
			counter = 5;
		}
	}
	else {
		if ( document.getElementById( partsarry[0] ) ) { document.getElementById( partsarry[0] ).value = partsarry[1]; }
	}
}

function refreshValue() {
	console.log( "send: SAV" );
	if ( connect ) connection.send( "SAV" );
	console.log('send: STA');
	if ( connect ) connection.send('STA');
}

function getStatus() {
	console.log('send: STS');
	if ( connect ) connection.send('STS');
	counter = counter - 1;
	savecounter = savecounter -1;
	if ( counter < 1 && savecounter < 0 ) {
		if ( document.getElementById( 'status' ) ) { 
			document.getElementById('fixedfooter').style.background = "#800000";
			document.getElementById( 'status' ).firstChild.nodeValue = 'offline';
		}
	}
	if ( savecounter < 0 ) {
		document.getElementById('fixedfooter').style.background = "#333333";
	}
}

function SaveSetting( value ) {
	console.log( "send: " + value  + "\\" + document.getElementById( value ).value );
	if ( connect ) connection.send( value + "\\" + document.getElementById( value ).value );
}

function OScopeProbe() {
	console.log( "send: OSC" );
	if ( connect ) connection.send( "OSC" );
}

var pause_osc = true;

function GotOScope(data)
{
	var mult = Number(document.getElementById('OSCMultIn').value);
	document.getElementById('OSCMultOut').innerHTML = mult;
	var canvas = document.getElementById('OScopeCanvas');
	var ctx = canvas.getContext('2d');
	var h = canvas.height;
	var w = canvas.width;
	if( ctx.canvas.width != canvas.clientWidth )   ctx.canvas.width = canvas.clientWidth;
	if( ctx.canvas.height != canvas.clientHeight ) ctx.canvas.height = canvas.clientHeight;

	var secs = data.split( "\\" );

	var samps = Number( secs[1] );
	var iratio = 512/(secs[2]*4096) * 10;
	var data = secs[3];
	var lastsamp = parseInt( data.substr(0,4),16 );

	ctx.clearRect(0, 0, canvas.width, canvas.height);
	ctx.beginPath();
	ctx.strokeStyle = "#000000";
	ctx.strokeRect( 0,  0, canvas.clientWidth, canvas.clientHeight );
	
	for (var i = 0; i < samps; i++)
	{
		var x2 = (i+1) * canvas.clientWidth / samps;
		var samp = parseInt(data.substr(i * 4, 4), 16);
		var y2 = ( 1.-mult*samp / 4096 ) * canvas.clientHeight ;
		
		if( i == 0 )
		{
			var x1 = i * canvas.clientWidth / samps;
			var y1 = ( 1.-mult*lastsamp / 4096 ) * canvas.clientHeight ;
			ctx.moveTo( x1, y1 + ( ( mult * canvas.clientHeight ) / 2 ) - ( canvas.clientHeight / 2 ) );
		}

		ctx.lineTo( x2, y2  + ( ( mult * canvas.clientHeight ) / 2 ) - ( canvas.clientHeight / 2 ) );

		lastsamp = samp;
	}

	ctx.stroke();
	ctx.beginPath();

	ctx.strokeStyle = "#FF8080";

	for (i = 1; (iratio * mult * i) < (canvas.clientHeight / 2); i++)
	{
		ctx.moveTo(0, canvas.clientHeight / 2 + (iratio * mult * i));
		ctx.lineTo(ctx.canvas.width, canvas.clientHeight / 2 + (iratio * mult * i));
		ctx.moveTo(0, canvas.clientHeight / 2 - (iratio * mult * i));
		ctx.lineTo(ctx.canvas.width, canvas.clientHeight / 2 - (iratio * mult * i));
	}

	ctx.stroke();
	ctx.beginPath();
	
	ctx.strokeStyle = "#FF0000";
	ctx.moveTo(0, canvas.clientHeight / 2);
	ctx.lineTo(canvas.clientWidth, canvas.clientHeight / 2);

	ctx.moveTo(1, canvas.clientHeight / 2 - 20 );
	ctx.lineTo(1, canvas.clientHeight / 2 + 20);

	ctx.moveTo(ctx.canvas.width / 8, canvas.clientHeight / 2 - 10);
	ctx.lineTo(ctx.canvas.width / 8 , canvas.clientHeight / 2 + 10);

	ctx.moveTo(ctx.canvas.width / 8 * 2, canvas.clientHeight / 2 - 10 );
	ctx.lineTo(ctx.canvas.width / 8 * 2, canvas.clientHeight / 2 + 10);

	ctx.moveTo(ctx.canvas.width / 8 * 3, canvas.clientHeight / 2 - 10 );
	ctx.lineTo(ctx.canvas.width / 8 * 3, canvas.clientHeight / 2 + 10);

	ctx.moveTo(ctx.canvas.width / 2, canvas.clientHeight / 2 - 20);
	ctx.lineTo(ctx.canvas.width / 2 , canvas.clientHeight / 2 + 20);

	ctx.moveTo(ctx.canvas.width / 8 * 5, canvas.clientHeight / 2 - 10 );
	ctx.lineTo(ctx.canvas.width / 8 * 5, canvas.clientHeight / 2 + 10);

	ctx.moveTo(ctx.canvas.width / 8 * 6, canvas.clientHeight / 2 - 10);
	ctx.lineTo(ctx.canvas.width / 8 * 6, canvas.clientHeight / 2 + 10);

	ctx.moveTo(ctx.canvas.width / 8 * 7, canvas.clientHeight / 2 - 10);
	ctx.lineTo(ctx.canvas.width / 8 * 7, canvas.clientHeight / 2 + 10);

	ctx.moveTo(ctx.canvas.width - 1, canvas.clientHeight / 2 - 20 );
	ctx.lineTo(ctx.canvas.width - 1, canvas.clientHeight / 2 + 20);

	ctx.stroke();

	var samp = parseInt(data.substr(i * 4, 4), 16) - 2048;

	if (!pause_osc)
		OScopeProbe();
}

function ToggleOScopePause()
{
	pause_osc = !pause_osc;
}