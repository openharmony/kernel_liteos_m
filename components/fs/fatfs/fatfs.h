/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _FATFS_H
#define _FATFS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* Format options */
#define FMT_FAT      0x01
#define FMT_FAT32    0x02
#define FMT_ANY      0x07

/**
  * @brief divide a physical drive (SD card, U disk, and MMC card), this function is OHOS-specific
  * @param pdrv physical drive number.
  * @param partTbl list of partition size to create on the drive.
  *   -- item is <= 100: specifies the partition size in percentage of the entire drive space.
  *   -- item is > 100: specifies number of sectors.
  * @return fdisk result
  * @retval -1 fdisk error
  * @retval 0 fdisk successful
  */
int fatfs_fdisk(int pdrv, const unsigned int *partTbl);

/**
  * @brief format FAT device (SD card, U disk, and MMC card), this function is OHOS-specific
  * @param dev device name.
  * @param sectors sectors per cluster, can be 0 OR power of 2. The sector size for standard FAT volumes is 512 bytes.
  *    -- sector number is 0 OR >128: automatically choose the appropriate cluster size.
  *    -- sector number is 1 ~ 128: cluster size = sectors per cluster * 512B.
  * @param option file system type.
  *    -- FMT_FAT
  *    -- FMT_FAT32
  *    -- FMT_ANY
  * @return format result
  * @retval -1 format error
  * @retval 0 format successful
  */
int fatfs_format(const char *dev, int sectors, int option);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _FATFS_H */