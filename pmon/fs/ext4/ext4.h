#ifndef __INCLUDE_EXT4_H
#define __INCLUDE_EXT4_H

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;
typedef unsigned long long __u64;
typedef signed long long __s64;
typedef signed int __s32;
typedef signed short __s16;

#define	BLOCK_1KB							1024
#define	EXT4_NDIR_BLOCKS					12
#define	EXT4_IND_BLOCK						EXT2_NDIR_BLOCKS
#define	EXT4_DIND_BLOCK						(EXT2_IND_BLOCK + 1)
#define	EXT4_TIND_BLOCK						(EXT2_DIND_BLOCK + 1)
#define	EXT4_N_BLOCKS						(EXT2_TIND_BLOCK + 1)
#define EXT4_NAME_LEN						255

#define EXT4_GOOD_OLD_INODE_SIZE 128

/* s_state */
#define EXT4_FS_UMNT						0x0001
#define EXT4_FS_ERR							0x0002
#define EXT4_FS_ORPHANS						0x0004
/* s_feature_compat */
#define EXT4_FEAT_COMP_DIR_PREALLOC			0x1
#define EXT4_FEAT_COMP_IMAGIC_INODES		0x2
#define EXT4_FEAT_COMP_HAS_JOURNAL			0x4
#define EXT4_FEAT_COMP_EXT_ATTR				0x8
#define EXT4_FEAT_COMP_RESIZE_INODE			0x10
#define EXT4_FEAT_COMP_DIR_INDEX			0x20
#define EXT4_FEAT_COMP_LAZY_BG				0x40
#define EXT4_FEAT_COMP_EXCLUDE_INODE		0x80
#define EXT4_FEAT_COMP_EXCLUDE_BITMAP		0x100
#define EXT4_FEAT_COMP_SPARSE_SUPER2		0x200
/* s_feature_incompat */
#define EXT4_FEAT_INCOMP_COMPRESSION		0x1
#define EXT4_FEAT_INCOMP_FILETYPE			0x2
#define EXT4_FEAT_INCOMP_RECOVER			0x4
#define EXT4_FEAT_INCOMP_JOURNAL_DEV		0x8
#define EXT4_FEAT_INCOMP_META_BG			0x10
#define EXT4_FEAT_INCOMP_EXTENTS			0x40
#define EXT4_FEAT_INCOMP_64BIT				0x80
#define EXT4_FEAT_INCOMP_MMP				0x100
#define EXT4_FEAT_INCOMP_FLEX_BG			0x200
#define EXT4_FEAT_INCOMP_EA_INODE			0x400
#define EXT4_FEAT_INCOMP_DIRDATA			0x1000
#define EXT4_FEAT_INCOMP_CSUM_SEED			0x2000
#define EXT4_FEAT_INCOMP_LARGEDIR			0x4000
#define EXT4_FEAT_INCOMP_INLINE_DATA		0x8000
#define EXT4_FEAT_INCOMP_ENCRYPT			0x10000
/* s_feature_ro_compat */
#define EXT4_FEAT_RO_COMP_SPARSE_SUPER		0x1
#define EXT4_FEAT_RO_COMP_LARGE_FILE    	0x2
#define EXT4_FEAT_RO_COMP_BTREE_DIR     	0x4
#define EXT4_FEAT_RO_COMP_HUGE_FILE     	0x8
#define EXT4_FEAT_RO_COMP_GDT_CSUM      	0x10
#define EXT4_FEAT_RO_COMP_DIR_NLINK     	0x20
#define EXT4_FEAT_RO_COMP_EXTRA_ISIZE   	0x40
#define EXT4_FEAT_RO_COMP_HAS_SNAPSHOT  	0x80
#define EXT4_FEAT_RO_COMP_QUOTA         	0x100
#define EXT4_FEAT_RO_COMP_BIGALLOC      	0x200
#define EXT4_FEAT_RO_COMP_METADATA_CSUM		0x400
#define EXT4_FEAT_RO_COMP_REPLICA           0x800
#define EXT4_FEAT_RO_COMP_READONLY          0x1000
#define EXT4_FEAT_RO_COMP_PROJECT			0x2000

/*
 * The fourth extended filesystem constants/structures
 */

/* data type for block offset of block group */
typedef int ext4_grpblk_t;

/* data type for filesystem-wide blocks number */
typedef unsigned long long ext4_fsblk_t;

/* data type for file logical block number */
typedef __u32 ext4_lblk_t;

/* data type for block group number */
typedef unsigned int ext4_group_t;

/* file type in Directory Entries */
enum {
	EXT_FT_UNKNOWN,
	EXT_FT_REG_FILE,
	EXT_FT_DIR,
	EXT_FT_CHRDEV,
	EXT_FT_BLKDEV,
	EXT_FT_FIFO,
	EXT_FT_SOCK,
	EXT_FT_SYMLINK,
	EXT_FT_MAX
};

enum SHIFT_DIRECTION {
	SHIFT_LEFT = 0,
	SHIFT_RIGHT,
};

/*
 * Flags used in mballoc's allocation_context flags field.
 *
 * Also used to show what's going on for debugging purposes when the
 * flag field is exported via the traceport interface
 */

/* prefer goal again. length */
#define EXT4_MB_HINT_MERGE		0x0001
/* blocks already reserved */
#define EXT4_MB_HINT_RESERVED		0x0002
/* metadata is being allocated */
#define EXT4_MB_HINT_METADATA		0x0004
/* first blocks in the file */
#define EXT4_MB_HINT_FIRST		0x0008
/* search for the best chunk */
#define EXT4_MB_HINT_BEST		0x0010
/* data is being allocated */
#define EXT4_MB_HINT_DATA		0x0020
/* don't preallocate (for tails) */
#define EXT4_MB_HINT_NOPREALLOC		0x0040
/* allocate for locality group */
#define EXT4_MB_HINT_GROUP_ALLOC	0x0080
/* allocate goal blocks or none */
#define EXT4_MB_HINT_GOAL_ONLY		0x0100
/* goal is meaningful */
#define EXT4_MB_HINT_TRY_GOAL		0x0200
/* blocks already pre-reserved by delayed allocation */
#define EXT4_MB_DELALLOC_RESERVED	0x0400
/* We are doing stream allocation */
#define EXT4_MB_STREAM_ALLOC		0x0800
/* Use reserved root blocks if needed */
#define EXT4_MB_USE_ROOT_BLOCKS		0x1000
/* Use blocks from reserved pool */
#define EXT4_MB_USE_RESERVED		0x2000

struct ext4_allocation_request {
	/* target inode for block we're allocating */
	struct inode *inode;
	/* how many blocks we want to allocate */
	unsigned int len;
	/* logical block in target inode */
	ext4_lblk_t logical;
	/* the closest logical allocated block to the left */
	ext4_lblk_t lleft;
	/* the closest logical allocated block to the right */
	ext4_lblk_t lright;
	/* phys. target (a hint) */
	ext4_fsblk_t goal;
	/* phys. block for the closest logical allocated block to the left */
	ext4_fsblk_t pleft;
	/* phys. block for the closest logical allocated block to the right */
	ext4_fsblk_t pright;
	/* flags. see above EXT4_MB_HINT_* */
	unsigned int flags;
};

/*
 * Logical to physical block mapping, used by ext4_map_blocks()
 *
 * This structure is used to pass requests into ext4_map_blocks() as
 * well as to store the information returned by ext4_map_blocks().  It
 * takes less room on the stack than a struct buffer_head.
 */
#define EXT4_MAP_NEW		(1 << BH_New)
#define EXT4_MAP_MAPPED		(1 << BH_Mapped)
#define EXT4_MAP_UNWRITTEN	(1 << BH_Unwritten)
#define EXT4_MAP_BOUNDARY	(1 << BH_Boundary)
#define EXT4_MAP_FLAGS		(EXT4_MAP_NEW | EXT4_MAP_MAPPED |\
				 EXT4_MAP_UNWRITTEN | EXT4_MAP_BOUNDARY)

struct ext4_map_blocks {
	ext4_fsblk_t m_pblk;
	ext4_lblk_t m_lblk;
	unsigned int m_len;
	unsigned int m_flags;
};

/*
 * Special inodes numbers
 */
