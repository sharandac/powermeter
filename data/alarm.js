var pause_osc = true;
var connect = false;
var timeout = 3;
var savecounter = 0;
var connection;

getconnect();
setInterval(function () {getStatus();}, 1000);

function getconnect() {
    connection = new WebSocket('ws://' + location.hostname + '/ws', ['arduino']);

    connection.onopen = function () {
        connect = true; 	
        sendCMD("STA");

        if( document.getElementById( 'OScope' ) )
            sendCMD( "get_channel_list" );

        if( document.getElementById( 'measurement_settings' ) ) {
            sendCMD( "get_measurement_settings" );
            sendCMD( "get_channel_config" );
        }

        if( document.getElementById( 'wlan_settings' ) )
            sendCMD( "get_wlan_settings" );

        if( document.getElementById( 'group_settings' ) )
            sendCMD( "get_group_settings" );

        if( document.getElementById( 'mqtt_settings' ) )
            sendCMD( "get_mqtt_settings" );

        if( document.getElementById( 'hostname_settings' ) )
            sendCMD( "get_hostname_settings" );

        if( document.getElementById( 'display_settings' ) )
            sendCMD( "get_display_settings" );

        if( document.getElementById( 'ioport_settings' ) )
            sendCMD( "get_ioport_settings" );
    };

    connection.onmessage = function (e) {
        console.log('Server: ', e.data);
        partsarry = e.data.split('\\');
    
        if( partsarry[0] == 'OScopeProbe' ) {
            GotOScope(e.data );
            return;
        }
        
        if( partsarry[0] == 'status' ) {
            if ( partsarry[1] == 'Save' ) {
                document.getElementById('fixedfooter').style.background = "#008000";
                savecounter = 2;
            }
            else {
                document.getElementById('status').firstChild.nodeValue = partsarry[1];
            }
    
            timeout = 4;
            return;
        }
  
        if(partsarry[0] == 'get_channel_list' ) {
            if ( document.getElementById( partsarry[1] ) ) {
                const label = document.getElementById( partsarry[1] );
                label.textContent = partsarry[2];
            }
            return;
        }
  
        if(partsarry[0] == 'get_channel_use_list') {
            if ( document.getElementById( partsarry[1] ) ) {
                if ( partsarry[2] == 'true' )
                    document.getElementById( partsarry[1] ).checked = true;
                else
                    document.getElementById( partsarry[1] ).checked = false;
            }
            if ( document.getElementById( partsarry[3] ) ) {
                document.getElementById( partsarry[3] ).options[ partsarry[4] ].text = partsarry[5];
            }
            return;
        }
  	
        if(partsarry[0] == 'get_group_use_list') {
            if ( document.getElementById( partsarry[1] ) && partsarry[2] == 'true' ) {
                document.getElementById( partsarry[1] ).checked = true;
            }
            if ( document.getElementById( partsarry[3] ) ) {
                const label = document.getElementById( partsarry[3] );
                label.textContent = partsarry[4];
            }    
            return;
        }
  
        if( partsarry[0] == 'option') {
            if ( document.getElementById( partsarry[1] ).options[ partsarry[2] ] ) {
                document.getElementById( partsarry[1] ).options[ partsarry[2] ].text = partsarry[3];
                refreshOpcode('channel_opcodeseq_str');
            }
            return;
        }
  	
        if( partsarry[0] == 'label') {
            if ( document.getElementById( partsarry[1] ) ) {
                document.getElementById( partsarry[1] ).textContent = partsarry[2];
            }
            return;
        }
    
        if( partsarry[0] == 'checkbox') {
            if ( document.getElementById( partsarry[1] ) ) {
                if( partsarry[2] == 'true' )
                    document.getElementById( partsarry[1] ).checked = true;
                else
                    document.getElementById( partsarry[1] ).checked = false;
            }
            return;
        }

        if ( document.getElementById( partsarry[0] ) ) {
            document.getElementById( partsarry[0] ).value = partsarry[1];
            if( document.getElementById( 'channel_opcodeseq_str' ) )
                refreshOpcode('channel_opcodeseq_str');
        }
    }

    connection.onclose = function(e) {
        console.log('Socket is closed. Reconnect will be attempted in 1 second.', e.reason);
        connect = false;
        setTimeout(function() { getconnect(); }, 1000);
    };

    connection.onerror = function (error) {
        connect = false; 
        console.log('WebSocket Error ', error);
        connection.close();
    };
}

