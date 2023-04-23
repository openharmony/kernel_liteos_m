#  (2022-03-30)


### Bug Fixes

* c-sky等芯片架构的BUILD.gn优化 ([3311d73](https://gitee.com/openharmony/kernel_liteos_m/commits/3311d73406b68d289b6f715a099e90f62c86abd1)), closes [#I4D129](https://gitee.com/openharmony/kernel_liteos_m/issues/I4D129)
* change the sensitive words ([8e2248c](https://gitee.com/openharmony/kernel_liteos_m/commits/8e2248c062aaca2444cc1001873fe8e8d8cdd3c6)), closes [#I4K7LL](https://gitee.com/openharmony/kernel_liteos_m/issues/I4K7LL)
* cortex-m55栈优化 ([59e9c6e](https://gitee.com/openharmony/kernel_liteos_m/commits/59e9c6ed73caeeed56081a768198eccc7448df6c)), closes [#I4SQIQ](https://gitee.com/openharmony/kernel_liteos_m/issues/I4SQIQ)
* csky等新增模块的HalTickStart函数定义和实现不一致 ([07ab6a5](https://gitee.com/openharmony/kernel_liteos_m/commits/07ab6a5b77c6955f6093beeab976bdc1b32d1db9)), closes [#I4D1HY](https://gitee.com/openharmony/kernel_liteos_m/issues/I4D1HY)
* FD_SETSIZE compatible newlibc ([82c6d5f](https://gitee.com/openharmony/kernel_liteos_m/commits/82c6d5f17478fb47ea5977a32fb76d783ee3e815))
* fix compile warning saying dereference void * pointer ([29126a9](https://gitee.com/openharmony/kernel_liteos_m/commits/29126a98fe7a8f75695910bdf8fa1ccc6ff67288))
* gcc编译testsuite编译问题修复 ([f7d26b8](https://gitee.com/openharmony/kernel_liteos_m/commits/f7d26b8e12fb1ba4bebe6c0fc9ca9e392537419f)), closes [#I4DAOU](https://gitee.com/openharmony/kernel_liteos_m/issues/I4DAOU)
* Kconfig中打开littlefs选项默认关联打开LOSCFG_SUPPORT_LITTLEFS宏 ([b2bcab3](https://gitee.com/openharmony/kernel_liteos_m/commits/b2bcab3a5bd794c86dd4d93b1b5aeca2d097289b)), closes [#I4ENQ1](https://gitee.com/openharmony/kernel_liteos_m/issues/I4ENQ1)
* kernel接口融合，添加/修改kernel函数 ([cee9714](https://gitee.com/openharmony/kernel_liteos_m/commits/cee9714a90f2660a0b4455f4f5a2fc42870a6aa5))
* L0 pthread_cond_timedwait接口实现存在的几个问题 ([2219c32](https://gitee.com/openharmony/kernel_liteos_m/commits/2219c32784071b6c37ea57185382e3228ed4b0a7)), closes [#I4N9P8](https://gitee.com/openharmony/kernel_liteos_m/issues/I4N9P8)
* liteos_kernel_only=true编译内核报错 ([adc2105](https://gitee.com/openharmony/kernel_liteos_m/commits/adc21052b2b9b55de9fc493cf74645f470edf9b5)), closes [#I4O204](https://gitee.com/openharmony/kernel_liteos_m/issues/I4O204)
* los_exc.S中_ExcInMsp主栈中异常处理函数中异常返回类型错误 ([c51df7a](https://gitee.com/openharmony/kernel_liteos_m/commits/c51df7a94c1f21803db25f46a73a7fb291588eda)), closes [#I4EJI4](https://gitee.com/openharmony/kernel_liteos_m/issues/I4EJI4)
* los_interrupt.c存在未使用的参数编译出错 ([5af4c2e](https://gitee.com/openharmony/kernel_liteos_m/commits/5af4c2e2139cabe49049283f7a4280460bbf353a)), closes [#I4SJPZ](https://gitee.com/openharmony/kernel_liteos_m/issues/I4SJPZ)
* LOS_Panic接口中增加需要增加LOS_BackTrace()，方便定位主动异常位置 ([f51d675](https://gitee.com/openharmony/kernel_liteos_m/commits/f51d675fee217e0ad519f2fa79f70b033f313dce)), closes [#I4IGGY](https://gitee.com/openharmony/kernel_liteos_m/issues/I4IGGY)
* los_trace.h接口注释错误修正 ([693468e](https://gitee.com/openharmony/kernel_liteos_m/commits/693468e5274af310721f5962602a915d701a7bf0)), closes #I4CYPZ
* m4核在任务中异常时backtrace使用的是MSP所在的栈，而不是PSP所在的栈 ([3e8aea2](https://gitee.com/openharmony/kernel_liteos_m/commits/3e8aea224e481322efef9397e4aafc3450a09d28)), closes [#I4D7GE](https://gitee.com/openharmony/kernel_liteos_m/issues/I4D7GE)
* M核posix接口fs模块用例完善 ([ee380fb](https://gitee.com/openharmony/kernel_liteos_m/commits/ee380fb9be45ef033979ec3b8875d980b7f13198)), closes [#I4L7BF](https://gitee.com/openharmony/kernel_liteos_m/issues/I4L7BF)
* optimize default configuration ([b2e3849](https://gitee.com/openharmony/kernel_liteos_m/commits/b2e3849bfbd470981602b0dc02f47bffb38bfe11)), closes [#I4FSC9](https://gitee.com/openharmony/kernel_liteos_m/issues/I4FSC9)
* OsGetAllTskInfo调用和异常时无任务回调函数地址 ([1c605a3](https://gitee.com/openharmony/kernel_liteos_m/commits/1c605a338ad2317aeb464bdac85df5b9a79c99fd)), closes [#I4MG2T](https://gitee.com/openharmony/kernel_liteos_m/issues/I4MG2T)
* pm codex告警清零 ([9666380](https://gitee.com/openharmony/kernel_liteos_m/commits/96663805ecfd3248e33b4deb877c8d55cc4e8eec)), closes [#I4OWPE](https://gitee.com/openharmony/kernel_liteos_m/issues/I4OWPE)
* pm模块解冻线程时存在删除空链表且时间片频繁唤醒系统 ([9f185b5](https://gitee.com/openharmony/kernel_liteos_m/commits/9f185b5b52ed9506a4913f233c9ddfe45452ec05)), closes [#I4AKUS](https://gitee.com/openharmony/kernel_liteos_m/issues/I4AKUS)
* **posix unittest:** posix unittest 融合 ([ad6f249](https://gitee.com/openharmony/kernel_liteos_m/commits/ad6f249d1ea878f06864d79bdbed772b21eeac49))
* **posix:** posix兼容回退 ([207efc1](https://gitee.com/openharmony/kernel_liteos_m/commits/207efc15b859fe18d8b1dac2a3da9d7a5831aa6e))
* **posix:** posix接口融合，接口规范兼容性修改 ([e0b1275](https://gitee.com/openharmony/kernel_liteos_m/commits/e0b12758da4a25e42110ba32da25b6d18ebed8ef))
* **posix接口融合:** posix接口融合，接口规范兼容性修改 ([4b607fd](https://gitee.com/openharmony/kernel_liteos_m/commits/4b607fd0740a5379cd373890482235845d782b62))
* posix线程和LOS_TaskCreate任务不兼容，补齐接口防护，防止访问野指针 ([60805e1](https://gitee.com/openharmony/kernel_liteos_m/commits/60805e1a7c3736db86695ed86cd03cfaab70a427))
* pr模板补充说明 ([7b86f69](https://gitee.com/openharmony/kernel_liteos_m/commits/7b86f69cc0ccd5512b69fd3a1181b3440747ddb2))
* ReadMe中无对动态加载模块待加载的共享库的限制说明。 ([f86b971](https://gitee.com/openharmony/kernel_liteos_m/commits/f86b97180f97903bb2def271b7b11fc1ae1bf9bb)), closes [#I4ICK8](https://gitee.com/openharmony/kernel_liteos_m/issues/I4ICK8)
* shell 输入不识别命令时内存泄漏 ([0245b1a](https://gitee.com/openharmony/kernel_liteos_m/commits/0245b1a7b24fd36d23f9602d0755b274581a84ce)), closes [#I4UR9P](https://gitee.com/openharmony/kernel_liteos_m/issues/I4UR9P)
* **shell:** cat针对大文件无法工作 ([d518bad](https://gitee.com/openharmony/kernel_liteos_m/commits/d518bad0dc7dd9e3fbb45423a7db5cb8c1c922dc)), closes [#I4J4TK](https://gitee.com/openharmony/kernel_liteos_m/issues/I4J4TK)
* shell模块task命令踩内存问题修改 ([6ffd55d](https://gitee.com/openharmony/kernel_liteos_m/commits/6ffd55dcd07ce17b693a97bb678668f763480e4d)), closes [#I4A70V](https://gitee.com/openharmony/kernel_liteos_m/issues/I4A70V)
* switch inet_addr to a function ([b30e913](https://gitee.com/openharmony/kernel_liteos_m/commits/b30e9139d5502acab8c36ae9a56d4cd640786c44)), closes [#I4G4I4](https://gitee.com/openharmony/kernel_liteos_m/issues/I4G4I4)
* **testsuites:** cpup和测试套解耦 ([692651f](https://gitee.com/openharmony/kernel_liteos_m/commits/692651fe40f668f19b479186e52fea34d092b458))
* tick timer时钟频率赋值修改为运行时赋值 ([ae02afc](https://gitee.com/openharmony/kernel_liteos_m/commits/ae02afc8504f64ff6bc09828f55e9602bcbcae56)), closes [#I4PGUR](https://gitee.com/openharmony/kernel_liteos_m/issues/I4PGUR)
* tick中断处理时，无挂起任务时应更新当前运行任务的时间片, 并触发一次调度 ([ea12d40](https://gitee.com/openharmony/kernel_liteos_m/commits/ea12d40caf1301b4a7b6d60a2c599ceff4746c90)), closes [#I4LRZG](https://gitee.com/openharmony/kernel_liteos_m/issues/I4LRZG)
* **time_func_test_01.c:** 修复testTimes测试单板配置LOSCFG_BASE_CORE_TICK_PER_SECOND = 1000的问题 ([5c982f1](https://gitee.com/openharmony/kernel_liteos_m/commits/5c982f1087f95174f2836c72648eab58adf37fd7))
* 中断向量表对齐大小支持可配置 ([cf9c838](https://gitee.com/openharmony/kernel_liteos_m/commits/cf9c8387e3785d994aacf948b7c1b61324a3dd63)), closes [#I4M20E](https://gitee.com/openharmony/kernel_liteos_m/issues/I4M20E)
* 优化trace buffer初始化，删除swtmr 桩中的无效参数 ([afbc9a5](https://gitee.com/openharmony/kernel_liteos_m/commits/afbc9a593b61461072556cb5531d90b30ce2e394)), closes [#I4DPR7](https://gitee.com/openharmony/kernel_liteos_m/issues/I4DPR7)
* 优化低功耗流程 ([c6600d9](https://gitee.com/openharmony/kernel_liteos_m/commits/c6600d9dddb666f397ba659d47d64f334691af6b)), closes [#I46VXK](https://gitee.com/openharmony/kernel_liteos_m/issues/I46VXK)
* 修复arch目录调整影响到的target目录下的文件 ([d5725b2](https://gitee.com/openharmony/kernel_liteos_m/commits/d5725b22644da8ab3e719169230303e44c7d8750)), closes [#I4JM7Z](https://gitee.com/openharmony/kernel_liteos_m/issues/I4JM7Z)
* 修复cortex-m系列系统提供的timer在低频下时间不准的问题 ([3e569ba](https://gitee.com/openharmony/kernel_liteos_m/commits/3e569bac587d90dcd21aff7d8ec53216dfbb8f1c)), closes [#I4HBGR](https://gitee.com/openharmony/kernel_liteos_m/issues/I4HBGR)
* 修复hook层LOS_HOOK_TYPE_SEM_PEND的runningTask入参可能为空 ([3fd22ac](https://gitee.com/openharmony/kernel_liteos_m/commits/3fd22ac2b0b4c690669a659246aba099fd4311ec)), closes [#I4BU4P](https://gitee.com/openharmony/kernel_liteos_m/issues/I4BU4P)
* 修复irqNum = HwiNumGet()编译错误的问题 ([16e87d7](https://gitee.com/openharmony/kernel_liteos_m/commits/16e87d78d89bec7d85c076fc738ed4ad0614b865)), closes [#I4S81B](https://gitee.com/openharmony/kernel_liteos_m/issues/I4S81B)
* 修复Kconfig信息错误 ([bb04582](https://gitee.com/openharmony/kernel_liteos_m/commits/bb045829104211522cd12a7273d5853400c1ea26)), closes [#I4MZ1F](https://gitee.com/openharmony/kernel_liteos_m/issues/I4MZ1F)
* 修复L0_Emulator 门禁高概率失败的问题 ([7ad2f62](https://gitee.com/openharmony/kernel_liteos_m/commits/7ad2f62a03481a3919fef1f1e46e4c9677b1e65c)), closes [#I4NB0N](https://gitee.com/openharmony/kernel_liteos_m/issues/I4NB0N)
* 修复liteos-m在iar环境下的编译问题 ([4c4784e](https://gitee.com/openharmony/kernel_liteos_m/commits/4c4784e33d7944e7850009be192375aaa6e2b125)), closes [#I4Q5Q5](https://gitee.com/openharmony/kernel_liteos_m/issues/I4Q5Q5)
* 修复los_interrupt.c文件中HalHwiInit函数里未使能未对齐异常问题 ([d32e25f](https://gitee.com/openharmony/kernel_liteos_m/commits/d32e25f3ae8e8e5ec24effa98dc8f0b3ba94d0c1)), closes [#I4EJGR](https://gitee.com/openharmony/kernel_liteos_m/issues/I4EJGR)
* 修复newlib缺省PTHREAD_KEYS_MAX 问题 ([65f04bf](https://gitee.com/openharmony/kernel_liteos_m/commits/65f04bf093376a478e9585dadfc1238158cc3641)), closes [#I4OX47](https://gitee.com/openharmony/kernel_liteos_m/issues/I4OX47)
* 修复OsQueueMailFree提早释放内存的问题 ([ec88d16](https://gitee.com/openharmony/kernel_liteos_m/commits/ec88d1628937b3def6dd613246116f6bfa4141eb))
* 修复pm编译告警导致内核编译失败 ([18b0524](https://gitee.com/openharmony/kernel_liteos_m/commits/18b052458e1cea113df7a6bb4798a6d5e1eb751e)), closes [#I49MIN](https://gitee.com/openharmony/kernel_liteos_m/issues/I49MIN)
* 修复pthread 编译告警 ([cc57f81](https://gitee.com/openharmony/kernel_liteos_m/commits/cc57f81ab85a5eb9bb4be9bd7d2863978996bcf3)), closes [#I4U16U](https://gitee.com/openharmony/kernel_liteos_m/issues/I4U16U)
* 修复pthread_create相关问题 ([cd949dd](https://gitee.com/openharmony/kernel_liteos_m/commits/cd949ddae0aa4e7291b0b7f6ba83c2034cdf1301)), closes [#I4P78J](https://gitee.com/openharmony/kernel_liteos_m/issues/I4P78J)
* 修复pthread_create行为与posix不一致问题 ([2b1e5a7](https://gitee.com/openharmony/kernel_liteos_m/commits/2b1e5a7a130fbca1b86a787b54f474e183541d6d)), closes [#I49W9F](https://gitee.com/openharmony/kernel_liteos_m/issues/I49W9F)
* 修复readme中文档链接错误 ([8ee70a0](https://gitee.com/openharmony/kernel_liteos_m/commits/8ee70a0f8f856da29139a36ba6f8589201cdb109)), closes [#I4FD5G](https://gitee.com/openharmony/kernel_liteos_m/issues/I4FD5G)
* 修复risc-v异常时，异常信息混乱且backtrace无法输出的问题 ([0fb49d4](https://gitee.com/openharmony/kernel_liteos_m/commits/0fb49d4422245e83c2435468eb7f2690f2e2c6e4)), closes #I4BPHX
* 修复shcmd.h需要用宏包起来的问题 ([a7a8a62](https://gitee.com/openharmony/kernel_liteos_m/commits/a7a8a62bcb7398b4194b37f62098da9cb1ce717e)), closes [#I45FU2](https://gitee.com/openharmony/kernel_liteos_m/issues/I45FU2)
* 修复shell 模块存在大量的编译告警及codeCheck。 ([e0d8b53](https://gitee.com/openharmony/kernel_liteos_m/commits/e0d8b5328f53dcef9427a34e8b0462250bb6a40f)), closes [#I48V2J](https://gitee.com/openharmony/kernel_liteos_m/issues/I48V2J)
* 修复任务栈对齐问题及补充posix和cmsis测试用例 ([ed863e9](https://gitee.com/openharmony/kernel_liteos_m/commits/ed863e90dd9aceb5d3b6d92b0a94703101b83f80))
* 修复公版gcc编译内核，调用calloc崩溃 ([8d0ced2](https://gitee.com/openharmony/kernel_liteos_m/commits/8d0ced206951f35d5caab61969344b0ab259e5fb)), closes [#I4OFG6](https://gitee.com/openharmony/kernel_liteos_m/issues/I4OFG6)
* 修复未使用的参数和类型比较编译报错问题 ([21b46e8](https://gitee.com/openharmony/kernel_liteos_m/commits/21b46e82f31030703d052369701f4d6d736a4a86))
* 修复调度时间最大值不一致问题 ([9f393bc](https://gitee.com/openharmony/kernel_liteos_m/commits/9f393bcc6df592efb6940e9864ff93e932ae59ad))
* 修正liteos-m内核README不准确信息 ([0e067b2](https://gitee.com/openharmony/kernel_liteos_m/commits/0e067b239354c28e16b05489274b33051d002508)), closes [#I4PLWP](https://gitee.com/openharmony/kernel_liteos_m/issues/I4PLWP)
* 修正部分编码风格问题 ([91903a3](https://gitee.com/openharmony/kernel_liteos_m/commits/91903a37165ef364750916104be4f7dfd1d8f9a6)), closes [#I4K8M7](https://gitee.com/openharmony/kernel_liteos_m/issues/I4K8M7)
* 内核ERR打印，线程信息，不方便问题定位。 ([232fec2](https://gitee.com/openharmony/kernel_liteos_m/commits/232fec27ac8cd2f46b5a2f5cfbe4677d0e287314)), closes #I4DAKM
* 内源检视问题修复 ([c24363b](https://gitee.com/openharmony/kernel_liteos_m/commits/c24363bc50364b0be1fba3b993a83c6936fc3d08))
* 删除C文件中无用的__cplusplus ([7fe3aad](https://gitee.com/openharmony/kernel_liteos_m/commits/7fe3aaddf56b300ffcb2b186ae01ed64bef1c069)), closes [#I4D0YK](https://gitee.com/openharmony/kernel_liteos_m/issues/I4D0YK)
* 删除OsMemboxExcInfoGetSub内存调测方法中多余的赋值语句。 ([94db0fe](https://gitee.com/openharmony/kernel_liteos_m/commits/94db0fe7a30edaa10786101f1ef662987a27734a)), closes [#I4CFNX](https://gitee.com/openharmony/kernel_liteos_m/issues/I4CFNX)
* 去掉没有使用的入参consoleid。 ([a627953](https://gitee.com/openharmony/kernel_liteos_m/commits/a62795304da31e13021ea2c675d124afd2594102)), closes [#I43](https://gitee.com/openharmony/kernel_liteos_m/issues/I43)
* 去掉编译选项-fno-short-enums ([b542aab](https://gitee.com/openharmony/kernel_liteos_m/commits/b542aab09dd92397f8c1305ac656ef14a012b5f8)), closes [#I4JY32](https://gitee.com/openharmony/kernel_liteos_m/issues/I4JY32)
* 取消cpu进入低功耗前开中断操作 ([bd39e9e](https://gitee.com/openharmony/kernel_liteos_m/commits/bd39e9e6d67310222c363e132141bb7fee1213f1)), closes [#I4KDP8](https://gitee.com/openharmony/kernel_liteos_m/issues/I4KDP8)
* 告警修复 ([7d2fd01](https://gitee.com/openharmony/kernel_liteos_m/commits/7d2fd01c2d7d6d4c653a307a1419bc094f5332f0))
* 告警修复 ([2d7bf3b](https://gitee.com/openharmony/kernel_liteos_m/commits/2d7bf3b7d8ea84961e802c366899aca891f38377))
* 在头文件中补充LOSCFG_MEM_FREE_BY_TASKID函数的声明 ([ad18026](https://gitee.com/openharmony/kernel_liteos_m/commits/ad18026a59fc2853c9580e040f959f9f42082640)), closes [#I4JBOV](https://gitee.com/openharmony/kernel_liteos_m/issues/I4JBOV)
* 将curl依赖的函数新增至newlib库 ([bd04d9f](https://gitee.com/openharmony/kernel_liteos_m/commits/bd04d9f29a0b77445a71378f88b01ffdae24554f)), closes [#I4RG71](https://gitee.com/openharmony/kernel_liteos_m/issues/I4RG71)
* 将未定义小写"true"，"false"改为"TRUE"，"FALSE"。 ([9819eb5](https://gitee.com/openharmony/kernel_liteos_m/commits/9819eb567636adf1774de9fd6039d274bee8f69b)), closes [#I43D20](https://gitee.com/openharmony/kernel_liteos_m/issues/I43D20)
* 当前仓代码编译告警的问题 ([194ac58](https://gitee.com/openharmony/kernel_liteos_m/commits/194ac5898d1e2ffaa17f6dca405614b73da6f8bb)), closes #I4N50W
* 支持硬浮点编译 ([988fd85](https://gitee.com/openharmony/kernel_liteos_m/commits/988fd85fae454a0d9fe01ef1c921b4f3667d5ac8)), closes [#I48KJP](https://gitee.com/openharmony/kernel_liteos_m/issues/I48KJP)
* 测试用例任务栈的宏定义重复定义，导致在osTest.h中修改不生效 ([ce6cf68](https://gitee.com/openharmony/kernel_liteos_m/commits/ce6cf6879f4564393765a24c4d65448e0fddc3eb)), closes [#I4BA5F](https://gitee.com/openharmony/kernel_liteos_m/issues/I4BA5F)
* 添加L0 测试门禁 ([5c168fc](https://gitee.com/openharmony/kernel_liteos_m/commits/5c168fc3e0d08dd4a8924696e17a498eb444fa70)), closes #I4C5R4
* 添加newlib对ipv6的支持 ([f7400fb](https://gitee.com/openharmony/kernel_liteos_m/commits/f7400fbcb275cbb3b75f1aef2af1459b794432de)), closes [#I4M1DB](https://gitee.com/openharmony/kernel_liteos_m/issues/I4M1DB)
* 添加文件版权头等信息 ([cf4e017](https://gitee.com/openharmony/kernel_liteos_m/commits/cf4e017970f8a3113bc8b6028d75775f747512c6)), closes [#I4BXBH](https://gitee.com/openharmony/kernel_liteos_m/issues/I4BXBH)
* 清除因g_tm变量没有使用引入的编译告警 ([9689522](https://gitee.com/openharmony/kernel_liteos_m/commits/9689522a8e08b79244ad339dc24074843b99d33b)), closes [#I4LW3H](https://gitee.com/openharmony/kernel_liteos_m/issues/I4LW3H)
* 移除los_arch_interrupt.h文件中冗余声明的函数VOID OsExcInit(VOID) ([136952f](https://gitee.com/openharmony/kernel_liteos_m/commits/136952f193fe97cfd6a88932e71811e12edfa7a5)), closes [#I4D9OO](https://gitee.com/openharmony/kernel_liteos_m/issues/I4D9OO)
* 补充net相关头文件 ([93f616b](https://gitee.com/openharmony/kernel_liteos_m/commits/93f616b64e961013be5c5267e50d58a40b5cc6c9))
* 解决gmtime和localtime接口由于g_tm全局变量导致的竞态问题 ([1454b76](https://gitee.com/openharmony/kernel_liteos_m/commits/1454b76482d42e78c1904028de55d82261e5ecaa)), closes [#I4LW3H](https://gitee.com/openharmony/kernel_liteos_m/issues/I4LW3H)
* 解决los_pm.c:743:19: warning 编译告警 ([0c72c8b](https://gitee.com/openharmony/kernel_liteos_m/commits/0c72c8bf94b053ff90bbf2019006c52adf1eb8df)), closes [#I4D4L8](https://gitee.com/openharmony/kernel_liteos_m/issues/I4D4L8)
* 解决定时器超时但还在队列中无法删除的问题 ([ecce17e](https://gitee.com/openharmony/kernel_liteos_m/commits/ecce17ea4878c7bdf7d7de9a6848e6bf871563de)), closes [#I4LFVD](https://gitee.com/openharmony/kernel_liteos_m/issues/I4LFVD)
* 解除net对cmsis的依赖，net属于内核模块，不应该依赖内核对外接口 ([9982624](https://gitee.com/openharmony/kernel_liteos_m/commits/99826240d979c3aa820ab2aace62e3338978e6d1))
* 调度中将pm相关的部分用LOSCFG_KERNEL_PM包含 ([d7c0059](https://gitee.com/openharmony/kernel_liteos_m/commits/d7c0059dc4fcca3f85d2cad1012353892ce28021)), closes [#I4L00F](https://gitee.com/openharmony/kernel_liteos_m/issues/I4L00F)
* 调用LOS_TaskDetach操作已退出的joinable的任务时，未正确回收该任务 ([49c93b3](https://gitee.com/openharmony/kernel_liteos_m/commits/49c93b37c5e215612bbeb4619e73c809f9eb1cac)), closes [#I4DZL7](https://gitee.com/openharmony/kernel_liteos_m/issues/I4DZL7)
* 针对pr是否同步至release分支，增加原因说明规则 ([1ee543d](https://gitee.com/openharmony/kernel_liteos_m/commits/1ee543d5d31524d543ffb297b121224dedf8602f))
* 队列相关的API实现中,参数的校验不完全,导致程序异常 ([eef7c80](https://gitee.com/openharmony/kernel_liteos_m/commits/eef7c80a725d68af254ae55091bd36f978cf4284))


### Features

* adapt net ([8594ee4](https://gitee.com/openharmony/kernel_liteos_m/commits/8594ee469ad2d40a363e5d77f94f292ab3cb8c21))
* add option for ioctl ([5e8ac4b](https://gitee.com/openharmony/kernel_liteos_m/commits/5e8ac4b8883f64bbffa5bb3f063a601112058396)), closes [#I4I3W5](https://gitee.com/openharmony/kernel_liteos_m/issues/I4I3W5)
* add option SIOCGIFBRDADDR for ioctl ([33ee492](https://gitee.com/openharmony/kernel_liteos_m/commits/33ee49295329e3c394d1cc7b16f77a23ac2255a8)), closes [#I4EGNG](https://gitee.com/openharmony/kernel_liteos_m/issues/I4EGNG)
* added assembly atomic interface ([1513f50](https://gitee.com/openharmony/kernel_liteos_m/commits/1513f50a474d1227a83fb655ad18c19cd792f235))
* backtrace for arm9 ([03de7f3](https://gitee.com/openharmony/kernel_liteos_m/commits/03de7f37450a8821ea05e60e94a5d1dd359c4f99)), closes [#I4BMLG](https://gitee.com/openharmony/kernel_liteos_m/issues/I4BMLG)
* Board 和 SoC分离，编译适配 ([3f1c04d](https://gitee.com/openharmony/kernel_liteos_m/commits/3f1c04dd57bca0379ce37162d9d824801e7d880f))
* **build:** add module_group and comment out arch_cflags ([e5636f7](https://gitee.com/openharmony/kernel_liteos_m/commits/e5636f7acaf7e8b7d14dc07930b2a16c86e63c85))
* **build:** support Kconfig ([7e67343](https://gitee.com/openharmony/kernel_liteos_m/commits/7e673430dc42c55b3a0a6d955ba8fea21d6d58ea))
* **cmsis:** support max_count for osSemaphoreNew ([d478152](https://gitee.com/openharmony/kernel_liteos_m/commits/d478152762013975551e1af154df42987596c9a6))
* cortex-m55支持psplimit ([db7d641](https://gitee.com/openharmony/kernel_liteos_m/commits/db7d641c223db8ab25501d79c2a6331f743d830b)), closes [#I4T7R3](https://gitee.com/openharmony/kernel_liteos_m/issues/I4T7R3)
* littlefs最大打开文件个数修改为可通过menuconfig配置 ([9bc9f3a](https://gitee.com/openharmony/kernel_liteos_m/commits/9bc9f3aa53be7bf392d3da09adcb184dec8e3da2))
* newlib支持pthread_equal ([ce5357e](https://gitee.com/openharmony/kernel_liteos_m/commits/ce5357eee1daf1c1b3a3aa7bdeaef0cc5a8901bf)), closes [#I4QERS](https://gitee.com/openharmony/kernel_liteos_m/issues/I4QERS)
* provide read write for socket by #define ([d044eda](https://gitee.com/openharmony/kernel_liteos_m/commits/d044eda3c3640ae135ef528739d0c9609f40c822)), closes [#I4H8TL](https://gitee.com/openharmony/kernel_liteos_m/issues/I4H8TL)
* synchronizing arch api ([04bf3a6](https://gitee.com/openharmony/kernel_liteos_m/commits/04bf3a682f149482d8ebdbb9ff96d6889edb487a)), closes [#I4N7XV](https://gitee.com/openharmony/kernel_liteos_m/issues/I4N7XV)
* synchronous los_hwicreate interface ([6c68ada](https://gitee.com/openharmony/kernel_liteos_m/commits/6c68adad4db7f6112990764eafe6a3bd5f8759a5))
* 低内存资源回收low memory killer ([ab886d8](https://gitee.com/openharmony/kernel_liteos_m/commits/ab886d848ab809942c11d0e408b8ff76c41b92b2)), closes [#I4ID0M](https://gitee.com/openharmony/kernel_liteos_m/issues/I4ID0M)
* 低功耗支持冻结线程等需求 ([9b5739a](https://gitee.com/openharmony/kernel_liteos_m/commits/9b5739ab11ae7a89d35ba6f54fc62137ddc6d897)), closes [#I49FJF](https://gitee.com/openharmony/kernel_liteos_m/issues/I49FJF)
* 内核提供Interrupt框架，支持多架构多平台通用化 ([e8538d0](https://gitee.com/openharmony/kernel_liteos_m/commits/e8538d041a93817cea4ae3914afcfdc6182483e7))
* 内核提供tick timer框架，支持多架构多平台通用化 ([f635450](https://gitee.com/openharmony/kernel_liteos_m/commits/f635450d7cf7f674de3803b5eb9d759b6e4d005b)), closes [#I4N7XV](https://gitee.com/openharmony/kernel_liteos_m/issues/I4N7XV)
* 同步调度部分优化至liteos_m ([fb11ab1](https://gitee.com/openharmony/kernel_liteos_m/commits/fb11ab181e69289f7a2edfd5857595b89f1d5d67))
* 支持cortex-m55 ([6e17805](https://gitee.com/openharmony/kernel_liteos_m/commits/6e1780546c173365262c7a50b81896f5b3c46389)), closes [#I4Q9OQ](https://gitee.com/openharmony/kernel_liteos_m/issues/I4Q9OQ)
* 支持Lms ([7b838e8](https://gitee.com/openharmony/kernel_liteos_m/commits/7b838e8a7b8e9f7fb34ca495e5cb46d70d12722b)), closes [#I4HYBG](https://gitee.com/openharmony/kernel_liteos_m/issues/I4HYBG)
* 支持posix 和 cmsis join能力 ([ecfdf7f](https://gitee.com/openharmony/kernel_liteos_m/commits/ecfdf7ff9bf1adf06835968dc6f06b7c45e04862)), closes [#I44V26](https://gitee.com/openharmony/kernel_liteos_m/issues/I44V26)
* 支持posix线程私有数据能力 ([a856303](https://gitee.com/openharmony/kernel_liteos_m/commits/a856303b9cdadcfade33de0d3a335db0332f5761)), closes [#I4ODEB](https://gitee.com/openharmony/kernel_liteos_m/issues/I4ODEB)
* 支持pthread_condattr_setclock ([68dad2c](https://gitee.com/openharmony/kernel_liteos_m/commits/68dad2ccc9856dc2c1ac112bc8f30a703c1af5d9)), closes [#I4ETLN](https://gitee.com/openharmony/kernel_liteos_m/issues/I4ETLN)
* 支持pthread_mutex属性相关接口 ([d2be4d7](https://gitee.com/openharmony/kernel_liteos_m/commits/d2be4d7a460365844a7732df8a3eb59e6bb64897)), closes [#I4G4RQ](https://gitee.com/openharmony/kernel_liteos_m/issues/I4G4RQ)
* 支持select.h的FD_SETSIZE宏配置 ([05642f0](https://gitee.com/openharmony/kernel_liteos_m/commits/05642f05ae6b220df977f4402bc31f396bf08603)), closes [#I4RYK4](https://gitee.com/openharmony/kernel_liteos_m/issues/I4RYK4)
* 支持任务栈可配置 ([96cc92d](https://gitee.com/openharmony/kernel_liteos_m/commits/96cc92d03561e1a9f1030c71916068584b349626))
* 调度、任务及pm解耦 ([cc8a794](https://gitee.com/openharmony/kernel_liteos_m/commits/cc8a794229f4236a1611183fd482740b3badbcd7)), closes [#I4JTN6](https://gitee.com/openharmony/kernel_liteos_m/issues/I4JTN6)
* 适配三方库FatFs升级至r0.14a版本 ([92ea355](https://gitee.com/openharmony/kernel_liteos_m/commits/92ea355756ebd92b8809ecb638ffb5355d89c5e9))


### BREAKING CHANGES

* pthread_create 支持任务栈设置
osThreadNew 支持任务栈设置
TSK_INIT_PARAM_S 结构体添加stackAddr 字段

* 涉及接口修改：
    LOS_HwiCreate LOS_HwiDelete

改动内容：
    中断处理函数参数类型由HWI_ARG_T统一改为HwiIrqParam，封装原有arg参数及riscv部分实际使用变量pDevId, 这样一方面统一LiteOS接口，也统一了中断处理函数的arg参数
    同步LOS_HwiDelete接口入参，新增HWI_IRQ_PARAM_S *irqParam参数，为后续共享中断功能的添加预留

* 新增接口：
LOS_TaskResRecycle
LOS_CurrNanosec
LOS_MDelay

接口修改：
LOS_QueueCreate：第一个入参添加const修饰并增加一种异常情况处理
los_memory.c中 OS_ERROR 修改为LOS_NOK，重定义LOS_NOK为（UINT32）-1。

接口位置转移：
LOS_UDelay 由los_task.h/.c 转移到los_tick.h/.c 

宏修改：
LOS_ERRNO_MUX_PEND_INTERR 改名为 LOS_ERRNO_MUX_IN_INTERR

* **posix接口融合:**

接口修改：
calloc
pthread_mutex_init
mq_unlink
pthread_join
pthread_exit
pthread_attr_init
pthread_attr_destroy
pthread_condattr_destroy
pthread_cond_timedwait
pthread_mutexattr_init
pthread_mutexattr_settype
pthread_mutex_destroy
pthread_mutex_timedlock
pthread_mutex_trylockk
pthread_mutex_unlock
sem_getvalue
sem_timedwait
pthread_cond_timedwait
nanosleep
timer_create
timer_settime
timer_gettime
testStdlibStrtoull005
testStdlibStrtol011
testStdlibStrtoul007
testPthread004

新增接口：
CheckForCancel
pthread_setcancelstate
pthread_setcanceltype
pthread_once
pthread_setschedprio
pthread_attr_setstack
pthread_attr_getstack
pthread_condattr_getpshared
pthread_condattr_setpshared
pthread_condattr_getclock
pthread_mutexattr_gettype
CheckMutexAttr
OsMuxPreCheck
sem_trywait
MuxPendForPosix
MuxPostForPosix
增加overrun功能

接口变化：
mq_getsetattr 改为内部接口MqGetSetAttr
GetTickTimeFromNow->OsGetTickTimeFromNow

接口位置转移：
pthread_equal接口由newlib下转移到kal/posix/src/pthread.c中

* **posix:** 接口修改：
calloc
pthread_mutex_init
mq_unlink
pthread_join
pthread_exit
pthread_attr_init
pthread_attr_destroy
pthread_condattr_destroy
pthread_cond_timedwait
pthread_mutexattr_init
pthread_mutexattr_settype
pthread_mutex_destroy
pthread_mutex_timedlock
pthread_mutex_trylockk
pthread_mutex_unlock
sem_getvalue

新增接口：
CheckForCancel
pthread_setcancelstate
pthread_setcanceltype
pthread_once
pthread_setschedprio
pthread_attr_setstack
pthread_attr_getstack
pthread_condattr_getpshared
pthread_condattr_setpshared
pthread_condattr_getclock
pthread_mutexattr_gettype
CheckMutexAttr
OsMuxPreCheck
sem_trywait

接口变化：
mq_getsetattr 改为内部接口OsMqGetSetAttr

接口位置转移：
pthread_equal接口由newlib下转移到kal/posix/src/pthread.c中

* 新增接口：
    LOS_HwiTrigger ArchIntTrigger
    LOS_HwiEnable ArchIntEnable
    LOS_HwiDisable ArchIntDisable
    LOS_HwiClear ArchIntClear
    LOS_HwiSetPriority ArchIntSetPriority

* Assembly implementation:
    ArchAtomicRead
    ArchAtomicSet
    ArchAtomicAdd
    ArchAtomicSub
    ArchAtomicInc
    ArchAtomicIncRet
    ArchAtomicDec
    ArchAtomicDecRet

* 原来版本中每个架构下提供的tick timer相关操作函数为弱函数：
    WEAK UINT32 HalTickStart(OS_TICK_HANDLER handler);
    WEAK VOID HalSysTickReload(UINT64 nextResponseTime);
    WEAK UINT64 HalGetTickCycle(UINT32 *period);
    WEAK VOID HalTickLock(VOID);
    WEAK VOID HalTickUnlock(VOID);
  用户如果需要启用自己的tick timer需要自己实现相关接口(强属性)，在 "内核初始化之前" 通过调用：
  LOS_TickTimerRegister 接口替换系统默认提供的tick timer相关接口。
  无论用户提供的tick timer 还是系统默认提供的，均在内核初始化时启动。
*   int pthread_key_create(pthread_key_t *k, void (*dtor)(void *))
  int pthread_key_delete(pthread_key_t k)
  int pthread_setspecific(pthread_key_t k, const void *x)
  void *pthread_getspecific(pthread_key_t k)
* 1.修改arch/include下接口，以ArchXX命名函数
2.提取公共函数声明:ArchAtomicXchg32bits、ArchAtomicDecRet、ArchAtomicCmpXchg32bits
3.新增部分原子操作c内联实现
* 增加低内存资源回收注册相关接口LOS_LmkOpsNodeRegister、LOS_LmkOpsNodeUnregister和内存资源释放和任务恢复接口LOS_LmkTasksKill和LOS_LmkTasksRestore.
* SwtmrHandlerItem结构体新增swtmrID字段，用于标识超时队列中软件定时器id
* 新增支持API:

LOS_LmsCheckPoolAdd使能检测指定内存池
LOS_LmsCheckPoolDel不检测指定内存池
LOS_LmsAddrProtect为指定内存段上锁，不允许访问
LOS_LmsAddrDisableProtect去能指定内存段的访问保护


#  (2021-09-30)


### Bug Fixes

*  M核代码告警清零 ([cb5f862](https://gitee.com/openharmony/kernel_liteos_m/commits/cb5f862eafd91675bf53d0320616cc8f707a6e5a)), closes [#I4386N](https://gitee.com/openharmony/kernel_liteos_m/issues/I4386N)
*  M核告警消除 ([5a9d53b](https://gitee.com/openharmony/kernel_liteos_m/commits/5a9d53b6444a3bbe084f575ef079f0da36c27c2a)), closes [#I46](https://gitee.com/openharmony/kernel_liteos_m/issues/I46)
*  M核编译配置修改 ([deff7c9](https://gitee.com/openharmony/kernel_liteos_m/commits/deff7c921f606ed85663f91d21c12163c15aa7f8)), closes [#I46](https://gitee.com/openharmony/kernel_liteos_m/issues/I46)
*  shell模块task命令踩内存问题修改 ([b5e84d5](https://gitee.com/openharmony/kernel_liteos_m/commits/b5e84d5516b599257151f88b5a830d6fe9e0e188)), closes #II4
*  trustzone 中断适配 ([15a2c17](https://gitee.com/openharmony/kernel_liteos_m/commits/15a2c17f791b7d6703863718d92fa35402c41067)), closes [#I42TFV](https://gitee.com/openharmony/kernel_liteos_m/issues/I42TFV)
* add API LOS_HwiCreate and LOS_HwDelete ([eb668eb](https://gitee.com/openharmony/kernel_liteos_m/commits/eb668eb9e0021961230858f9bfa1599646da9a8f)), closes [#I3R75F](https://gitee.com/openharmony/kernel_liteos_m/issues/I3R75F)
* add liteos_m vfs ([5479dd6](https://gitee.com/openharmony/kernel_liteos_m/commits/5479dd66d7ec96d2206a9890586fe5f07d50fbb9)), closes [#I3R493](https://gitee.com/openharmony/kernel_liteos_m/issues/I3R493)
* backtrace代码段判断函数定义成弱函数，如果有多个代码段，那么可根据实际情况重定义 ([8692ed3](https://gitee.com/openharmony/kernel_liteos_m/commits/8692ed33a3b975bac7aa5b6ce3c40a84e281f6c5)), closes [#I420PQ](https://gitee.com/openharmony/kernel_liteos_m/issues/I420PQ)
* change cpup retval range ([d00a35b](https://gitee.com/openharmony/kernel_liteos_m/commits/d00a35ba9288047e9eaba06a8ed1d15806bfca40))
* change description according to review ([a38365e](https://gitee.com/openharmony/kernel_liteos_m/commits/a38365ed097fb398f922a5d1a72305933ae3e7b3))
* change queuePosion to queuePosition ([e772ccb](https://gitee.com/openharmony/kernel_liteos_m/commits/e772ccb3ccd3b31269066fa59a434a23af98ce51))
* change to OpenHarmony LiteOS-M ([259fb54](https://gitee.com/openharmony/kernel_liteos_m/commits/259fb54b3c1a208c5672f1f6551ba0424505e4b2))
* comment error ([a37b73d](https://gitee.com/openharmony/kernel_liteos_m/commits/a37b73d7599a62a728dbf564de9d9927f995ec1b))
* compile warning fix ([8dea15c](https://gitee.com/openharmony/kernel_liteos_m/commits/8dea15c4fcaff2957c60351781180f964c0a882d)), closes #I4330
* compile warnings ([75f6187](https://gitee.com/openharmony/kernel_liteos_m/commits/75f61870a002e238385bc9a7632d42b0b2488c9a)), closes [#I3Y57X](https://gitee.com/openharmony/kernel_liteos_m/issues/I3Y57X)
* correct ECB in comment ([e2aeb1f](https://gitee.com/openharmony/kernel_liteos_m/commits/e2aeb1ff1d4cd54a752c9f6ef976ee8e353da620))
* correct spelling ([dd5cefb](https://gitee.com/openharmony/kernel_liteos_m/commits/dd5cefb0dace4b7a7276c880e45d5520950e9844))
* correct spelling ([e7ea50f](https://gitee.com/openharmony/kernel_liteos_m/commits/e7ea50f13142e34a9860e75be051538fe91e234e))
* correct the counter of exception type ([bff6587](https://gitee.com/openharmony/kernel_liteos_m/commits/bff65877a281ac1cdcbefd325dcd3ad4933edac9))
* correct typo ([3ceface](https://gitee.com/openharmony/kernel_liteos_m/commits/3ceface13e1c15d107bc3c8460c17b9dd4c95e26))
* cpup funciton repair ([c842251](https://gitee.com/openharmony/kernel_liteos_m/commits/c842251bab7ea6365f2bee8ae880e6efb60f0c73))
* delete unused symbols for default config ([7ad3a22](https://gitee.com/openharmony/kernel_liteos_m/commits/7ad3a22b4d62c8b00876ff338c4ce8d1e3f3cbe2)), closes [#I45HN3](https://gitee.com/openharmony/kernel_liteos_m/issues/I45HN3)
* fix __cplusplus typo ([c3eb3e5](https://gitee.com/openharmony/kernel_liteos_m/commits/c3eb3e59e692371f67a4e21e4b38004195ce5e10))
* fix backtrace check issue ([b6664e6](https://gitee.com/openharmony/kernel_liteos_m/commits/b6664e66f14d432b02101c883a53cb333dc4c195))
* fix bad params def in HalTickStart ([4443b74](https://gitee.com/openharmony/kernel_liteos_m/commits/4443b74ea63d2e458af69723a7198bbbaeb856ea)), closes [#I48YWT](https://gitee.com/openharmony/kernel_liteos_m/issues/I48YWT)
* fix comment typo issue ([0c2a675](https://gitee.com/openharmony/kernel_liteos_m/commits/0c2a675e2d163eaf05aff7df226404888d78ca37))
* fix destroy typo ([4ba6259](https://gitee.com/openharmony/kernel_liteos_m/commits/4ba6259e94bd677764922b0470fddc2590f6efa5))
* fix function name ([f7d50d0](https://gitee.com/openharmony/kernel_liteos_m/commits/f7d50d0fbf53cdd352e94a387cf707083f1641bb))
* fix length typo ([98d6580](https://gitee.com/openharmony/kernel_liteos_m/commits/98d65801de9e37199b5df9c8734ebcb6ff5d853d))
* fix LOS_MEMBOX_ALIGNED macro name ([fced899](https://gitee.com/openharmony/kernel_liteos_m/commits/fced8992cf4fbc26c9722d528d642a7afe479170))
* fix LOS_TaskNameGet comment ([c70b28b](https://gitee.com/openharmony/kernel_liteos_m/commits/c70b28bf785da12fd60ecc92a5a63fb54e52c22a))
* fix macro typo ([1600360](https://gitee.com/openharmony/kernel_liteos_m/commits/16003604a948d081151ca4fcf0cfeea91c9e1960))
* fix misspell in comments ([4572814](https://gitee.com/openharmony/kernel_liteos_m/commits/457281427ba286e24386d7916c4bc81f112f1c60))
* fix NULL UNUSED redefined issue ([765bd55](https://gitee.com/openharmony/kernel_liteos_m/commits/765bd551990e906f457ec1145aea4f2bf5856547))
* fix OS_TASK_STACK_PROTECT_SIZE undeclared error ([2a5a515](https://gitee.com/openharmony/kernel_liteos_m/commits/2a5a515f50a0f344d416218046bf0880facfb97c)), closes [#I4C5RW](https://gitee.com/openharmony/kernel_liteos_m/issues/I4C5RW)
* fix print format compilation warning ([ef12c89](https://gitee.com/openharmony/kernel_liteos_m/commits/ef12c89c7a79791665125a0b7391f9e533a06a0f))
* fix readme link issue ([9e332c6](https://gitee.com/openharmony/kernel_liteos_m/commits/9e332c69a5813027575786424b78f0ae2f9a4c5a))
* fix some typo issues ([f6b8cab](https://gitee.com/openharmony/kernel_liteos_m/commits/f6b8cab2539e4d1ee9e47ab8e3eb12f1630f7393))
* fix typo in comment ([811d3fa](https://gitee.com/openharmony/kernel_liteos_m/commits/811d3fae8a39686a1c66dfd61c897b7fd3fac2ac))
* fix typos ([191644e](https://gitee.com/openharmony/kernel_liteos_m/commits/191644e4412fccb7142cfcd34316ad8e2d24fcdf))
* Fixed a spelling error in FS_LOCK_TIMEOUT_SEC. ([42dc308](https://gitee.com/openharmony/kernel_liteos_m/commits/42dc3083693be13c5fa8f2ad8213245465e28adf)), closes [#I3TL66](https://gitee.com/openharmony/kernel_liteos_m/issues/I3TL66)
* include file log_config.h in los_mpu.c ([1932d71](https://gitee.com/openharmony/kernel_liteos_m/commits/1932d714a589c9e87258d7c8ed647b6110bde543))
* L0 GN compilation problem ([2f9bfcc](https://gitee.com/openharmony/kernel_liteos_m/commits/2f9bfcc6ee436ee3568d0c8158a8e43c9ef02623)), closes [I3ORHS](https://gitee.com/openharmony/kernel_liteos_m/issues/I3ORHS)
* l0 shell adapt new master ([fd2227e](https://gitee.com/openharmony/kernel_liteos_m/commits/fd2227e5c858c840f63629c27e559580e1cbdf2f))
* littlefs inappropriate return value and errno ([452f364](https://gitee.com/openharmony/kernel_liteos_m/commits/452f36403dd4376572d8b52352d0d8129a4617e4)), closes [I3YIVT](https://gitee.com/openharmony/kernel_liteos_m/issues/I3YIVT)
* littlefs三方源码采用GN编译修改上库 ([bdb614b](https://gitee.com/openharmony/kernel_liteos_m/commits/bdb614bc2b82011d77c7c8ffb95e415d2640d19d))
* LOS_QueueInfoGet函数统计等待读写任务有误 ([215a1f6](https://gitee.com/openharmony/kernel_liteos_m/commits/215a1f6dc06cf2348c5875f7b05193b26ec17dde)), closes [#I4C6P2](https://gitee.com/openharmony/kernel_liteos_m/issues/I4C6P2)
* los_swtmr.h不满足自包含要求对los_config.h存在依赖，但未包含los_config.h ([ad8e96a](https://gitee.com/openharmony/kernel_liteos_m/commits/ad8e96a00f2fcaebe57feae4e3f23f99d7f1d066)), closes [#I40DHM](https://gitee.com/openharmony/kernel_liteos_m/issues/I40DHM)
* **mbedtls:** mbedtls l0 support ([761b860](https://gitee.com/openharmony/kernel_liteos_m/commits/761b86096c3cf5642c95e556c11a45a3d5f413e4))
* misspell ([30578a6](https://gitee.com/openharmony/kernel_liteos_m/commits/30578a6a68a0d3a203b9a00e7bd1a946356d13d2))
* modify event API description ([9481f1d](https://gitee.com/openharmony/kernel_liteos_m/commits/9481f1d8406960b8b859cc5a84b3831eadaf040c))
* modify the return type of LOS_IntLock from UINTPTR to UINT32. ([bcc34e2](https://gitee.com/openharmony/kernel_liteos_m/commits/bcc34e22ede5aeccea63aba9f9ce3cb9aa92cbeb))
* modify type INLNIE ([294a6d2](https://gitee.com/openharmony/kernel_liteos_m/commits/294a6d246b6a1f7cba8ce1bc4eeb9ae11f17a003))
* Mutex锁释放后，无任务持有的情况下，owner未清空 ([c136cfd](https://gitee.com/openharmony/kernel_liteos_m/commits/c136cfdf973ae0068154606afa3a23858822c347)), closes [#I40OC8](https://gitee.com/openharmony/kernel_liteos_m/issues/I40OC8)
* M核littlefs当文件open失败时，需要回收fd，否则会有fd泄露 ([b458e12](https://gitee.com/openharmony/kernel_liteos_m/commits/b458e12ed54905db6667f9d5db1410f94a3d6c71)), closes [#I3XU6R](https://gitee.com/openharmony/kernel_liteos_m/issues/I3XU6R)
* m核上littlefs新增错误码功能 ([a062378](https://gitee.com/openharmony/kernel_liteos_m/commits/a06237863121816fa6421aed233446a3fa4e2665)), closes [#I3UVMQ](https://gitee.com/openharmony/kernel_liteos_m/issues/#I3UVMQ)
* net compile fix ([299813c](https://gitee.com/openharmony/kernel_liteos_m/commits/299813cdb43ae1e24200f0a6b006e04e4d4fcb05))
* open shell & fix qemu compile ([911a767](https://gitee.com/openharmony/kernel_liteos_m/commits/911a767992ee540a96ee9f67e0b87195656c3eab))
* ping cmd compile fail ([ae05de3](https://gitee.com/openharmony/kernel_liteos_m/commits/ae05de3ffda86d312d33b2e38f237dd76daf4949))
* protect from accssing NULL pointer, access g_handle after fslock ([bc7a914](https://gitee.com/openharmony/kernel_liteos_m/commits/bc7a914bce1b227a4f57265d5ecc3f82d78533e7)), closes [#I3UCX3](https://gitee.com/openharmony/kernel_liteos_m/issues/I3UCX3)
* remove changelog in README ([37ef8ca](https://gitee.com/openharmony/kernel_liteos_m/commits/37ef8ca7ae73bf78f56dfef36c48bcd790afb6ff)), closes [#I3ZECY](https://gitee.com/openharmony/kernel_liteos_m/issues/I3ZECY)
* remove libsec_kernel ([1144b75](https://gitee.com/openharmony/kernel_liteos_m/commits/1144b75f3ff9ce9e8fa114cf614d3deade8ac7c2))
* remove project files ([4ad01ce](https://gitee.com/openharmony/kernel_liteos_m/commits/4ad01ced93684e5c7ba144ca267f8fcc2fe03105))
* remove targets project files ([f39da5a](https://gitee.com/openharmony/kernel_liteos_m/commits/f39da5a6da59a3b1291c6b9ef7c50e6185df5ea1))
* risc-v 锁中断在调度后失效 ([70f3aa1](https://gitee.com/openharmony/kernel_liteos_m/commits/70f3aa1c112e949d9af8e0245af5879140adefba)), closes [#I42OLJ](https://gitee.com/openharmony/kernel_liteos_m/issues/I42OLJ)
* riscv_nuclei 编译失败 ([d3fb23c](https://gitee.com/openharmony/kernel_liteos_m/commits/d3fb23cca06580b6e447a9476a9c16643c4ea8fe)), closes [#I41PQJ](https://gitee.com/openharmony/kernel_liteos_m/issues/I41PQJ)
* rollback resources when opendir failed ([82fad6c](https://gitee.com/openharmony/kernel_liteos_m/commits/82fad6c36f183de93d936dd2007b899c2932bcee)), closes [#I3Y577](https://gitee.com/openharmony/kernel_liteos_m/issues/I3Y577)
* Scheduling module code to join GN compilation. ([8bf166b](https://gitee.com/openharmony/kernel_liteos_m/commits/8bf166b418ce1a6c027dc304d45fc9431923764c)), closes [#I3OA95](https://gitee.com/openharmony/kernel_liteos_m/issues/I3OA95)
* The hook module fails to compile under the c99 compilation standard. ([a078c30](https://gitee.com/openharmony/kernel_liteos_m/commits/a078c3086fa47e05e3ffdc4e94ca202284a1487b)), closes [#I3S4ME](https://gitee.com/openharmony/kernel_liteos_m/issues/I3S4ME)
* tick 动态化计算优化，减小中断执行时间对系统总体时间的影响，保证软件定时器的响应精度。 ([2118c84](https://gitee.com/openharmony/kernel_liteos_m/commits/2118c84616cce5b5d4569d5a82a4d5de02af1e1c)), closes [#I3YGP1](https://gitee.com/openharmony/kernel_liteos_m/issues/I3YGP1)
* timer qemu 头文件缺失 ([12fed74](https://gitee.com/openharmony/kernel_liteos_m/commits/12fed747226c377c656b8acb8346154627845b6f))
* update according to review comments ([4dc0d31](https://gitee.com/openharmony/kernel_liteos_m/commits/4dc0d313d1b4a39ebb62156d0bb6429ebb8b996d))
* update arch_spec.md ([cf43988](https://gitee.com/openharmony/kernel_liteos_m/commits/cf439885234e0955bab09834b3b876d627c81605))
* update doc usage ([5c19fd4](https://gitee.com/openharmony/kernel_liteos_m/commits/5c19fd4beca15a8b7faad847a13943a9abfe3c8c))
* update document description ([954c2fc](https://gitee.com/openharmony/kernel_liteos_m/commits/954c2fcfb531d0da1dcc6354a0c0b14335566ed7))
* update macro to config item ([be57ece](https://gitee.com/openharmony/kernel_liteos_m/commits/be57ece9ce6200c8868164bc0bb2f26e3062cd37))
* 以g_sysSchedStartTime是否为0判断时间轴是否生效存在极限场景导致调度时间不生效 ([509cf59](https://gitee.com/openharmony/kernel_liteos_m/commits/509cf59befb40839a376be31958d615f066c66fd)), closes [#I45HP5](https://gitee.com/openharmony/kernel_liteos_m/issues/I45HP5)
* 修复littlefs的多分区挂载时，对pathname的处理逻辑异常 ([b55cfc1](https://gitee.com/openharmony/kernel_liteos_m/commits/b55cfc1ba7fc470b876c555ac7d3be0aa1d3d363)), closes [#I3WWFA](https://gitee.com/openharmony/kernel_liteos_m/issues/I3WWFA)
* 修复littlefs编译fs.c缺少头文件件依赖配置+若干musl库的缺失问题+多分区规格宏配置问题 ([7259289](https://gitee.com/openharmony/kernel_liteos_m/commits/7259289bd98a743f596060bc607f77ab8f149129)), closes [#I3VT11](https://gitee.com/openharmony/kernel_liteos_m/issues/I3VT11)
* 修复M核littlefs GN编译的若干错误 ([d77828e](https://gitee.com/openharmony/kernel_liteos_m/commits/d77828e206f3324424c3b1372332828a0afb84c2))
* 修复M核上musl库缺失strdup,c的问题 ([139f83a](https://gitee.com/openharmony/kernel_liteos_m/commits/139f83af391a4f5e64ab0fccbd783dd651b87e6a)), closes [#I3VZSI](https://gitee.com/openharmony/kernel_liteos_m/issues/I3VZSI)
* 修复QUEUE_INFO_S中waitReadTask无法保存大于32的任务编号 ([704cca9](https://gitee.com/openharmony/kernel_liteos_m/commits/704cca97b17972d1640fd33b1fa0b5917bb92b19)), closes [#I41N78](https://gitee.com/openharmony/kernel_liteos_m/issues/I41N78)
* 修复task命令格式错误，增加copyright ([ad15bbd](https://gitee.com/openharmony/kernel_liteos_m/commits/ad15bbd6a4d298aabc05b26d7838de6425dbc5c5))
* 修复timer_settime只设置第一次到期时间时，到期时间不准的缺陷 ([a5f4e37](https://gitee.com/openharmony/kernel_liteos_m/commits/a5f4e37e535bff39a333c469e2f1bf1817216fe8)), closes [#I3TCHA](https://gitee.com/openharmony/kernel_liteos_m/issues/I3TCHA)
* 修复timer_settime只设置第一次到期时间时，到期时间不准的缺陷 ([e4b5d4e](https://gitee.com/openharmony/kernel_liteos_m/commits/e4b5d4ebe9677d49d490fe74e5ac33c4a29435a2)), closes [#I3TCHA](https://gitee.com/openharmony/kernel_liteos_m/issues/I3TCHA)
* 修复不规范的copyright ([2d9bbd2](https://gitee.com/openharmony/kernel_liteos_m/commits/2d9bbd2769e71e7b464bd22fc335999dd8000682)), closes [#I3SPJ1](https://gitee.com/openharmony/kernel_liteos_m/issues/I3SPJ1)
* 修复中断操作接口头文件位置变更，导致的编译错误 ([013a953](https://gitee.com/openharmony/kernel_liteos_m/commits/013a953926ca7cefd8a2f7c74d23c36c49765b59)), closes [#I4665Z](https://gitee.com/openharmony/kernel_liteos_m/issues/I4665Z)
* 修复文档链接失效问题 ([5d53c84](https://gitee.com/openharmony/kernel_liteos_m/commits/5d53c848124319a95f509b56ba001357c6dd8b05)), closes [#I4529I](https://gitee.com/openharmony/kernel_liteos_m/issues/I4529I)
* 内核模块编译增加-Werror编译选项，且清除内核编译告警 ([3798091](https://gitee.com/openharmony/kernel_liteos_m/commits/3798091d87441ea445ec642de57e036dbbc8370e)), closes [#I46E6S](https://gitee.com/openharmony/kernel_liteos_m/issues/I46E6S)
* 删除MQ内冗余的重复宏定义 ([6d72916](https://gitee.com/openharmony/kernel_liteos_m/commits/6d7291660ad04682f05850876f81dfd0cba8ea6b)), closes [#I3U5QQ](https://gitee.com/openharmony/kernel_liteos_m/issues/I3U5QQ)
* 删除延时节点后，存在时间无法有效刷新的场景 ([0a87c04](https://gitee.com/openharmony/kernel_liteos_m/commits/0a87c04d58d11da2082470698de0760d72259533)), closes [#I45I9Y](https://gitee.com/openharmony/kernel_liteos_m/issues/I45I9Y)
* 在内核初始化前期启动tick timer ([b3841ed](https://gitee.com/openharmony/kernel_liteos_m/commits/b3841ed1ddbfb3359c7a0ceca04fc0f645d440c3)), closes [#I410F0](https://gitee.com/openharmony/kernel_liteos_m/issues/I410F0)
* 增加MPU功能的开关宏，默认关闭 ([9709d29](https://gitee.com/openharmony/kernel_liteos_m/commits/9709d29b87a3888412ffda586e16eba86adccfe7)), closes [#I3WE0S](https://gitee.com/openharmony/kernel_liteos_m/issues/I3WE0S)
* 宏LOSCFG_SUPPORT_FATFS在fs.c中存在使用问题，导致无fatfs的情况下，编译出错 ([c1614ce](https://gitee.com/openharmony/kernel_liteos_m/commits/c1614ce2b4d4dfcda9532cbeeff25ad7a6558196)), closes [#I44XNR](https://gitee.com/openharmony/kernel_liteos_m/issues/I44XNR)
* 延时队列为NULL时，返回的响应时间为64位最大值，导致无法更新tick timer的响应周期 ([bcec32e](https://gitee.com/openharmony/kernel_liteos_m/commits/bcec32e389634097102a203ee7e937ed3193c984)), closes [#I3W1LF](https://gitee.com/openharmony/kernel_liteos_m/issues/I3W1LF)
* 支持smoke 测试用例 ([3ad5942](https://gitee.com/openharmony/kernel_liteos_m/commits/3ad5942301a1094a2c6b6ffee280cf09cfdb2a88)), closes [#I424RX](https://gitee.com/openharmony/kernel_liteos_m/issues/I424RX)
* 新增函数regcomp,regfree和regexec支持 ([86d5544](https://gitee.com/openharmony/kernel_liteos_m/commits/86d5544a3fd5f41f7f2abe3b143e9f1dc9b430f8)), closes [#I3U9LE](https://gitee.com/openharmony/kernel_liteos_m/issues/I3U9LE)
* 清理残留无效的trace代码 ([3c48951](https://gitee.com/openharmony/kernel_liteos_m/commits/3c489518576583ab6bf0d581810ef0a02b6ccd51)), closes [#I3ZMB4](https://gitee.com/openharmony/kernel_liteos_m/issues/I3ZMB4)
* 解决arm9编译冲突问题 ([c1a6245](https://gitee.com/openharmony/kernel_liteos_m/commits/c1a624536887fef44fed37134638bf40191b9aae)), closes [#I420](https://gitee.com/openharmony/kernel_liteos_m/issues/I420)
* 调度过程低概率触发OsSchedTaskEnQueue断言，导致系统异常 ([e5e9289](https://gitee.com/openharmony/kernel_liteos_m/commits/e5e9289a08cf1db71c09647b71a864f5a1f8a4a2)), closes [#I3VX9Y](https://gitee.com/openharmony/kernel_liteos_m/issues/I3VX9Y)
* 调整hook类型和位置，方便调试 ([369320d](https://gitee.com/openharmony/kernel_liteos_m/commits/369320de3710486f252c55c522e8c06e19541b77)), closes [#I3TGZS](https://gitee.com/openharmony/kernel_liteos_m/issues/I3TGZS)


### Features

* add cmsis ThreadFlags support ([cea551e](https://gitee.com/openharmony/kernel_liteos_m/commits/cea551e99ea0da8eefbaa826de2f68813252fb83))
* add deps to musl and securec ([bfa25fc](https://gitee.com/openharmony/kernel_liteos_m/commits/bfa25fcaa355c28a6139f683c4453f677e23abd7))
* add file type support for LfsStat ([ceaed2d](https://gitee.com/openharmony/kernel_liteos_m/commits/ceaed2dc86de2ed242aab4dd02c1f34e064583b3)), closes [#I3ZCDR](https://gitee.com/openharmony/kernel_liteos_m/issues/I3ZCDR)
* add interface of dhcp_is_bound ([84a180d](https://gitee.com/openharmony/kernel_liteos_m/commits/84a180d585686cda693bf8c6ed6b86f7c5dd5eee)), closes [#I46W08](https://gitee.com/openharmony/kernel_liteos_m/issues/I46W08)
* add L0 shell function ([97283b7](https://gitee.com/openharmony/kernel_liteos_m/commits/97283b7c5dd44b266e94fcf0f5d21e63ee1ec7e0))
* add LOS_ListHeadInsert function ([430b186](https://gitee.com/openharmony/kernel_liteos_m/commits/430b186be7ec1554a29a6d509188eb877d796a7e)), closes [#I3TAQA](https://gitee.com/openharmony/kernel_liteos_m/issues/I3TAQA)
* **kernel:** 补充获取最高、最低优先级接口 ([c24c38b](https://gitee.com/openharmony/kernel_liteos_m/commits/c24c38b084c13a2cdc5616f88bb5f63153600c28)), closes [#I42HM7](https://gitee.com/openharmony/kernel_liteos_m/issues/I42HM7)
* L0 armquem 剥离工具链 ([67d7455](https://gitee.com/openharmony/kernel_liteos_m/commits/67d74559d1bb3820bd51da2e2bf43e41e1224314))
* L0 支持Trace ([56c93a6](https://gitee.com/openharmony/kernel_liteos_m/commits/56c93a641b3c7a3d1ef9293ee3c95cccb559191f)), closes [#I41Y9Y](https://gitee.com/openharmony/kernel_liteos_m/issues/I41Y9Y)
* L0 支持低功耗框架 ([558ce14](https://gitee.com/openharmony/kernel_liteos_m/commits/558ce14becab05102497afca90a43d89abaf5531)), closes [#I3UDNV](https://gitee.com/openharmony/kernel_liteos_m/issues/I3UDNV)
* L0 测试用例支持risc-v qemu 编译运行 ([2dd6c08](https://gitee.com/openharmony/kernel_liteos_m/commits/2dd6c080daab52eaf61e5eefe7b581190670e8a7)), closes [#I3YILG](https://gitee.com/openharmony/kernel_liteos_m/issues/I3YILG)
* **liteos_m:** support backtrace for riscv ([ca1792a](https://gitee.com/openharmony/kernel_liteos_m/commits/ca1792aa4b5a85ca4ea6e41b265858c6b24a6897)), closes [#I3RVXY](https://gitee.com/openharmony/kernel_liteos_m/issues/I3RVXY)
* los_memory.c代码重复率处理 ([475db62](https://gitee.com/openharmony/kernel_liteos_m/commits/475db62db1ac285fc6cab91f09af618870269580)), closes [#I44WNU](https://gitee.com/openharmony/kernel_liteos_m/issues/I44WNU)
* M33平台支持GCC环境的Trustzone ([d469da2](https://gitee.com/openharmony/kernel_liteos_m/commits/d469da29c6d316640eaa7865f98ad8d6e2eb6d44)), closes [#I3Y85I](https://gitee.com/openharmony/kernel_liteos_m/issues/I3Y85I)
* provide getifaddrs for liteos_m ([948501a](https://gitee.com/openharmony/kernel_liteos_m/commits/948501ac73e7887f4585eca156ad760acc2a3b1f)), closes [#I400S5](https://gitee.com/openharmony/kernel_liteos_m/issues/I400S5)
* risc-v gn 编译框架添加汇编编译选项 ([76dc343](https://gitee.com/openharmony/kernel_liteos_m/commits/76dc343c888ec4e65eba43feb49921111b144640)), closes [#I3YS5Y](https://gitee.com/openharmony/kernel_liteos_m/issues/I3YS5Y)
* Support arm9 ([6d7c9df](https://gitee.com/openharmony/kernel_liteos_m/commits/6d7c9dfe597affb260e8bd74fa606b6d68c2b49a)), closes [#I3WLCN](https://gitee.com/openharmony/kernel_liteos_m/issues/I3WLCN)
* Support arm9 fiq swi mode and using system mode. ([97b5b08](https://gitee.com/openharmony/kernel_liteos_m/commits/97b5b0837be4b8df1381af29a6f650f243322840)), closes [#I3WLCN](https://gitee.com/openharmony/kernel_liteos_m/issues/I3WLCN)
* support non-continuous memory regions ([0facb0c](https://gitee.com/openharmony/kernel_liteos_m/commits/0facb0c0179f29a08b037655bbe691c98384dc84)), closes [#I43XOP](https://gitee.com/openharmony/kernel_liteos_m/issues/I43XOP)
* 完善m核qemu串口驱动，支持shell输入 ([c4dc5ab](https://gitee.com/openharmony/kernel_liteos_m/commits/c4dc5ab0f8b5caae55efdbedf59ab07f5006f6c2)), closes [#I46N7F](https://gitee.com/openharmony/kernel_liteos_m/issues/I46N7F)
* 按任务统计已经alloc的内存大小 ([53117f9](https://gitee.com/openharmony/kernel_liteos_m/commits/53117f9f47a8dd3023e83dc0cc0de224eab9719f)), closes [#I44WNU](https://gitee.com/openharmony/kernel_liteos_m/issues/I44WNU)
* 支持native动态加载组件 ([d753834](https://gitee.com/openharmony/kernel_liteos_m/commits/d75383400efe5d9ca0ae4093e4f2e65fee03024b)), closes [#I418HJ](https://gitee.com/openharmony/kernel_liteos_m/issues/I418HJ)
* 新增LOSCFG_BASE_CORE_TSK_SWITCH_HOOK 宏定义配置 ([39efd6f](https://gitee.com/openharmony/kernel_liteos_m/commits/39efd6fb30ee7dc9159811f5d37dffcdfe81bae0)), closes [#I40Q9O](https://gitee.com/openharmony/kernel_liteos_m/issues/I40Q9O)
* 新增ROM/RAM分析工具 ([ee3403d](https://gitee.com/openharmony/kernel_liteos_m/commits/ee3403deaf24dc612b51354f31f795fb0b9a49b5))
* 补充fs相关函数用例 ([2a95af5](https://gitee.com/openharmony/kernel_liteos_m/commits/2a95af5bf1b412de27b4518d0abf7c335366d4d7)), closes [#I3UQ4F](https://gitee.com/openharmony/kernel_liteos_m/issues/I3UQ4F)
* 解决fclose等函数工具链重定义问题 ([b244ad4](https://gitee.com/openharmony/kernel_liteos_m/commits/b244ad4ad0da8d151b69071fdd6db808c91f85a5)), closes [#I3UG6O](https://gitee.com/openharmony/kernel_liteos_m/issues/I3UG6O)


### BREAKING CHANGES

* 新增4个对外接口，声明在los_dynlink.h文件中，分别为：
(1) LOS_DynlinkInit: 动态加载模块初始化。
(2) LOS_SoLoad: 加载指定路径的共享库。
(3) LOS_FindSym: 根据共享库句柄查找指定符号。
(4) LOS_SoUnload: 卸载共享库。
* 1.新增一系列trace的对外API，位于los_trace.h中.
LOS_TRACE_EASY简易插桩
LOS_TRACE标准插桩
LOS_TraceInit配置Trace缓冲区的地址和大小
LOS_TraceStart开启事件记录
LOS_TraceStop停止事件记录
LOS_TraceRecordDump输出Trace缓冲区数据
LOS_TraceRecordGet获取Trace缓冲区的首地址
LOS_TraceReset清除Trace缓冲区中的事件
LOS_TraceEventMaskSet设置事件掩码，仅记录某些模块的事件
LOS_TraceHwiFilterHookReg注册过滤特定中断号事件的钩子函数
* 1.原调度中基于tick timer的低功耗扩展和当前的pm模块合并，删除原对外接口LOS_SchedSleepInit,
　变为pm模块统一提供的LOS_PmRegistered接口.
2.原来在arch los_timer.h下提供的低功耗模式为枚举LOS_SysSleepEnum，其中OS_SYS_NORMAL_SLEEP
  和OS_SYS_DEEP_SLEEP不符合对外定义，统一修改为LOS_SYS_NORMAL_SLEEP和LOS_SYS_DEEP_SLEEP,
  并移至los_pm.h中.
3.VOID HalEnterSleep(LOS_SysSleepEnum sleep) 变更为UINT32 HalEnterSleep(VOID).



