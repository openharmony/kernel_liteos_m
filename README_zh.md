# LiteOS-M内核<a name="ZH-CN_TOPIC_0000001096757661"></a>

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [约束](#section119744591305)
-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

OpenHarmony LiteOS-M内核是面向IoT领域构建的轻量级物联网操作系统内核，具有小体积、低功耗、高性能的特点，其代码结构简单，主要包括内核最小功能集、内核抽象层、可选组件以及工程目录等，分为硬件相关层以及硬件无关层，硬件相关层提供统一的HAL（Hardware Abstraction Layer）接口，提升硬件易适配性，不同编译工具链和芯片架构的组合分类，满足AIoT类型丰富的硬件和编译工具链的拓展。其架构图如图1所示：

**图 1**  OpenHarmony LiteOS-M核内核架构图<a name="fig0865152210223"></a>  
![](figures/OpenHarmony-LiteOS-M核内核架构图.png "OpenHarmony-LiteOS-M核内核架构图")

## 目录<a name="section161941989596"></a>

```
/kernel/liteos_m
├── components           # 可选组件
│   ├── cppsupport       # C++支持
│   └── cpup             # CPUP功能
├── kal                  # 内核抽象层
│   ├── cmsis            # cmsis标准接口支持
│   └── posix            # posix标准接口支持
├── kernel               # 内核最小功能集支持
│   ├── arch             # 内核指令架构层代码
│   │   ├── arm          # arm32架构的代码
│   │   └── include      # 对外接口存放目录
│   ├── include          # 对外接口存放目录
│   └── src              # 内核最小功能集源码
├── targets              # 板级工程目录
├── utils                # 通用公共目录
```

## 约束<a name="section119744591305"></a>

开发语言：C/C++；

适用架构：当前只适用于cortex-m3、cortex-m4、cortex-m7、risc-v芯片架构。

## 修改日志

v1.0.1
1. 删除以下KAL接口: `KalThreadGetInfo`,`KalDelayUs`,`KalTimerCreate`,`KalTimerStart`,`KalTimerChange`,`KalTimerStop`,`KalTimerDelete`,`KalTimerIsRunning`,`KalTickToMs`,`KalMsToTick`,`KalGetMemInfo`
2. 添加部分POSIX接口

v1.0
1. 首次发布

## 相关仓<a name="section1371113476307"></a>

[内核子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E5%86%85%E6%A0%B8%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

**kernel\_liteos\_m**

