const WebSocket = require('ws');

const listen_port = 8888;
const wss = new WebSocket.Server({ port: listen_port });

wss.on('listening', function () {
    console.log("[INFO] Server listen at %d\n", listen_port);
}).on('connection', function (ws) {
    console.log("[INFO] Client %s connected!\n");
    ws.on('message', function (data, flags) {
        //console.log("typeof == %s", typeof data);
        if (flags.binary) { // raw data
            if (ws.role === 'source') {
                console.log("[INFO] data: %s, typeof == %s", data, typeof data);
                // send the data to all 'sinks'
                wss.clients.forEach(function each(client) {
                    if (client !== ws && client.role === 'sink' && client.readyState == WebSocket.OPEN) {
                        ws.send(data);
                    }
                });
            }
        } else { // string
            //console.log('Received: %s\n', data);
            if (data === 'source') {
                ws.role = 'source';
                console.log("[INFO] Client indicated as a source.");
            }
            else if (data === 'sink') {
                ws.role = 'sink';
                console.log("[INFO] Client indicated as a sink.");
            }
            else {
                console.log("[WARNING] Unkown role %s.", data)
            }
        }
    }).on('close', function (code, reason) {
        console.log("[INFO] WebSocket closed %s, %s\n", code, reason);
    }).on('error', function (error) {
        console.log("[ERROR] WebSocket error %s\n", error);
    });
}).on('error', function (error) {
    console.log("[ERROR] WebSocketServer error: %s", error);
});

//FIXME: needs graceful exit 

