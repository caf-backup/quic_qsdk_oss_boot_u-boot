/*
 * Copyright (c) 2015, 2016 The Linux Foundation. All rights reserved.
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
#include <asm/io.h>
#include <asm/errno.h>
#include <environment.h>
#include <configs/ipq40xx.h>
#include <nand.h>
#include <part.h>
#include <asm/arch-qcom-common/smem.h>
#include <asm/arch-ipq40xx/clk.h>
#include <asm/arch-qcom-common/scm.h>
#include <asm/arch-qcom-common/qpic_nand.h>
#include <asm/arch-qcom-common/gpio.h>
#include <jffs2/load_kernel.h>
#include <fdtdec.h>
#include <asm/arch-qcom-common/uart.h>
#include "fdt_info.h"
#include <asm/arch-ipq40xx/ess/ipq40xx_edma.h>
#include <phy.h>
#include "ipq40xx_edma_eth.h"
#include "qca_common.h"

DECLARE_GLOBAL_DATA_PTR;

qca_mmc mmc_host;

const char *rsvd_node = "/reserved-memory";
const char *del_node[] = {"rsvd1",
			  "rsvd2",
			  "wifi_dump",
			  NULL};
const add_node_t add_node[] = {
	{
		.nodename = "rsvd1",
		.val[0] = htonl(RESERVE_ADDRESS_START),
		.val[1] = htonl(RESERVE_ADDRESS_SIZE)
	},
	{
	}
};

extern loff_t board_env_offset;
extern loff_t board_env_range;
extern loff_t board_env_size;

extern int ipq_spi_init(u16);
extern int mmc_env_init(void);
extern void mmc_env_relocate_spec(void);

extern int ipq40xx_edma_init(ipq40xx_edma_board_cfg_t *edma_cfg);
extern int ipq40xx_qca8075_phy_init(struct ipq40xx_eth_dev *cfg);
extern int ipq40xx_qca8033_phy_init(struct ipq40xx_eth_dev *cfg);
extern void ipq40xx_register_switch(
	int (*sw_init)(struct ipq40xx_eth_dev *cfg));

void qca_serial_init(struct ipq_serial_platdata *plat)
{
	int node;
	node = fdt_path_offset(gd->fdt_blob, "/serial/serial_gpio");
	if (node < 0) {
		printf("Could not find serial_gpio node\n");
		return;
	}

	qca_gpio_init(node);
}

void reset_cpu(ulong addr)
{
}

void reset_crashdump(void)
{
	unsigned int magic_cookie = CLEAR_MAGIC;
	unsigned int clear_info[] =
		{ 1 /* Disable wdog debug */, 0 /* SDI enable*/, };
        scm_call(SCM_SVC_BOOT, SCM_CMD_TZ_CONFIG_HW_FOR_RAM_DUMP_ID,
		 (const void *)&clear_info, sizeof(clear_info), NULL, 0);
        scm_call(SCM_SVC_BOOT, SCM_CMD_TZ_FORCE_DLOAD_ID, &magic_cookie,
		 sizeof(magic_cookie), NULL, 0);
}

void board_nand_init(void)
{
	struct qpic_nand_init_config config;
	int node, gpio_node;
	fdt_addr_t nand_base;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
				      COMPAT_QCOM_QPIC_NAND_V1_4_20);

	if (node < 0) {
		printf("Could not find nand-flash in device tree\n");
		return;
	}

	nand_base = fdtdec_get_addr(gd->fdt_blob, node, "reg");

	if (nand_base == FDT_ADDR_T_NONE) {
		printf("No valid NAND base address found in device tree\n");
		return;
        }
	config.pipes.read_pipe = DATA_PRODUCER_PIPE;
	config.pipes.write_pipe = DATA_CONSUMER_PIPE;
	config.pipes.cmd_pipe = CMD_PIPE;

	config.pipes.read_pipe_grp = DATA_PRODUCER_PIPE_GRP;
	config.pipes.write_pipe_grp = DATA_CONSUMER_PIPE_GRP;
	config.pipes.cmd_pipe_grp = CMD_PIPE_GRP;

	config.bam_base = QPIC_BAM_CTRL_BASE;
	config.nand_base = nand_base;
	config.ee = QPIC_NAND_EE;
	config.max_desc_len = QPIC_NAND_MAX_DESC_LEN;

	gpio_node = fdt_subnode_offset(gd->fdt_blob, node, "nand_gpio");
	if (gpio_node >= 0) {
		qca_gpio_init(gpio_node);
		qpic_nand_init(&config);
	}

	gpio_node = fdt_path_offset(gd->fdt_blob, "/spi/spi_gpio");
	if (gpio_node >= 0)
		qca_gpio_init(gpio_node);

#ifdef CONFIG_IPQ40XX_SPI
	ipq_spi_init(CONFIG_IPQ_SPI_NOR_INFO_IDX);
#endif
}

static void ipq40xx_edma_common_init(void)
{
	writel(1, GCC_ESS_BCR);
	mdelay(10);
	writel(0, GCC_ESS_BCR);
	mdelay(100);

	writel(1, GCC_MDIO_AHB_CBCR);
	writel(MDIO_CTRL_0_DIV(0xff) |
		MDIO_CTRL_0_MDC_MODE |
		MDIO_CTRL_0_GPHY(0xa), MDIO_CTRL_0_REG);
}

