### <distributable>:是否可分布式处理
	distributable 元素为空标签,它的存在与否可以指定站台是否可分布式处理.如果web.xml中出现这个元素,则代表站台在开发时已经
	被设计为能在多个JSP Container 之间分散执行.
	<!-- 配置集群的时候，要用到 -->
	<distributable/> 
