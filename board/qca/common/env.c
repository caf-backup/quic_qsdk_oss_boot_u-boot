/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch-qcom-common/smem.h>

extern void nand_env_relocate_spec(void);
extern int nand_env_init(void);

#ifdef CONFIG_QCA_MMC
extern int mmc_env_init(void);
extern void mmc_env_relocate_spec(void);
#endif

/*
 * Function description: Board specific initialization.
 * I/P : None
 * O/P : integer, 0 - no error.
 */
int env_init(void)
{
	int ret = 0;
	qca_smem_flash_info_t sfi;

	smem_get_boot_flash(&sfi.flash_type,
			    &sfi.flash_index,
			    &sfi.flash_chip_select,
			    &sfi.flash_block_size,
			    &sfi.flash_density);

	if (sfi.flash_type != SMEM_BOOT_MMC_FLASH) {
		ret = nand_env_init();
#ifdef CONFIG_QCA_MMC
	} else {
		ret = mmc_env_init();
#endif
	}

	return ret;
}

void env_relocate_spec(void)
{
	qca_smem_flash_info_t sfi;

	smem_get_boot_flash(&sfi.flash_type,
			    &sfi.flash_index,
			    &sfi.flash_chip_select,
			    &sfi.flash_block_size,
			    &sfi.flash_density);

	if (sfi.flash_type != SMEM_BOOT_MMC_FLASH) {
		nand_env_relocate_spec();
#ifdef CONFIG_QCA_MMC
	} else {
                mmc_env_relocate_spec();
#endif
	}

};