int board_eth_init(bd_t *bis)
{
	u32 status;
	int gpio_node, node, len;
	ipq40xx_edma_board_cfg_t* edma_cfg =
		(ipq40xx_edma_board_cfg_t*)malloc(sizeof(ipq40xx_edma_board_cfg_t));

	gpio_node = fdt_path_offset(gd->fdt_blob, "/ess-switch/sw_gpio");
	if (gpio_node >= 0)
		qca_gpio_init(gpio_node);

	ipq40xx_edma_common_init();
	switch (gd->bd->bi_arch_number) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
		/* 8075 out of reset */
		mdelay(100);
		gpio_set_value(62, 1);
		ipq40xx_register_switch(ipq40xx_qca8075_phy_init);
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		/* 8075 out of reset */
		mdelay(100);
		gpio_set_value(59, 1);
		ipq40xx_register_switch(ipq40xx_qca8075_phy_init);
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
		/* 8075 out of reset */
		mdelay(100);
		gpio_set_value(47, 1);
		ipq40xx_register_switch(ipq40xx_qca8075_phy_init);
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		/* 8075 out of reset */
		mdelay(100);
		gpio_set_value(67, 1);
		ipq40xx_register_switch(ipq40xx_qca8075_phy_init);
		break;
	case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		/* 8075 out of reset */
		mdelay(100);
		gpio_set_value(19, 1);
		ipq40xx_register_switch(ipq40xx_qca8075_phy_init);
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
		/* 8075 out of reset */
		mdelay(100);
		gpio_set_value(41, 1);
		ipq40xx_register_switch(ipq40xx_qca8075_phy_init);
		break;
	default:
		break;
	}
	node = fdt_path_offset(gd->fdt_blob, "/edma_cfg");
	if (node < 0) {
		printf("Error: edma_cfg not specified in dts");
		return -1;
	}
	edma_cfg->unit = fdtdec_get_uint(gd->fdt_blob, node, "unit", 0);
	edma_cfg->phy = fdtdec_get_uint(gd->fdt_blob, node, "phy", 0);
	strcpy(edma_cfg->phy_name, fdt_getprop(gd->fdt_blob, node, "phy_name", &len));

	status = ipq40xx_edma_init(edma_cfg);
	return status;
}

#ifdef CONFIG_QCA_MMC
int board_mmc_env_init(void)
{
	block_dev_desc_t *blk_dev;
	disk_partition_t disk_info;
	int ret;

	if (mmc_init(mmc_host.mmc)) {
		/* The HS mode command(cmd6) is getting timed out. So mmc card is
		 * not getting initialized properly. Since the env partition is not
		 * visible, the env default values are writing into the default
		 * partition (start of the mmc device). So do a reset again.
		 */
		if (mmc_init(mmc_host.mmc)) {
			printf("MMC init failed \n");
			return -1;
		}
	}
	blk_dev = mmc_get_dev(mmc_host.dev_num);
	ret = get_partition_info_efi_by_name(blk_dev,
				"0:APPSBLENV", &disk_info);

	if (ret == 0) {
		board_env_offset = disk_info.start * disk_info.blksz;
		board_env_size = disk_info.size * disk_info.blksz;
		board_env_range = board_env_size;
		BUG_ON(board_env_size > CONFIG_ENV_SIZE_MAX);
	}
	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int ret;
	int node, gpio_node;
	fdt_addr_t base;
	qca_smem_flash_info_t *sfi = &qca_smem_flash_info;

	node = fdt_path_offset(gd->fdt_blob, "/sdhci");
	if (node < 0) {
		printf("Could not find mmc-flash in device tree\n");
		return -1;
	}

	gpio_node = fdt_subnode_offset(gd->fdt_blob, node, "mmc_gpio");
	qca_gpio_init(gpio_node);

	base = fdtdec_get_addr(gd->fdt_blob, node, "reg");
	if (base == FDT_ADDR_T_NONE) {
		printf("No valid MMC base address found in device tree\n");
		return -1;
        }

	mmc_host.base = base;
	mmc_host.clk_mode = MMC_IDENTIFY_MODE;
	emmc_clock_config(mmc_host.clk_mode);

	ret = qca_mmc_init(bis, &mmc_host);

	if (!ret && sfi->flash_type == SMEM_BOOT_MMC_FLASH) {
		ret = board_mmc_env_init();
	}

	return ret;
}

void board_mmc_deinit(void)
{
	emmc_clock_disable();
}
#endif


static void pcie_clock_init()
{

	/* Enable PCIE CLKS */
	pcie_clock_enable(GCC_PCIE_SLEEP_CBCR);
	pcie_clock_enable(GCC_PCIE_AXI_M_CBCR);
	pcie_clock_enable(GCC_PCIE_AXI_S_CBCR);
	pcie_clock_enable(GCC_PCIE_AHB_CBCR);

}

void board_pci_init(int id)
{
	int node, gpio_node;
	char name[16];

	sprintf(name, "pci%d", id);
	node = fdt_path_offset(gd->fdt_blob, name);
	if (node < 0) {
		printf("Could not find PCI in device tree\n");
		return;
	}
	gpio_node = fdt_subnode_offset(gd->fdt_blob, node, "pci_gpio");
	if (gpio_node >= 0)
		qca_gpio_init(gpio_node);

	pcie_clock_init();
}

void board_pci_deinit(void)
{

	/* Disable PCIE CLKS */
	pcie_clock_disable(GCC_PCIE_SLEEP_CBCR);
	pcie_clock_disable(GCC_PCIE_AXI_M_CBCR);
	pcie_clock_disable(GCC_PCIE_AXI_S_CBCR);
	pcie_clock_disable(GCC_PCIE_AHB_CBCR);
}