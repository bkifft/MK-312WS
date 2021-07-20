var ws_url = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener("load", onload);

function onload(event) {
    initWebSocket();
}


function initWebSocket() {
    websocket = new WebSocket(ws_url);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    websocket.send("getValues");
}

function onClose(event) {
    setTimeout(initWebSocket, 2000);
}

function update_slider(element) {
    var  value = document.getElementById(element.id).value;
    document.getElementById(element.id + "_value").innerHTML = value;
    websocket.send(element.id + "?" + value.toString());
}

function onMessage(event) 
{
    console.log(event.data);
    var values = JSON.parse(event.data);

    for (const key in values) 
	{
        document.getElementById(key+"_value").innerHTML = values[key];
        document.getElementById(key).value = values[key];
    }
}
