var pause_osc = true;
var connect = false;
var counter = 10;
var savecounter = 0;

var connection = new WebSocket('ws://' + location.hostname + '/ws', ['arduino']);

setInterval(function () { if ( connect == true ) getStatus();}, 1000);

connection.onopen = function () {
	connect = true; 	
//	console.log('STA');
	connection.send('STA');
};

connection.onerror = function (error) {
	connect = false; 
//	console.log('WebSocket Error ', error);
	connection = new WebSocket('ws://' + location.hostname + '/ws', ['arduino']); 
};

connection.onmessage = function (e) {
//	console.log('Server: ', e.data);
	partsarry = e.data.split('\\');
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
//	console.log( "send: SAV" );
	if ( connect ) connection.send( "SAV" );
//	console.log('send: STA');
	if ( connect ) connection.send('STA');
}

function getStatus() {
//	console.log('send: STS');
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
//	console.log( "send: " + value  + "\\" + document.getElementById( value ).value );
	if ( connect ) connection.send( value + "\\" + document.getElementById( value ).value );
}

function OScopeProbe() {
//	console.log( "send: OSC" );
	if ( connect ) connection.send( "OSC" );
}

function GotOScope(data)
{
	var mult = Number(document.getElementById('OSCMultIn').value);
	document.getElementById('OSCMultOut').innerHTML = mult;

	var ocanvas = document.getElementById('OScopeCanvas');
	var otx = ocanvas.getContext('2d');
	var fcanvas = document.getElementById('FFTCanvas');
	var ftx = fcanvas.getContext('2d');

	if( otx.canvas.width != ocanvas.clientWidth )   otx.canvas.width = ocanvas.clientWidth;
	if( otx.canvas.height != ocanvas.clientHeight ) otx.canvas.height = ocanvas.clientHeight;

	if( ftx.canvas.width != fcanvas.clientWidth )   ftx.canvas.width = fcanvas.clientWidth;
	if( ftx.canvas.height != fcanvas.clientHeight ) ftx.canvas.height = fcanvas.clientHeight;

	var secs = data.split( "\\" );

	var channels = Number( secs[1] );
	var samps = Number( secs[2] );
	var fftsamps = secs[3];
	var iratio = ocanvas.clientHeight / ( secs[4]*4096 ) * 10;
	var data = secs[5];
	var fftdata = secs[6];

	otx.clearRect(0, 0, ocanvas.width, ocanvas.height);
	
	for( var round=0 ; round < channels ; round++ )
	{
		if ( round == 0 && !document.getElementById("channel0").checked ) continue;
		if ( round == 1 && !document.getElementById("channel1").checked ) continue;
		if ( round == 2 && !document.getElementById("channel2").checked ) continue;
		if ( round == 3 && !document.getElementById("channelv").checked ) continue;

		otx.beginPath();
		if ( round == 0 ) {
			otx.strokeStyle = "#0000FF";
		}
		if ( round == 1 ) {
			otx.strokeStyle = "#00FF00";
		}
		if ( round == 2 ) {
			otx.strokeStyle = "#00FFFF";
		}
		if ( round == 3 ) {
			otx.strokeStyle = "#FF0000";
		}

		var lastsamp = parseInt( data.substr( samps * round ,3),16 );

		for (var i = samps * round ; i < samps * round + samps ; i++)
		{
			var x2 = ((i-(samps * round)) ) * ocanvas.clientWidth / ( samps - 1 );
			var samp = parseInt(data.substr(i * 3, 3), 16);
			var y2 = ( 1.-mult*samp / 4096 ) * ocanvas.clientHeight ;
			
			if( i == 0 )
			{
				var x1 = i * ocanvas.clientWidth / samps;
				var y1 = ( 1.-mult*lastsamp / 4096 ) * ocanvas.clientHeight ;
				otx.moveTo( x1, y1 + ( ( mult * ocanvas.clientHeight ) / 2 ) - ( ocanvas.clientHeight / 2 ) );
			}

			otx.lineTo( x2, y2  + ( ( mult * ocanvas.clientHeight ) / 2 ) - ( ocanvas.clientHeight / 2 ) );

			lastsamp = samp;
		}
		otx.stroke();
	}

	ftx.clearRect(0, 0, fcanvas.width, fcanvas.height);
	
	for( var round=0 ; round < channels ; round++ )
	{
		if ( round == 0 && !document.getElementById("channel0").checked ) continue;
		if ( round == 1 && !document.getElementById("channel1").checked ) continue;
		if ( round == 2 && !document.getElementById("channel2").checked ) continue;
		if ( round == 3 && !document.getElementById("channelv").checked ) continue;

		ftx.beginPath();
		if ( round == 0 ) {
			ftx.strokeStyle = "#0000FF";
		}
		if ( round == 1 ) {
			ftx.strokeStyle = "#00FF00";
		}
		if ( round == 2 ) {
			ftx.strokeStyle = "#00FFFF";
		}
		if ( round == 3 ) {
			ftx.strokeStyle = "#FF0000";
		}

		var lastsamp = parseInt( fftdata.substr( fftsamps * round ,3),16 ) * mult;

		var x1 = 0;
		var y1 = 0;

		for (var i = fftsamps * round ; i < fftsamps * round + fftsamps ; i++)
		{
			var x2 = ((i-(fftsamps * round)) ) * fcanvas.clientWidth / ( fftsamps - 1 );
			var samp = parseInt(fftdata.substr(i * 3, 3), 16) * mult;
			var y2 = ( 1.-samp / 1024 ) * fcanvas.clientHeight - 1;
			
			ftx.moveTo( x1, y1 );
      ftx.bezierCurveTo( x2, y1, x1, y2, x2, y2 );
      x1 = x2;
      y1 = y2;
//			ftx.lineTo( x2, y2 );

			lastsamp = samp;
		}
		ftx.stroke();
	}
	
	ftx.beginPath();
	ftx.strokeStyle = "#000000";
  ftx.font = "30px Arial";
  ftx.fillText("Spectrum",10,30);
  ftx.stroke();

	ftx.beginPath();
	ftx.strokeStyle = "#FF8080";
	for (i = 1; (iratio * mult * i) < ( fcanvas.clientHeight ); i++)
	{
		ftx.moveTo(0, fcanvas.clientHeight - (iratio * mult * i) );
		ftx.lineTo( ftx.canvas.width, fcanvas.clientHeight - (iratio * mult * i) );
	}
	ftx.stroke();
	ftx.beginPath();
	
	ftx.strokeStyle = "#000000";
	ftx.strokeRect( 0,  0, fcanvas.clientWidth, fcanvas.clientHeight );
	ftx.stroke();
	
	otx.beginPath();
	otx.strokeStyle = "#FF8080";

	for (i = 1; (iratio * mult * i) < ( ocanvas.clientHeight / 2); i++)
	{
		otx.moveTo(0, ocanvas.clientHeight / 2 + (iratio * mult * i));
		otx.lineTo(otx.canvas.width, ocanvas.clientHeight / 2 + (iratio * mult * i));
		otx.moveTo(0, ocanvas.clientHeight / 2 - (iratio * mult * i));
		otx.lineTo(otx.canvas.width, ocanvas.clientHeight / 2 - (iratio * mult * i));
	}

	otx.stroke();
	otx.beginPath();
	
	otx.strokeStyle = "#FF0000";
	otx.moveTo(0, ocanvas.clientHeight / 2);
	otx.lineTo(ocanvas.clientWidth, ocanvas.clientHeight / 2);

	otx.moveTo(1, ocanvas.clientHeight / 2 - 20 );
	otx.lineTo(1, ocanvas.clientHeight / 2 + 20);

	otx.moveTo(otx.canvas.width / 8, ocanvas.clientHeight / 2 - 10);
	otx.lineTo(otx.canvas.width / 8 , ocanvas.clientHeight / 2 + 10);

	otx.moveTo(otx.canvas.width / 8 * 2, ocanvas.clientHeight / 2 - 10 );
	otx.lineTo(otx.canvas.width / 8 * 2, ocanvas.clientHeight / 2 + 10);

	otx.moveTo(otx.canvas.width / 8 * 3, ocanvas.clientHeight / 2 - 10 );
	otx.lineTo(otx.canvas.width / 8 * 3, ocanvas.clientHeight / 2 + 10);

	otx.moveTo(otx.canvas.width / 2, ocanvas.clientHeight / 2 - 20);
	otx.lineTo(otx.canvas.width / 2 , ocanvas.clientHeight / 2 + 20);

	otx.moveTo(otx.canvas.width / 8 * 5, ocanvas.clientHeight / 2 - 10 );
	otx.lineTo(otx.canvas.width / 8 * 5, ocanvas.clientHeight / 2 + 10);

	otx.moveTo(otx.canvas.width / 8 * 6, ocanvas.clientHeight / 2 - 10);
	otx.lineTo(otx.canvas.width / 8 * 6, ocanvas.clientHeight / 2 + 10);

	otx.moveTo(otx.canvas.width / 8 * 7, ocanvas.clientHeight / 2 - 10);
	otx.lineTo(otx.canvas.width / 8 * 7, ocanvas.clientHeight / 2 + 10);

	otx.moveTo(otx.canvas.width - 1, ocanvas.clientHeight / 2 - 20 );
	otx.lineTo(otx.canvas.width - 1, ocanvas.clientHeight / 2 + 20);

	otx.strokeStyle = "#000000";
	otx.strokeRect( 0,  0, ocanvas.clientWidth, ocanvas.clientHeight );

	otx.stroke();

	var samp = parseInt(data.substr(i * 3, 3), 16) - 2048;

  otx.beginPath();
	ftx.strokeStyle = "#000000";
  otx.font = "30px Arial";
  otx.fillText("Oscilloscope",10,30);
  otx.font = "15px Arial";
  otx.fillText("div: 5ms/10A",10,50);
  otx.stroke();

	if (!pause_osc)
		OScopeProbe();
}

function ToggleOScopePause()
{
	pause_osc = !pause_osc;
}

function SampleratePlus()
{
	if ( connect ) connection.send( "FQ+" );	
}

function SamplerateMinus()
{
	if ( connect ) connection.send( "FQ-" );		
}

