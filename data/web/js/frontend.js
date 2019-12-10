$(function () {
    "use strict";

    // for better performance - to avoid searching in DOM
    var content = $('#content');
    var input = $('#input');
    var status = $('#status');
	var wsUrl   = 'ws://localhost:8082' //'ws://192.168.4.1/msg'

    // my color assigned by the server
    var myColor = false;
    // my name sent to the server
    var myName  = localStorage.getItem('myName')
	var updateName = true
	
    // if user is running mozilla then use it's built-in WebSocket
    window.WebSocket = window.WebSocket || window.MozWebSocket;

    // if browser doesn't support WebSocket, just show some notification and exit
    if (!window.WebSocket) {
        content.html($('<p>', { text: 'Sorry, but your browser doesn\'t '
                                    + 'support WebSockets.'} ));
        input.hide();
        $('span').hide();
        return;
    }

    var connection = createConnection(wsUrl)
    
    function createConnection(wsUrl){
    	
    	// open connection
    	var connection = new WebSocket(wsUrl);
    
    	connection.onopen = function () {
        	updateName = true
        	// first we want users to enter their names
        	input.removeAttr('disabled');
        	status.text('Choose name:');
        	input.val(myName || '');
    	};
		/*
    	connection.onerror = function (error) {
        	// just in there were some problems with conenction...
        	content.html($('<p>', { text: 'Sorry, but there\'s some problem with your '
                                    	+ 'connection or the server is down.' } ));
    	};
		*/
    	// most important part - incoming messages
    	connection.onmessage = function (message) {
        	input.removeAttr('disabled');
        	message = message.data
        	var chunks = message.split('|') 
        	addMessage(chunks[0], chunks[1],'red', new Date());
    	}
    	
    	return connection
    }
	
    /**
     * Send mesage when user presses Enter key
     */
    input.keydown(function(e) {
        if (e.keyCode === 13) {
            var msg = $(this).val();
            if (!msg) {
                return;
            }
             // we know that the first message sent from a user their name
            if (updateName) {
                myName = msg;
                $(this).val('');
                status.text('message:');
                localStorage.setItem('myName',myName)
            	updateName = false
            }else{
                connection.send(myName + '|' + msg);
                $(this).val('');
                // disable the input field to make the user wait until server
                // sends back response
                input.attr('disabled', 'disabled');
            }
        }
    });

    /**
     * This method is optional. If the server wasn't able to respond to the
     * in 3 seconds then show some error message to notify the user that
     * something is wrong.
     */
    setInterval(function() {
        if (connection.readyState !== 1) {
            status.text('Error, reconnecting');
            input.attr('disabled', 'disabled').val('Unable to comminucate '
                                                 + 'with the WebSocket server.');
            connection.close()
            connection = createConnection(wsUrl)
                                                 
        }
    }, 2000);

    /**
     * Add message to the chat window
     */
    function addMessage(author, message,color, dt) {
        content.prepend('<p><span style="color:' + color + '">' + author + '</span> @ ' +
             + (dt.getHours() < 10 ? '0' + dt.getHours() : dt.getHours()) + ':'
             + (dt.getMinutes() < 10 ? '0' + dt.getMinutes() : dt.getMinutes())
             + ': ' + message + '</p>');
    }
});
