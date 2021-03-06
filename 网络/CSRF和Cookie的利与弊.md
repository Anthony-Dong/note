# CSRF和Cookie的利与弊以及安全的Token设计

​	   我们知道Http是一个无状态协议, 这个是指当前的Http协议, 并不是掺杂着其他的比如Websocket 或者其他基于Http之上的实现. 

​		那么无状态并不适合当前的网站或者API设计 , 因为更多人不是为了浏览新闻, 而且个人空间, 这就加入了个人认证. 随着时代的进步, 不得不加入一个方式来如何解决个人信息的问题, 这个信息要满足安全 要满足可以有收缩性, 等等很多. 随之而来cookie诞生了. 这是一个Http规范, 指的是浏览器方的实现要基于这个规范来做. 比如跨域等限制等等. 比如cookie-set命令等实现. 很多的规范需要客户端和服务端都遵循. 

​		这样就很好地满足了用户信息的保存. 我们先不考虑身份认证这点.  比如A用户去请求www.xxx.com网站,此时这个网站是一个新闻网站, 需要个性推荐, 但是服务器不认识这个人, 因此他会在他第一次访问改网站给他一个唯一标识符. 并且放在他本地 , 这样以后每次访问都会携带上做这个标识符 , 服务器知道这个人喜欢看啥, 并且就给予推荐, 网站流量就增加了, 网站就得以快速发展. 

​		但是有些人想到的是, 如果cookie可以用来做身份认证多好哇, 就不用每次输入密码了, 因为为了用户体验. 所以在用户输入密码确认后, 会将一个身份认证的token信息保存本地的cookie ,服务器就可以很轻松的做到身份认证, 比如讲这个信息存储在进程内部, 获取存储在一个统一的服务器中, 比如redis等等. 就可以很好地实现身份认证,还有身份信息保存等. 

​		但是哇, 有些人就想到了这个漏洞. 你拿着cookie用来做身份校验. 那么我是不是可以借助这个cookie做点啥. 

因此出现了一群人开始利用这个cookie了. 

举个例子. (先不考虑技术漏洞,别说你可以这么实现防止等问题.)

比如A用户身份认证了一个存在交易风险的网站. 比如银行等. 每次转账只需要 www.bank.com?transfer=1000&id=123456  , 这个接口, 并且每次携带这cookie. 就可以完成等 给123456这张银行卡转账1000元. 

但是有些网站知道了 www.bank.com的漏洞, 所以他会在各种论坛发送链接. 他发送的是 www.bank.com?transfer=1000&id=654321  这个链接, 有些人就胡点击进去 . 此时有些人可能登陆过www.bank.com认证过, 此时就会给 654321这个账号转账1000元. 



首先上诉问题, 我们如何防止呢 ? 

一 . 聪明的人会说, 用post请求哇. 这么链接就不行了哇.  (可以)  但是我不会写那个post请求的脚本,他的链接可以做到post请求.

二. 有些人就说, 限制上时间, 比如cookie过期时间, 也是可以的 . 

三. 有些人说, 可以让cookie无法让脚本获取到 . 比如这么设置 `response.setHeader( "Set-Cookie", "cookiename=cookievalue;HttpOnly");`   , 这样脚本无法拿到. 

四. 就是弥补cookie的不足. 就是增加额外信息. 比如加入随机token值. 可以这么实现,比如要二次校验, 因为对于大多数交易平台来说 , 并不会存在一个密码就可以交易的, 比如支付宝, 有登录账号的密码, 也有支付密码.  这种双重认证就可以弥补住. 你可以通过cookie去请求转账, 但是你必须输入用户的支付密码.  这些都是为了安全考虑. 

​			总结一下, 其实现在主流的网站都是基于一个校验值来做的.  

​			API请求必须携带校验值(可以说是token), 如果没有你的请求无效的. 比如你请求查看好友信息等私密信息. 请求时必须携带有唯一的校验值. 这个校验值他会通过特定的方式给你的,是一个安全的.  (第三方授权也是基于这个的.)  

​		  我给大家设计一个简单的校验值, 比如我们通过一种加密方式(不限制) , 加密一段  `校验值+过期时间+IP`, 那么在不考虑攻击者知道我们的加密方式的情况下, 也不知道用户IP是什么. 那么他即便使用了我们的校验值, 他的请求IP和校验值的IP对不上也是不行, 或者他知道的时候已经过期了.  

​		 没有绝对的安全, 只有相对的, 越安全越麻烦.  但是我觉得密码这种死的东西是会被淘汰的. 二维码登录, 短信验证码登录等等这些短期时效性的就是一个好的方式. 如何做到token的安全其实就是我们设计授权API的一个核心问题. 设计到普通接口API , 授权认证API等 , 以及拓展第三方业务登录. 

​		如果有兴趣我接触到微信,QQ,支付宝等我会介绍给大家的. 最近太忙了. 以后会补跟的, 其实不要在乎什么安全框架,没有绝对的,只有加入我们灵活可靠的设计, 项目才有灵魂. 一层不变的框架会限制住我们的思想,但是可以学习别人框架的设计的亮点.  



​		

​		可以看看**OAuth2.0**协议的设计. 留个链接,阮一峰大佬写的. http://www.ruanyifeng.com/blog/2019/04/oauth_design.html , 阮一峰大佬写文章并不会通篇的概念, 我觉得是一个很好地学习参考. 



​	名词解释: 我就不说了, 留个链接大家自己看.. 

​	一. **COOKIE** :  https://baike.baidu.com/item/cookie/1119?fr=aladdin

​		cookie的几个参数值 : 我觉得是有必要了解几个参数的含义.

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-30-33/1cdd13f1-e821-4738-87d5-63fcea01a4ba.png)

​	[这篇文章](https://blog.csdn.net/cuishizun/article/details/81536860?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task)不错 , 讲的比较全 : [https://blog.csdn.net/cuishizun/article/details/81536860?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task](https://blog.csdn.net/cuishizun/article/details/81536860?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task)



二. **跨站请求伪造**（英语：Cross-site request forgery），也被称为 **one-click attack** 或者 **session riding**，通常缩写为 **CSRF** 或者 **XSRF**， 是一种挟制用户在当前已登录的Web应用程序上执行非本意的操作的攻击方法。跟[跨网站脚本](https://baike.baidu.com/item/跨网站脚本)（XSS）相比，**XSS** 利用的是用户对指定网站的信任，CSRF 利用的是网站对用户网页浏览器的信任。

