## Win10下QT+NCNN实现Android开发的踩坑记录

**Xu Jing**


### 1.我为什么选择QT和NCNN？

作为算法工程师，比较惭愧的告诉大家我对JAVA不是很熟悉，对android和ios等移动端设备的开发也知之甚少，我有过的经验就是用Python的Kivy开发过若干Android程序，
用QT5的C++和Python版本开发过Windows和Linux桌面程序，同时研究过很多很多深度学习相关的算法，对深度学习模型的部署有一些研究心得，NCNN号称是在在移动端推断速度最快
的开源框架，我很想尝试一下。
为了尝试NCNN在移动端的部署，查了一圈资料基本都是Java+NCNN的栗子。没办法，我只能选择QT+NCNN。

+ 该项目的代码和编译的配置环境我将完全开源，希望对一些像我一样的小伙伴有帮助
+ 手摸手从零开始的环境搭建，干净又卫生


------

#TODO

### 2.环境搭建

+ 1.QT

+ JDK

+ android NDK

+ android JDK

+ opencv android


+ ncnn vs2019编译

+ ncnn 安卓编译




### 3. QT编译环境配置

+ QT android环境配置

+ 为什么选择QT5.12.4和NDK21


+ QT + Opencv Android环境配置


+ 为什么没有选择 opencv-mobile


### 4. 你以为这样就OK了吗？

+ 问题1： opencv和ncnn的问题

+ 问题2： assets导致的模型加载不了

+ 问题3：libomp.so的问题

+ 问题4： ...


### 5.关于代码部分的一些说明

+ ncnn模型部分不应该这样应该有个模型类实现并在程序开启时加载模型

+ 没有使用Android GPU和Valkan，编译部分为了简单起见我都关闭掉了

+ 怎样解决像YOLO v5或YOLOX这种调用自定义层的方式

+ 为什么没有选择qml


### 6.吸收了哪些大神们的精华

+ nihui的知乎

+ 知乎大佬多ncnn的总结

+ ncnn的官方repo

+ ncnn的模型仓库

+ ncnn的编译

+ QT+ opencv android的配置

+ QT+ncnn的配置


+ QT安卓环境配置

+ QT opencv环境配置

+ QT 修改android程序的图标

+ QT打包文件到安卓程序

+ QT打开安卓相册

+ QT打开adroid摄像头

+ libomp。so的问题解决

+ assets问题解决


### 7. `F**K ` QT + NCNN终于成功在小米手机上跑起来了


+ 打开相册进行识别


+ 打开摄像头进行识别


### 8. APK下载


### 9.最后想说点什么


+ 关于ncnn

+ 关于QT

+ 一个AI图像算法工程师为什么要折腾这个？







