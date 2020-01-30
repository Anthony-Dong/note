# CURL参数讲解

## GET

`curl -X GET "http://localhost:8080/user/get"` 和  `curl "http://localhost:8080/user/get"`  一样



## POST 

-d 加请求参数 

-H  请求头

-v 显示请求过程

-F  上传文件   `-F "file=@/Users/fungleo/Downloads/401.png"`



`curl -X POST "http://localhost:8080/file" -F "file=@/home/user/log4j.properties" -H "Content-Type: multipart/form-data"`