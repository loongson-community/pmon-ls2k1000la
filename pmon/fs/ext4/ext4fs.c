#include <string.h>
#include <pmon.h>
#include <file.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include "ext4_extents.h"
#include <diskfs.h>
/*#define DEBUG_IDE*/

#define EXT_SUPER_MAGIC	0xEF53
#define SECTOR_SIZE	512
#define START_PARTION (start_sec * SECTOR_SIZE)

static unsigned int is_power_of_2(unsigned long n)
{
    return (n != 0 && ((n & (n - 1)) == 0));
}
#ifdef DEBUG_IDE
static void dump_feature(struct ext4_super_block *);
static void dump_feature(struct ext4_super_block *ext4_sb)
{
	if(ext4_sb->s_feature_incompat & EXT4_FEAT_INCOMP_EXTENTS)
		printf("feature_incomp_extents\n");
	if(ext4_sb->s_feature_incompat & EXT4_FEAT_INCOMP_64BIT)
		printf("feature_incomp_64bit\n");
	if(ext4_sb->s_feature_incompat & EXT4_FEAT_INCOMP_META_BG)
		printf("feature_incomp_meta_bg\n");
	if(ext4_sb->s_feature_incompat & EXT4_FEAT_INCOMP_FLEX_BG)
		printf("feature_incomp_flex_bg\n");
	if(ext4_sb->s_feature_incompat & EXT4_FEAT_INCOMP_EA_INODE)
		printf("feature_incomp_ea_inode\n");
}
#endif
extern int devio_open(int , const char * , int , int);
extern int devio_close(int);
extern int devio_read(int , void * , size_t);
extern int devio_write(int , const void * , size_t);
extern off_t devio_lseek(int , off_t , int);
extern int check_disk(unsigned char *buf);


static int ext4_read_file(int , void *, size_t , size_t , struct ext4_inode *);
static int ReadFromIndexBlock(int , __u32 , __u32 , __u8 ** , size_t *, size_t * , __u32 *);
static int ReadFromIndexBlock2(int fd,__u32 start_index,__u32 end_index,__u8 **start,	size_t *size,size_t *position,__u32 *d_addr_start);
static inline ext4_dirent *ext4_next_entry(ext4_dirent *);
static int ext4_entrycmp(char *, void *, int );
static int ext4_get_inode(int , unsigned long , struct ext4_inode ** );
static int   ext4_load_linux(int , int , const unsigned char *);
static int   read_super_block(int , int);
int ext4_open(int , const char * , int , int);
int ext4_close(int);
int ext4_read(int , void * , size_t);
int ext4_write(int , const void * , size_t);
off_t ext4_lseek(int , off_t , int);

__u8  g_log_groups_per_flex;
__u32 g_log_block_size;
__u32 g_log_cluster_size;
__u32 g_first_ino;
//__u32 g_clusters_per_group;
unsigned long g_grpdecs_per_block = 32;
unsigned long g_inodes_per_grp = 0;
unsigned long g_inodes_per_block = 0;
unsigned long g_ext4_inode_size = 128;
unsigned long g_grpdesc_size = 32;
off_t start_sec;
static struct ext4_inode file_inode;
static struct ext4_inode *the_inode = &file_inode;

/* Identify some global variables by analyzing ext's superblock
 * return status :
 * 0 : error
 * 1 : regular
 * 2 : special
 */
