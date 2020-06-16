# http-client

> ​	底层包的问题就是，GET、POST等方法，并没有提供超时机制，以及其他个性化配置，业务中为了提高开发效率，往往需要封装，或者直接找现成的。
>
> ​	我们的需求，往往很多变，比如请求参数的自定义，响应结果校验，请求地址需要去注册中心找，token校验等等，所以需要设计

### 1、理解http-client

```go
type Client struct {
  // kernal，可以理解为这个就是客户端，它是请求的载体
	Transport RoundTripper
  // 重定向,客户端需要重定向的时候会调用这个方法
	CheckRedirect func(req *Request, via []*Request) error
  // cookie缓存，看业务场景是否需要，比如a请求了b拿到cookie，开启了这个，下次请求就会携带这个cookie，底层是个map来维护
	Jar CookieJar
 // 超时，底层是timer
	Timeout time.Duration
}
```

继续看

```go
type RoundTripper interface {
	// 很直接，请求，响应
	RoundTrip(*Request) (*Response, error)
}
```

一般使用这个：`/usr/local/go/src/net/http/transport.go:95`这个transport，只要知道这个可以复用就可以了，这里不考虑这个。



## 2、send请求

```go
http.Get("") // 直接请求
http.Post("","",nil)// 
```

这个显然不满足我们的需求，因为拓展，不能局限于方法。

所以核心就是这个

```go
http.Client{}.Do(&http.Request{})
```

那么我们就开始吧。

### 3、封装