#define	EXT4_BAD_INO		 1	/* Bad blocks inode */
#define EXT4_ROOT_INO		 2	/* Root inode */
#define EXT4_USR_QUOTA_INO	 3	/* User quota inode */
#define EXT4_GRP_QUOTA_INO	 4	/* Group quota inode */
#define EXT4_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT4_UNDEL_DIR_INO	 6	/* Undelete directory inode */
#define EXT4_RESIZE_INO		 7	/* Reserved group descriptors inode */
#define EXT4_JOURNAL_INO	 8	/* Journal inode */

/* First non-reserved inode for old ext4 filesystems */
#define EXT4_GOOD_OLD_FIRST_INO	11

/*
 * Maximal count of links to a file
 */
#define EXT4_LINK_MAX		65000

/*
 * Macro-instructions used to manage several block sizes
 */
#define EXT4_MIN_BLOCK_SIZE		1024
#define	EXT4_MAX_BLOCK_SIZE		65536
#define EXT4_MIN_BLOCK_LOG_SIZE		10
#define EXT4_MAX_BLOCK_LOG_SIZE		16
#define EXT4_MAX_CLUSTER_LOG_SIZE	30
#define	EXT4_ADDR_PER_BLOCK(s)		(EXT4_BLOCK_SIZE(s) / sizeof(__u32))
#define EXT4_CLUSTER_SIZE(s)		(EXT4_BLOCK_SIZE(s) << \
					 EXT4_SB(s)->s_cluster_bits)
#define EXT4_BLOCK_ALIGN(size, blkbits)		ALIGN((size), (1 << (blkbits)))
#define EXT4_MAX_BLOCKS(size, offset, blkbits) \
	((EXT4_BLOCK_ALIGN(size + offset, blkbits) >> blkbits) - (offset >> \
								  blkbits))

/* Translate a block number to a cluster number */
#define EXT4_B2C(sbi, blk)	((blk) >> (sbi)->s_cluster_bits)
/* Translate a cluster number to a block number */
#define EXT4_C2B(sbi, cluster)	((cluster) << (sbi)->s_cluster_bits)
/* Translate # of blks to # of clusters */
#define EXT4_NUM_B2C(sbi, blks)	(((blks) + (sbi)->s_cluster_ratio - 1) >> \
				 (sbi)->s_cluster_bits)
/* Mask out the low bits to get the starting block of the cluster */
#define EXT4_PBLK_CMASK(s, pblk) ((pblk) &				\
				  ~((ext4_fsblk_t) (s)->s_cluster_ratio - 1))
#define EXT4_LBLK_CMASK(s, lblk) ((lblk) &				\
				  ~((ext4_lblk_t) (s)->s_cluster_ratio - 1))
/* Get the cluster offset */
#define EXT4_PBLK_COFF(s, pblk) ((pblk) &				\
				 ((ext4_fsblk_t) (s)->s_cluster_ratio - 1))
#define EXT4_LBLK_COFF(s, lblk) ((lblk) &				\
				 ((ext4_lblk_t) (s)->s_cluster_ratio - 1))

/*
 * Structure of a blocks group descriptor
 */
struct ext4_group_desc
{
	__u32	bg_block_bitmap_lo;	/* Blocks bitmap block */
	__u32	bg_inode_bitmap_lo;	/* Inodes bitmap block */
	__u32	bg_inode_table_lo;	/* Inodes table block */
	__u16	bg_free_blocks_count_lo;/* Free blocks count */
	__u16	bg_free_inodes_count_lo;/* Free inodes count */
	__u16	bg_used_dirs_count_lo;	/* Directories count */
	__u16	bg_flags;		/* EXT4_BG_flags (INODE_UNINIT, etc) */
	__u32  bg_exclude_bitmap_lo;   /* Exclude bitmap for snapshots */
	__u16  bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bbitmap) LE */
	__u16  bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+ibitmap) LE */
	__u16  bg_itable_unused_lo;	/* Unused inodes count */
	__u16  bg_checksum;		/* crc16(sb_uuid+group+desc) */
	__u32	bg_block_bitmap_hi;	/* Blocks bitmap block MSB */
	__u32	bg_inode_bitmap_hi;	/* Inodes bitmap block MSB */
	__u32	bg_inode_table_hi;	/* Inodes table block MSB */
	__u16	bg_free_blocks_count_hi;/* Free blocks count MSB */
	__u16	bg_free_inodes_count_hi;/* Free inodes count MSB */
	__u16	bg_used_dirs_count_hi;	/* Directories count MSB */
	__u16  bg_itable_unused_hi;    /* Unused inodes count MSB */
	__u32  bg_exclude_bitmap_hi;   /* Exclude bitmap block MSB */
	__u16  bg_block_bitmap_csum_hi;/* crc32c(s_uuid+grp_num+bbitmap) BE */
	__u16  bg_inode_bitmap_csum_hi;/* crc32c(s_uuid+grp_num+ibitmap) BE */
	__u32   bg_reserved;
};

#define EXT4_BG_INODE_UNINIT	0x0001 /* Inode table/bitmap not in use */
#define EXT4_BG_BLOCK_UNINIT	0x0002 /* Block bitmap not in use */
#define EXT4_BG_INODE_ZEROED	0x0004 /* On-disk itable initialized to zero */

/*
 * Macro-instructions used to manage group descriptors
 */
#define EXT4_MIN_DESC_SIZE		32
#define EXT4_MIN_DESC_SIZE_64BIT	64
#define	EXT4_MAX_DESC_SIZE		EXT4_MIN_BLOCK_SIZE
#define EXT4_DESC_SIZE(s)		(EXT4_SB(s)->s_desc_size)

/*
 * Constants relative to the data blocks
 */
#define	EXT4_NDIR_BLOCKS		12
#define	EXT4_IND_BLOCK			EXT4_NDIR_BLOCKS
#define	EXT4_DIND_BLOCK			(EXT4_IND_BLOCK + 1)
#define	EXT4_TIND_BLOCK			(EXT4_DIND_BLOCK + 1)
#define	EXT4_N_BLOCKS			(EXT4_TIND_BLOCK + 1)

/*
 * Inode flags
 */
#define	EXT4_SECRM_FL			0x00000001 /* Secure deletion */
#define	EXT4_UNRM_FL			0x00000002 /* Undelete */
#define	EXT4_COMPR_FL			0x00000004 /* Compress file */
#define EXT4_SYNC_FL			0x00000008 /* Synchronous updates */
#define EXT4_IMMUTABLE_FL		0x00000010 /* Immutable file */
#define EXT4_APPEND_FL			0x00000020 /* writes to file may only append */
#define EXT4_NODUMP_FL			0x00000040 /* do not dump file */
#define EXT4_NOATIME_FL			0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define EXT4_DIRTY_FL			0x00000100
#define EXT4_COMPRBLK_FL		0x00000200 /* One or more compressed clusters */
#define EXT4_NOCOMPR_FL			0x00000400 /* Don't compress */
	/* nb: was previously EXT2_ECOMPR_FL */
#define EXT4_ENCRYPT_FL			0x00000800 /* encrypted file */
/* End compression flags --- maybe not all used */
#define EXT4_INDEX_FL			0x00001000 /* hash-indexed directory */
#define EXT4_IMAGIC_FL			0x00002000 /* AFS directory */
#define EXT4_JOURNAL_DATA_FL		0x00004000 /* file data should be journaled */
#define EXT4_NOTAIL_FL			0x00008000 /* file tail should not be merged */
#define EXT4_DIRSYNC_FL			0x00010000 /* dirsync behaviour (directories only) */
#define EXT4_TOPDIR_FL			0x00020000 /* Top of directory hierarchies*/
#define EXT4_HUGE_FILE_FL               0x00040000 /* Set to each huge file */
#define EXT4_EXTENTS_FL			0x00080000 /* Inode uses extents */
#define EXT4_EA_INODE_FL	        0x00200000 /* Inode used for large EA */
#define EXT4_EOFBLOCKS_FL		0x00400000 /* Blocks allocated beyond EOF */
#define EXT4_INLINE_DATA_FL		0x10000000 /* Inode has inline data. */
#define EXT4_PROJINHERIT_FL		0x20000000 /* Create with parents projid */
#define EXT4_RESERVED_FL		0x80000000 /* reserved for ext4 lib */