int judge_ext_details(struct ext4_super_block *ext4_sb)
{
	int status, ext_flag = 0;
	/* The XFS file system has the same OStype value as ext, but not the same magic number */
	if (ext4_sb->s_magic != EXT_SUPER_MAGIC) {
#ifdef DEBUG_IDE
		printf("this is not a ext file system!\n");
#endif
		return status;
	}
	status = 1;
	/* Temporarily determine what kind of Ext it is. */
	if (ext4_sb->s_feature_incompat & EXT4_FEAT_INCOMP_EXTENTS) {
		ext_flag += 4;
	}
	if (ext4_sb->s_feature_compat & EXT4_FEAT_COMP_HAS_JOURNAL) {
		ext_flag += 2;
	} else {
		ext_flag += 1;
	}
	if (ext_flag >= 4) {
		ext_flag = 4;
	} else if (ext_flag >= 2) {
		ext_flag = 3;
	} else {
		ext_flag = 2;
	}
#ifdef DEBUG_IDE
	printf("this is a ext%d file type!\n", ext_flag);
#endif

	/* some global variable depends on super block */
	g_log_block_size = 1024 << ext4_sb->s_log_block_size;
	if (g_log_block_size < EXT4_MIN_BLOCK_SIZE ||
		g_log_block_size > EXT4_MAX_BLOCK_SIZE) {
		printf("Unsupported filesystem blocksize\n");
	}
	g_log_groups_per_flex = (ext4_sb->s_log_groups_per_flex > 0) ? (2 << (ext4_sb->s_log_groups_per_flex - 1)) : 2;
	g_log_cluster_size = (ext4_sb->s_feature_ro_compat & EXT4_FEAT_RO_COMP_BIGALLOC) ? (1024 << (ext4_sb->s_log_block_size + ext4_sb->s_log_cluster_size)) : ext4_sb->s_log_block_size;

	g_grpdesc_size = ext4_sb->s_desc_size;
	/* Some features will determine whether the file system is correct */
	if (ext4_sb->s_feature_incompat & EXT4_FEAT_INCOMP_64BIT) {
		if (ext4_sb->s_creator_os == 1) {
			status = 0;
			printf("Hurd is not supported by 64 bit fs\n");
		}
		if (ext4_sb->s_desc_size < EXT4_MIN_DESC_SIZE_64BIT || ext4_sb->s_desc_size > EXT4_MAX_DESC_SIZE || !is_power_of_2(ext4_sb->s_desc_size)) {
			status = 0;
			printf("unsupported descriptor size %lu",ext4_sb->s_desc_size);
		}
	} else {
		g_grpdesc_size = EXT4_MIN_DESC_SIZE;
	}

	if (ext4_sb->s_feature_incompat & EXT4_FEAT_INCOMP_EA_INODE) {
		if (ext4_sb->s_creator_os == 1) {
			status = 0;
			printf("Hurd is not supported by ea_node\n");
		}
	}
	if (ext4_sb->s_feature_compat & EXT4_FEAT_COMP_EXT_ATTR) {
#ifdef DEBUG_IDE
		printf("super extent attr\n");
#endif
	}
	if ((ext4_sb->s_feature_ro_compat & EXT4_FEAT_RO_COMP_METADATA_CSUM) && (ext4_sb->s_feature_ro_compat & EXT4_FEAT_RO_COMP_GDT_CSUM)) {
		printf("metadata_csum and uninit_bg are redundant flags; please run fsck.\n");
	}

	/*  global use */
	if(ext4_sb->s_rev_level == 0) { //old version
		g_ext4_inode_size = 128;
		g_first_ino = 11;
	} else {
		g_ext4_inode_size = ext4_sb->s_inode_size;
		g_first_ino = ext4_sb->s_first_ino;
		if (g_first_ino < EXT4_GOOD_OLD_FIRST_INO) {
			status = 0;
			printf("invalid first ino\n");
		}
		if ((g_ext4_inode_size < EXT4_GOOD_OLD_INODE_SIZE) ||
			!is_power_of_2(g_ext4_inode_size) ||
			(g_ext4_inode_size > g_log_block_size)) {
			status = 0;
			printf("error : unsupported inode size");
		}
	}

	g_inodes_per_grp = ext4_sb->s_inodes_per_group;
	g_grpdecs_per_block = g_log_block_size / g_grpdesc_size;

	if ((ext4_sb->s_log_block_size) >
	    (EXT4_MAX_BLOCK_LOG_SIZE - EXT4_MIN_BLOCK_LOG_SIZE)) {
		status = 0;
		printf("Invalid log block size: %u",ext4_sb->s_log_block_size);
	}
	if ((ext4_sb->s_log_cluster_size) >
	    (EXT4_MAX_CLUSTER_LOG_SIZE - EXT4_MIN_BLOCK_LOG_SIZE)) {
		status = 0;
		printf("Invalid log cluster size: %u",ext4_sb->s_log_cluster_size);
	}

	if ((ext4_sb->s_reserved_gdt_blocks) > (g_log_block_size / 4)) {
		status = 0;
		printf("Number of reserved GDT blocks insanely large: %d",ext4_sb->s_reserved_gdt_blocks);
	}

	g_inodes_per_block = g_log_block_size / g_ext4_inode_size;
	if (g_inodes_per_block == 0) {
		status = 0;
		printf("can't find ext fs\n");
	}
	if (g_inodes_per_grp < g_inodes_per_block || g_inodes_per_grp > g_log_block_size * 8) {
		status = 0;
		printf("invalid inodes per group: %lu\n", g_inodes_per_grp);
	}

	return status;
}

