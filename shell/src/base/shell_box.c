/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef LOSCFG_BOX

#include "shell_pri.h"
#include "los_shell.h"

UINT32 LOS_ShellCreate(VOID)
{
    if (OsShellBoxPermCheck() != LOS_OK) {
        return LOS_ERRNO_SHELL_PERM_ERROR;
    }

    if (OsShellInit(0) != LOS_OK) {
        return LOS_ERRNO_SHELL_INIT_ERROR;
    }
    return LOS_OK;
}

UINT32 LOS_ShellCmdReg(CmdType cmdType, CHAR *cmdKey, UINT32 paraNum, CmdCallBackFunc cmdProc)
{
    if (OsShellBoxShCbGet() == NULL) {
        return LOS_ERRNO_SHELL_PERM_ERROR;
    }

    return osCmdReg(cmdType, cmdKey, paraNum, cmdProc);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdate-time"
STATIC UINT32 OsShellCmdUname(UINT32 argc, const CHAR *argv[])
{
    if (argc == 0) {
        PRINTK("%s\n", HW_LITEOS_SYSNAME);
        return 0;
    }

    if (argc == 1) {
        if (strcmp(argv[0], "-a") == 0) {
            PRINTK("%s %s %s\n", HW_LITEOS_KERNEL_VERSION_STRING, __DATE__, __TIME__);
            return 0;
        } else if (strcmp(argv[0], "-s") == 0) {
            PRINTK("%s\n", HW_LITEOS_SYSNAME);
            return 0;
        } else if (strcmp(argv[0], "-t") == 0) {
            PRINTK("build date : %s %s", __DATE__, __TIME__);
            return 0;
        } else if (strcmp(argv[0], "-v") == 0) {
            PRINTK("%s %s %s\n", HW_LITEOS_KERNEL_VERSION_STRING, __DATE__, __TIME__);
            return 0;
        } else if (strcmp(argv[0], "--help") == 0) {
            PRINTK("-a,            print all information\n"
                   "-s,            print the kernel name\n"
                   "-t,            print the build date\n"
                   "-v,            print the kernel version\n");
            return 0;
        }
    }

    PRINTK("uname: invalid option %s\n"
           "Try 'uname --help' for more information.\n",
           argv[0]);
    return OS_ERROR;
}
#pragma GCC diagnostic pop

STATIC UINT32 OsShellCmdBoxsh(UINT32 argc, const CHAR **argv)
{
    if (argc != 1) {
        PRINTK("\nargc != 1\n");
        return OS_ERROR;
    }

    if ((strlen(argv[0]) > 1) || (argv[0][0] < '0') || (argv[0][0] > '9')) {
        PRINTK("\nBox Id is invalid!\n");
        return OS_ERROR;
    }

    UINT16 id = (UINT16)(argv[0][0] - '0');
    if (OsShellBoxIdSet(id) != LOS_OK) {
        PRINTK("\nBox Id is wrong!\n");
        return OS_ERROR;
    }

    PRINTK("\nBox Id: %u\n", g_curShellBoxId);
    return 0;
}

STATIC UINT32 OsShellCmdExit(UINT32 argc, const CHAR **argv)
{
    (VOID)argv;
    if (argc != 0) {
        return OS_ERROR;
    }

    (VOID)OsShellBoxIdSet(LOS_SHELL_BOX_ID);
    PRINTK("\nShell Box Id: %u\n", g_curShellBoxId);
    return 0;
}

SHELLCMD_ENTRY(uname_shellcmd, CMD_TYPE_EX, "uname", XARGS, (CmdCallBackFunc)OsShellCmdUname);
SHELLCMD_ENTRY(boxinfo_shellcmd, CMD_TYPE_EX, "boxinfo", 0, (CmdCallBackFunc)OsShellCmdBoxInfo);
SHELLCMD_ENTRY(boxsh_shellcmd, CMD_TYPE_EX, "boxsh", 0, (CmdCallBackFunc)OsShellCmdBoxsh);
SHELLCMD_ENTRY(exit_shellcmd, CMD_TYPE_EX, "exit", 0, (CmdCallBackFunc)OsShellCmdExit);
#endif

