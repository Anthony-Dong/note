const obj1 = {
    a: 'a in obj1',
    foo: () => {
        console.log(this.a)
    }
}

const obj2 = {
    a: 'a in obj2',
    bar: obj1.foo
}

const obj3 = {
    a: 'a in obj3'
}

obj1.foo() // 输出 ？？
obj2.bar() // 输出 ？？
obj2.bar.call(obj3) // 输出 ？？