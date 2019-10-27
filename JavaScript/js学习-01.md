# js学习

## 1.学习js第一关(认清楚prototype、_  _proto_ _与constructor)

>这 三者 之间有着怎么样的关系
>
>1. prototype 记住它是函数特有的,当我们输出一个prototype 时发现他由constructor和proto(浏览器特有的,它是给开发者准备的) 和一些成员变量 组成
>2. __proto__ 是原型链,是由**一个对象指向一个对象**，即指向它们的原型对象（也可以理解为父对象）,它的作用就是当访问一个对象的属性时，如果该对象内部不存在这个属性，那么就会去它的__proto__属性所指向的那个对象（可以理解为父对象）里找，如果父对象也不存在这个属性，则继续往父对象的__proto__属性所指向的那个对象（可以理解为爷爷对象）里找.  所以称之为原型链
>3. 每个实例对象（ object ）都有一个私有属性（称之为 __proto__ ）指向它的构造函数的原型对像（**prototype** ）
>4. constructor 是什么呢,它是对象特有的,它是从**一个对象指向一个函数**,**指向该对象的构造函数**
>5. 还有this关键字的问题,当我们实例化一个对象时` let o1 = new object1();` 此时 函数体本身的this指向的是 o1这个对象 ;
>6. 我换一句更精辟的  么加this的就是java中的静态方法和常量,
>7. 在 ES2015/ES6 中引入了 `class` 关键字，但那只是语法糖，JavaScript 仍然是基于原型的
>8. 我在论坛看的 [三者关系讲解链接](https://blog.csdn.net/cc18868876837/article/details/81211729)
>9. 我们写个demo来验证一下

### 让我们用图来加深理解

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-06/cd23f2be-550e-45cf-9ad0-48b0b2a26a0c.png?x-oss-process=style/template01)



**当你真正读懂上面的图 你会发现 你就理解了 三者之间的关系了**    **然后看我的demo 吧**

```js
/**
 * function Foo() {...};
 * let f1 = new Foo();
 	这两段代码背后的东西是什么
 	
 	var f1 = new Object();
    f1.__proto__ = Foo.prototype;
    Foo.call(o);
 	
 	创建一个构造函数Foo()，并用new关键字实例化该构造函数得到一个实例化对象f1,从而在内存中分配一个实例对象。这里稍微补充一下new操作符将函数作为构造器进行调用时的过程：函数被调用，然后新创建一个对象，并且成了函数的上下文（也就是此时  函数内部的this是指向该新创建的对象(关键点,记住)  ，这意味着我们可以在构造器函数内部通过this参数初始化值），最后返回该新对象的引用
	
	当我们试着用 原生的 对象去写的时候 会关注很多细节 ,如果 你用es6 的新语法 细节几乎没有
	我会在第三章 讲解 es6 中prototype的用法
 * 
 */

let object1 = function () {
    this.a = 'a';
    this.b = 'b';
    object1.prototype.c = 'c'
    this.fun = function () {
        console.log('this.a :', this.a);
    }
    fun2 = function (params) {
        console.log('hhhhhhhhhh');
    }
    console.log('this :', this);
    return this
}

//  外部如何修改object1 prototype的属性 
object1.prototype = {
    constructor: object1,
    d: 'd',
}

object1.prototype.foo = "bar";
//或者可以 

console.log('------------函数体中的this关键字------------------');

object1()

console.log('--------------对象的this关键字----------------');

let o1 = new object1();

console.log('o1 :', o1);

console.log('------------开始我的表演------------------');



//  1. 比较是不类型一致 ?  为什么为 true
console.log('object :', o1 instanceof object1);
/**
 * 每个实例对象（ object ）都有一个私有属性（称之为 __proto__ ）指向它的构造函数的原型对像（**prototype** ）
 * A instanceof  B内部原理,其实  确实如此 o1的构造函数是谁 是 object1() , 所以 o1的 proto 会指向 object1()的原型对象,即  object1.prototype
 * var L = A.__proto__;
    var R = B.prototype;
    if(L === R)
    return true;
 * 
 */




//所以我们的结论是对的  这俩相等 都是 object1的构造函数 其实 等于  object1 输出是一样的
console.log('object1.prototype.constructor :', object1.prototype.constructor);
console.log('o1.constructor :', o1.constructor);



console.log('o1.constructor.prototype :', o1.constructor.prototype);
console.log('object1.prototype :', object1.prototype);




console.log('object1 :', object1);



console.log('object1() :', object1());

console.log('Function() :', Function());

// 这里能使用时因为 我object1() 返回了this 他指的是全文;所以可以直接使用;如果你没有返回this则报错
let oo = object1().fun2
oo()

object1().fun2()

console.log('object1().a :', object1().a);

console.log('----------最下面测试------------------ :');

console.log('o1 :', o1.a);

o1.fun();


console.log('c :', o1.c);
console.log('d :', o1.d);
o1.a = 6

console.log('o1.a :', o1.a);


注意 :

1. doSomething.prototype.name = "name";

2. doSomething.prototype = {
    constructor: doSomething,
    d: 'd',
}
3. let doSome=new doSomething()
	2 称为 构造器创建对象法; ;由于的在 doSome 被实例化时，doSome.[[Prototype]] 指向doSomething.prototype。

这俩 不能同时使用 如果 我用了 1 那么我可以写多个都么问题;但是如果我用了 2,那么我1的就失去效果了;2前面全部的 1 都失去效果;因为 什么呢 因为此时构造函数发生了变化;我 2的构造函数还是我原来函数的;所以 两者并不推荐一起使用;使用也切记 2 在 1 前面

```

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-06/795a166d-2fc8-4319-9bbf-0053e95b045b.png?x-oss-process=style/template01)



