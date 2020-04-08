# GoEasy练习

```html
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Document</title>
    <script type="text/javascript" src="https://cdn.goeasy.io/goeasy-1.0.5.js"></script>
    <script src="./js/goeasy/vue.js"></script>

</head>

<body>
    <div id="app">
        <p>{{title}}</p>
        <input type="text" v-model="content">
        <button v-on:click="sendMsg">SEND</button><br>
        <ul>
            <li v-for="(item, index) in msgs" :key="index">time: {{item.time}} , msg :{{item.content}}</li>
        </ul>
    </div>

    <script>
        new Vue({
            el: '#app',
            data: {
                channel: 'test',
                title: 'wellcome my room',
                content: '',
                msgs: [],
                goEasy: new GoEasy({
                    host: 'hangzhou.goeasy.io', //应用所在的区域地址: 【hangzhou.goeasy.io |singapore.goeasy.io】
                    appkey: "BC-49a4daf34e72452e850e2c7b20e3ea44", //替换为您的应用appkey
                    userId: "anthony", //必须指定，否则无法实现客户端上下线监听功能
                    userData: "name=anthony" //更多的用户信息，其它已监听上下线信息的用户，收到该用户上线信息里会包含此部分内容
                }),
            },
            methods: {
                sendMsg: function () {
                    this.goEasy.publish({
                        channel: this.channel, //替换为您自己的channel
                        message: this.content, //替换为您想要发送的消息内容
                        userId: "anthony"
                    });
                    this.content = ''
                }
            },
            watch: {
                content: function (o, n) {
                    if (o == '') {
                        console.log('clear msg');
                    }
                }
            },
            created: function () {
                this.goEasy.subscribePresence({
                    channel: this.channel,
                    onPresence: function (presenceEvents) {
                        console.log("Presence events: ", JSON.stringify(presenceEvents));
                    }
                });
                this.goEasy.subscribe({
                    channel: this.channel, //替换为您自己的channel
                    onMessage: message => {
                        console.log(message);
                        this.msgs.push({
                            "time": message.time,
                            "channel": message.channel,
                            "content": message.content
                        })
                    }
                });
            },
        })
    </script>
</body>

</html>
```

