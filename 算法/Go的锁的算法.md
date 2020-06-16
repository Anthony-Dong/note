# Go 锁



锁的结构体

```go
// A Mutex is a mutual exclusion lock.
// The zero value for a Mutex is an unlocked mutex.
type Mutex struct {
	state int32
	sema  uint32
}
```



加锁过程：

```go
func (m *Mutex) Lock() {
	// Fast path: grab unlocked mutex.
	if atomic.CompareAndSwapInt32(&m.state, 0, mutexLocked) {
		if race.Enabled {
			race.Acquire(unsafe.Pointer(m))
		}
		return
	}
	// Slow path (outlined so that the fast path can be inlined)
	m.lockSlow()
}
```





> ​		多个并行Goroutine如何获取锁？
>
> 　　锁有两种，正常模式和饥饿模式。
>
> 　　正常模式下，goroutine等待队列遵循先进先出原则，等待被唤醒。被唤醒的goroutine与新请求的goroutine（没进队列）竞争锁。队列中的进程可能要等很久。
>
> ​		（偏向模式）
>
> 　　饥饿模式下，等待队列的头部goroutine优先级最高，新请求的goroutine被强制放入队列尾部。（猜测会损失调一部分性能，CPU中的goroutine被强制放置play）
>
> ​		（公平模式）
>
> 　　
>
> 　　对于golang来说，混合使用了两种模式，正常模式下，当队列中等待的goroutine等待时间 > 1ms，会转为饥饿模式。饥饿模式时，当队列中最后一个goroutine也拿到了锁，或拿到锁的时间 < 1ms（即没有积压的goroutine。QA：怎么计时的？）会转回正常模式。