#define EXT4_FL_USER_VISIBLE		0x304BDFFF /* User visible flags */
#define EXT4_FL_USER_MODIFIABLE		0x204BC0FF /* User modifiable flags */

/* Flags we can manipulate with through EXT4_IOC_FSSETXATTR */
#define EXT4_FL_XFLAG_VISIBLE		(EXT4_SYNC_FL | \
					 EXT4_IMMUTABLE_FL | \
					 EXT4_APPEND_FL | \
					 EXT4_NODUMP_FL | \
					 EXT4_NOATIME_FL | \
					 EXT4_PROJINHERIT_FL)

/* Flags that should be inherited by new inodes from their parent. */
#define EXT4_FL_INHERITED (EXT4_SECRM_FL | EXT4_UNRM_FL | EXT4_COMPR_FL |\
			   EXT4_SYNC_FL | EXT4_NODUMP_FL | EXT4_NOATIME_FL |\
			   EXT4_NOCOMPR_FL | EXT4_JOURNAL_DATA_FL |\
			   EXT4_NOTAIL_FL | EXT4_DIRSYNC_FL |\
			   EXT4_PROJINHERIT_FL)

/* Flags that are appropriate for regular files (all but dir-specific ones). */
#define EXT4_REG_FLMASK (~(EXT4_DIRSYNC_FL | EXT4_TOPDIR_FL))

/* Flags that are appropriate for non-directories/regular files. */
#define EXT4_OTHER_FLMASK (EXT4_NODUMP_FL | EXT4_NOATIME_FL)

/* The only flags that should be swapped */
#define EXT4_FL_SHOULD_SWAP (EXT4_HUGE_FILE_FL | EXT4_EXTENTS_FL)

/*
 * Inode flags used for atomic set/get
 */
enum {
	EXT4_INODE_SECRM	= 0,	/* Secure deletion */
	EXT4_INODE_UNRM		= 1,	/* Undelete */
	EXT4_INODE_COMPR	= 2,	/* Compress file */
	EXT4_INODE_SYNC		= 3,	/* Synchronous updates */
	EXT4_INODE_IMMUTABLE	= 4,	/* Immutable file */
	EXT4_INODE_APPEND	= 5,	/* writes to file may only append */
	EXT4_INODE_NODUMP	= 6,	/* do not dump file */
	EXT4_INODE_NOATIME	= 7,	/* do not update atime */
/* Reserved for compression usage... */
	EXT4_INODE_DIRTY	= 8,
	EXT4_INODE_COMPRBLK	= 9,	/* One or more compressed clusters */
	EXT4_INODE_NOCOMPR	= 10,	/* Don't compress */
	EXT4_INODE_ENCRYPT	= 11,	/* Encrypted file */
/* End compression flags --- maybe not all used */
	EXT4_INODE_INDEX	= 12,	/* hash-indexed directory */
	EXT4_INODE_IMAGIC	= 13,	/* AFS directory */
	EXT4_INODE_JOURNAL_DATA	= 14,	/* file data should be journaled */
	EXT4_INODE_NOTAIL	= 15,	/* file tail should not be merged */
	EXT4_INODE_DIRSYNC	= 16,	/* dirsync behaviour (directories only) */
	EXT4_INODE_TOPDIR	= 17,	/* Top of directory hierarchies*/
	EXT4_INODE_HUGE_FILE	= 18,	/* Set to each huge file */
	EXT4_INODE_EXTENTS	= 19,	/* Inode uses extents */
	EXT4_INODE_EA_INODE	= 21,	/* Inode used for large EA */
	EXT4_INODE_EOFBLOCKS	= 22,	/* Blocks allocated beyond EOF */
	EXT4_INODE_INLINE_DATA	= 28,	/* Data in inode. */
	EXT4_INODE_PROJINHERIT	= 29,	/* Create with parents projid */
	EXT4_INODE_RESERVED	= 31,	/* reserved for ext4 lib */
};

/*
 * Since it's pretty easy to mix up bit numbers and hex values, we use a
 * build-time check to make sure that EXT4_XXX_FL is consistent with respect to
 * EXT4_INODE_XXX. If all is well, the macros will be dropped, so, it won't cost
 * any extra space in the compiled kernel image, otherwise, the build will fail.
 * It's important that these values are the same, since we are using
 * EXT4_INODE_XXX to test for flag values, but EXT4_XXX_FL must be consistent
 * with the values of FS_XXX_FL defined in include/linux/fs.h and the on-disk
 * values found in ext2, ext3 and ext4 filesystems, and of course the values
 * defined in e2fsprogs.
 *
 * It's not paranoia if the Murphy's Law really *is* out to get you.  :-)
 */
#define TEST_FLAG_VALUE(FLAG) (EXT4_##FLAG##_FL == (1 << EXT4_INODE_##FLAG))
#define CHECK_FLAG_VALUE(FLAG) BUILD_BUG_ON(!TEST_FLAG_VALUE(FLAG))

static inline void ext4_check_flag_values(void)
{
	CHECK_FLAG_VALUE(SECRM);
	CHECK_FLAG_VALUE(UNRM);
	CHECK_FLAG_VALUE(COMPR);
	CHECK_FLAG_VALUE(SYNC);
	CHECK_FLAG_VALUE(IMMUTABLE);
	CHECK_FLAG_VALUE(APPEND);
	CHECK_FLAG_VALUE(NODUMP);
	CHECK_FLAG_VALUE(NOATIME);
	CHECK_FLAG_VALUE(DIRTY);
	CHECK_FLAG_VALUE(COMPRBLK);
	CHECK_FLAG_VALUE(NOCOMPR);
	CHECK_FLAG_VALUE(ENCRYPT);
	CHECK_FLAG_VALUE(INDEX);
	CHECK_FLAG_VALUE(IMAGIC);
	CHECK_FLAG_VALUE(JOURNAL_DATA);
	CHECK_FLAG_VALUE(NOTAIL);
	CHECK_FLAG_VALUE(DIRSYNC);
	CHECK_FLAG_VALUE(TOPDIR);
	CHECK_FLAG_VALUE(HUGE_FILE);
	CHECK_FLAG_VALUE(EXTENTS);
	CHECK_FLAG_VALUE(EA_INODE);
	CHECK_FLAG_VALUE(EOFBLOCKS);
	CHECK_FLAG_VALUE(INLINE_DATA);
	CHECK_FLAG_VALUE(PROJINHERIT);
	CHECK_FLAG_VALUE(RESERVED);
}

/* Used to pass group descriptor data when online resize is done */
struct ext4_new_group_input {
	__u32 group;		/* Group number for this data */
	__u64 block_bitmap;	/* Absolute block number of block bitmap */
	__u64 inode_bitmap;	/* Absolute block number of inode bitmap */
	__u64 inode_table;	/* Absolute block number of inode table start */
	__u32 blocks_count;	/* Total number of blocks in this group */
	__u16 reserved_blocks;	/* Number of reserved blocks in this group */
	__u16 unused;
};

/* The struct ext4_new_group_input in kernel space, with free_blocks_count */
struct ext4_new_group_data {
	__u32 group;
	__u64 block_bitmap;
	__u64 inode_bitmap;
	__u64 inode_table;
	__u32 blocks_count;
	__u16 reserved_blocks;
	__u16 mdata_blocks;
	__u32 free_clusters_count;
};

/* Indexes used to index group tables in ext4_new_group_data */
enum {
	BLOCK_BITMAP = 0,	/* block bitmap */
	INODE_BITMAP,		/* inode bitmap */
	INODE_TABLE,		/* inode tables */
	GROUP_TABLE_COUNT,
};

/*
 * Flags used by ext4_map_blocks()
 */
	/* Allocate any needed blocks and/or convert an unwritten
	   extent to be an initialized ext4 */
#define EXT4_GET_BLOCKS_CREATE			0x0001
	/* Request the creation of an unwritten extent */
#define EXT4_GET_BLOCKS_UNWRIT_EXT		0x0002
#define EXT4_GET_BLOCKS_CREATE_UNWRIT_EXT	(EXT4_GET_BLOCKS_UNWRIT_EXT|\
						 EXT4_GET_BLOCKS_CREATE)
	/* Caller is from the delayed allocation writeout path
	 * finally doing the actual allocation of delayed blocks */
