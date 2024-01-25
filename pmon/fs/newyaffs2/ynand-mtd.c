
/*
 *   nand driver for testing YAFFS 
 */


#include "yaffs_guts.h"
#include <string.h>
#include <time.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include "ynand-mtd.h"



int ynand_initialise(struct yaffs_dev *dev)
{
	return YAFFS_OK;
}


int ynand_deinitialise(struct yaffs_dev *dev)
{
	return YAFFS_OK;
}

int ynand_rd_chunk (struct yaffs_dev *dev, int pageId,
					  u8 *data, int dataLength,
					  u8 *spare, int spareLength,
					  enum yaffs_ecc_result *ecc_result)
{
	struct mtd_info *mtd = dev->driver_context;
	struct nand_chip *chip = mtd->priv;
	struct nand_oobfree *free = chip->ecc.layout->oobfree;

	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, pageId);
	nand_generic_read(mtd, data, data?dataLength:0, 0, spare, spareLength, free->offset, 1);
	
	if (ecc_result) 
		*ecc_result  = YAFFS_ECC_RESULT_NO_ERROR;

	return YAFFS_OK;
}

int ynand_wr_chunk (struct yaffs_dev *dev, int pageId,
					   const u8 *data, int dataLength,
					   const u8 *spare, int spareLength)
{
	struct mtd_info *mtd = dev->driver_context;
	struct nand_chip *chip = mtd->priv;
	struct nand_oobfree *free = chip->ecc.layout->oobfree;
	int status;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, pageId);
	nand_generic_write(mtd, data, data?dataLength:0, 0, spare, spareLength, free->offset , 1);

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);

	return YAFFS_OK;
}


int ynand_erase(struct yaffs_dev *dev, int blockId)
{
	int status;
	struct mtd_info *mtd = dev->driver_context;
	struct nand_chip *chip = mtd->priv;
	int page = blockId  << (chip->phys_erase_shift - chip->page_shift);

	chip->cmdfunc(mtd, NAND_CMD_ERASE1, -1, page);
	chip->cmdfunc(mtd, NAND_CMD_ERASE2, -1, -1);

	status = chip->waitfunc(mtd, chip);
	return YAFFS_OK;
}

int ynand_check_block_bad(struct yaffs_dev *dev, int blockId)
{
	unsigned char s[2];
	struct mtd_info *mtd = dev->driver_context;
	struct nand_chip *chip = mtd->priv;
	int pageId = blockId * mtd->erasesize/mtd->writesize;
	
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, pageId);
	nand_generic_read(mtd, NULL, 0, 0, s, 2, 0, 1);
	
	/* Check that bad block marker is not set */
	if(yaffs_hweight8(s[0]) + yaffs_hweight8(s[1]) < 14)
	{
		//printf("check_block s[0]=%02x,s[1]=%02x, Y=%02x\n", s[0], s[1], 'Y');
		return YAFFS_FAIL;
	}
	else
		return YAFFS_OK;
}

int ynand_mark_block_bad(struct yaffs_dev *dev, int blockId)
{
	struct mtd_info *mtd = dev->driver_context;
	struct nand_chip *chip = mtd->priv;
	int page = blockId * mtd->erasesize/mtd->writesize;
	u8 *spare[2] = { 'Y', 'Y' };
	int status;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);
	nand_generic_write(mtd, NULL, 0, 0, spare, 2, 0, 1);
	chip->cmdfunc(mtd, NAND_CMD_CACHEDPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);

	printf("mark blockId=%d bad.\n", blockId);

	return YAFFS_OK;
}
