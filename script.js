// script.js

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function getValues() {
    websocket.send("getValues");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getValues();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log(event.data);
    let data = JSON.parse(event.data);
    document.getElementById('parkState1').textContent = data.parkState1;
    document.getElementById('parkState2').textContent = data.parkState2;
    document.getElementById('parkState3').textContent = data.parkState3;
}

// Function to send reservation information over WebSocket
function sendReservationInfo(message) {
    if (websocket.readyState === WebSocket.OPEN) {
        websocket.send(message);
        console.log("Reservation info sent over WebSocket: " + message);
    } else {
        console.error("WebSocket connection is not open.");
    }
}
