/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * 产品 BUG 用例总隔离宏(全局唯一定义点)。
 *
 * 单一宏控制所有因商用代码 BUG 导致崩溃/卡死/失败而隔离的用例:
 * 删除下方 define 即恢复全部隔离用例的编译执行。
 *
 * 当前隔离清单(缺陷编号见 总览.md 附录 B):
 *   - ItLosTestCmsisKernel008 (BUG_01: osKernelInitialize guard 失效,
 *     仅 qemu/ARM 隔离,ws63 guard 有效保持执行)
 *   - ItPosixPipe003          (BUG_02: poll-on-pipe ws63 复位/qemu sem 竞态)
 *   - ItPosixSignal091        (BUG_03: sigtimedwait 后续 semaphore 误 post)
 *
 * 历史隔离项(已消解):
 *   - ItPosixPthread034 (BUG_07 布局敏感) —— 经接口定性复审, __sched_cpucount
 *     为 musl 实现内部符号(双下划线保留命名空间,非 POSIX 对外接口,posix 串讲
 *     G2 分类"内部辅助"),本就不应直测;用例降级为不直测并从 BUILD.gn 移除,
 *     BUG_07 无需布局修复即消解。其可测行为由 pthread_032/033(affinity 族
 *     对外接口)间接覆盖。
 */

#ifndef _BUG_ISOLATE_H
#define _BUG_ISOLATE_H

#define PRODUCT_BUG_ISOLATE 1

#endif /* _BUG_ISOLATE_H */
