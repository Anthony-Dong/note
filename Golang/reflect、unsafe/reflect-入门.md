# reflect 入门



```go
func ObjToMap(bean interface{}, filter func(fieldName string) string) map[string]interface{} {
	_value := reflect.Indirect(reflect.ValueOf(bean))
	if _value.Kind() != reflect.Struct {
		panic("the bean mush struct")
	}
	_type := _value.Type()
	fieldNum := _value.NumField()
	_map := make(map[string]interface{}, fieldNum)
	for x := 0; x < fieldNum; x++ {
    // filed tyfe，value
		field := _type.Field(x)
		value := _value.Field(x)
    // 这个其实是判断 比如说非public字段，是不能取到interface
		if value.CanInterface() {
			_map[filter(field.Name)] = value.Interface()
		}
	}
	return _map
}
```

将struct 转换成了 map，demo很简单，无非拿到struct的字段信息然后封装到map里。

这种需求有很多，比如说，我们不想写dto结构体，可以直接使用这个。通过map传递出去。





```go
func MapToStruct(meta map[string]interface{},target interface{}) {
  // 必须是指针类型
	_value := reflect.ValueOf(target)
	if _value.Kind() != reflect.Ptr {
		panic("the target mush be ptr")
	}
  // 拿到包装类型的真实对象
	targetValue := _value.Elem()
	if targetValue.Kind()!=reflect.Struct {
		panic("the bean mush be ptr struct")
	}
  // 遍历map
	for k,e := range meta {
    // 拿到字段
		field := targetValue.FieldByName(k)
		if field.CanSet() {
      // 设置进去
			field.Set(reflect.ValueOf(e))
		}
	}
}
```

将Map的信息，填充到struct中





demo

```go
type Demo struct {
	Name string
	age  int
}

func main() {
	demo := Demo{
		Name: "hello",
		age:  1,
	}
	_map := reflect.ObjToMap(demo, func(fieldName string) string {
		return fieldName
	})
	fmt.Println(_map) //map[Name:hello]

	reflect.MapToStruct(map[string]interface{}{
		"Name": "tom",
		"age":  10,
	}, &demo)
	fmt.Printf("demo :%+v", demo)//demo :{Name:tom age:1}
}
```







