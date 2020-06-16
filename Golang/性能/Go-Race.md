# Go -race

### 介绍

`go run -race xxx...`

-race选项用于检测数据竞争，在使用了-race的情况下，go程序跑起来之后，如果发生数据竞争，它就能检测到，它就会一层一层地把错误栈打印出来，就像打印panic一样。通常用于开发。当然开启了该选项也未必能检测出潜在的数据竞争，当你的程序跑到数据竞争的片段它就会检测出来，你的程序可能有很多模块，当程序没有执行到数据竞争的地方那直到整个程序执行结束它也检测不出来。

使用-race选项相比不开启此选项会消耗更多的cpu计算资源和内存，实际上的情况是：
内存方面：不开启此选项时消耗113MB内存，开启之后550MB
cpu方面：不开启此选项1s可以完成的操作，开启之后15s
用的是"golang.org/x/crypto/bcrypt"包的bcrypt.CompareHashAndPassword方法，非常消耗资源的方法。



总结一下，其实就是race选项其实就是检测数据的安全性的，同时写，等情况。



### demo （race 到底是什么）

下面这个demo就是一个常见的  懒汉式单例模式，依靠go的共享变量不需要担心可见性的问题。

```go
package main

import (
	"fmt"
	"os"
	"strconv"
	"time"
)

var config map[string]string

func main() {
	count, _ := strconv.Atoi(os.Args[1])
	for x := 0; x < count; x++ {
		go getConfig()
	}
	<-time.After(time.Second)
}
func getConfig() map[string]string {
	if config == nil {
    fmt.Println("init config")
		config = map[string]string{}
		return config
	}
	return config
}
```

执行`go run  -race demo.go 100`

```go
sgcx015@172-15-68-151:~/go/code/com.anthony.http % go run -race cmd/once_demo.go 100
init config // load
==================
WARNING: DATA RACE
init config //load
Write at 0x0000012109c0 by goroutine 7: // g7在22行写
  main.getConfig()
      /Users/sgcx015/go/code/com.anthony.http/cmd/once_demo.go:22 +0xd2

Previous read at 0x0000012109c0 by goroutine 8: //g8在20行读 （race）
  main.getConfig()
      /Users/sgcx015/go/code/com.anthony.http/cmd/once_demo.go:20 +0x3e

Goroutine 7 (running) created at:// 这些无效信息
  main.main()
      /Users/sgcx015/go/code/com.anthony.http/cmd/once_demo.go:15 +0xae

Goroutine 8 (running) created at:
  main.main()
      /Users/sgcx015/go/code/com.anthony.http/cmd/once_demo.go:15 +0xae
==================
Found 1 data race(s)
exit status 66
```

发现出现读写竞争了，那么对于我们这种写法来说，确实存在多个线程同时去load，所以加载了两次。那么我们的业务场景是无关紧要的，因为配置加载几次无所谓。

> ​	这里总结一下，race触发的条件不是  同时写，而是读写同时发生，这个问题很严重，严重在哪呢，其实看一下tomic.Value就知道了，计算机64位的，8个字节，对于32位的机器，回去读两次。可能会出现一种情况是 a入32位字节，此时b读取了32位。然后a继续写入32位，此时发生的问题，就是读写不一致。所以atomic解决了这个问题。
>
> 

**那么咱们也需要解决问题是让他加载一次。**

简单点，就是加个锁。然后双重检测一下。

```go
func getConfig() map[string]string {
	if config == nil {
		lock.Lock()
		defer lock.Unlock()
		if config != nil {
			return config
		}
		config = map[string]string{}
		fmt.Println("init config")
		return config
	}
	return config
}
```

执行结果还是出现了竞争读写的问题，必然的。

```go
sgcx015@172-15-68-151:~/go/code/com.anthony.http % go run -race cmd/once_demo.go 100
init config //加载一次
==================
WARNING: DATA RACE
Read at 0x0000012109c0 by goroutine 8:
  main.getConfig()
      /Users/sgcx015/go/code/com.anthony.http/cmd/once_demo.go:24 +0x5b

Previous write at 0x0000012109c0 by goroutine 7:
  main.getConfig()
      /Users/sgcx015/go/code/com.anthony.http/cmd/once_demo.go:30 +0xeb
==================
Found 1 data race(s)
```



