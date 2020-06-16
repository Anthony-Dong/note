## ebike-php项目如何http调用

首先代码结构中 ： 

项目目录下的`config/services.php`中定义了一些环境

```php
'ebike_op_api_go' => in_array(env('APP_ENV'), ['production']) ? [
    'name' => 'ebike_op_api_go',
    'url' => '/qconf_root/number7/ebike-factory-api',
] : (in_array(env('APP_ENV'), ['staging']) ? [
    'name' => 'ebike_op_api_go',
    'url' => '/qconf_root/number7/ebike-factory-api_slave',
] : (in_array(env('APP_ENV'), ['testing']) ? [
    'name' => 'ebike_op_api_go',
    'url' => '/qconf_root/number7/ebike-factory-api_test',
] : [
    'name' => 'ebike_op_api_go',
    'url' => '0.0.0.0:00000',       // 这个IP是：/qconf_root/number7/ebike-factory-api_dev
])),
```

这个含义就是如果是 生产环境走第一个， 其次是模拟生产环境中走第二个，其次是测试环境走第三个，最后是第四个就是本地开发环境



其次就是如何调用Http-Client ，我看看一下这个接口`\App\Http\Controllers\TroubleController::troubleInfo` ， 里面有一个http调用，

```php
try {
  // 1. 第一步获取name，我也不懂为啥要拿到name，name其实就等于ebike_op_api_go
    $serviceRequest = new ServiceRequest(config("services.ebike_op_api_go.name"));
  // 2. 远程调用的path
    $path = '/app/v1/trouble/trouble_info';
  // 3. 获取远程主机的地址，然后和path ， 就创造出url了
    $uri = $serviceRequest->getUri(config('services.ebike_op_api_go.url'), $path);
    $params = [
        'type' => $type,
        'bike_sn' => $bikeSn,
    ];
  // 4. 请求，设置超时时间
    $res = $serviceRequest->request($uri, $params, config('timeout.threeSeconds'));
  // 5. 就是判断error是不是为空
    if (isset($res['error'])) {
        Log::warning(__FILE__ . '(' . __LINE__ . '), change bike warehouse fail', [
            'user_id' => $params,
            'res' => $res,
        ]);
      // 这个[][]是二维数组，php，数据就是字典其次就是它就类似于Java的Map
        throw new Exception($res['error']['message'], $res['error']['code']);
    }
} catch (Exception $e) {
    return response()->clientFail($e->getCode(), $e->getMessage());
}
// 返回结果就行了，这里需要注意的是，这玩意竟然还可以这么玩，闭包哇。。。
return response()->clientSuccess($res);
```



基本就是上诉的流程了 ， 其次就是一些其他的细节了， 比如我们需要打印一些信息，比如异常，最后还要抛出异常。这里反正遵守项目结构的流程就可以了。