#define EXT4_GET_BLOCKS_DELALLOC_RESERVE	0x0004
	/* caller is from the direct IO path, request to creation of an
	unwritten extents if not allocated, split the unwritten
	extent if blocks has been preallocated already*/
#define EXT4_GET_BLOCKS_PRE_IO			0x0008
#define EXT4_GET_BLOCKS_CONVERT			0x0010
#define EXT4_GET_BLOCKS_IO_CREATE_EXT		(EXT4_GET_BLOCKS_PRE_IO|\
					 EXT4_GET_BLOCKS_CREATE_UNWRIT_EXT)
	/* Convert extent to initialized after IO complete */
#define EXT4_GET_BLOCKS_IO_CONVERT_EXT		(EXT4_GET_BLOCKS_CONVERT|\
					 EXT4_GET_BLOCKS_CREATE_UNWRIT_EXT)
	/* Eventual metadata allocation (due to growing extent tree)
	 * should not fail, so try to use reserved blocks for that.*/
#define EXT4_GET_BLOCKS_METADATA_NOFAIL		0x0020
	/* Don't normalize allocation size (used for fallocate) */
#define EXT4_GET_BLOCKS_NO_NORMALIZE		0x0040
	/* Request will not result in inode size update (user for fallocate) */
#define EXT4_GET_BLOCKS_KEEP_SIZE		0x0080
	/* Convert written extents to unwritten */
#define EXT4_GET_BLOCKS_CONVERT_UNWRITTEN	0x0100
	/* Write zeros to newly created written extents */
#define EXT4_GET_BLOCKS_ZERO			0x0200
#define EXT4_GET_BLOCKS_CREATE_ZERO		(EXT4_GET_BLOCKS_CREATE |\
					EXT4_GET_BLOCKS_ZERO)
	/* Caller will submit data before dropping transaction handle. This
	 * allows jbd2 to avoid submitting data before commit. */
#define EXT4_GET_BLOCKS_IO_SUBMIT		0x0400

/*
 * The bit position of these flags must not overlap with any of the
 * EXT4_GET_BLOCKS_*.  They are used by ext4_find_extent(),
 * read_extent_tree_block(), ext4_split_extent_at(),
 * ext4_ext_insert_extent(), and ext4_ext_create_new_leaf().
 * EXT4_EX_NOCACHE is used to indicate that the we shouldn't be
 * caching the extents when reading from the extent tree while a
 * truncate or punch hole operation is in progress.
 */
#define EXT4_EX_NOCACHE				0x40000000
#define EXT4_EX_FORCE_CACHE			0x20000000

/*
 * Flags used by ext4_free_blocks
 */
#define EXT4_FREE_BLOCKS_METADATA		0x0001
#define EXT4_FREE_BLOCKS_FORGET			0x0002
#define EXT4_FREE_BLOCKS_VALIDATED		0x0004
#define EXT4_FREE_BLOCKS_NO_QUOT_UPDATE		0x0008
#define EXT4_FREE_BLOCKS_NOFREE_FIRST_CLUSTER	0x0010
#define EXT4_FREE_BLOCKS_NOFREE_LAST_CLUSTER	0x0020

/*
 * ioctl commands
 */
#define	EXT4_IOC_GETFLAGS		FS_IOC_GETFLAGS
#define	EXT4_IOC_SETFLAGS		FS_IOC_SETFLAGS
#define	EXT4_IOC_GETVERSION		_IOR('f', 3, long)
#define	EXT4_IOC_SETVERSION		_IOW('f', 4, long)
#define	EXT4_IOC_GETVERSION_OLD		FS_IOC_GETVERSION
#define	EXT4_IOC_SETVERSION_OLD		FS_IOC_SETVERSION
#define EXT4_IOC_GETRSVSZ		_IOR('f', 5, long)
#define EXT4_IOC_SETRSVSZ		_IOW('f', 6, long)
#define EXT4_IOC_GROUP_EXTEND		_IOW('f', 7, unsigned long)
#define EXT4_IOC_GROUP_ADD		_IOW('f', 8, struct ext4_new_group_input)
#define EXT4_IOC_MIGRATE		_IO('f', 9)
 /* note ioctl 10 reserved for an early version of the FIEMAP ioctl */
 /* note ioctl 11 reserved for filesystem-independent FIEMAP ioctl */
#define EXT4_IOC_ALLOC_DA_BLKS		_IO('f', 12)
#define EXT4_IOC_MOVE_EXT		_IOWR('f', 15, struct move_extent)
#define EXT4_IOC_RESIZE_FS		_IOW('f', 16, __u64)
#define EXT4_IOC_SWAP_BOOT		_IO('f', 17)
#define EXT4_IOC_PRECACHE_EXTENTS	_IO('f', 18)
#define EXT4_IOC_SET_ENCRYPTION_POLICY	FS_IOC_SET_ENCRYPTION_POLICY
#define EXT4_IOC_GET_ENCRYPTION_PWSALT	FS_IOC_GET_ENCRYPTION_PWSALT
#define EXT4_IOC_GET_ENCRYPTION_POLICY	FS_IOC_GET_ENCRYPTION_POLICY

#define EXT4_IOC_FSGETXATTR		FS_IOC_FSGETXATTR
#define EXT4_IOC_FSSETXATTR		FS_IOC_FSSETXATTR

#define EXT4_IOC_SHUTDOWN _IOR ('X', 125, __u32)

/*
 * Flags for going down operation
 */
#define EXT4_GOING_FLAGS_DEFAULT		0x0	/* going down */
#define EXT4_GOING_FLAGS_LOGFLUSH		0x1	/* flush log but not data */
#define EXT4_GOING_FLAGS_NOLOGFLUSH		0x2	/* don't flush log nor data */


/* Max physical block we can address w/o extents */
#define EXT4_MAX_BLOCK_FILE_PHYS	0xFFFFFFFF

/* Max logical block we can support */
#define EXT4_MAX_LOGICAL_BLOCK		0xFFFFFFFF

/*
 * Structure of an inode on the disk
 */