/* Find the ext partition */
static inline int find_exist_ext(int fd, int i, __u8 *leadbuf, __u8 *diskbuf)
{
	int err = -1;
	struct ext4_super_block *ext4_sb;
	gpt_partentry entry;
	memcpy(entry.ent, (char *)((unsigned long)leadbuf + i * 128), sizeof(entry.ent));
	if (memcmp(entry.type, gpt_type[15].type, sizeof(entry.type)) == 0 ||
			memcmp(entry.type, gpt_type[1].type, sizeof(entry.type)) == 0) {
		start_sec = entry.start;
		devio_lseek(fd, start_sec * SECTOR_SIZE, 0);
		if(devio_read(fd, diskbuf, 16 * SECTOR_SIZE) !=
				16 * SECTOR_SIZE) {
			printf("Read the super block error!\n");
			return err;
		}
		ext4_sb = (struct ext4_super_block *)(diskbuf + 1024);
		if (ext4_sb == NULL) {
			return err;
		}
		if (judge_ext_details(ext4_sb)) {
			err = 0;
			return err;
		}
	}
	return err;
}
static int read_super_block(int fd, int index)
{
	__u8 *diskbuf, *leadbuf;
	struct ext4_super_block *ext4_sb;
	int i, n, find_linux_partion;
	int err, status;
	gpt_header header;
	gpt_partentry entry;

	find_linux_partion = 0;
	diskbuf = 0;
	leadbuf = 0;
	err = -1;
	start_sec = 0;

	if(!(leadbuf = (__u8 *)malloc(SECTOR_SIZE))) {
		printf("Can't alloc memory for the super block!\n");
		goto out;
	}
	if(!(diskbuf = (__u8 *)malloc(16 * SECTOR_SIZE))) {
		printf("Can't alloc memory for the super block!\n");
		goto out;
	}

	if (index == -1) {
		goto out;
	}

	devio_lseek(fd, 0, 0);
	if(devio_read(fd, leadbuf, SECTOR_SIZE) != SECTOR_SIZE) {
		printf("Can't read the leading block from disk!\n");
		goto out;
	}
	status = check_disk(leadbuf);
	if (status == 2) {
		/* Read the GPT header.  */
		devio_lseek(fd, SECTOR_SIZE, 0);
		if ((devio_read(fd, header.gpt, SECTOR_SIZE)) != SECTOR_SIZE) {
			printf("-> %d\n", __LINE__);
			goto out;
		}
		if (!memcmp(header.magic, gpt_magic, sizeof(gpt_magic)) == 0) {
			printf("read [EFI PART] is erro!\n");
			goto out;
		}
		if (index) {
			/* find the ext partition, and the index bumber is the partition number */
			i = (index + 7) % 4;
			devio_lseek(fd, (index + 7)/4 * SECTOR_SIZE, 0);
			if ((devio_read(fd, leadbuf, SECTOR_SIZE)) != SECTOR_SIZE) {
				goto out;
			}
			err = find_exist_ext(fd, i, leadbuf, diskbuf);
			goto out;
		} else {
			/* find the first linux partition for boot menu, when index is 0 */
			for (n = 8; n < header.maxpart; n++) {
				i = n % 4;
				if (i == 0) {
					devio_lseek(fd, n/4 * SECTOR_SIZE, 0);
					if ((devio_read(fd, leadbuf, SECTOR_SIZE)) != SECTOR_SIZE) {
						goto out;
					}
				}
				if (!find_exist_ext(fd, i, leadbuf, diskbuf)) {
					err = 0;
					goto out;
				}
			}
		}
	} else if (status == 1){
		/*
		 * search the partion table to find the linux partion
		 * with id = 83. And the index is the partition number,
		 * but not linux partition number, so remove the loop here.
		 * ATTENTION: XFS's id is also 83.
		 * */
		i = index ? (446 + 0x10 * (index - 1)) : 446;
		for (; index == 0 && i < 511 && leadbuf[i + 4] != 0x83; i += 0x10);

		if (i < 511) {
			if(leadbuf[i + 4] == 0x83) {
				start_sec = *(unsigned short *)(leadbuf + i + 8 + 2);
				start_sec <<= 16;
				start_sec += *(unsigned short *)(leadbuf + i + 8);

				devio_lseek(fd, start_sec * SECTOR_SIZE, 0);
				if(devio_read(fd, diskbuf, 16 * SECTOR_SIZE) !=
						16 * SECTOR_SIZE) {
					printf("Read the super block error!\n");
					goto out;
				}

				ext4_sb = (struct ext4_super_block *)(diskbuf + 1024);
				if (ext4_sb == NULL) {
					printf("Read the super block error!\n");
					goto out;
				}
				if (judge_ext_details(ext4_sb)) {
					err = 0;
					goto out;
				}
			}
		}
	}

out:
	if(leadbuf)
		free(leadbuf);
	if(diskbuf)
		free(diskbuf);
	return err;
}

/*
 * p is at least 6 bytes before the end of page
 */
static inline ext4_dirent *ext4_next_entry(ext4_dirent *p)
{
	return (ext4_dirent *)((char*)p + (p->rec_len));
}

/* ext4 entry name is not null terminated,so we could not use strcmp
 * return 0 if the first 'len' characters of 'entry' match those of 's'
 */
static int ext4_entrycmp(char * s,void * entry , int len)
{
	int i;
	if (strlen(s) != len)
		return -1;
	for(i = 0; i < len; i++)
		if(*(char *)(s + i) != *(char *)((char *)entry + i))
			return -1;
	return 0;
}

/*
 * allocated a ext4_inode, and filled with inode info pointed by ino
 * out: ext4_raw_inode_ptr
 * return 0 for success
 * -1 for error */