function setSensor( sensor ) {
    console.log('Server: ', sensor );
	partsarry = sensor.split('\\');
	
	console.log('Sensor ratio: ', partsarry[ 0 ] );
	console.log('Sensor phaseshift: ', partsarry[ 1 ] );
	console.log('Sensor offset: ', partsarry[ 2 ] );
	
	if( document.getElementById( 'channel_ratio' ) )
	    document.getElementById( 'channel_ratio' ).value = partsarry[ 0 ];
	if( document.getElementById( 'channel_phaseshift' ) )
	    document.getElementById( 'channel_phaseshift' ).value = partsarry[ 1 ];
	if( document.getElementById( 'channel_offset' ) )
	    document.getElementById( 'channel_offset' ).value = partsarry[ 2 ];
}

function sendCMD( value ) {
    console.log( "Client: " + value );
	if ( connect )
        connection.send( value );
    else
        console.log( "no connection" );
}

function check_opcode_and_options( id, option ) {
    if( document.getElementById( id ).value == '5' ) {
        document.getElementById( option ).options[ 0 ].text = "ADC1-CH5 GPIO33";
        document.getElementById( option ).options[ 1 ].text = "ADC1-CH6 GPIO34";
        document.getElementById( option ).options[ 2 ].text = "ADC1-CH7 GPIO35";
        document.getElementById( option ).options[ 3 ].text = "ADC1-CH0 GPIO36 (VP)";
        document.getElementById( option ).options[ 4 ].text = "ADC1-CH3 GPIO39 (VN)";
        document.getElementById( option ).options[ 5 ].text = "ADC1-CH4 GPIO32";
        document.getElementById( option ).options[ 6 ].text = "not available";
        document.getElementById( option ).options[ 7 ].text = "not available";
        document.getElementById( option ).options[ 8 ].text = "not available";
        document.getElementById( option ).options[ 9 ].text = "not available";
        document.getElementById( option ).options[ 10 ].text = "not available";
        document.getElementById( option ).options[ 11 ].text = "not available";
        document.getElementById( option ).options[ 12 ].text = "not available";
    }
    else if( document.getElementById( id ).value == '6' ) {
        document.getElementById( option ).options[ 0 ].text = "0";
        document.getElementById( option ).options[ 1 ].text = "1";
        document.getElementById( option ).options[ 2 ].text = "2";
        document.getElementById( option ).options[ 3 ].text = "3";
        document.getElementById( option ).options[ 4 ].text = "4";
        document.getElementById( option ).options[ 5 ].text = "5";
        document.getElementById( option ).options[ 6 ].text = "6";
        document.getElementById( option ).options[ 7 ].text = "7";
        document.getElementById( option ).options[ 8 ].text = "8";
        document.getElementById( option ).options[ 9 ].text = "9";
        document.getElementById( option ).options[ 10 ].text = "10";
        document.getElementById( option ).options[ 11 ].text = "11";
        document.getElementById( option ).options[ 12 ].text = "12";
    }
    else if( document.getElementById( id ).value == '7' ) {
        document.getElementById( option ).options[ 0 ].text = "level 0";
        document.getElementById( option ).options[ 1 ].text = "level 1";
        document.getElementById( option ).options[ 2 ].text = "level 2";
        document.getElementById( option ).options[ 3 ].text = "level 3";
        document.getElementById( option ).options[ 4 ].text = "level 4";
        document.getElementById( option ).options[ 5 ].text = "level 5";
        document.getElementById( option ).options[ 6 ].text = "not available";
        document.getElementById( option ).options[ 7 ].text = "not available";
        document.getElementById( option ).options[ 8 ].text = "not available";
        document.getElementById( option ).options[ 9 ].text = "not available";
        document.getElementById( option ).options[ 10 ].text = "not available";
        document.getElementById( option ).options[ 11 ].text = "not available";
        document.getElementById( option ).options[ 12 ].text = "not available";
    }
    else if( ( document.getElementById( id ).value >= '1' && document.getElementById( id ).value <= '3' ) || document.getElementById( id ).value == '8' || document.getElementById( id ).value == 'c' || document.getElementById( id ).value == '9'  || document.getElementById( id ).value == 'b' ) {
        for( var i = 0 ; i < 13 ; i++ )
            document.getElementById( option ).options[ i ].text = document.getElementById( "channel" ).options[ i ].text;
    }
    else if( document.getElementById( id ) ) {
        document.getElementById( option ).value = 0;
        for( var i = 0 ; i < 13 ; i++ )
            document.getElementById( option ).options[ i ].text = "-";
    }
}

function get_channel_config() {
    sendCMD("get_channel_config");
}

function get_wlan_settings() {
    sendCMD("get_wlan_settings");
}

function get_mqtt_settings() {
    sendCMD("get_mqtt_settings");
}

function get_measurement_settings() {
    sendCMD("get_measurement_settings");
}

function get_group_settings() {
    sendCMD("get_group_settings");
}

