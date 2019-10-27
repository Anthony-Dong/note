/**
 * 什么是  call  apply 以及 bind
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

let call = fun.call({
    call: 'call'
}, 'call1', 'call2', 'call3')

console.log('call :', call);

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