static int ext4_get_inode(int fd, unsigned long ino, struct ext4_inode **ext4_raw_inode_ptr)
{
	unsigned long offset, block, block_group, group_desc, desc;

	struct ext4_group_desc * ext4_gd;
	unsigned char * bh;
	off_t temp;
	int err = -1;
	/* in which block group*/
	block_group = (ino - 1) / g_inodes_per_grp;
	/* in which block */
	group_desc = block_group / g_grpdecs_per_block ;
	/*
	 * introduction block maybe the same as super block
	 */
	block = 1024 / g_log_block_size + 1 + group_desc;
	/* which descriptor,inside the block */
	desc = block_group % g_grpdecs_per_block;
	bh = (unsigned char *)malloc(g_log_block_size);
#ifdef DEBUG_IDE
	printf("ext4_get_inode:ino=%d,block group=%d,block=%d,desc=%d\n", ino, block_group, block, desc);
	printf("1 bh:%lx\n", bh);
#endif

	temp = (off_t)block * g_log_block_size + start_sec * 512;
	devio_lseek(fd, temp, 0);
	if(g_log_block_size != devio_read(fd, bh, g_log_block_size)) {
		printf("io read error\n");
		goto out;
	}
	ext4_gd = (struct ext4_group_desc *)(bh + desc * g_grpdesc_size);
#ifdef DEBUG_IDE
	printf("ext4_group_desc -> bg_free_blocks_count=%d\n", ext4_gd->bg_free_blocks_count_lo);
	printf("ext4_group_desc -> bg_free_inodes_count=%d\n", ext4_gd->bg_free_inodes_count_lo);
	printf("ext4_group_desc -> bg_inode_table_lo=%d\n", ext4_gd->bg_inode_table_lo);
	printf("ext4_group_desc -> bg_block_bitmap=%d\n", ext4_gd->bg_block_bitmap_lo);
	printf("ext4_group_desc -> bg_inode_bitmap=%d\n", ext4_gd->bg_inode_bitmap_lo);
#endif
	offset = ((ino - 1) % g_inodes_per_grp) * g_ext4_inode_size;
	block  = ext4_gd->bg_inode_table_lo + (offset / g_log_block_size);
	offset = offset % g_log_block_size;
#ifdef DEBUG_IDE
	printf("ext4_get_inode: offset is %d,block is %d\n", offset, block);
	printf("2 bh:%lx\n", bh);
#endif
	memset(bh, 0, g_log_block_size);
	/*bzero(bh, g_log_block_size);*/
	temp = (off_t)block * g_log_block_size + start_sec * 512;

#ifdef DEBUG_IDE
	printf("the g_log_block_size is 0x%x\n", g_log_block_size);
	printf("In ext4fs.c	The seek offset is %llx\n", temp);

#endif
	devio_lseek(fd, temp, 0);
	if(g_log_block_size != devio_read(fd, bh, g_log_block_size)) {
		printf("io read error\n");
		goto out;
	}

	*ext4_raw_inode_ptr = (struct ext4_inode *)malloc(g_ext4_inode_size);
	if(!*ext4_raw_inode_ptr) {
		printf("no mem\n");
		goto out;
	}
	memcpy(*ext4_raw_inode_ptr, bh + offset, g_ext4_inode_size);
	err = 0;

#ifdef DEBUG_IDE
	printf("inode->i_block[0]=%d,the inode->i_size_lo=%d \n",
			(*ext4_raw_inode_ptr)->i_block[0], (*ext4_raw_inode_ptr)->i_size_lo);
#endif
out:
	free(bh);
	return err;
}

/*
 * load linux kernel from ext4 partition
 * return 0 if success,else return -1 if error, return 1 if special.
 */

static int ext4_load_linux(int fd,int index, const unsigned char *path)
{
	struct ext4_inode *ext4_raw_inode;
	ext4_dirent *de;
	unsigned char *bh;
	int i, bh_size;
	unsigned int inode;
	int find = 1;
	unsigned char s[EXT4_NAME_LEN];
	unsigned char pathname[EXT4_NAME_LEN], *pathnameptr;
	unsigned char *directoryname;
	int showdir, lookupdir;

	showdir = 0;
	lookupdir = 0;
	bh = 0;
	bh_size = 0;

	if(read_super_block(fd,index))
		return -1;

	if((path[0]==0) || (path[strlen(path)-1] == '/'))
		lookupdir = 1;

	strncpy(pathname,path,sizeof(pathname));
	pathnameptr = pathname;
	for(inode = EXT4_ROOT_INO; find; ) {
		for(i = 0; pathnameptr[i] && pathnameptr[i] != '/'; i++);

		pathnameptr[i] = 0;

		directoryname = (unsigned char *)pathnameptr;
		pathnameptr = (unsigned char *)(pathnameptr + i + 1);

		if(!strlen(directoryname) && lookupdir)
			showdir = 1;
		if(ext4_get_inode(fd, inode, &ext4_raw_inode)) {
			printf("load EXT4_ROOT_INO error");
			return -1;
		}
		if (!bh || bh_size < g_log_block_size + ext4_raw_inode->i_size_lo)
		{
			if(bh) free(bh);
			bh = (unsigned char *)malloc(g_log_block_size + ext4_raw_inode->i_size_lo);
			bh_size = g_log_block_size + ext4_raw_inode->i_size_lo;
		}

		if(!bh) {
			printf("Error in allocting memory for file content!\n");
			return -1;
		}
		if(ext4_read_file(fd, bh, ext4_raw_inode->i_size_lo, 0,
					ext4_raw_inode) != ext4_raw_inode->i_size_lo)
			return -1;
		de = (ext4_dirent *)bh;
		find = 0;

		for ( ; ((unsigned char *) de < bh + ext4_raw_inode->i_size_lo) &&
				(de->rec_len > 0) && (de->name_len > 0); de = ext4_next_entry(de)) {
			strncpy(s,de->name,de->name_len);
			s[de->name_len]='\0';//*(de->name+de->name_len)='\0';
#ifdef DEBUG_IDE
			printf("entry:name=%s,inode=%d,rec_len=%d,name_len=%d,file_type=%d\n",s,de->inode,de->rec_len,de->name_len,de->file_type);
#endif
			if(showdir)
				printf("%s%s",s,((de->file_type)&2)?"/ ":" ");

			if (!ext4_entrycmp(directoryname, de->name, de->name_len)) {
				if(de->file_type == EXT_FT_REG_FILE || de->file_type == EXT_FT_UNKNOWN) {
					if (ext4_get_inode(fd, de->inode, &the_inode)) {
						printf("load EXT4_ROOT_INO error");
						free(bh);
						return -1;
					}
					free(bh);
					return 0;
				}
				find = 1;
				inode = de->inode;
				break;
			}
		}
		if(!find) {
			free(bh);
			if(!lookupdir) {
				printf("Not find the file or directory!\n");
				return -1;
			} else {
				/* when we load /dev/fs/wd0/, filename wasn't given, return 1 to avoid call fat_open() */
				printf("\n");
				return 1;
			}
		}
	}
	return -1;
}



