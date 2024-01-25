
int ynand_initialise(struct yaffs_dev *dev);
int ynand_deinitialise(struct yaffs_dev *dev);
int ynand_rd_chunk (struct yaffs_dev *dev, int pageId,
					  u8 *data, int dataLength,
					  u8 *spare, int spareLength,
					  enum yaffs_ecc_result *ecc_result);

int ynand_wr_chunk (struct yaffs_dev *dev, int pageId,
					   const u8 *data, int dataLength,
					   const u8 *spare, int spareLength);

int ynand_erase(struct yaffs_dev *dev, int blockId);

int ynand_check_block_bad(struct yaffs_dev *dev, int blockId);

int ynand_mark_block_bad(struct yaffs_dev *dev, int blockId);
