# Map 并发读写的问题（concurrent map read and map write）

## 问题

我们知道写入一定会出现 并发写出 panic ，那么并发读写是为啥呢， 一下很常见， 一个goroutine 不断的拉去配置， 大量的线程拉去配置信息

```go
package main

import (
	"fmt"
	"math/rand"
	"runtime"
	"strconv"
	"sync"
	"time"
)

func main() {
	_map := map[string]interface{}{}

	wg := sync.WaitGroup{}
	wg.Add(1)
	for x := 0; x < 1; x++ {
		go func() {
			for {
				_map["1"] = strconv.FormatInt(int64(rand.Intn(1000)), 10)
				_map["2"] = strconv.FormatInt(int64(rand.Intn(1000)), 10)
				<-time.After(time.Millisecond)
			}
		}()
	}

	for x := 0; x < 1; x++ {
		//
		go func() {
			for {
				fmt.Println(_map["1"])
				fmt.Println(_map["2"])
			}
		}()
	}
	
	runtime.Gosched()
	wg.Wait()
}
```



上面这个代码执行一次

```go
265
fatal error: concurrent map read and map write

goroutine 7 [running]:
runtime.throw(0x10d7f15, 0x21)
        /usr/local/go/src/runtime/panic.go:774 +0x72 fp=0xc000076ef8 sp=0xc000076ec8 pc=0x1029bc2
runtime.mapaccess1_faststr(0x10b46a0, 0xc0000721b0, 0x10d32ab, 0x1, 0x1) // 这个地方出现了问题
        /usr/local/go/src/runtime/map_faststr.go:21 +0x44f fp=0xc000076f68 sp=0xc000076ef8 pc=0x101020f
main.main.func2(0xc0000721b0)
        /Users/sgcx015/go/code/com.anthony.http/cmd/type_demo.go:32 +0xd5 fp=0xc000076fd8 sp=0xc000076f68 pc=0x109b835
runtime.goexit()
        /usr/local/go/src/runtime/asm_amd64.s:1357 +0x1 fp=0xc000076fe0 sp=0xc000076fd8 pc=0x10539f1
created by main.main
        /Users/sgcx015/go/code/com.anthony.http/cmd/type_demo.go:29 +0xbf

exit status 2
```



找到位置

```go
func mapaccess1_faststr(t *maptype, h *hmap, ky string) unsafe.Pointer {
	if raceenabled && h != nil {
		callerpc := getcallerpc()
		racereadpc(unsafe.Pointer(h), callerpc, funcPC(mapaccess1_faststr))
	}
	if h == nil || h.count == 0 {
		return unsafe.Pointer(&zeroVal[0])
	}
  // 这个地方，// hashWriting  = 4 // a goroutine is writing to the map
  // 所以在access map的时候，回去做一下状态判断
	if h.flags&hashWriting != 0 {
		throw("concurrent map read and map write")
	}
	key := stringStructOf(&ky)
	if h.B == 0 {
		// One-bucket table.
		b := (*bmap)(h.buckets)
		if key.len < 32 {
			// short key, doing lots of comparisons is ok
			for i, kptr := uintptr(0), b.keys(); i < bucketCnt; i, kptr = i+1, add(kptr, 2*sys.PtrSize) {
				k := (*stringStruct)(kptr)
				if k.len != key.len || isEmpty(b.tophash[i]) {
					if b.tophash[i] == emptyRest {
						break
					}
					continue
				}
				if k.str == key.str || memequal(k.str, key.str, uintptr(key.len)) {
					return add(unsafe.Pointer(b), dataOffset+bucketCnt*2*sys.PtrSize+i*uintptr(t.elemsize))
				}
			}
			return unsafe.Pointer(&zeroVal[0])
		}
		// long key, try not to do more comparisons than necessary
		keymaybe := uintptr(bucketCnt)
		for i, kptr := uintptr(0), b.keys(); i < bucketCnt; i, kptr = i+1, add(kptr, 2*sys.PtrSize) {
			k := (*stringStruct)(kptr)
			if k.len != key.len || isEmpty(b.tophash[i]) {
				if b.tophash[i] == emptyRest {
					break
				}
				continue
			}
			if k.str == key.str {
				return add(unsafe.Pointer(b), dataOffset+bucketCnt*2*sys.PtrSize+i*uintptr(t.elemsize))
			}
			// check first 4 bytes
			if *((*[4]byte)(key.str)) != *((*[4]byte)(k.str)) {
				continue
			}
			// check last 4 bytes
			if *((*[4]byte)(add(key.str, uintptr(key.len)-4))) != *((*[4]byte)(add(k.str, uintptr(key.len)-4))) {
				continue
			}
			if keymaybe != bucketCnt {
				// Two keys are potential matches. Use hash to distinguish them.
				goto dohash
			}
			keymaybe = i
		}
		if keymaybe != bucketCnt {
			k := (*stringStruct)(add(unsafe.Pointer(b), dataOffset+keymaybe*2*sys.PtrSize))
			if memequal(k.str, key.str, uintptr(key.len)) {
				return add(unsafe.Pointer(b), dataOffset+bucketCnt*2*sys.PtrSize+keymaybe*uintptr(t.elemsize))
			}
		}
		return unsafe.Pointer(&zeroVal[0])
	}
dohash:
	hash := t.key.alg.hash(noescape(unsafe.Pointer(&ky)), uintptr(h.hash0))
	m := bucketMask(h.B)
	b := (*bmap)(add(h.buckets, (hash&m)*uintptr(t.bucketsize)))
	if c := h.oldbuckets; c != nil {
		if !h.sameSizeGrow() {
			// There used to be half as many buckets; mask down one more power of two.
			m >>= 1
		}
		oldb := (*bmap)(add(c, (hash&m)*uintptr(t.bucketsize)))
		if !evacuated(oldb) {
			b = oldb
		}
	}
	top := tophash(hash)
	for ; b != nil; b = b.overflow(t) {
		for i, kptr := uintptr(0), b.keys(); i < bucketCnt; i, kptr = i+1, add(kptr, 2*sys.PtrSize) {
			k := (*stringStruct)(kptr)
			if k.len != key.len || b.tophash[i] != top {
				continue
			}
			if k.str == key.str || memequal(k.str, key.str, uintptr(key.len)) {
				return add(unsafe.Pointer(b), dataOffset+bucketCnt*2*sys.PtrSize+i*uintptr(t.elemsize))
			}
		}
	}
	return unsafe.Pointer(&zeroVal[0])
}
```

