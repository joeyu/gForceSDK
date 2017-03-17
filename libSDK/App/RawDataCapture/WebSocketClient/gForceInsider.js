$(document).ready(function () {
    var   ws = null;  
    const rawDataSize = 128;
    const channelCount = 8;
    const rawDataSizePerChannel = rawDataSize / channelCount;
    const maxSizePerChannel = rawDataSizePerChannel * 128;
    const rawData = Array.apply(null, Array(channelCount)).map(() => []);

    const ctxArray = new Array(channelCount);
    const canvasArray = Array.apply(null, Array(channelCount)).map(function (e, i) {
        var canvas = document.getElementById('chart' + i);
        var ctx = canvas.getContext('2d');
        ctx.strokeStyle = '#FFFFFF';
        ctxArray[i] = ctx;
        var r = canvas.getBoundingClientRect();
        canvas.width = r.width * 2;
        canvas.height = r.height * 2;
        //console.log("canvas pixel buffer width = %d, height = %d", canvas.width, canvas.height);
        return canvas;
    });

    function drawChart() {
        const step = 1;
        ctxArray.forEach(function (ctx, i) {
            const width = ctx.canvas.width;
            const height = ctx.canvas.height;
            var x = step;
            ctx.strokeStyle = '#00FFFF';
            ctx.clearRect(0, 0, width, height);
            ctx.beginPath();
            //console.log(rawData[i][0]);
            ctx.moveTo(0, height - rawData[i][0] / 256 * height);
            for (let j = 1; j < rawData[i].length; j++) {
                if (x > width) {
                    break;
                }
                ctx.lineTo(x, height - rawData[i][j] / 256 * height);
                x += step;
            }
            ctx.stroke();

        });
    }

    $('#btn-go').click(function() {
        url = $(this).closest(".input-group").find("input").val();
        if (url) {
            //const port = "8888";
            //const url = "ws://127.0.0.1:" + port + "/RawData";
            var url = "ws://" + url;
            ws = new WebSocket(url);
            ws.onmessage = function (messageEvent) {
                //console.log("typeof = %s, size = %s", typeof evt.data, evt.data.size);

                // data should a blob of 128 bytes
                var blob = messageEvent.data;
                if (blob.size !== rawDataSize) {
                    console.log("[WARNING] The size of the received data is not %d[%d]", rawDataSize, blob.size);
                }

                // Read from the blob and resemble the data
                var fileReader = new FileReader();
                fileReader.onload = function () {
                    var uint8View = new Uint8Array(this.result);
                    for (var i = 0; i < channelCount; i++) {
                        var singleChannel = [];
                        for (var j = 0; j < rawDataSizePerChannel; j++) {
                            rawData[i].push(uint8View[i + j * 8]);
                        }
                        if (rawData[i].length > maxSizePerChannel) {
                            rawData[i].splice(0, rawDataSizePerChannel);
                        }
                    }
                    window.requestAnimationFrame(drawChart);
                };
                fileReader.readAsArrayBuffer(blob);
            };
            ws.onopen = function () {
                ws.error = false;
                $("#status").removeClass().addClass("alert alert-success").html("Connected to " + url);

                // Indicate as a data sink.
                var data = 'sink';
                ws.send(data);
            };
            ws.onclose = function (closeEvent) {
                if (ws.error) {
                    ws.error = false;
                    return;
                }
                $("#status").removeClass().addClass("alert alert-info").html("Connection to " + url + " closed.");
            };
            ws.onerror = function () {
                ws.error = true;
                $("#status").removeClass().addClass("alert alert-danger").html("Failed to connect to " + url);
            }
        }
    });
    

    // Record EMG raw data
    $('#btn-emgrawdatarecord').click(function() {
        console.log($(this).closest(".input-group").find("input").val());
    });
});