**如何解决竞争呢，上面都说了，用atomic类。**

```go
import (
	"fmt"
	"os"
	"strconv"
	"sync/atomic"
	"time"
)

var config atomic.Value

func main() {
	count, _ := strconv.Atoi(os.Args[1])
	for x := 0; x < count; x++ {
		go getConfig()

	}
	<-time.After(time.Second * 2)
}
func getConfig() map[string]string {
	if config.Load() == nil {
		fmt.Println("init config")
		config.Store(map[string]string{})
		return config.Load().(map[string]string)
	}
	return config.Load().(map[string]string)
}
```

执行: 发现确实没有竞争，原因很简单，就是atomic原子操作。然后load了两次

```go
~/go/code/com.anthony.http % go run -race cmd/demo.go 1000
init config
init config
```



### atomic源码分析

```go
// A Value must not be copied after first use.(copy ，而不是指针传递，后面可以看一下实现)
type Value struct {
	v interface{}
}
```



// load源码

```go
func (v *Value) Load() (x interface{}) {
  // 首先转换成了一个标准的interface{}指针(完整的地址长度)
  // v 的data是一个地址
  // v 的type是一个标志符^uintptr(0)，表示是否插入成功
	vp := (*ifaceWords)(unsafe.Pointer(v))
  // 原子加载类型地址
	typ := LoadPointer(&vp.typ)
  // 空是没有存， uintptr(typ) == ^uintptr(0)是表示没有存储完成（中间态，其实我感觉就是个乐观锁）
	if typ == nil || uintptr(typ) == ^uintptr(0) {
		// First store not yet completed.// 第一次加载还没有完成。（需要看store的源码）
		return nil
	}
  // 原子加载值的数据
	data := LoadPointer(&vp.data)
	xp := (*ifaceWords)(unsafe.Pointer(&x))
	xp.typ = typ
	xp.data = data
	return
}
```

// store源码

```go
// Store sets the value of the Value to x.
// All calls to Store for a given Value must use values of the same concrete type.
// Store of an inconsistent type panics, as does Store(nil).
func (v *Value) Store(x interface{}) {
	if x == nil {
		panic("sync/atomic: store of nil value into Value")
	}
	vp := (*ifaceWords)(unsafe.Pointer(v))
	xp := (*ifaceWords)(unsafe.Pointer(&x))
	for {
		typ := LoadPointer(&vp.typ)
		if typ == nil {
			// Attempt to start first store.
			// Disable preemption so that other goroutines can use
			// active spin wait to wait for completion; and so that
			// GC does not see the fake type accidentally.
      // 禁止抢占（其实gorouting的内部调度是抢占模式）//这个不理解，它的解释是为了防止GC回收掉
			runtime_procPin()
      // atomic赋值，失败继续赋值（也就是type确定了一定成功赋值了）
			if !CompareAndSwapPointer(&vp.typ, nil, unsafe.Pointer(^uintptr(0))) {
				runtime_procUnpin()
				continue
			}
			// Complete first store.
      // 完成替换。所以^uintptr(0)这个就是一个记号，区分开nil，属于一个存储中间态
			StorePointer(&vp.data, xp.data)
			StorePointer(&vp.typ, xp.typ)
      //
			runtime_procUnpin()
			return
		}
		if uintptr(typ) == ^uintptr(0) {
			// First store in progress. Wait.
			// Since we disable preemption around the first store,
			// we can wait with active spinning.
			continue
		}
		// First store completed. Check type and overwrite data.
		if typ != xp.typ {
			panic("sync/atomic: store of inconsistently typed value into Value")
		}
    // 这里就确定了，存储了一种类型，这个永远只能存储一个类型。
		StorePointer(&vp.data, xp.data)
		return
	}
}
```



// 其他需要掌握的地方

```go
// ifaceWords is interface{} internal representation.
type ifaceWords struct {
	typ  unsafe.Pointer
	data unsafe.Pointer
}
```

```go
// LoadPointer atomically loads *addr. （原子的加载地址）
func LoadPointer(addr *unsafe.Pointer) (val unsafe.Pointer)
```





### 总结

1、race  和 单例没有关系，也检测不出来。

2、race 只是解决读写不一致的现场，出现同时读写的现象。	