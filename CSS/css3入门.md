## font 综合属性

```css
{font : font-style  font-weight font-size/font-hight font-family};

font: italic bolder 20px "微软雅黑"
```

## link 标签 

### 伪类选择器

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

