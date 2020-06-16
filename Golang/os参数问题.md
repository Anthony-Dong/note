# os 参数问题

### 1、参数

项目中获取 根目录是

```go
func GetRootPath() string {
	curFilename := os.Args[0]
	binaryPath, err := exec.LookPath(curFilename)
	if err != nil {
		panic(err)
	}
	binaryPath, err = filepath.Abs(binaryPath)
	if err != nil {
		panic(err)
	}
	return filepath.Dir(filepath.Dir(binaryPath))
}
```



有时候我们拿开发工具去调试的时候，发现项目启动失败，是因为配置文件的问题，这就是 `go run` 的问题，它默认的项目路径是一个系统路径

```go
current-path : /private/var/folders/cs/_wm77lv95j5d88s_q9vr959w0000gn/T/___go_build_os_go
```

是不是很奇怪，所以我们本地调试起来很麻烦。debug走不通，可以借助于 go的debug工具dlv。

这个目录看一下:  config 是我加的，为了本地调试方便

```go
sgcx015@172-15-68-151:/private/var/folders/cs/_wm77lv95j5d88s_q9vr959w0000gn % ls -al
total 0
drwxr-xr-x@   6 sgcx015  staff   192  5 18 11:36 .
drwxr-xr-x@   3 root     wheel    96  3 26 17:25 ..
drwxr-xr-x@  16 sgcx015  staff   512  3 28 19:10 0
drwx------@ 201 sgcx015  staff  6432  5 18 17:02 C
drwxr-x---@  97 sgcx015  staff  3104  5 19 11:22 T
drwxr-xr-x    3 sgcx015  staff    96  5 18 11:37 config
```



其次就是为啥 `go build` 可以走的通，因为这个参数，其实是对于可执行二进制文件的问题。

