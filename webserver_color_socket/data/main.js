var websocket = new WebSocket('ws://' + window.location.hostname + ':81');
websocket.onmessage = function (evt) {
 var obj = JSON.parse(evt.data);
 document.getElementById("current_color_div").style.backgroundColor = 'rgb(' + obj.red + ',' + obj.green + ',' + obj.blue + ')';

};
