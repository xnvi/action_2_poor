# 基本功能
udp推流验证视频系统能否正常工作

# 准备工作
电脑需准备ffplay用于播放h264裸流
ffplay如果在 ffmpeg.org 上面有可执行程序的就直接下载，没有的话就下载源码自己编译
有路由器的可以通过网线连接路由器
没有路由器的直连电脑网口

# 使用方法
设备端和PC端设置IP地址

例如
设备端设置如下
```
ifconfig lo 127.0.0.1
ifconfig eth0 192.168.1.99 netmask 255.255.255.0
route add default gw 192.168.1.1
```
PC端设置如下
```
ifconfig eth0 192.168.1.106 netmask 255.255.255.0
```

设备端运行程序

PC端输入以下命令观看h264视频
`./ffplay "udp://192.168.1.106:34543" -fflags nobuffer -flags low_delay -f h264 -threads 1 -thread_type frame -infbuf -an -sync video -fast -x 640 -y 360 -analyzeduration 100000 -framerate 30 -vf setpts=0.9*PTS`

PC端输入以下命令播放aac音频
`./ffplay "udp://192.168.1.106:34543" -fflags nobuffer -flags low_delay -f aac -threads 1 -thread_type frame -infbuf -vn -sync audio -fast -analyzeduration 100000`

简单解释几个参数
1. udp://192.168.1.106:34543 这个不用解释了吧
2. -x、-y是图像大小，如果电脑屏幕分辨率小于图像分辨率，你的鼠标会点不到关闭按钮，对于0基础的人来说就傻眼了
3. -analyzeduration 100000 码流探测的时间，单位微秒，可以设置在1秒以内，加快出图时间
4. -framerate 30 不用解释吧？填写实际帧率
5. -vf setpts=0.9*PTS 设置回放速率为真实速率的 10/9 倍，也就是略高于真实速率，一定程度上可以避免因为误差等原因导致延迟越来越大的问题，具体效果是不是有用也不好说，有时候不加这个参数也没问题

更多命令参数自行查阅ffplay文档