## 2. apply call  bind的区别

```js
/**
 * 什么是 apply call 以及 bind
 */

/**
 * 先看call函数  其中当我 输入 一个对象时就改变了this的指向; 第一个参数是 所要绑定的对象
 * 第二个参数是 ...ages  他是函数的参数,由原函数控制
 */

function fun(arg1, arg2) {
    console.log('arg1 :', arg1);
    console.log('arg2 :', arg2);
    console.log('this :', this);
}

fun('name')

console.log('this is call :', 'this is call');
fun.call({
    call: 'call'
}, 'call1', 'call2', 'call3')


/**
 * apply  和 call相同 ,只是第二个参数是一个数组,数组里面是原函数的 参数
 */

console.log('this is appliy :', 'this is appliy');
fun.apply({
    apply: 'apply'
}, ['apply', 'apply2', 'apply3'])

/**
 * 然后我们看看bind  我们发现 bind 和 call 唯一的区别就是 他其实是重新定义了一个 函数;
 * 其实 fun2就是个新的函数,我个人使这么理解,当我们使用call和apply时他无返回值; 而bind返回一个原对象
 */
console.log('this is bind :', 'this is bind');
let fun2 = fun.bind({
    bind: 'bind'
}, 'bind1', 'bind2', 'bind3')
fun2()
```

## 3. prototype

[prototype的讲解](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Inheritance_and_the_prototype_chain)

> 对于使用过基于类的语言 (如 Java 或 C++) 的开发人员来说，JavaScript 有点令人困惑，因为它是动态的，并且本身不提供一个 `class` 实现。（在 ES2015/ES6 中引入了 `class` 关键字，但那只是语法糖，JavaScript 仍然是基于原型的）。



所以我们看待 js 要以一种 动态语言去看待 我感觉是js区别其他语言的一个特点;

- 这里 就用到了  原型 - prototype
- 他可以动态的修改原对象中定义的函数

>
>
>创建对象的多种方式:
>
>1. 使用语法结构创建对象
>2. 使用构造器创建对象
>3. Object.create() 创建对象
>4. class创建对象

```js
// 1.最初
        function User(name, pwd) {
            this.name = name
            this.pwd = pwd
        }
        User.prototype.showName = function () {
            console.log(this.name)
        }
        User.prototype.showPwd = function () {
            console.log(this.pwd)
        }

        let u1 = new User('tom', '123456')
        u1.showName()
        u1.showPwd()



// 2.

        var o = {
            a: 2,
            m: function () {
                return this.a + 1;
            }
        };

        // 当我们理解原生的对象时这个就很好理解了
        console.log('o :', o);
        console.log(o.m()); // 3
        // 当调用 o.m 时，'this' 指向了 o.

        var p = Object.create(o);
        // p是一个继承自 o 的对象

        p.a = 4; // 创建 p 的自身属性 'a'
        console.log(p.m()); // 5
        // 调用 p.m 时，'this' 指向了 p
        // 又因为 p 继承了 o 的 m 函数
        // 所以，此时的 'this.a' 即 p.a，就是 p 的自身属性 'a'


// 3. 
        class Student {
            constructor(name, password) {
                this.name = name;
                this.password = password
            };
            info() {
                return this.name + ':' + this.password
            }
        }

        let student = new Student('tom', '123456')
        console.log(student.info())
        console.log('帅')


 
	// 4.借用构造函数 (组合继承)
	// 4.1构造器继承,使用call和apply借用其他构造函数的成员, 可以解决给父构造函数传递参数的问题, 但是获取不到父构造函数原型上的成员.也不存在共享问题
        function VipUser(name,password,level){
            User.call(this,name,password)
            this.level=level
        }


	//4.2原型式继承 VipUser.prototype=User.prototype ,此时构造函数是 user 我们需要改变
		// new User() 对象的原型链 指向的是 User.prototype
        VipUser.prototype=new User();// User.prototype
	
	
	//4.3修改子构造函数的原型的构造器属性  所以此时 构造函数变成了我们自己
        VipUser.prototype.constructor=VipUser;
	

	
        VipUser.prototype.showLevel=function(){
            console.log(this.level)
        }

       let vipu= new VipUser('tony','12345','6')

       vipu.showName()
       vipu.showPwd()
       vipu.showLevel()
        
 //5. es6  继承  
       class VipStudent extends Student{
            constructor(name,pwd,level){
                super(name,pwd)
                this.level=level
            }
            showLevel(){
                console.log(this.level)
            }
       }

       let vips=new VipStudent('rose','123','5')
       console.log(vips.info())
       vips.showLevel()
```