function get_hostname_settings() {
    sendCMD("get_hostname_settings");
}

function get_display_settings() {
    sendCMD("get_display_settings");
}

function get_ioport_settings() {
    sendCMD("get_ioport_settings");
}

function send_channel_groups() {
    var channel_group_str = 'channel_group\\';
    
    for( var channel = 0 ;; channel++ ) {
        if ( !document.getElementById( "channel" + channel + "_0" ) )
            break;
        
        for( var group = 0 ;; group++ ) {
            if ( document.getElementById( "channel" + channel + "_"+ group ) ) {
                if( document.getElementById( "channel" + channel + "_"+ group ).checked ) {
                    channel_group_str += group;
                    break;          
                }
            }
            else {
                break;      
            }
        }
    }
    sendCMD( channel_group_str );
}

function refreshValue() {
    sendCMD("STA");
}

function SaveSettings() {
    sendCMD("SAV");
}

function SendCheckboxSetting( value ) {
    if( document.getElementById( value ) ) {
        if( document.getElementById( value ).checked )
            sendCMD( value + "\\" + 1 );
        else
            sendCMD( value + "\\" + 0 );
    }
}

function SendSetting( value ) {
    sendCMD( value + "\\" + document.getElementById( value ).value );
}

function getStatus() {
    sendCMD( "STS" );
    savecounter--;
    timeout--;

    if ( timeout > 0 && savecounter < 0 ) {
        document.getElementById('fixedfooter').style.background = "#333333";
    }
    else if ( timeout < 0 ) {
            if ( document.getElementById( 'status' ) ) { 
                document.getElementById('fixedfooter').style.background = "#800000";
                document.getElementById( 'status' ).firstChild.nodeValue = 'offline ... wait for reconnect';
            }
    }
}

/**
 * refresh opcodes from opcode string
 * 
 * @param value opcode string id
 */
function refreshOpcode( value ) {
    if( !document.getElementById( value ) )
        return;
        
    var data = document.getElementById( value ).value;
    
    for( var i = 0;; i++ ) {
        /**
         * check if opcode id exist
         */
        if ( document.getElementById( "opcode_" + i ) )
            document.getElementById( "opcode_" + i ).value = data.substr( ( i * 2 ), 1);
        else
            break;
        /**
         * check if channel id exist
         */
        if ( document.getElementById( "channel_" + i ) )
            document.getElementById( "channel_" + i ).value = data.substr( ( i * 2 ) + 1, 1);
        else
            break;
    }
    /**
     * crawl all opcodes and channels for change channel names
     */
    for( var i = 0;; i++ ) {
        if ( document.getElementById( "opcode_" + i ) && document.getElementById( "channel_" + i ) )
            check_opcode_and_options( "opcode_" + i, "channel_" + i );
        else
            break;
    }

    if( document.getElementById( "realtime_edit" ) ) {
        if( document.getElementById( "realtime_edit" ).checked )
            SendSetting( value );
    }
    return;
}

/**
 * refresh opcode string
 * 
 * @param value opcode string id
 */
function refreshOpcodeStr( value ) {
    var opcode_str = '';
    /**
     * check if id value exist
     */
    if( !document.getElementById( value ) )
        return;
    /**
     * crawl all opcodes and channel
     */
    for( var i = 0;; i++ ) {
        /**
         * check if opcode id exist
         */
        if ( document.getElementById( "opcode_" + i ) )
            opcode_str += document.getElementById( "opcode_" + i ).value;
        else
            break;
        /**
         * check if channel id exist
         */
        if ( document.getElementById( "channel_" + i ) )
            opcode_str += document.getElementById( "channel_" + i ).value;
        else
            break;
        /**
         * check opcode and this channel option
         */
        check_opcode_and_options( "opcode_" + i, "channel_" + i );
    }
    /**
     * puch opcode string out
     */
    document.getElementById( value ).value = opcode_str;
    console.log( opcode_str );

    if( document.getElementById( "realtime_edit" ) ) {
        if( document.getElementById( "realtime_edit" ).checked )
            SendSetting( value );
    }

}

function OScopeProbe() {
    channel_list_str = "";
    
    for( var i = 0; i < 13 ; i++ ) {
        /**
         * check if channel id exist
         */
        if ( document.getElementById( "channel" + i ) ) {
        if( document.getElementById( "channel" + i ).checked )
            channel_list_str += "1";
        else
            channel_list_str += "0";
        }
    }
    
    sendCMD( "OSC\\"+ channel_list_str );
}

