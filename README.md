

# 简述
QT开发的一个window 记日记的小应用，版本是4  
与之前的版本3不同之处是，版本4新增了与linux 服务器的交互，  
使得此应用可以实现多用户的云端存储，支持同步更新。  
因此，把对应的服务器端应用部署到互联网上，那么理论上只要有网的  
地方，你就能就能获取日记。  

但是，我不得不说，由于整体的设计还是摸着石头过河，很多不完善的  
底层接口设计，而在分层、分模块、健壮性、调试等方面，也比较笼统的，  
即使我写完程序后，也会发现，有些细节的设置还是难以清晰明了。我现  
在只求将最简单、实用的功能实现。  



# 2020/12月12日 diary v4 版本的试用

总体感觉还行，达到了预期的效果，即实现了同步到云端、多用户区别处理  
这两个功能。但还是有不足之处，一个是本地的账户密码系统并没有很完善  
，没有对日记进行加密保护，任何人都可以查看；另一个是上传到云端，通  
常状况下的增、删、改没有问题，但是服务器运行一段时间后，曾出现过未  
知错误而宕机，也就是说不稳定。

# 2020/12月12日  代码说明：

文件夹：   
（1）linux：是服务器端的代码，编译时，先进入到此目录，使用 make 命令进行编译，编译出的目标文件是 obj/tcpserver  
（2）qt：客户端代码，用qt软件打开即可  
（3）si4：diary4-tcp.si4project 工程是用sourceInsight4 建立的以linux文件夹为基础的代码工程，方便管理、编辑服务器端的代码。  
（4）wkspace：工作空间，用户文件所在的地方  
（5）备份：即备份  



