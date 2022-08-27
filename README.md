# 简介
买不起DJI Action 2的我仿造了一台Action 2 Poor


# 目录说明
* doc 为项目文档、图片、部分芯片的数据手册等。
* firmware 为编译好的固件，直接烧录就可以运行，例如uboot、kernel、rootfs等。
* hardware 为硬件相关内容，包括原理图、PCB、外壳模型等。
* software 为软件源码。其中 app 内为各个可独立运行的程序，包括 hello world、camera、功能测试程序等；ext_lib 为第三方库如 lvgl、libmpeg等；hisi_sdk 为海思官方sdk文件；hisi_sample 为海思官方例程；utils 内为本工程的一些未分类的通用代码。


# 准备工作
1. 准备海思官方SDK，版本为 Hi3516EV200R001C01SPC012 （这东西是保密的，别找我、我没有、不知道。【疯狂暗示：某些购物软件的搜索功能比“众里寻他千XX”好用】）
2. 按照海思SDK中的说明安装交叉编译工具链 arm-himix100-linux
3. 如果你想要自己编译uboot、kernal、rootfs，请按照海思SDK中的相关文档搭建开发环境；如果不想编译上述内容，可以跳过这个步骤，直接用我提供的
4. 拷贝海思官方SDK内的相关文件，本工程用到以下文件：
	hisi_sdk 为海思官方sdk文件
	请自行拷贝以下文件
	Hi3516EV200R001C01SPC012.rar -> Hi3516EV200R001C01SPC012\01.software\board\Hi3516EV200_SDK_V1.0.1.2.tgz -> Hi3516EV200_SDK_V1.0.1.2\package\mpp.tgz -> mpp\
	这个目录下的 include, ko, lib 三个目录

	hisi_sample 为海思官方例程
	请自行拷贝以下文件
	Hi3516EV200R001C01SPC012.rar -> Hi3516EV200R001C01SPC012\01.software\board\Hi3516EV200_SDK_V1.0.1.2.tgz -> Hi3516EV200_SDK_V1.0.1.2\package\mpp.tgz -> mpp\sample\


# 编译说明
在 `software/CMakeLists.txt` 中设置海思交叉编译工具链路径
编译命令
```
mkdir ./build
cd ./build
cmake ../
make
```


# 烧写固件
firmware目录下为我编译好的镜像
u-boot-hi3516ev300.bin、kernel.bin、rootfs_uclibc_64k.jffs2 为独立镜像，须按地址独立烧写

独立镜像烧写地址
| 文件名                  | 地址       |
| ----------------------- | ---------- |
| u-boot-hi3516ev300.bin  | 0x00000000 |
| kernel.bin              | 0x00100000 |
| rootfs_uclibc_64k.jffs2 | 0x00500000 |

0-0x100000用于存放uboot和uboot环境变量
kernel和rootfs的烧写地址可以在0x100000以后灵活调整，如果修改了烧写地址，uboot的bootargs记得做相应修改

烧写方法灵活多样，flash内有uboot的可以直接用uboot命令烧写，没有uboot的（如不小心全片擦除、全新flash、flash内数据损坏等）flash可以用hisi sdk内的烧写工具或拆下flash用烧写器进行烧写。无论哪种方式只要能烧进去就行，具体操作步骤请自行百度或查阅hisi sdk内相关文档，这里不再赘述。


# 初次运行程序
**有嵌入式linux开发经验的可以自行操作，本章节内容仅供参考**

烧写完成后需设置uboot环境变量
输入以下命令设置环境变量，保存并重启，正常情况都会成功
```
setenv bootargs 'mem=32M console=ttyAMA0,115200 clk_ignore_unused rw root=/dev/mtdblock2 rootfstype=jffs2 mtdparts=hi_sfc:1M(u-boot.bin),4M(kernel),11M(rootfs.jffs2)'
setenv bootcmd 'sf probe 0; sf read 42000000 100000 400000; bootm 42000000'
saveenv
reset
```

**以挂载nfs并运行程序为例**
linux内核启动后输入以下命令启动网卡、设置IP、网关等
```
ifconfig lo 127.0.0.1
ifconfig eth0 192.168.1.2 netmask 255.255.255.0
route add default gw 192.168.1.1
```

在虚拟机中安装nfs server，设置有线网卡的ip为`192.168.1.3`，在开发板上挂载nfs（土豪可以用NAS之类的，普通人用linux虚拟机）
```
mount -t nfs -o nolock 192.168.1.3:/home/nfs /tmp/nfs
```

在nfs server的目录内准备上文提到的ko、lib目录，以及要运行的程序，输入以下命令加载ko
```
cd /tmp/nfs/ko
./load3516ev200 -i -sensor0 imx335
```
load3516ev200 脚本中的内存可以自行修改，但必须和bootargs的mem保持一致

输入以下命令设置环境变量
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tmp/nfs/lib
```

运行你的程序

懒人做法：上述必要的操作写在启动脚本里，lib和ko放在jffs文件系统内，一劳永逸

