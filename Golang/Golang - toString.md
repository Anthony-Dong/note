# Golang 小细节

## toString方法

```go
type Struct struct {
	Name string
}

func (Struct)String() string {
	return "name"
}

func main() {
	fmt.Println(&Struct{})
}
```

