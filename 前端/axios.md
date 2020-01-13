# AXIOS

[axios链接](https://www.kancloud.cn/yunye/axios/234845)

```javascript
baseURL: 'http://localhost:9090', //这是基础url
    headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
    },
    transformRequest: [function (data) {
        let ret = ''
        for (let it in data) {
            ret += encodeURIComponent(it) + '=' + encodeURIComponent(data[it]) + '&'
        }
        if (ret.length > 0) {
            return ret.slice(0, ret.length - 1)
        }
        return ret
    }]
});


// post 方式
HTTP.post('/user', {
        name: 'test',
        password: 111
    })
    .then(function (response) {
        console.log(response);
    })
    .catch(function (error) {
        console.log(error);
    });

// get方式    
HTTP.get('/user', {
        params: {
            ID: 12345
        }
    })
    .then(function (response) {
        console.log(response);
    })
    .catch(function (error) {
        console.log(error);
    });
```