struct ext4_inode {
	__u16	i_mode;		/* File mode */
	__u16	i_uid;		/* Low 16 bits of Owner Uid */
	__u32	i_size_lo;	/* Size in bytes */
	__u32	i_atime;	/* Access time */
	__u32	i_ctime;	/* Inode Change time */
	__u32	i_mtime;	/* Modification time */
	__u32	i_dtime;	/* Deletion Time */
	__u16	i_gid;		/* Low 16 bits of Group Id */
	__u16	i_links_count;	/* Links count */
	__u32	i_blocks_lo;	/* Blocks count */
	__u32	i_flags;	/* File flags */
	union {
		struct {
			__u32  l_i_version;
		} linux1;
		struct {
			__u32  h_i_translator;
		} hurd1;
		struct {
			__u32  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	__u32	i_block[EXT4_N_BLOCKS];/* Pointers to blocks */
	__u32	i_generation;	/* File version (for NFS) */
	__u32	i_file_acl_lo;	/* File ACL */
	__u32	i_size_high;
	__u32	i_obso_faddr;	/* Obsoleted fragment address */
	union {
		struct {
			__u16	l_i_blocks_high; /* were l_i_reserved1 */
			__u16	l_i_file_acl_high;
			__u16	l_i_uid_high;	/* these 2 fields */
			__u16	l_i_gid_high;	/* were reserved2[0] */
			__u16	l_i_checksum_lo;/* crc32c(uuid+inum+inode) LE */
			__u16	l_i_reserved;
		} linux2;
		struct {
			__u16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			__u16	h_i_mode_high;
			__u16	h_i_uid_high;
			__u16	h_i_gid_high;
			__u32	h_i_author;
		} hurd2;
		struct {
			__u16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			__u16	m_i_file_acl_high;
			__u32	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
	__u16	i_extra_isize;
	__u16	i_checksum_hi;	/* crc32c(uuid+inum+inode) BE */
	__u32  i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
	__u32  i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
	__u32  i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
	__u32  i_crtime;       /* File Creation time */
	__u32  i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
	__u32  i_version_hi;	/* high 32 bits for 64-bit version */
	__u32	i_projid;	/* Project ID */
};

struct move_extent {
	__u32 reserved;		/* should be zero */
	__u32 donor_fd;		/* donor file descriptor */
	__u64 orig_start;	/* logical start offset in block for orig */
	__u64 donor_start;	/* logical start offset in block for donor */
	__u64 len;		/* block length to be moved */
	__u64 moved_len;	/* moved block length */
};

#define EXT4_EPOCH_BITS 2
#define EXT4_EPOCH_MASK ((1 << EXT4_EPOCH_BITS) - 1)
#define EXT4_NSEC_MASK  (~0UL << EXT4_EPOCH_BITS)

enum {
	I_DATA_SEM_NORMAL = 0,
	I_DATA_SEM_OTHER,
	I_DATA_SEM_QUOTA,
};

/*
 * File system states
 */
#define	EXT4_VALID_FS			0x0001	/* Unmounted cleanly */
#define	EXT4_ERROR_FS			0x0002	/* Errors detected */
#define	EXT4_ORPHAN_FS			0x0004	/* Orphans being recovered */

/*
 * Misc. filesystem flags
 */
#define EXT2_FLAGS_SIGNED_HASH		0x0001  /* Signed dirhash in use */
#define EXT2_FLAGS_UNSIGNED_HASH	0x0002  /* Unsigned dirhash in use */
#define EXT2_FLAGS_TEST_FILESYS		0x0004	/* to test development code */

/*
 * Mount flags set via mount options or defaults
 */
#define EXT4_MOUNT_NO_MBCACHE		0x00001 /* Do not use mbcache */
#define EXT4_MOUNT_GRPID		0x00004	/* Create files with directory's group */
#define EXT4_MOUNT_DEBUG		0x00008	/* Some debugging messages */
#define EXT4_MOUNT_ERRORS_CONT		0x00010	/* Continue on errors */
#define EXT4_MOUNT_ERRORS_RO		0x00020	/* Remount fs ro on errors */
#define EXT4_MOUNT_ERRORS_PANIC		0x00040	/* Panic on errors */
#define EXT4_MOUNT_ERRORS_MASK		0x00070
#define EXT4_MOUNT_MINIX_DF		0x00080	/* Mimics the Minix statfs */
#define EXT4_MOUNT_NOLOAD		0x00100	/* Don't use existing journal*/
#ifdef CONFIG_FS_DAX
#define EXT4_MOUNT_DAX			0x00200	/* Direct Access */
#else
#define EXT4_MOUNT_DAX			0
#endif
#define EXT4_MOUNT_DATA_FLAGS		0x00C00	/* Mode for data writes: */
#define EXT4_MOUNT_JOURNAL_DATA		0x00400	/* Write data to journal */
#define EXT4_MOUNT_ORDERED_DATA		0x00800	/* Flush data before commit */
#define EXT4_MOUNT_WRITEBACK_DATA	0x00C00	/* No data ordering */
#define EXT4_MOUNT_UPDATE_JOURNAL	0x01000	/* Update the journal format */
#define EXT4_MOUNT_NO_UID32		0x02000  /* Disable 32-bit UIDs */
#define EXT4_MOUNT_XATTR_USER		0x04000	/* Extended user attributes */
#define EXT4_MOUNT_POSIX_ACL		0x08000	/* POSIX Access Control Lists */
#define EXT4_MOUNT_NO_AUTO_DA_ALLOC	0x10000	/* No auto delalloc mapping */
#define EXT4_MOUNT_BARRIER		0x20000 /* Use block barriers */
#define EXT4_MOUNT_QUOTA		0x40000 /* Some quota option set */
#define EXT4_MOUNT_USRQUOTA		0x80000 /* "old" user quota,
						 * enable enforcement for hidden
						 * quota files */
#define EXT4_MOUNT_GRPQUOTA		0x100000 /* "old" group quota, enable
						  * enforcement for hidden quota
						  * files */
#define EXT4_MOUNT_PRJQUOTA		0x200000 /* Enable project quota
						  * enforcement */
#define EXT4_MOUNT_DIOREAD_NOLOCK	0x400000 /* Enable support for dio read nolocking */
#define EXT4_MOUNT_JOURNAL_CHECKSUM	0x800000 /* Journal checksums */
#define EXT4_MOUNT_JOURNAL_ASYNC_COMMIT	0x1000000 /* Journal Async Commit */
#define EXT4_MOUNT_WARN_ON_ERROR	0x2000000 /* Trigger WARN_ON on error */
#define EXT4_MOUNT_DELALLOC		0x8000000 /* Delalloc support */
#define EXT4_MOUNT_DATA_ERR_ABORT	0x10000000 /* Abort on file data write */
#define EXT4_MOUNT_BLOCK_VALIDITY	0x20000000 /* Block validity checking */
#define EXT4_MOUNT_DISCARD		0x40000000 /* Issue DISCARD requests */
#define EXT4_MOUNT_INIT_INODE_TABLE	0x80000000 /* Initialize uninitialized itables */

/*
 * Mount flags set either automatically (could not be set by mount option)
 * based on per file system feature or property or in special cases such as
 * distinguishing between explicit mount option definition and default.
 */
#define EXT4_MOUNT2_EXPLICIT_DELALLOC	0x00000001 /* User explicitly
						      specified delalloc */
#define EXT4_MOUNT2_STD_GROUP_SIZE	0x00000002 /* We have standard group
						      size of blocksize * 8
						      blocks */
#define EXT4_MOUNT2_HURD_COMP		0x00000004 /* Support HURD-castrated
						      file systems */

#define EXT4_MOUNT2_EXPLICIT_JOURNAL_CHECKSUM	0x00000008 /* User explicitly
						specified journal checksum */

#define clear_opt(sb, opt)		EXT4_SB(sb)->s_mount_opt &= \
						~EXT4_MOUNT_##opt
#define set_opt(sb, opt)		EXT4_SB(sb)->s_mount_opt |= \
						EXT4_MOUNT_##opt
#define test_opt(sb, opt)		(EXT4_SB(sb)->s_mount_opt & \
					 EXT4_MOUNT_##opt)

#define clear_opt2(sb, opt)		EXT4_SB(sb)->s_mount_opt2 &= \
						~EXT4_MOUNT2_##opt
#define set_opt2(sb, opt)		EXT4_SB(sb)->s_mount_opt2 |= \
						EXT4_MOUNT2_##opt
#define test_opt2(sb, opt)		(EXT4_SB(sb)->s_mount_opt2 & \
					 EXT4_MOUNT2_##opt)

#define ext4_test_and_set_bit		__test_and_set_bit_le
#define ext4_set_bit			__set_bit_le
#define ext4_set_bit_atomic		ext2_set_bit_atomic
#define ext4_test_and_clear_bit		__test_and_clear_bit_le
#define ext4_clear_bit			__clear_bit_le
#define ext4_clear_bit_atomic		ext2_clear_bit_atomic
#define ext4_test_bit			test_bit_le
#define ext4_find_next_zero_bit		find_next_zero_bit_le
#define ext4_find_next_bit		find_next_bit_le


/*
 * Maximal mount counts between two filesystem checks
 */
#define EXT4_DFL_MAX_MNT_COUNT		20	/* Allow 20 mounts */
#define EXT4_DFL_CHECKINTERVAL		0	/* Don't use interval check */

/*
 * Behaviour when detecting errors
 */
#define EXT4_ERRORS_CONTINUE		1	/* Continue execution */
#define EXT4_ERRORS_RO			2	/* Remount fs read-only */
#define EXT4_ERRORS_PANIC		3	/* Panic */
#define EXT4_ERRORS_DEFAULT		EXT4_ERRORS_CONTINUE

/* Metadata checksum algorithm codes */
#define EXT4_CRC32C_CHKSUM		1

/*
 * Structure of the super block up-to-date [2021.7.24]
 */
struct ext4_super_block {
/*00*/	__u32	s_inodes_count;		/* Inodes count */
	__u32	s_blocks_count_lo;	/* Blocks count */
	__u32	s_r_blocks_count_lo;	/* Reserved blocks count */
	__u32	s_free_blocks_count_lo;	/* Free blocks count */
/*10*/	__u32	s_free_inodes_count;	/* Free inodes count */
	__u32	s_first_data_block;	/* First Data Block */
	__u32	s_log_block_size;	/* Block size */
	__u32	s_log_cluster_size;	/* Allocation cluster size */
/*20*/	__u32	s_blocks_per_group;	/* # Blocks per group */
	__u32	s_clusters_per_group;	/* # Clusters per group */
	__u32	s_inodes_per_group;	/* # Inodes per group */
	__u32	s_mtime;		/* Mount time */
/*30*/	__u32	s_wtime;		/* Write time */
	__u16	s_mnt_count;		/* Mount count */
	__u16	s_max_mnt_count;	/* Maximal mount count */
	__u16	s_magic;		/* Magic signature */
	__u16	s_state;		/* File system state */
	__u16	s_errors;		/* Behaviour when detecting errors */
	__u16	s_minor_rev_level;	/* minor revision level */
/*40*/	__u32	s_lastcheck;		/* time of last check */
	__u32	s_checkinterval;	/* max. time between checks */
	__u32	s_creator_os;		/* OS */
	__u32	s_rev_level;		/* Revision level */
/*50*/	__u16	s_def_resuid;		/* Default uid for reserved blocks */
	__u16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT4_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	__u32	s_first_ino;		/* First non-reserved inode */
	__u16  s_inode_size;		/* size of inode structure */
	__u16	s_block_group_nr;	/* block group # of this superblock */
	__u32	s_feature_compat;	/* compatible feature set */
/*60*/	__u32	s_feature_incompat;	/* incompatible feature set */
	__u32	s_feature_ro_compat;	/* readonly-compatible feature set */
/*68*/	__u8	s_uuid[16];		/* 128-bit uuid for volume */
/*78*/	char	s_volume_name[16];	/* volume name */
/*88*/	char	s_last_mounted[64] __attribute__((nonstring));	/* directory where last mounted */
/*C8*/	__u32	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT4_FEAT_COMP_DIR_PREALLOC flag is on.
	 */
	__u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	__u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	__u16	s_reserved_gdt_blocks;	/* Per group desc for online growth */
	/*
	 * Journaling support valid if EXT4_FEAT_COMP_HAS_JOURNAL set.
	 */
/*D0*/	__u8	s_journal_uuid[16];	/* uuid of journal superblock */
/*E0*/	__u32	s_journal_inum;		/* inode number of journal file */
	__u32	s_journal_dev;		/* device number of journal file */
	__u32	s_last_orphan;		/* start of list of inodes to delete */
	__u32	s_hash_seed[4];		/* HTREE hash seed */
	__u8	s_def_hash_version;	/* Default hash version to use */
	__u8	s_jnl_backup_type;
	__u16  s_desc_size;		/* size of group descriptor */
/*100*/	__u32	s_default_mount_opts;
	__u32	s_first_meta_bg;	/* First metablock block group */
	__u32	s_mkfs_time;		/* When the filesystem was created */
	__u32	s_jnl_blocks[17];	/* Backup of the journal inode */
	/* 64bit support valid if EXT4_FEAT_COMP_64BIT */
/*150*/	__u32	s_blocks_count_hi;	/* Blocks count */
	__u32	s_r_blocks_count_hi;	/* Reserved blocks count */
	__u32	s_free_blocks_count_hi;	/* Free blocks count */
	__u16	s_min_extra_isize;	/* All inodes have at least # bytes */
	__u16	s_want_extra_isize; 	/* New inodes should reserve # bytes */
	__u32	s_flags;		/* Miscellaneous flags */
	__u16  s_raid_stride;		/* RAID stride */
	__u16  s_mmp_update_interval;  /* # seconds to wait in MMP checking */
	__u64  s_mmp_block;            /* Block for multi-mount protection */
	__u32  s_raid_stripe_width;    /* blocks on all data disks (N*stride)*/
	__u8	s_log_groups_per_flex;  /* FLEX_BG group size */
	__u8	s_checksum_type;	/* metadata checksum algorithm used */
	__u8	s_encryption_level;	/* versioning level for encryption */
	__u8	s_reserved_pad;		/* Padding to next 32bits */
	__u64	s_kbytes_written;	/* nr of lifetime kilobytes written */
	__u32	s_snapshot_inum;	/* Inode number of active snapshot */
	__u32	s_snapshot_id;		/* sequential ID of active snapshot */
	__u64	s_snapshot_r_blocks_count; /* reserved blocks for active
					      snapshot's future use */
	__u32	s_snapshot_list;	/* inode number of the head of the
					   on-disk snapshot list */
#define EXT4_S_ERR_START offsetof(struct ext4_super_block, s_error_count)
	__u32	s_error_count;		/* number of fs errors */
	__u32	s_first_error_time;	/* first time an error happened */
	__u32	s_first_error_ino;	/* inode involved in first error */
	__u64	s_first_error_block;	/* block involved of first error */
	__u8	s_first_error_func[32] __attribute__((nonstring));	/* function where the error happened */
	__u32	s_first_error_line;	/* line number where error happened */
	__u32	s_last_error_time;	/* most recent time of an error */
	__u32	s_last_error_ino;	/* inode involved in last error */
	__u32	s_last_error_line;	/* line number where error happened */
	__u64	s_last_error_block;	/* block involved of last error */
	__u8	s_last_error_func[32] __attribute__((nonstring));	/* function where the error happened */
#define EXT4_S_ERR_END offsetof(struct ext4_super_block, s_mount_opts)
	__u8	s_mount_opts[64];
	__u32	s_usr_quota_inum;	/* inode for tracking user quota */
	__u32	s_grp_quota_inum;	/* inode for tracking group quota */
	__u32	s_overhead_clusters;	/* overhead blocks/clusters in fs */
	__u32	s_backup_bgs[2];	/* groups with sparse_super2 SBs */
	__u8	s_encrypt_algos[4];	/* Encryption algorithms in use  */
	__u8	s_encrypt_pw_salt[16];	/* Salt used for string2key algorithm */
	__u32	s_lpf_ino;		/* Location of the lost+found inode */
	__u32	s_prj_quota_inum;	/* inode for tracking project quota */
	__u32	s_checksum_seed;	/* crc32c(uuid) if csum_seed set */
	__u8	s_wtime_hi;
	__u8	s_mtime_hi;
	__u8	s_mkfs_time_hi;
	__u8	s_lastcheck_hi;
	__u8	s_first_error_time_hi;
	__u8	s_last_error_time_hi;
	__u8	s_pad[2];
	__u32	s_reserved[96];		/* Padding to the end of the block */
	__u32	s_checksum;		/* crc32c(superblock) */
};

#define EXT4_S_ERR_LEN (EXT4_S_ERR_END - EXT4_S_ERR_START)

/*
 * run-time mount flags
 */
#define EXT4_MF_MNTDIR_SAMPLED		0x0001
#define EXT4_MF_FS_ABORTED		0x0002	/* Fatal error detected */
#define EXT4_MF_TEST_DUMMY_ENCRYPTION	0x0004

#ifdef CONFIG_EXT4_FS_ENCRYPTION
#define DUMMY_ENCRYPTION_ENABLED(sbi) (unlikely((sbi)->s_mount_flags & \
						EXT4_MF_TEST_DUMMY_ENCRYPTION))
#else
#define DUMMY_ENCRYPTION_ENABLED(sbi) (0)
#endif

/* Number of quota types we support */
#define EXT4_MAXQUOTAS 3

/*
 * fourth extended-fs super-block data in memory
 */
struct ext4_sb_info {
	unsigned long s_desc_size;	/* Size of a group descriptor in bytes */
	unsigned long s_inodes_per_block;/* Number of inodes per block */
	unsigned long s_blocks_per_group;/* Number of blocks in a group */
	unsigned long s_clusters_per_group; /* Number of clusters in a group */
	unsigned long s_inodes_per_group;/* Number of inodes in a group */
	unsigned long s_itb_per_group;	/* Number of inode table blocks per group */
	unsigned long s_gdb_count;	/* Number of group descriptor blocks */
	unsigned long s_desc_per_block;	/* Number of group descriptors per block */
	ext4_group_t s_groups_count;	/* Number of groups in the fs */
	ext4_group_t s_blockfile_groups;/* Groups acceptable for non-extent files */
	unsigned long s_overhead;  /* # of fs overhead clusters */
	unsigned int s_cluster_ratio;	/* Number of blocks per cluster */
	unsigned int s_cluster_bits;	/* log2 of s_cluster_ratio */
	__u64 s_bitmap_maxbytes;	/* max bytes for bitmap files */
	struct buffer_head * s_sbh;	/* Buffer containing the super block */
	struct ext4_super_block *s_es;	/* Pointer to the super block in the buffer */
	struct buffer_head  *s_group_desc;
	unsigned int s_mount_opt;
	unsigned int s_mount_opt2;
	unsigned int s_mount_flags;
	unsigned int s_def_mount_opt;
	ext4_fsblk_t s_sb_block;
//	atomic64_t s_resv_clusters;
//	kuid_t s_resuid;
//	kgid_t s_resgid;
	unsigned short s_mount_state;
	unsigned short s_pad;
	int s_addr_per_block_bits;
	int s_desc_per_block_bits;
	int s_inode_size;
	int s_first_ino;
	unsigned int s_inode_readahead_blks;
	unsigned int s_inode_goal;
	__u32 s_hash_seed[4];
	int s_def_hash_version;
	int s_hash_unsigned;	/* 3 if hash should be signed, 0 if not */
//	struct percpu_counter s_freeclusters_counter;
//	struct percpu_counter s_freeinodes_counter;
//	struct percpu_counter s_dirs_counter;
//	struct percpu_counter s_dirtyclusters_counter;
//	struct blockgroup_lock *s_blockgroup_lock;
//	struct proc_dir_entry *s_proc;
//	struct kobject s_kobj;
//	struct completion s_kobj_unregister;
//	struct super_block *s_sb;

