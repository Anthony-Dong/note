## 书写规范

### 空格问题 

```css
.p-text(空格){
  color:(空格)#ff5a28;
  font-size:(空格)15px;
}
除了我标注空格的地方 其他不准有空格
是否是最后一个属性,都加分号;
```



## 选择器

### 并集选择器

```css
.t-color,
.p-color {
  color: #83ff3b
}
含义就是 t-color 和 p-color都是 这个颜色 , 中间用 ',' 隔开
```

### 子孙选择器

```css
.t-div li {
  color: #ff5a28
}

  <ul class="t-div">
    <li>儿子
      <ol>
        <li>曾孙</li>
        <li>曾孙</li>
      </ol>
    </li>
  </ul>
比如这个问题 , 他的 儿子 和 曾孙都会被 染色 . 所以只要是 子类就可以
```

### 亲儿子元素选择器

```css
.t-div li {
  color: #ffed49
}

.t-div>li {
  color: beige
}

这个需要注意的是,必须 有子孙选择器的前提下,才可以使用亲儿子选择器
```



## Text

### font 综合标签

```css
{font : font-style  font-weight font-size/font-hight font-family};

font-style : 字体风格 普通 ： normal ， 斜体 ： italic ， 倾斜 ： oblique
font-weight ： 字体粗细 ，  数字 400 等价于 normal，而 700 等价于 bold ，100~900数字之间。
font-size ： 字体大小 ，尽量使用px，同时是偶数， 20px
font-family ： 字体类型 ， 比如 "Times New Roman" ， 记得用引号引起来
font: italic bolder 20px "微软雅黑"
```

### color 文字颜色

主要分 ： 

1.预定义的颜色值，如red，green，blue等。

2.十六进制，如#FF0000，#FF6600，#29D794等。 最常用

3.RGB代码，如红色可以表示为rgb(255,0,0)或rgb(100%,0%,0%)。如果是百分号书写,百分号不能省略

### color: rgba(r,g,b,a) 颜色半透明

```css
color: rgba(216, 36, 36, 0.8)

color: rgba(r,g,b,a)  a 是alpha透明的意思,a也就是第四个参数,取值范围 0~1之间 
```

### text-shadow	文字阴影

```css
text-shadow: 1px 1px 1px rgba(216, 36, 36, 0.2)

第一个参数 : h-shadow 水平阴影位置 (必填)
第二个参数 : v-shadow 垂直阴影位置 (必填)
第三个参数 : blur 模糊距离 (必填)
第四个参数 : color 阴影颜色 (必填)
```

### line-height 行距

一般情况下，行距比字号大7.8像素左右就可以了。 包含文字像素的高度，27.8实际上是，文字大小20，行距7.8px

### text-align  水平对齐方式

默认是 left ， 其中包含 center ，right  ，

### text-indent  首行缩进

 1em 就是一个字的宽度 ， 如果是汉字的段落， 1em 就是一个汉字的宽度

### letter-spacing  文字间距/字母间距

其属性值可为不同单位的数值，允许使用负值，默认为normal。 

### word-spacing  单词间距

word-spacing属性用于定义英文单词之间的间距，对中文字符无效 ， word-spacing和letter-spacing均可对英文进行设置。不同的是letter-spacing定义的为字母之间的间距，而word-spacing定义的为英文单词之间的间距。

### 伪元素选择器

E::first-letter : 表示第一个文字的颜色

E::first-line : 表示第一行文字的颜色

E::selection : 选中文字,指的是鼠标拖动选中文字的颜色

```css
.p-text {
  color: #ff5a28;
  font-size: 15px
}

.p-text::first-letter {
  color: #4599ff
}

首字母会被染成 #4599ff这个颜色 ,覆盖原来的


.p-text::first-line {
  color: #4599ff
}

第一行会被染色

.p-text::selection {
  color: greenyellow
}

表示鼠标选中的文字的颜色
```

**E::before和E::after**  表示在盒子的内部的前面和后面添加content , 但是添加的内容不能选中

```css
.p-text::before {
  content: "hello"
}

.p-text::after {
  content: "bye"
}
```



### 文字凸起 和 凹下

```css
<style>
    div {
        background: rgb(139, 134, 127);
        color: rgb(139, 134, 127);
        font: normal 700 100px "微软雅黑";
    }

    div:first-child {
        text-shadow: 1px 1px 2px black, -1px -1px 2px white
    }

    div:last-child {
        text-shadow: -1px -1px 1px black, 1px 1px 1px white
    }
</style>

<div class="text-bg">
    <div>凸起</div>
    <div>凹下的</div>
</div>
```





## link 标签 

### 伪类选择器

常给a标签使用

```css
.a_link:link {
    color: black
}

.a_link:visited {
    color: azure
}

.a_link:hover {
    color: red
}

.a_link:active {
    color: yellow
}

顺序不能变 : 
```

常用写法

```css
.a_link {
    color: black
}

.a_link:hover {
    color: red
}
```



### li 标签的 伪类选择器

```css
li:first-child {
    color: salmon
}

li:last-child {
    color: aquamarine
}

li:nth-child(2) {
    color: chartreuse
}
li:nth-child(odd) {
    color: chartreuse
}

odd  奇数 
even 偶数
2n  也是 偶数
```



### : target 伪类选择器

```css
:target {
    color: cadetblue
}

//  #1 代表 #1这个 id 选择器 , 这属于前端路由
 <a href="#1" class="a_link">AAA</a><br>

<div>
    <h3 id="1">AAA</h3>
    <hr>
    <ul class="list_c">
        <li>SON</li>
    </ul>
</div> 	  
```



## 背景

### 背景图片

```css
.container {
    background-color: beige;
    width: 1500px;
    height: 1000px;
    /* 路径,相对路径就行 */
    background-image: url(image/background.jpg);
    /* 默认是平铺 */
    background-repeat: no-repeat;
    /* 只写一个 位置,另一个默认是center */
    background-position: center center;
    /* 图片不随滚动条滚动 */
    background-attachment: fixed;
    background-clip: border-box;
}
```

```css
// 
background:背景颜色 背景图片地址 背景平铺 背景滚动 背景位置
background 属性的值的书写顺序官方并没有强制标准的。为了可读性，建议大家如下写：

background: transparent url(image.jpg) repeat-y  scroll 50% 0 ;

// 
background-size: 500px  背景图片大小, 一般是只设置一个值就行,会等比例缩放

```



多背景图片 , 多背景图片,最后在一个背景再写颜色

```shell
background-image: url('images/gyt.jpg'),url('images/robot.png');
```



### 背景颜色

```css
background: rgba(238, 17, 17, 0.1);
```

 注意背景半透明是指盒子背景半透明， 盒子里面的内容不收影响。

主要是做,比如说我有一张背景图片,然后我需要一个下拉栏,又不想影响到效果,此时就是需要一个背景颜色



## 盒子模型

### border

```css
border : border-width || border-style || border-color 

 border: 5px solid red;
边框宽度为 5px  , 单实线 ,红色 
```