function GotOScope(data) {
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
	var channeltype = secs[7];

	otx.clearRect(0, 0, ocanvas.width, ocanvas.height);
	
	for( var round=0 ; round < channels ; round++ ) {
		otx.beginPath();
		
		switch( channeltype.substr( round, 1) ) {
            case '0':
            case '2':
                otx.strokeStyle = "#ff0000"
                break;
            case '1':
            case '3':
                otx.strokeStyle = "#0000ff"
                break;
            case '4':
            case '6':
                otx.strokeStyle = "#000000"
                break;
            case '5':
                otx.strokeStyle = "#00B000"
                break;
            case '7':
                otx.strokeStyle = "#a0a0a0"
                break;
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
	
	for( var round=0 ; round < channels ; round++ ) {
		ftx.beginPath();
		
		switch( channeltype.substr( round, 1) ) {
            case '0':
            case '2':
                ftx.strokeStyle = "#ff0000"
                break;
            case '1':
            case '3':
                ftx.strokeStyle = "#0000ff"
                break;
            case '4':
            case '6':
                ftx.strokeStyle = "#000000"
                break;
            case '5':
                ftx.strokeStyle = "#00B000"
                break;
            case '7':
                ftx.strokeStyle = "#a0a0a0"
                break;
		}

		var lastsamp = parseInt( fftdata.substr( fftsamps * round ,3),16 ) * mult;
		var x1 = 0;
		var y1 = 0;

		for (var i = fftsamps * round ; i < fftsamps * round + fftsamps ; i++) {
			var x2 = ((i-(fftsamps * round)) ) * fcanvas.clientWidth / ( fftsamps - 1 );
			var samp = parseInt(fftdata.substr(i * 3, 3), 16) * mult;
			var y2 = ( 1.-samp / 1024 ) * fcanvas.clientHeight - 1;
			
			ftx.moveTo( x1, y1 );
            ftx.bezierCurveTo( x2, y1, x1, y2, x2, y2 );
            x1 = x2;
            y1 = y2;
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
	ftx.strokeStyle = "#c0c0c0";
	for (i = 1; (iratio * mult * i) < ( fcanvas.clientHeight ); i++) {
		ftx.moveTo(0, fcanvas.clientHeight - (iratio * mult * i) );
		ftx.lineTo( ftx.canvas.width, fcanvas.clientHeight - (iratio * mult * i) );
	}
	ftx.stroke();
	ftx.beginPath();
	
	ftx.strokeStyle = "#000000";
	ftx.strokeRect( 0,  0, fcanvas.clientWidth, fcanvas.clientHeight );
	ftx.stroke();
	
	otx.beginPath();
	otx.strokeStyle = "#c0c0c0";

    for ( i = 1; ( iratio * mult * i ) < ( ocanvas.clientHeight / 2 ); i++ ) {
		otx.moveTo(0, ocanvas.clientHeight / 2 + (iratio * mult * i));
		otx.lineTo(otx.canvas.width, ocanvas.clientHeight / 2 + ( iratio * mult * i ) );
		otx.moveTo(0, ocanvas.clientHeight / 2 - (iratio * mult * i));
		otx.lineTo(otx.canvas.width, ocanvas.clientHeight / 2 - ( iratio * mult * i ) );
	}

	otx.stroke();
	otx.beginPath();
	
	otx.strokeStyle = "#c0c0c0";
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

    otx.beginPath();
    otx.fillStyle = "#FF0000";
    otx.fillRect(10,60, 10,10);
    otx.font = "15px Arial";
    otx.fillText("current",30,70);
    otx.stroke();

    otx.beginPath();
    otx.fillStyle = "#0000FF";
    otx.fillRect(10,80, 10,10);
    otx.font = "15px Arial";
    otx.fillText("voltage",30,90);
    otx.stroke();

    otx.beginPath();
    otx.fillStyle = "#000000";
    otx.fillRect(10,100, 10,10);
    otx.font = "15px Arial";
    otx.fillText("power",30,110);
    otx.stroke();

    otx.beginPath();
    otx.fillStyle = "#00B000";
    otx.fillRect(10,120, 10,10);
    otx.font = "15px Arial";
    otx.fillText("reactive power",30,130);
    otx.stroke();

    otx.beginPath();
	otx.fillStyle = "#000000";
    otx.font = "30px Arial";
    otx.fillText("Oscilloscope",10,30);
    otx.font = "15px Arial";
    otx.fillText("div: 5 or 4.17ms",10,50);
    otx.stroke();
  
	if (!pause_osc)
		OScopeProbe();
}

function ToggleOScopePause() {
	pause_osc = !pause_osc;
}

function PhaseshiftPlus() {
    sendCMD("PS+");
}

function PhaseshiftMinus() {
    sendCMD("PS-");
}

function SampleratePlus() {
    sendCMD( "FQ+" );	
}

function SamplerateMinus() {
    sendCMD( "FQ-" );		
}