	/* Journaling */
	struct journal_s *s_journal;
//	struct list_head s_orphan;
//	struct mutex s_orphan_lock;
//	unsigned long s_ext4_flags;		/* Ext4 superblock flags */
	unsigned long s_commit_interval;
	__u32 s_max_batch_time;
	__u32 s_min_batch_time;
	struct block_device *journal_bdev;
#ifdef CONFIG_QUOTA
	/* Names of quota files with journalled quota */
	char __rcu *s_qf_names[EXT4_MAXQUOTAS];
	int s_jquota_fmt;			/* Format of quota to use */
#endif
	unsigned int s_want_extra_isize; /* New inodes should reserve # bytes */
//	struct ext4_system_blocks __rcu *system_blks;

#ifdef EXTENTS_STATS
	/* ext4 extents stats */
	unsigned long s_ext_min;
	unsigned long s_ext_max;
	unsigned long s_depth_max;
	spinlock_t s_ext_stats_lock;
	unsigned long s_ext_blocks;
	unsigned long s_ext_extents;
#endif

	/* for buddy allocator */
//	struct ext4_group_info ** __rcu *s_group_info;
	struct inode *s_buddy_cache;
//	spinlock_t s_md_lock;
	unsigned short *s_mb_offsets;
	unsigned int *s_mb_maxs;
	unsigned int s_group_info_size;
	unsigned int s_mb_free_pending;
//	struct list_head s_freed_data_list;	/* List of blocks to be freed
//						   after commit completed */

