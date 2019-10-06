## nodejs人脸比较RESTful 服务

### 实现方法
    整合dlib, opencv, boost写成一个nodejs native插件以实现人脸特征提取/比较，再通过express和socket.io对外提供服务

### 编译方法
    0. 安装Windows 64位系统，及vc编译器
    1. 先安装boost，opencv，dlib，再通过cmake-js或node-gyp编译，具体请参见各编译工具的官方文档。下面以node-gyp举例          
    可用vcpkg直接安装相关的C++依赖库，如：    
    vcpkg install boost:x64-windows-static dlib:x64-windows-static opencv:x64-windows-static

    2. 安装： nodejs, python（node-gyp需要），pkg（用以把node app打包成可执行文件），node-gyp
    3. 根据vcpkg的安装目录修改binding.gyp中.h, .lib的包含/链接路径
    4. 打开命令行cd进此代码目录，运行：node-gyp configure && node-gyp build      
    编译成功后的node插件在build/Release目录下
    5. 编写nodejs服务调用插件，及将其打包成发布版（可选）

###  简要说明
    demo的node代码中有个addon.startVideo()的调用，这个主要是演示用的（实际使用时可以不调用），这个方法是用opencv打开摄像头，对每帧的人脸特征进行描边（如果有人脸的话），然后把原始图片和描过边的图片，及人脸个数，这3个参数传入回调函数，这个回调函数再通过socket.io将这些内容广播至每个浏览器客户端，然后用<img>标签在网页上显示出来

    对外提供5个接口
    1、文字转语音服务
    2、通过图片获取人脸特征
    3、根据人脸特征比较是否为同一人
    4、根据图片和人脸特征比较是否为同一人
    5、根据两张图片比较是否为同一人

    实际上都是先从图片中提取特征再比较，提取特征很耗时，这个特征提取后可以存数据库或哪里，然后直接比较两个特征就很快。
    具体实现细节请自行参考代码。
    
**[服务接口说明](api.txt)**

***[demo下载](https://github.com/novice79/node_face/releases/download/1.0/dist.rar)***