/*
 * for cmd: load /dev/fs/ext4@wd0/boot/vmlinux
 * the path we got here is wd0/boot/vmlinux
 * */
int ext4_open(int fd,const char *path,int flags,int mode)
{
	int i, index;
	char strbuf[EXT4_NAME_LEN], *str;
	char *p;

	strncpy(strbuf,path,sizeof(strbuf));

	for(i = 0; strbuf[i]&&(strbuf[i]!='/'); i++) ;

	if(!strbuf[i]){
		printf("the DEV Name  is expected!\n");
		return -1;
	}
	strbuf[i] = 0;
	p = &strbuf[strlen(strbuf)-1];
	if((p[0]>='a') && (p[0]<='z')) {
		index=p[0]-'a'+1;
		p[0]=0;
	}
	else if(p[0]=='A'||!strcmp(strbuf,"fd0"))
		index=-1;
	else
		index=0;

	/* extract the device name */
	if(devio_open(fd,strbuf,flags,mode) < 0)
		return -1;
#ifdef DEBUG_IDE
	printf("Open the device %s ok\n", strbuf);
#endif
	str = strbuf + i + 1;
	/* return 0 to avoid error while open directory */
	if(!(ext4_load_linux(fd, index, str) - 1))
		return 0;
	if(!(ext4_load_linux(fd, index, str)))
		return fd;
	if((str[0] != 0) && (str[strlen(str)-1] != '/'))
#ifdef DEBUG_IDE
		printf("we can't locate root directory in super block!\n");
#endif
	return -1;
}

int ext4_close(int fd)
{
	_file[fd].posn = 0;
	return devio_close(fd);
}

/*
 * ReadBuf: point to the file content
 * size: the real number still need to be read
 * position: the file point where the read start
 * return:
 * 0 -- successful
 * others -- error
 * */
static int ReadFromIndexBlock(int fd, __u32 start_block, __u32 end_block, __u8 **ReadBuf, size_t *size, size_t *position, __u32 *addr_start)
{
	__u32 remain_count;
	int re;
	off_t remain_size, addr_abosolute_start;
	if(start_block > end_block)
		return 0;
#ifdef DEBUG_IDE
	printf("I want to read data size :%u, start_block is %d,end_block is %u\n",
			*size , start_block, end_block);
#endif
	/*Read the unaligned data within a block. */
	remain_size = g_log_block_size - *position % g_log_block_size;
	if(remain_size > *size) {
		remain_size = *size;
		remain_count = 0;
	}
	else if(remain_size == g_log_block_size) {
		remain_size = 0;
		remain_count = 0;
	}
	else
		remain_count = 1;

	addr_start += start_block;

	/*  start_block starts with 0. 0-11:direct others 0-1023.*/
	addr_abosolute_start =
		(off_t)*addr_start * g_log_block_size + *position % g_log_block_size;
	if(remain_count) {
		devio_lseek(fd, addr_abosolute_start + START_PARTION, 0);
		re = devio_read(fd, (__u8 *)*ReadBuf, remain_size);
		if(re != remain_size) {
			printf("Can't Read Data From the Disk \n");
			return -1;
		}
		start_block += remain_count;
		addr_start += remain_count;
		remain_count = 0;
		*ReadBuf += remain_size;
		*position += remain_size;
		*size -= remain_size;
		addr_abosolute_start =
			(off_t)*addr_start * g_log_block_size + *position % g_log_block_size;
		remain_size = 0;
	}
	/*Read the BLOCK aligned data*/
	while(*size && remain_size < *size &&
			(remain_count+start_block <= end_block)) {
		if( *(addr_start + remain_count + 1) ==
				*(addr_start + remain_count) + 1) {
			if(remain_size + g_log_block_size > *size)
				remain_size = *size;
			else {
				remain_count++;
				remain_size += g_log_block_size;
			}
		} else {
#ifdef DEBUG_IDE
			printf("The remain size is %u ,size is%u\n", remain_size, *size);
			printf("Block begin at %u, end at %u\n",
					*addr_start, *(addr_start + remain_count));
#endif
			if(!remain_size) {
				/*if not continuous,we just read one block*/

				if(*size < g_log_block_size)
					remain_size =* size;
				else {
					remain_size = g_log_block_size;
					remain_count = 1;
				}
			}
			if(*addr_start == 0 && remain_count == 1) {
				memset((__u8 *)*ReadBuf,0,remain_size);
				re=remain_size;
			} else {
				devio_lseek(fd, addr_abosolute_start + START_PARTION, 0);
				re = devio_read(fd, (__u8 *)*ReadBuf, remain_size);
			}
			if(re!=remain_size) {
				printf("We can't read data from disk!\n");
				return -1;
			}
			start_block += remain_count;
			addr_start += remain_count;
			remain_count = 0;
			*ReadBuf += remain_size;
			*position += remain_size;
			*size -= remain_size;
			if((*position % g_log_block_size) && *size) {
				printf("Oh,My God!When I read in the aligned data,I met one unaligned position\n");
				return -1;
			}
			addr_abosolute_start = ((off_t)*addr_start) * g_log_block_size;
			remain_size = 0;
		}
	}

	/*No data need to read.Wonderful!*/
	if(!*size)
		return 0;
	if(remain_size) {
		if(remain_size > *size)
			remain_size = *size;
		devio_lseek(fd, addr_abosolute_start + START_PARTION, 0);
#ifdef DEBUG_IDE
		printf("The remain size is %u,size is %u\n", remain_size, *size);
		printf("Additional Block begin at %d,end at %d\n", *addr_start, *(addr_start + remain_count));
#endif
		re = devio_read(fd, (__u8 *)*ReadBuf, remain_size);
		*ReadBuf += remain_size;
		if(re != remain_size) {
			printf("We can't read data from disk!\n");
			return -1;
		}
		*position += remain_size;
		*size -= remain_size;
	}
	return 0;
}