	/* tunables */
	unsigned long s_stripe;
	unsigned int s_mb_stream_request;
	unsigned int s_mb_max_to_scan;
	unsigned int s_mb_min_to_scan;
	unsigned int s_mb_stats;
	unsigned int s_mb_order2_reqs;
	unsigned int s_mb_group_prealloc;
	unsigned int s_max_dir_size_kb;
	/* where last allocation was done - for stream allocation */
	unsigned long s_mb_last_group;
	unsigned long s_mb_last_start;

	/* stats for buddy allocator */
//	atomic_t s_bal_reqs;	/* number of reqs with len > 1 */
//	atomic_t s_bal_success;	/* we found long enough chunks */
//	atomic_t s_bal_allocated;	/* in blocks */
//	atomic_t s_bal_ex_scanned;	/* total extents scanned */
//	atomic_t s_bal_goals;	/* goal hits */
//	atomic_t s_bal_breaks;	/* too long searches */
//	atomic_t s_bal_2orders;	/* 2^order hits */
//	spinlock_t s_bal_lock;
//	unsigned long s_mb_buddies_generated;
//	unsigned long long s_mb_generation_time;
//	atomic_t s_mb_lost_chunks;
//	atomic_t s_mb_preallocated;
//	atomic_t s_mb_discarded;
//	atomic_t s_lock_busy;

	/* locality groups */
//	struct ext4_locality_group __percpu *s_locality_groups;

	/* for write statistics */
	unsigned long s_sectors_written_start;
//	u64 s_kbytes_written;

	/* the size of zero-out chunk */
	unsigned int s_extent_max_zeroout_kb;

	unsigned int s_log_groups_per_flex;
//	struct flex_groups * __rcu *s_flex_groups;
	ext4_group_t s_flex_groups_allocated;

	/* workqueue for reserved extent conversions (buffered io) */
	struct workqueue_struct *rsv_conversion_wq;

	/* timer for periodic error stats printing */
//	struct timer_list s_err_report;

	/* Lazy inode table initialization info */
	struct ext4_li_request *s_li_request;
	/* Wait multiplier for lazy initialization thread */
	unsigned int s_li_wait_mult;

	/* Kernel thread for multiple mount protection */
	struct task_struct *s_mmp_tsk;

	/* record the last minlen when FITRIM is called. */
//	atomic_t s_last_trim_minblks;

	/* Reference to checksum algorithm driver via cryptoapi */
	struct crypto_shash *s_chksum_driver;

	/* Precomputed FS UUID checksum for seeding other checksums */
	__u32 s_csum_seed;

	/* Reclaim extents from extent status tree */
//	struct shrinker s_es_shrinker;
//	struct list_head s_es_list;	/* List of inodes with reclaimable extents */
//	long s_es_nr_inode;
//	struct ext4_es_stats s_es_stats;
//	struct mb_cache *s_ea_block_cache;
//	struct mb_cache *s_ea_inode_cache;
//	spinlock_t s_es_lock ____cacheline_aligned_in_smp;

	/* Ratelimit ext4 messages. */
//	struct ratelimit_state s_err_ratelimit_state;
//	struct ratelimit_state s_warning_ratelimit_state;
//	struct ratelimit_state s_msg_ratelimit_state;

	/*
	 * Barrier between writepages ops and changing any inode's JOURNAL_DATA
	 * or EXTENTS flag.
	 */
	//struct percpu_rw_semaphore s_writepages_rwsem;
	struct dax_device *s_daxdev;
};


/*
 * Inode dynamic state flags
 */
enum {
	EXT4_STATE_JDATA,		/* journaled data exists */
	EXT4_STATE_NEW,			/* inode is newly created */
	EXT4_STATE_XATTR,		/* has in-inode xattrs */
	EXT4_STATE_NO_EXPAND,		/* No space for expansion */
	EXT4_STATE_DA_ALLOC_CLOSE,	/* Alloc DA blks on close */
	EXT4_STATE_EXT_MIGRATE,		/* Inode is migrating */
	EXT4_STATE_DIO_UNWRITTEN,	/* need convert on dio done*/
	EXT4_STATE_NEWENTRY,		/* File just added to dir */
	EXT4_STATE_MAY_INLINE_DATA,	/* may have in-inode data */
	EXT4_STATE_EXT_PRECACHED,	/* extents have been precached */
	EXT4_STATE_LUSTRE_EA_INODE,	/* Lustre-style ea_inode */
};

/*
 * Returns true if the inode is inode is encrypted
 */
#define NEXT_ORPHAN(inode) EXT4_I(inode)->i_dtime

/*
 * Codes for operating systems
 */
#define EXT4_OS_LINUX		0
#define EXT4_OS_HURD		1
#define EXT4_OS_MASIX		2
#define EXT4_OS_FREEBSD		3
#define EXT4_OS_LITES		4

/*
 * Revision levels
 */
#define EXT4_GOOD_OLD_REV	0	/* The good old (original) format */
#define EXT4_DYNAMIC_REV	1	/* V2 format w/ dynamic inode sizes */

#define EXT4_CURRENT_REV	EXT4_GOOD_OLD_REV
#define EXT4_MAX_SUPP_REV	EXT4_DYNAMIC_REV

#define EXT4_GOOD_OLD_INODE_SIZE 128


#define EXT2_FEAT_COMP_SUPP	EXT4_FEAT_COMP_EXT_ATTR
#define EXT2_FEAT_INCOMP_SUPP	(EXT4_FEAT_INCOMP_FILETYPE| \
					 EXT4_FEAT_INCOMP_META_BG)
#define EXT2_FEAT_RO_COMP_SUPP	(EXT4_FEAT_RO_COMP_SPARSE_SUPER| \
					 EXT4_FEAT_RO_COMP_LARGE_FILE| \
					 EXT4_FEAT_RO_COMP_BTREE_DIR)

