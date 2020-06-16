# CookieJar 的作用和疑点

### [golang关于cookiejar用法的疑问](https://segmentfault.com/q/1010000010339661)

**1、 如果使用了CookieJar，那么复用一个请求，会出现请求，cookie累积的问题**

```go
func TestCookies(t *testing.T) {
	jar, _ := cookiejar.New(nil)
	client := http.Client{
		Jar: jar,
	}
	request, _ := http.NewRequest("GET", "http://localhost:9999/get", nil)

	for x := 0; x < 10; x++ {
		res, err := client.Do(request)
		if err != nil {
			fmt.Println(err)
			return
		}
		fmt.Printf("req %+v\n", request.Cookies())
		fmt.Printf("resp %+v\n", res.Cookies())
	}
}
```

输出、

```go
=== RUN   TestCookies
req []
resp [XMEN=STORM69; Expires=Mon, 09 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69]
resp [XMEN=STORM70; Expires=Tue, 10 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69 XMEN=STORM70]
resp [XMEN=STORM71; Expires=Wed, 11 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69 XMEN=STORM70 XMEN=STORM71]
resp [XMEN=STORM72; Expires=Thu, 12 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69 XMEN=STORM70 XMEN=STORM71 XMEN=STORM72]
resp [XMEN=STORM73; Expires=Fri, 13 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69 XMEN=STORM70 XMEN=STORM71 XMEN=STORM72 XMEN=STORM73]
resp [XMEN=STORM74; Expires=Sat, 14 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69 XMEN=STORM70 XMEN=STORM71 XMEN=STORM72 XMEN=STORM73 XMEN=STORM74]
resp [XMEN=STORM75; Expires=Sun, 15 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69 XMEN=STORM70 XMEN=STORM71 XMEN=STORM72 XMEN=STORM73 XMEN=STORM74 XMEN=STORM75]
resp [XMEN=STORM76; Expires=Mon, 16 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69 XMEN=STORM70 XMEN=STORM71 XMEN=STORM72 XMEN=STORM73 XMEN=STORM74 XMEN=STORM75 XMEN=STORM76]
resp [XMEN=STORM77; Expires=Tue, 17 Aug 2021 06:08:53 GMT]
req [XMEN=STORM69 XMEN=STORM70 XMEN=STORM71 XMEN=STORM72 XMEN=STORM73 XMEN=STORM74 XMEN=STORM75 XMEN=STORM76 XMEN=STORM77]
resp [XMEN=STORM78; Expires=Wed, 18 Aug 2021 06:08:53 GMT]
--- PASS: TestCookies (0.01s)
PASS
```



2、cookie jar的作用的将cookie缓存起来，而且只缓存请求的cookie， 比如先发送一个请求，那么发送完成后，会将这次响应的cookie 缓存起来。下一次请求的时候，会携带cookie。



