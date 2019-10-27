/**
 * 我们学习 js创建对象的几种方式
 * 
 */
// 第一种方式
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



// console.log('ob :', object1);

//  外部如何修改 prototype的属性 原生 对象中
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

console.log('o1 :', o1);
console.log('o1.a :', o1.a);
console.log('o1.foo :', o1.foo);

console.log('ob :', object1.prototype);