#define EXT3_FEAT_COMP_SUPP	EXT4_FEAT_COMP_EXT_ATTR
#define EXT3_FEAT_INCOMP_SUPP	(EXT4_FEAT_INCOMP_FILETYPE| \
					 EXT4_FEAT_INCOMP_RECOVER| \
					 EXT4_FEAT_INCOMP_META_BG)
#define EXT3_FEAT_RO_COMP_SUPP	(EXT4_FEAT_RO_COMP_SPARSE_SUPER| \
					 EXT4_FEAT_RO_COMP_LARGE_FILE| \
					 EXT4_FEAT_RO_COMP_BTREE_DIR)

#define EXT4_FEAT_COMP_SUPP	EXT4_FEAT_COMP_EXT_ATTR
#define EXT4_FEAT_INCOMP_SUPP	(EXT4_FEAT_INCOMP_FILETYPE| \
					 EXT4_FEAT_INCOMP_RECOVER| \
					 EXT4_FEAT_INCOMP_META_BG| \
					 EXT4_FEAT_INCOMP_EXTENTS| \
					 EXT4_FEAT_INCOMP_64BIT| \
					 EXT4_FEAT_INCOMP_FLEX_BG| \
					 EXT4_FEAT_INCOMP_EA_INODE| \
					 EXT4_FEAT_INCOMP_MMP | \
					 EXT4_FEAT_INCOMP_INLINE_DATA | \
					 EXT4_FEAT_INCOMP_ENCRYPT | \
					 EXT4_FEAT_INCOMP_CSUM_SEED | \
					 EXT4_FEAT_INCOMP_LARGEDIR)
#define EXT4_FEAT_RO_COMP_SUPP	(EXT4_FEAT_RO_COMP_SPARSE_SUPER| \
					 EXT4_FEAT_RO_COMP_LARGE_FILE| \
					 EXT4_FEAT_RO_COMP_GDT_CSUM| \
					 EXT4_FEAT_RO_COMP_DIR_NLINK | \
					 EXT4_FEAT_RO_COMP_EXTRA_ISIZE | \
					 EXT4_FEAT_RO_COMP_BTREE_DIR |\
					 EXT4_FEAT_RO_COMP_HUGE_FILE |\
					 EXT4_FEAT_RO_COMP_BIGALLOC |\
					 EXT4_FEAT_RO_COMP_METADATA_CSUM|\
					 EXT4_FEAT_RO_COMP_QUOTA |\
					 EXT4_FEAT_RO_COMP_PROJECT)


/*
 * Superblock flags
 */
#define EXT4_FLAGS_RESIZING	0
#define EXT4_FLAGS_SHUTDOWN	1

/*
 * Default values for user and/or group using reserved blocks
 */
#define	EXT4_DEF_RESUID		0
#define	EXT4_DEF_RESGID		0

/*
 * Default project ID
 */
#define	EXT4_DEF_PROJID		0

#define EXT4_DEF_INODE_READAHEAD_BLKS	32

/*
 * Default mount options
 */
#define EXT4_DEFM_DEBUG		0x0001
#define EXT4_DEFM_BSDGROUPS	0x0002
#define EXT4_DEFM_XATTR_USER	0x0004
#define EXT4_DEFM_ACL		0x0008
#define EXT4_DEFM_UID16		0x0010
#define EXT4_DEFM_JMODE		0x0060
#define EXT4_DEFM_JMODE_DATA	0x0020
#define EXT4_DEFM_JMODE_ORDERED	0x0040
#define EXT4_DEFM_JMODE_WBACK	0x0060
#define EXT4_DEFM_NOBARRIER	0x0100
#define EXT4_DEFM_BLOCK_VALIDITY 0x0200
#define EXT4_DEFM_DISCARD	0x0400
#define EXT4_DEFM_NODELALLOC	0x0800

/*
 * Default journal batch times
 */
#define EXT4_DEF_MIN_BATCH_TIME	0
#define EXT4_DEF_MAX_BATCH_TIME	15000 /* 15ms */

/*
 * Minimum number of groups in a flexgroup before we separate out
 * directories into the first block group of a flexgroup
 */
#define EXT4_FLEX_SIZE_DIR_ALLOC_SCHEME	4

/*
 * Structure of a directory entry
 */
#define EXT4_NAME_LEN 255

struct ext4_dir_entry {
	__u32	inode;			/* Inode number */
	__u16	rec_len;		/* Directory entry length */
	__u16	name_len;		/* Name length */
	char	name[EXT4_NAME_LEN];	/* File name */
};

/*
 * The new version of the directory entry.  Since EXT4 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext4_dir_entry_2 {
	__u32	inode;			/* Inode number */
	__u16	rec_len;		/* Directory entry length */
	__u8	name_len;		/* Name length */
	__u8	file_type;
	char	name[EXT4_NAME_LEN];	/* File name */
};

typedef struct ext4_dir_entry_2 ext4_dirent;

/*
 * This is a bogus directory entry at the end of each leaf block that
 * records checksums.
 */
struct ext4_dir_entry_tail {
	__u32	det_reserved_zero1;	/* Pretend to be unused */
	__u16	det_rec_len;		/* 12 */
	__u8	det_reserved_zero2;	/* Zero name length */
	__u8	det_reserved_ft;	/* 0xDE, fake file type */
	__u32	det_checksum;		/* crc32c(uuid+inum+dirblock) */
};

#define EXT4_DIRENT_TAIL(block, blocksize) \
	((struct ext4_dir_entry_tail *)(((void *)(block)) + \
					((blocksize) - \
					 sizeof(struct ext4_dir_entry_tail))))

/*
 * Ext4 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
#define EXT4_FT_UNKNOWN		0
#define EXT4_FT_REG_FILE	1
#define EXT4_FT_DIR		2
#define EXT4_FT_CHRDEV		3
#define EXT4_FT_BLKDEV		4
#define EXT4_FT_FIFO		5
#define EXT4_FT_SOCK		6
#define EXT4_FT_SYMLINK		7

#define EXT4_FT_MAX		8

#define EXT4_FT_DIR_CSUM	0xDE

/*
 * EXT4_DIR_PAD defines the directory entries boundaries
 *
 * NOTE: It must be a multiple of 4
 */
#define EXT4_DIR_PAD			4
#define EXT4_DIR_ROUND			(EXT4_DIR_PAD - 1)
#define EXT4_DIR_REC_LEN(name_len)	(((name_len) + 8 + EXT4_DIR_ROUND) & \
					 ~EXT4_DIR_ROUND)
#define EXT4_MAX_REC_LEN		((1<<16)-1)

/*
 * Hash Tree Directory indexing
 * (c) Daniel Phillips, 2001
 */

#define is_dx(dir) (ext4_has_feature_dir_index((dir)->i_sb) && \
		    ext4_test_inode_flag((dir), EXT4_INODE_INDEX))
#define EXT4_DIR_LINK_MAX(dir) unlikely((dir)->i_nlink >= EXT4_LINK_MAX && \
		    !(ext4_has_feature_dir_nlink((dir)->i_sb) && is_dx(dir)))
#define EXT4_DIR_LINK_EMPTY(dir) ((dir)->i_nlink == 2 || (dir)->i_nlink == 1)

/* Legal values for the dx_root hash_version field: */

#define DX_HASH_LEGACY			0
#define DX_HASH_HALF_MD4		1
#define DX_HASH_TEA			2
#define DX_HASH_LEGACY_UNSIGNED		3
#define DX_HASH_HALF_MD4_UNSIGNED	4
#define DX_HASH_TEA_UNSIGNED		5


#define EFSBADCRC	EBADMSG		/* Bad CRC detected */
#define EFSCORRUPTED	EUCLEAN		/* Filesystem is corrupted */

#endif	/* _EXT4_H */
