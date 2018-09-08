var websocket = new WebSocket('ws://' + window.location.hostname + ':81');
websocket.onmessage = function (evt) {
 var obj = JSON.parse(evt.data);
 document.getElementById("temperature").innerHTML = obj.temperature + '&#8451';
 document.getElementById("humidity").innerHTML = obj.humidity + '%';
};
