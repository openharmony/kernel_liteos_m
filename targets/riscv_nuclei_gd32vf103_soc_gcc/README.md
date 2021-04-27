# RV-STAR开发板LiteOS使用说明

## RV-STAR开发板简介

RV-STAR是一款基于GD32VF103 MCU的RISC-V评估开发板，提供了板载调试器、Reset和Wakeup用户按键、RGB LED、USB OTG，以及EXMC、Arduino和PMOD扩展接口等资源。

开发板资料链接：

- [RV-STAR开发板电路原理图](https://www.rvmcu.com/quickstart-doc-u-pdf-id-235.html)
- [RV-STAR开发板用户手册（Segger Embedded Studio IDE版）](https://www.rvmcu.com/quickstart-doc-u-pdf-id-236.html)
- [RV-STAR开发板用户手册（Nuclei Studio IDE版）](https://www.rvmcu.com/quickstart-doc-u-pdf-id-531.html)
- [RV-STAR板载GD32VF103芯片资料](https://www.rvmcu.com/quickstart-doc-u-gd32vf103.html)
- [芯来RISC-V Bumblebee内核指令集手册](https://www.rvmcu.com/quickstart-doc-u-pdf-id-8.html)

## 文件结构

```
├── components                            # 可选组件
│   ├── cppsupport                        # C++支持
│   └── cpup                              # CPUP功能
├── kal                                   # 内核抽象层
│   └── posix                             # posix标准接口支持
├── kernel                                # 内核最小功能集支持
│   ├── arch                              # 内核指令架构层代码
│   │   ├── risc-v                        # risc-v架构的代码
│   │   │   └── nuclei                    # nuclei内核相关代码
│   │   │       └── gcc                   # gcc编译器相关代码
│   │   │           ├── nmsis             # nmsis内核标准
│   │   └── include                       # 对外接口存放目录
│   │       ├── los_arch_atomic.h         # 定义通用arch的原子操作
│   │       ├── los_arch_context.h        # 定义通用arch的上下文切换
│   │       ├── los_arch.h                # 定义通用arch初始化
│   │       └── los_arch_interrupt.h      # 定义通用arch中断
│   ├── include                           # 对外接口存放目录
│   │   ├── los_config.h                  # 功能开关和配置参数
│   │   ├── los_event.h                   # 事件
│   │   ├── los_liteos.h                  # liteos最小功能集对外提供的头文件
│   │   ├── los_memory.h                  # 堆内存管理
│   │   ├── los_mutex.h                   # 互斥锁
│   │   ├── los_queue.h                   # 队列
│   │   ├── los_scheduler.h               # 调度算法
│   │   ├── los_sem.h                     # 信号量
│   │   ├── los_task.h                    # 任务
│   │   └── los_timer.h                   # 定时器
│   └── src                               # 内核最小功能集源码
├── targets                               # 板级工程目录
│   ├── riscv_nuclei_gd32vf103_soc_gcc    # RV-STAR开发板相关代码
│       ├── GCC                           # 编译相关
│       ├── OS_CONFIG                     # 开发板配置功能开关和配置参数
│       ├── SoC                           # gd32vf103 SOC相关代码
│       └── Src                           # application相关代码
├── utils                                 # 通用公共目录
    ├── include
    │   ├── los_compiler.h                # 编译工具配置，类型定义
    │   ├── los_debug.h                   # debug，printf相关
    │   ├── los_error.h                   # 错误定义
    │   └── los_list.h
    └── src
```

## 使用说明

软件需求：linux环境

硬件需求：RV-STAR开发板

[环境配置](#sectionb1)

[编译源码](#sectionb2)

[下载调试、运行](#sectionb3)

本示例将新建并运行两个任务，PA0按键为外部中断，按下后可以查看任务列表信息。

### 环境配置<a name="sectionb1"></a>

- **工具链配置**

请先确认您使用的是centos系统或Ububntu 64bit。

1. 新建一个`Nuclei` 文件夹，比如`~/Software/Nuclei`
2. 参考下图，从[Nuclei Download Center](https://nucleisys.com/download.php)下载工具链和OpenOCD。
   - CentOS或Ubuntu系统请点击图中红框1下载RISC-V GNU工具链
   - 点击图中蓝框2-1下载64bit的OpenOCD
   - **确保Make工具版本不低于3.82**: ubuntu系统使用`sudo apt-get install make`指令安装`make`工具, CentOS系统使用`sudo yum install make`指令安装。

图1 Linux环境要下载的Nuclei Tools

![Nuclei Tools need to be downloaded for Linux](doc/image/nuclei_tools_download_linux.png)



3. 在之前新建的`Nuclei`文件夹中新建`gcc`文件夹和`openocd`文件夹。
   - 解压缩之前下载的**gnu工具链**到任意文件夹中，复制其中`bin`文件件所在层级的所有内容到`gcc`文件夹中。
   - 同样解压缩之前下载的**OpenOCD**到任意文件夹中，复制其中`bin`文件件所在层级的所有内容到`openocd`文件夹中。

> 注意：
>
> ​	请务必下载并解压缩Linux版本的工具，不要下载windows版本工具。

- **驱动配置**

驱动配置步骤如下：

1. 连接开发板到Linux中，确保USB被Linux识别出来。

2. 在控制台中使用lsusb指令查看信息，参考的打印信息如下：

   ```
   Bus 001 Device 010: ID 0403:6010 Future Technology Devices International, Ltd FT2232xxxx
   ```

3. 将github（https://github.com/riscv-mcu/ses_nuclei_sdk_projects/blob/master/misc/99-openocd.rules）上misc文件夹内99-openocd.rules文件复制到当前路径下，控制台中输入sudo cp 99-openocd.rules /etc/udev/rules.d/99-openocd.rules指令复制文件到指定路径下。

4. 断开调试器再重新连接到Linux系统中。

5. 使用ls /dev/ttyUSB*命令查看ttyUSB信息，参考输出如下：

   ```
   /dev/ttyUSB0 /dev/ttyUSB1
   ```

6. 使用ls -l /dev/ttyUSB1命令查看分组信息，参考输出如下: 

   ```
   crw-rw-r-- 1 root plugdev 188, 1 Nov 28 12:53 /dev/ttyUSB1
   ```

   可以看到ttyUSB1已经加入plugdev组，接下来我们要将自己添加到plugdev组。使用whoami命令查看当前用户名，我们将其记录为\< your_user_name >。

7. 使用sudo usermod -a -G plugdev \<your_user_name>命令将自己添加进plugdev组。

8. 再次确认当前用户名已属于plugdev组，使用groups命令，可以看到打印信息中有plugdev即成功将当前用户添加至plugdev组。

### 编译源码<a name="sectionb2"></a>

使用`git clone`复制代码到任意目录下，打开进入到工程根目录下，输入`git submodule update --init --recursive`下载更新子模块。

编译前请在当前控制台中配置`NUCLEI_TOOL_ROOT`路径，假设`Nuclei`文件夹所在路径为`/home/Nuclei`，输入`export NUCLEI_TOOL_ROOT=/home/Nuclei` 。或者使用时make选项增加`NUCLEI_TOOL_ROOT=/home/Nuclei`。

配置路径后打开至代码根目录下的/target/riscv_nuclei_gd32vf103_soc_gcc/GCC位置，输入如下指令开始编译：

```
make all
```

编译结束后部分参考输出如下：

```
   text    data     bss     dec     hex filename
  24330     112   16768   41210    a0fa build/Nuclei-rvstar-gd32vf103-soc.elf
```

若编译前想清理工程，请使用如下指令：

```
make clean
```

### 下载调试、运行<a name="sectionb3"></a>

调试或运行前请先连接RV-STAR开发板，确保已按照[环境配置](#sectionb1)中驱动配置部分配置完成。

同样配置好`NUCLEI_TOOL_ROOT`路径并打开至代码根目录下的/target/riscv_nuclei_gd32vf103_soc_gcc/GCC位置，输入如下指令进入GDB调试：

```
make debug
```

等待到进入GDB调试界面时，输入`load`指令下载编译好的elf文件，就可以开始调试。

若想直接运行，请在调试时所在位置输入如下指令：

```
make upload
```

运行时可以查看串口打印内容，使用串口查看工具，这里以`minicom`为例，若未安装此工具可自行安装或使用其他串口查看工具。打开控制台，输入`minicom -D /dev/ttyUSB1 -b 115200`指令打开串口查看工具。

运行时参考输出如下：

```
Nuclei SDK Build Time: Mar 30 2021, 18:26:53
Download Mode: FLASHXIP
CPU Frequency 108540000 Hz
entering kernel init...
TaskSampleEntry1 running...
TaskSampleEntry1 running...
TaskSampleEntry1 running...
TaskSampleEntry1 running...
TaskSampleEntry2 running...
TaskSampleEntry1 running...
TaskSampleEntry1 running...
EXTI0_IRQHandler running...
ID  Pri    Status     name 
--  ---    ---------  ----
0    0     QueuePend      Swt_Task 
1    31     Running      IdleCore000 
2    6     Delay      TaskSampleEntry1 
3    7     Delay      TaskSampleEntry2 
TaskSampleEntry1 running...
TaskSampleEntry1 running...
TaskSampleEntry1 running...
TaskSampleEntry2 running...
```
