# gForce Data Capture Utility

## Usage

Please make sure that `Node.js` has been installed.

1. Plug gForceDongle to the PC, and open `Device Manager` and check if there is
    a new serial port named like `USB Serial Port (COMX)` under the
    'Port (COM & LP)'. Remember the serial port number.

2. Open a command window, which would be running 'CMD' in Window.

3. `cd` to the root directory where this software is located.

4. Run `node WebSocketServer\WebSocketServer.js`, and wait until you see the
   following message:

   > [INFO] Server listen at 8888

   This indicates that the web socket server has run up.

5. Run `RawDataCapture.exe` by double clicking it. It will prompt you the
    following message:

   > The EMG raw data captured from your arm will be stored in the files specified by you later.
   > Furthermore, the data can also be real-time shown in your web browser as long as you set
   > up a websocket server. To do so, please make sure that `node.js` is installed first, and then
   > run `node WebSocketServer.js` in the command shell, and finally open file `WebSocketClient.html`
   > in your browser to wait for raw data to be shown up.
   >
   > Please press any key to continue if you're ready...
   >
   > Connecting to server 127.0.0.1:8888... connected successfully!
   >
   > Please make sure the gForce has been turned on, and then plug the gForce USB dongle
   > and enter its corresponding serial COM number:

   Enter the serial port number that you got in step 1. You will see lots of
   BLE messages subsequently, but you may just neglect them unless something looks
   wrong.

   The 'Connecting to server 127.0.0.1:888... connected successfully!' message
   just indicates that the `RawDataCapture` program has connected to the web
   socket server, which has been run up in step 4. You could double check it
   by checking if there is a message like the following in the `WebSocketServer`
   window:

   > [INFO] Client indicated as a source.

6. Power on the gForce armband, and approach it to the gForceDongle. After a
   few seconds, you will see the LED light of the gForce flashes frequently if
   connection is established successfully.

7. Open the `WebSocketClient\gForceInsider.html` file in your web browser by
   double clicking it. In the page shown up, click the 'GO!' button next to
   the web socket server URI like `ws://localhost:8888/RawData` to connect to
   the `WebSocketServer`, which you've run up in step 4. The following message
   will appear if connection is established successfully:

   > Connected to ws://localhost:8888/RawData

   You may double check by checking if there is a message like the following
   in the `WebSocketServer` window:

   > [INFO] Client indicated as a sink.

   Click the 'EMG Raw Data' button, you will be able to see the data chart
   keeps updating...

## Troubleshooting
If you terminate the `RawDataCatpure` program and restart it again, the
connection may remain. You will have to power-off the gForce armband, restart
the `RawDataCapture` program, and power on the gForce armband again. Whereas
You don't need to restart the web socket server however.