```go
package httpclient

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"gitlab.com/core/gauss/pkg/gauss-core/conf"
	"gitlab.com/core/gauss/pkg/gauss-core/engines"
	"gitlab.com/core/gauss/pkg/gauss-core/engines/binding"
	"io"
	"io/ioutil"
	"net"
	"net/http"
	"net/url"
	"reflect"
	"report/internal/logger"
	"report/internal/util/qconf"
	"strings"
	"time"
)

type RequestParams map[string]interface{}

// 这个设计真的巧，完美解决了go的方法重载
type Option func(*Options)
type Options struct {
	TimeOut       time.Duration // 超时时间,默认1s
	RetryMaxNum   int           // 重试次数，默认2次
	RetryIdleTime time.Duration // 每次重式的等待时间，默认5ms
}

const (
	DefaultretryNum      = 2      //默认重试2次，带上第一次请求，一共请求3次
	DefaultMethod        = "POST" //默认Post请求
	DefaultTimeOut1s     = 1 * time.Second
	DefaultTimeOut2s     = 2 * time.Second
	DefaultTimeOut3s     = 3 * time.Second
	DefaultTimeOut5s     = 5 * time.Second
	DefaultRetryIdleTime = 5 * time.Millisecond
)

var (
	defaultHttpTransport = &http.Transport{
		Proxy: http.ProxyFromEnvironment, //代理使用
		DialContext: (&net.Dialer{
			Timeout:   2 * time.Second,  //连接超时时间
			KeepAlive: 30 * time.Second, //连接保持超时时间
			DualStack: true,
		}).DialContext,
		MaxIdleConns:          100,              //client对与所有host最大空闲连接数总和
		IdleConnTimeout:       90 * time.Second, //空闲连接在连接池中的超时时间
		TLSHandshakeTimeout:   10 * time.Second, //TLS安全连接握手超时时间
		ExpectContinueTimeout: 1 * time.Second,  //发送完请求到接收到响应头的超时时间
	}
	validateError      = errors.New("the result can not be null and must be a ptr")
	nilError           = errors.New("response body is null")
	DefaultTimeOut3sOp = func(op *Options) {
		op.TimeOut = DefaultTimeOut3s
	}
	DefaultTimeOut5sOp = func(op *Options) {
		op.TimeOut = DefaultTimeOut5s
	}
	validator = binding.Validator
)

type HttpRequestInfo struct {
	Host        string
	Path        string
	Method      string
	Params      interface{}
	ServiceName string
}

// 判断响应结果是不是为空,适用于那种调用了但是没有响应结果的请求
func CheckResponseBodyIsEmpty(err error) bool {
	return err == nilError
}

// 这里有个问题是，json 的 unmarshal操作，属于一种懒加载模式，也就是json中没有匹配也不会err，只是没有赋值罢了
// 所以根据业务需求去bind响应结果，遵守规范 https://github.com/go-playground/validator
func HttpRequestAndDecode(ctx *engines.Context, serverName string, path string, params interface{}, result interface{}, options ...Option) error {
	if result == nil || reflect.ValueOf(result).Kind() != reflect.Ptr {
		return validateError
	}
	response, err := DefaultHttpRequest(ctx, serverName, path, params, options ...)
	if err != nil {
		return err
	}
	err = json.Unmarshal(response, result)
	if err != nil {
		return errors.New(fmt.Sprintf("json decode http response err: %s", err))
	}
	// 校验结构体，防止json懒检测的机制，使用gin框架自带的validate
	err = validator.ValidateStruct(result)
	if err != nil {
		return err
	}
	return nil
}

func DefaultHttpRequest(ctx *engines.Context, serverName string, path string, params interface{}, options ...Option) ([]byte, error) {
	beginTime := time.Now()
	request, err := NewProxyRequest(&HttpRequestInfo{
		ServiceName: serverName,
		Method:      DefaultMethod,
		Params:      params,
		Path:        path,
	}, ctx)
	if err != nil {
		return nil, err
	}
	logger.Infoc(ctx, "[HttpRequest] start,server_name=%s,url=%s,params=%+v", serverName, request.URL, params)
	result, err := HttpRequest(ctx, request, options ...)
	if err != nil {
		logger.Errorc(ctx, "[HttpRequest] request err,err=%v", err)
		return nil, err
	}

	logger.Infoc(ctx, "[HttpRequest] end,server_name=%s,url=%s,spend=%fs,result=%s", serverName, request.URL, time.Now().Sub(beginTime).Seconds(), result)
	return result, err
}

func NewProxyRequest(requestInfo *HttpRequestInfo, ctx *engines.Context) (*http.Request, error) {
	realHost, err := GetServerHost(requestInfo.ServiceName)
	if err != nil {
		logger.Errorc(ctx, "[HttpRequest] get server host fail, server_host=%s,request_info=%+v,err=%v", requestInfo.ServiceName, requestInfo, err)
		return nil, err
	}
	// 解析url
	_url, err := url.Parse(realHost)
	if err != nil {
		logger.Errorc(ctx, "[HttpRequest] parse url fail, server_host=%s,request_info=%+v,err=%v", requestInfo.ServiceName, requestInfo, err)
		return nil, err
	}
	_url.Path = requestInfo.Path

	// json请求参数
	reader, err := addJsonRequestParams(requestInfo.Params)
	if err != nil {
		logger.Errorc(ctx, "[HttpRequest] add request params err, server_host=%s,request_info=%+v,err=%v", requestInfo.ServiceName, requestInfo, err)
		return nil, err
	}
	// 创建request
	request, err := http.NewRequest(requestInfo.Method, _url.String(), reader)
	if err != nil {
		logger.Errorc(ctx, "[HttpRequest] new request err, server_host=%s,request_info=%+v,err=%v", requestInfo.ServiceName, requestInfo, err)
		return nil, err
	}
	// 添加token和请求头
	err = addRequestHeader(request, requestInfo.ServiceName)
	if err != nil {
		logger.Errorc(ctx, "[HttpRequest] add request header err, server_host=%s,request_info=%+v,err=%v", requestInfo.ServiceName, requestInfo, err)
		return nil, err
	}
	return request, nil
}

func addRequestHeader(req *http.Request, serverName string) error {
	token, err := GenerateToken(serverName)
	if err != nil {
		return err
	}
	req.Header.Add("Content-Type", "application/json")
	req.Header.Add("Service-Token", token)
	return nil
}

// 统一json格式 和 {params:{}}请求格式
func addJsonRequestParams(params interface{}) (io.Reader, error) {
	var buf bytes.Buffer
	if err := json.NewEncoder(&buf).Encode(map[string]interface{}{
		"params": params,
	}); err != nil {
		return nil, err
	}
	return ioutil.NopCloser(&buf), nil
}

func NewHttpRequest(method, url string) (*http.Request, error) {
	return http.NewRequest(method, url, nil)
}

// no cached cookie
func HttpRequest(ctx *engines.Context, request *http.Request, options ...Option) ([]byte, error) {
	option := loadOp(options ...)
	if option.RetryMaxNum <= 0 {
		option.RetryMaxNum = DefaultretryNum
	}
	if option.TimeOut < DefaultTimeOut1s {
		option.TimeOut = DefaultTimeOut1s
	}
	if option.RetryIdleTime < DefaultRetryIdleTime {
		option.RetryIdleTime = DefaultRetryIdleTime
	}
	client := http.Client{
		Transport: defaultHttpTransport,
		Timeout:   option.TimeOut,
	}
	retryNum := 0
REGENERATE:
	//请求开始
	retryNum++
	response, err := client.Do(request)
	if err != nil {
		if retryNum <= option.RetryMaxNum {
			logger.Warnc(ctx, "[HttpRequest] request err,cur_retry=%d,max_retry=%d,err=%v", retryNum, option.RetryMaxNum, err)
			time.Sleep(option.RetryIdleTime)
			goto REGENERATE
		} else {
			return nil, err
		}
	}
	body, err := ioutil.ReadAll(response.Body)
	if err != nil {
		return nil, err
	}
	if body == nil || len(body) == 0 {
		return nil, nilError
	}
	return body, nil
}

func loadOp(option ...Option) *Options {
	op := new(Options)
	for _, elem := range option {
		elem(op)
	}
	return op
}

func GetServerHost(serviceName string) (string, error) {
	env := conf.GetConfig().MustValue("application", "env", "")
	host := conf.GetConfig().MustValue(serviceName, "host", "")

	//开发环境使用的是ip地址，无法使用qconf
	if strings.Compare(env, "debug") == 0 {
		return "http://" + host, nil
	}

	qconfConfig, _ := conf.GetConfig().GetSection("qconf")
	host, err := qconf.GetHost(host, qconfConfig["qconfIdc"])
	if err != nil {
		return "", err
	}

	host = "http://" + host

	return host, nil
}
```

