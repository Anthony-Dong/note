# Laravel  web学习

### 注册路由

首先就是一个入口了 ， 所有的配置文件都是在 ： `app/Providers/RouteServiceProvider.php`这个接口下面， 

其中定义方式，就是路由定义的方式，在我们的项目中会存在一堆的路由表，其中全部都是在这里定义的，主要是注意一下几点，

```php
public function map()
{
  	// 这里的含义就是调用方法去注册
    $this->mapApiRoutes();

    $this->mapWebRoutes();

    //
}
```

其中看一下`mapApiRoutes`方法

```php
protected function mapApiRoutes()
{
    Route::prefix('api')
        ->middleware('api')
        ->namespace($this->namespace)
        ->group(base_path('routes/api.php'));
}
```

我们发现就是注册一下路由， 其中我们要care一下， prefix就是前缀， 然后还有一个拦截器，middleware，在我们的	`\App\Http\Kernel`中注册 ，  

```php
protected $routeMiddleware = [
    'auth' => \App\Http\Middleware\Authenticate::class,
		// ... 
];
```

其中我们只需要关注这个， 注意要care这些， 因为它是拦截器， 原理很简单 。 

```php
namespace App\Http\Middleware;
use Closure;
class PreInterceptor
{
    public function handle($request,Closure $next){
			
      // 这就是我们的web流程 ，因为可以在他的前面，后面做手脚，
      // 其中方法名称和参数类型必须是上面的
      	$response = $next($request);
        return $response;
    }
 
  // 这个一般没啥用，是发送前执行的
  	public function terminate($request, $response)
    {
        echo "end";
    }
}
```



下面讲讲路由的几种写法 ：

这个是前面是我们controller 方法，后者是拦截器， 其中要care一下，key一定要写对，是死的 

```php
Route::any('/test/post',['uses' => 'StudentController@post', 'middleware' => ['pre']]);
```



我们的项目中经常是这种 ,这种 也是常见的写法，

```php
Route::group([
    'prefix' => 'service/v1/penalty',
    'middleware' => ['cauth'],
], function () {
    //违章模版列表
    Route::any('template_list', ['uses' => 'PeccancyController@templateList', 'middleware' => []]);
})  
```

一般就这种写法， 其实还有很多高级的用法比如resturl 的方式， 但是项目没有使用



### Validator

> ​	具体是在这包了 `vendor/laravel/framework/src/Illuminate/Validation` 所以找到入口就好了， 校验还是很简单

首先我们看看长啥样子：

```php
use Illuminate\Support\Facades\Validator;
$validator = Validator::make($json_decode, [
    'user' => 'required'
]);
if ($validator->fails()) {
    return "error";
}
```

低一点需要注意的是make的参数

```php
public function make(array $data, array $rules, array $messages = [], array $customAttributes = [])
```

注意参数必须是 数组， 规则是数组

而对于http的content 是字符串， 因此我们需要将其json解码

```php
$json_decode = json_decode($request->getContent(), true);
```

为啥要加入true呢， 低一点是因为人家需要数组， 而如果我们不加的话，则是obj对象的stdclass ， 所以必须家一个参数true， 

其中有很多规则，[我们可以看看这个链接](https://blog.csdn.net/ZoeyZhao/article/details/99412647)



