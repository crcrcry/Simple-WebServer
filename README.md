# 一个轻量级web服务器
## 核心知识
1. 浏览器url回车时，会自动连接url:80的服务端socket
2. 字符串处理
    1. 解析http请求包
    2. 封装发回http响应包
3. 文件处理

## 目录说明
- /WebServer/WebServer：c语言代码
- /WebServer/WebServer.xcodeproj：xcode项目文件
- /Server_Root：浏览器根目录

## 排坑 
- 由于macbook无法使用相对路径进行c语言文件操作，代码中设置浏览器根目录使用的是绝对路径，所以在本机下需要先修改浏览器根目录路径才能正常使用
