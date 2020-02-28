# FASTJSON

## 介绍

地址  go  https://github.com/alibaba/fastjson

> ​	Fastjson is a Java library that can be used to convert Java Objects into their JSON representation. It can also be used to convert a JSON string to an equivalent(同等的) Java object. Fastjson can work with arbitrary Java objects including pre-existing objects that you do not have source-code of.
>
>   fastjson可以将一个java对象转换为json形式表示的一个java库,他也可以用来将json字符串转变为java对象.
>
> Fastjson写可以用任何java对象包括一些java对象,你源码中不存在的那些。





## 使用

###### java对象

```java
@Builder
@Data
class Order  {
    @JSONField(ordinal=1)
    public Integer id;

    @JSONField(ordinal=2)
    private  String name;

    @JSONField(ordinal=3)
    private Integer count;

    @JSONField(ordinal=4)
    private Integer version;
}

```



###### 序列化  普通的java对象

```java
   @Test
    public void testOne() {

        Order build = Order.builder().count(1).name("22").id(1).version(11).build();

        // 1. 序列化
        String json = JSON.toJSONString(build);

        // {"id":1,"name":"22","count":1,"version":11}

        // 2. 反序列化
        JSONObject jsonObject = JSON.parseObject(json);

        // 2.1  jsonObject进行 获取java对象
        Order orders = jsonObject.toJavaObject(Order.class);

        // 2. 可以获取 json 数据 的 k v 数据 
        String id = jsonObject.getString("id");

    }
```



###### 序列化  复杂的java对象

```java
    public void testComplex(){
        ArrayList<Order> orders = new ArrayList<>();
        IntStream.range(0, 4).forEach(e->{
            Order order = Order.builder().count(e+1).name("22").id(e).version(e+11).build();
            orders.add(order);
        });
        // 序列化
        String json = JSON.toJSONString(orders);

        //  可以使复合类型   反序列化
        ArrayList<Order> orders1 = JSON.parseObject(json, new TypeReference<ArrayList<Order>>() {
        });
    }
```





###### future  使用  (缺省是不输出空值的)

`String json = JSON.toJSONString(build, SerializerFeature.WriteNullNumberAsZero);`  这个意思是build对象的 int类型 为空的话,自动补0

|    SerializerFeature    |                  描述                   |
| :---------------------: | :-------------------------------------: |
|  WriteNullListAsEmpty   | 将Collection类型字段的字段空值输出为[]  |
| WriteNullStringAsEmpty  | 将字符串类型字段的空值输出为空字符串 "" |
|  WriteNullNumberAsZero  |       将数值类型字段的空值输出为0       |
| WriteNullBooleanAsFalse |    将Boolean类型字段的空值输出为fal     |



###### future  使用  (处理日期)

`String json = JSON.toJSONStringWithDateFormat(new Date(), "yyyy-MM-dd HH:mm:ss.SSS");`



```java
1. 直接 date format
String json = JSON.toJSONStringWithDateFormat(new Date(), "yyyy-MM-dd HH:mm:ss.SSS");

2. 全局 Feature
JSON.DEFFAULT_DATE_FORMAT = "yyyy-MM-dd";
String json = JSON.toJSONString(build, SerializerFeature.WriteDateUseDateFormat);

3.使用ISO-8601日期格式
JSON.toJSONString(obj, SerializerFeature.UseISO8601DateFormat);

4. 
```







## Fastjson 定制序列化



### @JSONField  字段个性化定制

###### name  其实就是 key 值 ,可以进行 

```java
@JSONField(name="ID")
private int id;

@JSONField(name="birthday",format="yyyy-MM-dd")
public Date date;

@JSONField(name="ID")
public int getId() { return id;}

@JSONField(name="ID")
public void setId(int id) {this.id = id;}
```

###### format 日期格式化

```java
@JSONField(name="birthday",format="yyyy-MM-dd")
public Date date;
```

###### ordinal 字段顺序  默认是0 无序

```java
@JSONField(name = "IDD",ordinal = 1)
public Integer id;
输出
{"IDD":0,"name":"22","count":0,"version":0,"date":1572017804462}


@JSONField(name = "IDD",ordinal = 6)
public Integer id;
输出
{"name":"22","count":0,"version":0,"date":1572017869609,"IDD":0}
```