int ext4_read(int fd, void *read_start,size_t size)
{
	int real_size;

	if((_file[fd].posn + size) > the_inode->i_size_lo) {
		size = the_inode->i_size_lo - _file[fd].posn;
	}

	memset(read_start, 0, size);

	real_size = ext4_read_file(fd, read_start, size, _file[fd].posn, the_inode);
	if((_file[fd].posn + real_size) > the_inode->i_size_lo) {
		real_size = the_inode->i_size_lo - _file[fd].posn;
		_file[fd].posn = the_inode->i_size_lo;
	} else
		_file[fd].posn += real_size;

	return real_size;
}

/*
 * return the extent node for file_block of file pointed
 * by extent_hdr
 * fd : file desc to block device
 * buff: the buffer provided by caller with 1 block size
 * extent_hdr: the root node of the extent tree of the file
 * file_block: the file block number of the file
 */
static struct ext4_extent_header * get_extent_node(int fd, __u8 *buff,
		struct ext4_extent_header *extent_hdr, __u32 file_block)
{
	struct ext4_extent_idx *idx;
	unsigned long long block;
	int i;

	while(1) {
		idx = (struct ext4_extent_idx *)(extent_hdr + 1);
#ifdef	DEBUG_IDE
		printf("eh_depth:%d, eh_entries:%d, eh_max:%d, block:%d\n",
				extent_hdr->eh_depth, extent_hdr->eh_entries,
				extent_hdr->eh_max, file_block);
#endif
		if((extent_hdr->eh_magic) != EXT4_EXT_MAGIC)
			return 0;

		if(!extent_hdr->eh_depth)
			return extent_hdr;
		i = -1;
		do {
			i++;
			if(i >= (extent_hdr->eh_entries))
				break;
#ifdef	DEBUG_IDE
			printf("ei_hi:%d, ei_block:%d, ei_lo:%d\n",
					idx[i].ei_leaf_hi, idx[i].ei_block,
					idx[i].ei_leaf_lo);
#endif

		} while(file_block >= (idx[i].ei_block));

		if(--i < 0)
			return 0;

		block = (idx[i].ei_leaf_hi);
		block = (block << 32) | (idx[i].ei_leaf_lo);

		devio_lseek(fd, block * g_log_block_size + START_PARTION, 0);
		if(devio_read(fd, buff, g_log_block_size) != g_log_block_size)
			return 0;
		else
			extent_hdr = (struct ext4_extent_header *)buff;

	}


}

/*
 * return the block nr for file logical number file_block
 * fd: the device holding the filesystem
 * idx: the root extent header for the filesystem
 * file_block: the logical file logical number
 */
static long long read_extent_block(int fd, struct ext4_extent_header *idx, __u8  *ext_buff,
		__u32 file_block, int *leftblks)
{
	unsigned long long blk;
	struct ext4_extent_header *leaf_node;
	struct ext4_extent *extent;
	int i = -1;
	__u32 next_block = file_block + 1;


	leaf_node = get_extent_node(fd, ext_buff, idx, file_block);
	if(!leaf_node) {
		printf("leaf error\n");
		return -1;
	}
#ifdef DEBUG_IDE
	printf("depth:%d, entry:%d, max%d, block:%d\n", leaf_node->eh_depth,
			leaf_node->eh_entries, leaf_node->eh_max,
			file_block);
#endif


	extent = (struct ext4_extent *)(leaf_node + 1);
	do {
		++i;
		if(i >= (leaf_node->eh_entries))
			break;
#ifdef DEBUG_IDE
		printf("ent:%d, ee_block:%d, ee_len:%d, ee_start_lo %d\n",
				i, extent[i].ee_block, extent[i].ee_len,
				extent[i].ee_start_lo);
#endif
		next_block = (extent[i].ee_block);
	} while(file_block >= next_block);


	if(--i >= 0) {
		file_block -= extent[i].ee_block;
#ifdef DEBUG_IDE
		printf("block:%d, ee_block:%d, ee_len:%d, ee_start_lo %d\n",
				file_block, extent[i].ee_block, extent[i].ee_len,
				extent[i].ee_start_lo);
#endif

		blk = (extent[i].ee_start_hi);
		blk = blk << 32 |
			(extent[i].ee_start_lo);
#ifdef DEBUG_IDE
		printf("blk:%ld, ret %ld\n", blk, file_block + blk);
#endif
		if(file_block >= (extent[i].ee_len)) {
			/* find a hole */
			*leftblks = next_block - file_block -  extent[i].ee_block;
			return -2;
		}
		else
			*leftblks = (extent[i].ee_len) - file_block;
		return file_block + blk;
	}

	*leftblks = next_block - file_block;
	return -2;
}


