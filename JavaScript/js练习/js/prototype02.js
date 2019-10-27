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


function doSomething() {}
console.log(doSomething.prototype);
// 和声明函数的方式无关，
// JavaScript 中的函数永远有一个默认原型属性。
var doSomething = function () {};
console.log(doSomething.prototype);

doSomething.prototype.foo = "bar";
console.log(doSomething.prototype);

let bb = new doSomething()
console.log('a.foo :', bb.foo);



console.log('------------------ :');

function doSomething() {}
doSomething.prototype.foo = "bar";




doSomething.prototype = {
    constructor: doSomething,
    d: 'd',
}

doSomething.prototype.name = "name";



let aa = new doSomething()
console.log('a.foo :', aa.foo);
console.log('a.name :', aa.name);



function A(a) {
    this.varA = a;
}

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

function VipUser(name, password, level) {
    User.call(this, name, password)
    this.level = level
}


// deepCopy(VipUser.prototype, User.prototype);
// VipUser.prototype = User.prototype // User.prototype
// VipUser.prototype.constructor = VipUser;