###### serialize 是否序列化

```
@JSONField(serialize = false)private Date date;
```



###### 自定义序列化方式

```java
public class ModelValueSerializer implements ObjectSerializer {
    @Override
    public void write(JSONSerializer serializer, Object object, Object fieldName, Type fieldType,
                      int features) throws IOException {


        // 第二个参数 为 输入对象
        System.out.println("object = " + object);
        Integer value = (Integer) object;
        
        // 第三个参数为  字段名  为 key
        System.out.println("fieldName = " + fieldName);

        // 第四个参数 为 Type
        System.out.println("fieldType = " + fieldType);

        // 第一个参数 为 序列化 接口 // 输出类型最好一直 
        serializer.write(1+value);
    }
}

@JSONField(name = "IDD",ordinal = 6,serializeUsing = ModelValueSerializer.class)
public Integer id;


输出 
object = 1
fieldName = IDD
fieldType = class java.lang.Integer
json  :  {"name":"22","IDD":2}
```







### @JSONType()  对象个性化定制

用在 类上

```
@Target({ ElementType.TYPE })
```





### 通过SerializeFilter定制序列化

根据名称我们可以轻松判断需要哪个

![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-26/8a3d4b81-720f-43db-917d-5060d3e7918d.png)



######  PropertyFilter  接口实现

```java
public class MySerializeFilter implements PropertyFilter {

    @Override
    public boolean apply(Object object, String name, Object value) {
        // 第一个参数 被序列化的对象 OBJ对象
        System.out.println("第一个参数 = " + object);

        // 第二个参数是  key 
        System.out.println("第二个参数 = " + name);

        // 第三个参数是 value
        System.out.println("第三个参数 = " + value);
        return true;
    }
}

使用 Filter 
String json = JSON.toJSONString(build, new MySerializeFilter());
```





## ParseProcess  定制反序列

Spring-boot 中使用 fastjson  作为 序列化使用

```java
package com.mybatisplus.mybatispractice.config;

import com.alibaba.fastjson.serializer.SerializerFeature;
import com.alibaba.fastjson.support.config.FastJsonConfig;
import com.alibaba.fastjson.support.spring.FastJsonHttpMessageConverter;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.MediaType;
import org.springframework.http.converter.HttpMessageConverter;
import org.springframework.web.servlet.config.annotation.EnableWebMvc;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;

@Configuration
@EnableWebMvc
public class WebConfig implements WebMvcConfigurer {
    @Override
    public void configureMessageConverters(List<HttpMessageConverter<?>> converters) {
        
        FastJsonHttpMessageConverter fastJsonConverter = new FastJsonHttpMessageConverter();
        FastJsonConfig config = new FastJsonConfig();
        config.setCharset(Charset.forName("UTF-8"));
        config.setDateFormat("yyyy-MM-dd HH:mm:ssS");
        //设置允许返回为null的属性
        config.setSerializerFeatures(SerializerFeature.WriteMapNullValue);
        fastJsonConverter.setFastJsonConfig(config);
        List<MediaType> list = new ArrayList<>();
        list.add(MediaType.APPLICATION_JSON_UTF8);
        list.add(MediaType.APPLICATION_XML);
        fastJsonConverter.setSupportedMediaTypes(list);

        converters.add(fastJsonConverter);
    }

}
```





## 支持JsonWriter

> 	对于文件流, FastJson处理起来也很方便. 

```java
@Test
public void testWriter() throws IOException {
    HashMap<String, Object> hashMap = new HashMap<>();
    hashMap.put("a", "1111");
    hashMap.put("b", "1234");
    hashMap.put("c", "1234");


    // 1. JSONWriter 配置启动
    JSONWriter writer = new JSONWriter(new FileWriter("test.json"));
    writer.config(SerializerFeature.WriteClassName, true);
    writer.startObject();

    Set<Map.Entry<String, Object>> entries = hashMap.entrySet();
    for (Map.Entry<String, Object> entry : entries) {
        writer.writeKey(entry.getKey());
        writer.writeValue(entry.getValue());
    }
    // 2.关闭, 释放资源
    writer.endObject();
    writer.close();
}
```

文件中会生成 : 

```json
{"a":"1111","b":"1234","c":"1234"}
```