/*
 * read file with extent feature
 */

static int ext4_read_file1(int fd, void *read_start,
		size_t size, size_t pos, struct ext4_inode *inode)
{
	struct ext4_extent_header *extent_hdr;

	long long blk;
	size_t off;
	int i;
	int leftblks;
	__u32 blk_start = pos / g_log_block_size;
	__u32 blk_end = (pos + size + g_log_block_size - 1) / g_log_block_size;
	__u8  *ext_buff;
	off = 0;
#ifdef DEBUG_IDE
	printf("size:%d, pos:%d flags:0x%x\n", size, pos, inode->i_flags);
#endif

	extent_hdr = (struct ext4_extent_header *)(inode->i_block);
	ext_buff = (unsigned char *)malloc(g_log_block_size);
	if (!ext_buff) {
		printf("no mem!\n");
		return -1;
	}

	for(i = blk_start, leftblks = 0; i < blk_end; i++) {
		int skip;
		int len;
		int ret;

		if (!leftblks)
			blk = read_extent_block(fd, extent_hdr, ext_buff, i, &leftblks);
		else if (blk >= 0)
			blk++;
		leftblks--;
#ifdef DEBUG_IDE
		printf("blk:%lld, file_block:%d\n", blk, i);
#endif
		if(blk == -1)
		{
			goto out;
		}

		if(i == pos / g_log_block_size) {
			skip = pos % g_log_block_size;
			len = (size <= (g_log_block_size - skip) ?
					size : g_log_block_size - skip);
		} else {
			skip = 0;
			len = (size <= g_log_block_size ?
					size : g_log_block_size);

		}

		if(!len)
			break;


		if(blk>=0)
		{
			devio_lseek(fd, blk * g_log_block_size + START_PARTION + skip, 0);
			ret = devio_read(fd, read_start + off, len);
		}
		else
		{
			memset(read_start + off, 0, len);
			ret = len;
		}
#ifdef DEBUG_IDE
		printf("ret:%d, size:%d, off:%d, skip:%d, len:%d \n",
				ret, size, off, skip, len);
#endif
		if(ret < 0)
		{
			goto out;
		}
		if(ret != len)
		{
			off = ret + off;
			goto out;
		}

		size -= len;
		off += len;
	}
out:

	free(ext_buff);
#ifdef DEBUG_IDE
	printf("size:%d, off:%d\n", size, off);
#endif
	return off;
}
static int ext4_read_file0(int fd, void *read_start, size_t size, size_t pos,
		struct ext4_inode *inode)
{
	__u32 *addr_start, *d_addr_start, start_block;
	int re, i;
	__u8 *buff, *index_buff, *start = (__u8 *)read_start;
	size_t read_size = size, position = pos;

	start_block = position / g_log_block_size;
	addr_start = inode->i_block;
#ifdef DEBUG_IDE
	printf("the pos is %llx,the size is %llx\n", position, read_size);
#endif
	re = ReadFromIndexBlock(fd, start_block, 11, &start, &read_size,
			&position, addr_start);
#ifdef DEBUG_IDE
	printf("The addr_start is 0x%x, g_log_block_size is 0x%x, start_sec is 0x%x\n",
			*addr_start, g_log_block_size, start_sec);
#endif
	if(re) {
		printf("Error in Reading from direct block\n");
		return 0;
	}
	if(!read_size) {
#ifdef DEBUG_IDE
		for(i = 0; i < size; i += g_log_block_size/4)
			printf("%4x",(__u8)*((__u8 *)read_start + i));
		printf("\n");
#endif
		return (int)size;
	}
	start_block = position / g_log_block_size - 12;
	buff = (__u8 *)malloc(g_log_block_size);
	if(!buff) {
		printf("Can't alloc memory!\n");
		return 0;
	}
	addr_start = &(inode->i_block[12]);
	devio_lseek(fd, (off_t)*addr_start * g_log_block_size + START_PARTION, 0);
	re = devio_read(fd, buff, g_log_block_size);
	if(re != g_log_block_size) {
		printf("Read the iblock[12] error!\n");
		return 0;
	}
	addr_start = (__u32 *)buff;
	re = ReadFromIndexBlock(fd, start_block, g_log_block_size/4-1, &start,
			&read_size, &position, addr_start);
	if(re) {
		free((char*)buff);	/*spark add*/
		return 0;
	}
	if(!read_size) {
#ifdef DEBUG_IDE
		for(i = 0; i < size; i += g_log_block_size/4)
			printf("%4x",(__u8)*((__u8 *)read_start + i));
		printf("\n");
#endif
		free((char*)buff);		/* spark add */
		return (int)size;
	}

