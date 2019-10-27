
### 样式

### 阑珊式布局

	必须放在 container 容器内 
	必须放在 row标签内
	一共 12列;根据 -* 做除法
	class属性 :
	.col-md-4  比如定义三个在电脑上可以显示3列 ;但是手机等设备之间奔1列了
	.col-xs-6 可以设置 手机上显示 两列
	.col-sm-*  平板设备;一般不考虑了
	这三个属性 都可以在电脑上展示正确;但是 xs可以兼顾手机电脑平板设备;sm只能兼顾平板
	但是 一般会设置 sx 和 md  兼顾电脑 和手机 


	.lead  文字突出显示
	<mark> 背景颜色
	.test-uppercase  文字大写
	.text-lowercase 文字小写
	<address>  地址
	<ul class="list-unstyled">  无样式的list标签
	<strong>  文字加粗
	<mark> 带有标记的文本
	<ins> 带有下划线的文本
	<u> 带有下划线的文本
	<s>  无用的文本
	<small> 小号文字
	<strong>  文字加粗(font-weigth)
	<em>  斜体字
	.text-capitalize  首字母大写
	.text-info 文字是浅色蓝色
	.text-success  文字是浅绿色
	.text-danger 文字是红色
	.text-justify  文字反正是 我也不知道做啥了
	.text-nowrap  文件禁止换行
	.text-warning  文字警告
	<abbr title="">  基本缩略语  就是解释说明 ,标签内部写文字;title设置隐藏文字信息
	<cite title="Source Title">source title</cite>  这个标签 点上去 会有文字提示
	<address> 人物地址标签
	<a href="mailto:#">first.last@example.com</a>  mailto 可以打开邮箱地址
	<blockquote> <footer>  块引用;比如名人名言 ;footer 必须在 blockquote里面
	<ul class="list-unstyled">  list集合 无样式 
	.list-inline 内联样式
	&lt;  <
	&gt;  >
	<code>  包裹代码块
	.active	鼠标悬停在行或单元格上时所设置的颜色
	.success	标识成功或积极的动作
	.info	标识普通的提示信息或动作
	.warning	标识警告或需要用户注意
	.danger	标识危险或潜在的带来负面影响的动作

### form 表单的基本写法  form 第一必须在 container 里面
####所有设置了 .form-control 类的 <input>、<textarea> 和 <select> 元素都将被默认设置宽度属性为 width: 100%;。 将 label 元素和前面提到的控件包裹在 .form-group 中可以获得最好的排列。
	form-inline  .form-horizontal
	<form class="form-inline">
	  <div class="form-group">
	    <label class="sr-only" for="exampleInputEmail3">Email address</label>
	    <input type="email" class="form-control" id="exampleInputEmail3" placeholder="Email">
	  </div>
	  <div class="form-group">
	    <label class="sr-only" for="exampleInputPassword3">Password</label>
	    <input type="password" class="form-control" id="exampleInputPassword3" placeholder="Password">
	  </div>
	  <div class="checkbox">
	    <label>
	      <input type="checkbox"> Remember me
	    </label>
	  </div>
	  <button type="submit" class="btn btn-default">Sign in</button>
	</form>