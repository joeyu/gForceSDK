$(document).ready(function () {
    var   ws = null;
    const emgRawDataSize = 128;
    const channelCount = 8;
    const emgRawDataSizePerChannel = emgRawDataSize / channelCount;
    const maxSizePerChannel = emgRawDataSizePerChannel * 128;
    const emgRawData = Array.apply(null, Array(channelCount)).map(() => []);

    const emgCanvasCtx = new Array(channelCount);
    const canvasArray = Array.apply(null, Array(channelCount)).map(function (e, i) {
        var canvas = document.getElementById('chart' + i);
        var ctx = canvas.getContext('2d');
        ctx.strokeStyle = '#FFFFFF';
        emgCanvasCtx[i] = ctx;
        //var r = canvas.getBoundingClientRect();
        //canvas.width = r.width * 2;
        //canvas.height = r.height * 2;
        canvas.width *= 4;
        canvas.height *= 4;
        console.log("canvas pixel buffer width = %d, height = %d", canvas.width, canvas.height);
        return canvas;
    });

    const gyroRawDataSize =  1024; //Fixme
    const gyroRawData = new Array(gyroRawDataSize);
    const gyroCanvas = document.getElementById('gyroscope_chart');
    var r = gyroCanvas.getBoundingClientRect();
    gyroCanvas.width = r.width * 2;
    gyroCanvas.height = r.height * 2;
    const gyroCanvasCtx = gyroCanvas.getContext('2d');

    function drawGyroChart(ctx, data) {
            const width = ctx.canvas.width;
            const height = ctx.canvas.height;
            const range = 65536;// Gyro value ranges from -2,000 to 2,000
            const step = 1;
            var x = stride;
            ctx.strokeStyle = '#00FFFF';
            ctx.clearRect(0, 0, width, height);
            ctx.beginPath();
            ctx.moveTo(0, height - (data[0] + range / 2) / range * height);
            for (let i = 1; i < data.length; i++) {
                if (x > width) {
                    break;
                }
                ctx.lineTo(x, height - (data[i] + range / 2) / range * height);
                x += step;
            }
            ctx.stroke();
    }

    function drawChart() {
        const step = 1;
        emgCanvasCtx.forEach(function (ctx, i) {
            const width = ctx.canvas.width;
            const height = ctx.canvas.height;
            var x = step;
            ctx.strokeStyle = '#00FFFF';
            ctx.clearRect(0, 0, width, height);
            ctx.beginPath();
            //console.log(emgRawData[i][0]);
            ctx.moveTo(0, height - emgRawData[i][0] / 256 * height);
            for (let j = 1; j < emgRawData[i].length; j++) {
                if (x > width) {
                    break;
                }
                ctx.lineTo(x, height - emgRawData[i][j] / 256 * height);
                x += step;
            }
            ctx.stroke();

        });

        drawGyroChart(gyroCanvasCtx, gyroRawData);
    }

    $('#btn-go').click(function() {
        url = $(this).closest(".input-group").find("input").val();
        if (url) {
            //const port = "8888";
            //const url = "ws://127.0.0.1:" + port + "/emgRawData";
            var url = "ws://" + url;
            ws = new WebSocket(url);
            ws.onmessage = function (messageEvent) {
                //console.log("typeof = %s, size = %s", typeof evt.data, evt.data.size);

                // data should a blob of 128 bytes
                var blob = messageEvent.data;
                if (blob.size !== emgRawDataSize) {
                    console.log("[WARNING] The size of the received data is not %d[%d]", emgRawDataSize, blob.size);
                }

                // Read from the blob and resemble the data
                var fileReader = new FileReader();
                fileReader.onload = function () {
                    var uint8View = new Uint8Array(this.result);
                    for (var i = 0; i < channelCount; i++) {
                        var singleChannel = [];
                        for (var j = 0; j < emgRawDataSizePerChannel; j++) {
                            emgRawData[i].push(uint8View[i + j * 8]);
                        }
                        if (emgRawData[i].length > maxSizePerChannel) {
                            emgRawData[i].splice(0, emgRawDataSizePerChannel);
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
    $('#btn-emgemgRawDatarecord').click(function() {
        console.log($(this).closest(".input-group").find("input").val());
    });
});