	/* Read Double index block */
	addr_start = &(inode->i_block[13]);
	devio_lseek(fd,(off_t)*addr_start * g_log_block_size + START_PARTION, 0);
	re = devio_read(fd, buff, g_log_block_size);
	if(re != g_log_block_size) {
		printf("Read the iblock[13] error!\n");
		free((char*)buff);	/* spark add */
		return 0;
	}
	d_addr_start = (__u32 *)buff;
	index_buff = (__u8 *)malloc(g_log_block_size);
	if(!index_buff) {
		printf("Can't alloc memory!\n");
		return 0;
	}
	for(i = 0;i < g_log_block_size/4; i++) {
		devio_lseek(fd, (off_t)*(d_addr_start+i) * g_log_block_size + START_PARTION, 0);
		re = devio_read(fd, index_buff, g_log_block_size);
		if(re != g_log_block_size) {
			printf("Can't read index block!\n");
			return 0;
		}
		addr_start = (__u32 *)index_buff;
		start_block = position/g_log_block_size - 12 - g_log_block_size/4 * (i + 1);
		re = ReadFromIndexBlock(fd, start_block, g_log_block_size/4 - 1, &start, &read_size,
				&position, addr_start);
		if(re) {
			printf("Can't read the double index block!\n");
			free((char*)buff);
			free((char*)index_buff);	/* spark add */
			return 0;
		}
		if(!read_size) {
			free((char*)buff);		/* spark add */
			free((char*)index_buff);
			return (int)size;
		}
	}
	//triple

	addr_start=&(inode->i_block[14]);
	devio_lseek(fd,(off_t)*addr_start*g_log_block_size+START_PARTION,0);
	re=devio_read(fd,buff,g_log_block_size);
	if(re!=g_log_block_size)
	{
		printf("Read the iblock[13] error!\n");
		free((char*)buff);	/* spark add */
		return 0;
	}
	d_addr_start=(__u32 *)buff;
	index_buff=(__u8 *)malloc(g_log_block_size);
	if(!index_buff)
	{
		printf("Can't alloc memory!\n");
		return 0;
	}

	start_block=(position/g_log_block_size-12-g_log_block_size/4-g_log_block_size/4*g_log_block_size/4)/(g_log_block_size/4*g_log_block_size/4);
	for(i=start_block;i<g_log_block_size/4-1;i++)
	{
		devio_lseek(fd,(off_t)*(d_addr_start+i)*g_log_block_size+START_PARTION,0);
		re=devio_read(fd,index_buff,g_log_block_size);
		if(re!=g_log_block_size)
		{
			printf("Can't read index block!\n");
			return 0;
		}
		addr_start=(__u32 *)index_buff;
		start_block=(position/g_log_block_size-12-g_log_block_size/4-g_log_block_size/4*g_log_block_size/4-g_log_block_size/4*g_log_block_size/4*i)/(g_log_block_size/4);
		re=ReadFromIndexBlock2(fd,start_block,g_log_block_size/4-1,&start,&read_size,&position,addr_start);
		if(re)
		{
			printf("Can't read the double index block!\n");
			free((char*)buff);
			free((char*)index_buff);	/* spark add */
			return 0;
		}
		if(!read_size) {
			free((char*)buff);		/* spark add */
			free((char*)index_buff);
			return (int)size;
		}
	}
	return 0;
}

static int ReadFromIndexBlock2(int fd,__u32 start_index,__u32 end_index,__u8 **start,	size_t *size,size_t *position,__u32 *d_addr_start)
{
	//addr_start is a array
	//start_block1 is arrys's index
	__u32 start_block;
	__u32 *addr_start;
	__u8 *index_buff;
	int i, re;

	if(start_index>end_index)
		return 0;

	index_buff=(__u8 *)malloc(g_log_block_size);
	if(!index_buff)
	{
		printf("Can't alloc memory!\n");
		return 0;
	}

	for(i=start_index;i<end_index;i++)
	{
		devio_lseek(fd,(off_t)*(d_addr_start+i)*g_log_block_size+START_PARTION,0);
		re=devio_read(fd,index_buff,g_log_block_size);
		if(re!=g_log_block_size)
		{
			printf("Can't read index block!\n");
			return 0;
		}
		addr_start=(__u32 *)index_buff;
		start_block= (*position/g_log_block_size-12)%(g_log_block_size/4);
		re=ReadFromIndexBlock(fd,start_block,g_log_block_size/4-1,start,size,position,addr_start);
		if(re)
		{
			printf("Can't read the double index block!\n");
			free((char*)index_buff);	/* spark add */
			return 0;
		}
		if(!*size) {
			free((char*)index_buff);
			return 0;
		}
	}
	return 0;
}

static int ext4_read_file(int fd, void *read_start, size_t size, size_t pos,
		struct ext4_inode *inode)
{
	int use_extent;
	use_extent = inode->i_flags & EXT4_EXTENTS_FL;

	if(!use_extent)
		return ext4_read_file0(fd, read_start, size, pos, inode);
	else
		return ext4_read_file1(fd, read_start, size, pos, inode);

}

int ext4_write(int fd,const void *start,size_t size)
{
	return 0;
}
off_t ext4_lseek(int fd,off_t offset,int where)
{
	_file[fd].posn = offset;
	return offset;
}
static FileSystem ext4fs = {
        "ext4", FS_FILE,
        ext4_open,
        ext4_read,
        ext4_write,
        ext4_lseek,
        ext4_close,
	NULL
};

static DiskFileSystem diskfile = {
	"ext4",
	ext4_open,
	ext4_read,
	ext4_write,
	ext4_lseek,
	ext4_close,
	NULL
};
static void init_fs(void) __attribute__ ((constructor));
static void init_fs()
{
	filefs_init(&ext4fs);
	diskfs_init(&diskfile);
}
