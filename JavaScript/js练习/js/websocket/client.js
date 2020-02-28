let ws = new WebSocket('ws://localhost:8080');
ws.onopen = function () {
    console.log("client：打开连接");
    ws.send("client：hello，服务端");
};
ws.onmessage = function (e) {
    console.log("client：接收到服务端的消息 " + e.data);
    setTimeout(() => {
        ws.close();
    }, 5000);
};
ws.onclose = function (params) {
    console.log("client：关闭连接");
};