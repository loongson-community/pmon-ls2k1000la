/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Example OS glue functions for running on a Linux/POSIX system.
 */
#include <pmon.h>
#include "yaffscfg.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "yaffs_trace.h"
#include "ynand-mtd.h"
//#include <assert.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <mtdfile.h>
#include <errno.h>
#include <file.h>
#include "yaffs_packedtags2.h"

#define assert(expression) \
		do {	\
				if(!(expression)){ \
					(void)printf("Assertion %s failed: FILE %s LINE %d\n", \
					#expression, __FILE__, __LINE__); \
				} \
		}while(0)

unsigned yaffs_trace_mask = 

//	YAFFS_TRACE_SCAN |  
//	/*YAFFS_TRACE_GC |*/
//	YAFFS_TRACE_ERASE | 
//	YAFFS_TRACE_ERROR | 
//	YAFFS_TRACE_TRACING | 
//	YAFFS_TRACE_ALLOCATE | 
//	YAFFS_TRACE_BAD_BLOCKS |
//	YAFFS_TRACE_VERIFY | 
	
YAFFS_TRACE_BAD_BLOCKS |
YAFFS_TRACE_ERROR |
YAFFS_TRACE_BUG |
	
	0;

static int yaffs_errno;


/*
 * yaffsfs_CheckMemRegion()
 * Check that access to an address is valid.
 * This can check memory is in bounds and is writable etc.
 *
 * Returns 0 if ok, negative if not.
 */
int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request)
{
	(void) size;
	(void) write_request;

	if(!addr)
		return -1;
	return 0;
}

void yaffs_bug_fn(const char *file_name, int line_no)
{
	printf("yaffs bug detected %s:%d\n",
		file_name, line_no);
	assert(0);
}

void *yaffsfs_malloc(size_t size)
{
	return malloc(size);
}


void yaffsfs_free(void *ptr)
{
	free(ptr);
}

void yaffsfs_SetError(int err)
{
	yaffs_errno = err;
}

int yaffsfs_GetLastError(void)
{
	return yaffs_errno;
}


int yaffsfs_GetError(void)
{
	return yaffs_errno;
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

void yaffsfs_LockInit(void)
{
}

/*
 * yaffsfs_CurrentTime() retrns a 32-bit timestamp.
 *
 * Can return 0 if your system does not care about time.
 */

u32 yaffsfs_CurrentTime(void)
{
	return ticks;
}


void yaffsfs_OSInitialisation(void)
{
	yaffsfs_LockInit();
}


static const char *yaffs_file_type_str(struct yaffs_stat *stat)
{
	switch (stat->st_mode & S_IFMT) {
	case S_IFREG: return "regular file";
	case S_IFDIR: return "directory";
	case S_IFLNK: return "symlink";
	default: return "unknown";
	}
}

static const char *yaffs_error_str(void)
{
	int error = yaffsfs_GetLastError();

	if (error < 0)
		error = -error;

	switch (error) {
	case EBUSY: return "Busy";
	case ENODEV: return "No such device";
	case EINVAL: return "Invalid parameter";
	case ENFILE: return "Too many open files";
	case EBADF:  return "Bad handle";
	case EACCES: return "Wrong permissions";
	case EXDEV:  return "Not on same device";
	case ENOENT: return "No such entry";
	case ENOSPC: return "Device full";
	case EROFS:  return "Read only file system";
	case ERANGE: return "Range error";
	case ENOTEMPTY: return "Not empty";
	case ENAMETOOLONG: return "Name too long";
	case ENOMEM: return "Out of memory";
	case EFAULT: return "Fault";
	case EEXIST: return "Name exists";
	case ENOTDIR: return "Not a directory";
	case EISDIR: return "Not permitted on a directory";
	case ELOOP:  return "Symlink loop";
	case 0: return "No error";
	default: return "Unknown error";
	}
}

int cmd_yaffs_tracemask(unsigned set, unsigned mask)
{
	if (set)
		yaffs_trace_mask = mask;

	printf("yaffs trace mask: %08x\n", yaffs_trace_mask);
	return 0;
}

static int yaffs_regions_overlap(int a, int b, int x, int y)
{
	return	(a <= x && x <= b) ||
		(a <= y && y <= b) ||
		(x <= a && a <= y) ||
		(x <= b && b <= y);
}

static inline char *strdup(const char *s)
{
	char *r;

	if ((r = malloc(strlen(s)+1)) == NULL) return NULL;
	return strcpy(r,s);
}

struct yaffs_dev *yaffs_devconfig(char *mp, struct mtd_info *mtd,
			int start_block, int end_block)
{
	struct yaffs_dev *dev = NULL;
	struct yaffs_dev *chk;
	struct nand_chip *chip;
	struct yaffs_param *p;
	struct yaffs_driver *d;

	dev = calloc(1, sizeof(*dev));

	if (!dev) {
		/* Alloc error */
		printf("Failed to allocate memory\n");
		goto err;
	}

	if (end_block == 0)
		end_block = mtd->size / mtd->erasesize - 1;

	if (end_block < start_block) {
		printf("Bad start/end\n");
		goto err;
	}

	chip =  mtd->priv;

	/* Check for any conflicts */
	yaffs_dev_rewind();
	while (1) {
		chk = yaffs_next_dev();
		if (!chk)
			break;
		if (strcmp(chk->param.name, mp) == 0) {
			printf("Mount point name already used\n");
			goto err;
		}
		if (chk->driver_context == mtd &&
			yaffs_regions_overlap(
				chk->param.start_block, chk->param.end_block,
				start_block, end_block)) {
			printf("Region overlaps with partition %s\n",
				chk->param.name);
			goto err;
		}

	}

	/* Seems sane, so configure */
	memset(dev, 0, sizeof(*dev));
	dev->param.name = strdup(mp);
	dev->driver_context = mtd;
	p = &dev->param;
	p->start_block = start_block;
	p->end_block = end_block;
	p->chunks_per_block = mtd->erasesize / mtd->writesize;
	p->total_bytes_per_chunk = mtd->writesize;
	p->spare_bytes_per_chunk= mtd->oobsize;
	p->is_yaffs2 = 1;
	p->use_nand_ecc = 0;
	p->n_reserved_blocks = 5;
        p->inband_tags = (chip->ecc.layout->oobavail < sizeof(struct yaffs_packed_tags2));
	p->n_caches = 0;

	d= &dev->drv;
	d->drv_initialise_fn = ynand_initialise;
	d->drv_deinitialise_fn = ynand_deinitialise;
	d->drv_read_chunk_fn = ynand_rd_chunk;
	d->drv_write_chunk_fn = ynand_wr_chunk;
	d->drv_erase_fn = ynand_erase;
	d->drv_check_bad_fn = ynand_check_block_bad;
	d->drv_mark_bad_fn = ynand_mark_block_bad;

	yaffs_add_device(dev);

	printf("Configures yaffs mount %s: start block %d, end block %d %s\n",
		mp, start_block, end_block,
		dev->param.inband_tags ? "using inband tags" : "");
	return dev;

err:
	free(dev);
	return NULL;
}

int cmd_yaffs_devconfig(char *mp, struct mtd_info *mtd,
			int start_block, int end_block)
{
	struct yaffs_dev *dev;

	dev = yaffs_devconfig(mp, mtd, start_block, end_block);
	return dev != NULL;
}

int cmd_yaffs_dev_ls(void)
{
	struct yaffs_dev *dev;
	int free_space;

	yaffs_dev_rewind();

	while (1) {
		dev = yaffs_next_dev();
		if (!dev)
			break;
		printf("%-10s 0x%05x 0x%05x %s",
			dev->param.name,
			dev->param.start_block, dev->param.end_block,
			dev->param.inband_tags ? "using inband tags, " : "");

		free_space = yaffs_freespace(dev->param.name);
		if (free_space < 0)
			printf("not mounted\n");
		else
			printf("free 0x%x\n", free_space);

	}

	return 0;
}

int make_a_file(char *yaffsName, char bval, int sizeOfFile)
{
	int outh;
	int i;
	int written;
	unsigned char buffer[100];

	outh = yaffs_open(yaffsName, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (outh < 0) {
		printf("Error opening file: %d. %s\n", outh, yaffs_error_str());
		return -1;
	}

	memset(buffer, bval, 100);

	written = 0;
	while (written < sizeOfFile) {
		i = sizeOfFile - written;
		if (i > 100)
			i = 100;

		if (yaffs_write(outh, buffer, i) != i)
			break;
		written += i;
	}

	yaffs_close(outh);

	return (written == sizeOfFile) ? 0 : -1;
}

int read_a_file(char *fn)
{
	int h;
	int i = 0;
	unsigned char b;

	h = yaffs_open(fn, O_RDWR, 0);
	if (h < 0) {
		printf("File not found\n");
		return -1;
	}

	while (yaffs_read(h, &b, 1) > 0) {
		printf("%02x ", b);
		i++;
		if (i > 32) {
			printf("\n");
			i = 0;;
		}
	}
	printf("\n");
	yaffs_close(h);

	return 0;
}

int cmd_yaffs_mount(char *mp)
{
	int retval = yaffs_mount(mp);
	if (retval < 0)
		printf("Error mounting %s, return value: %d, %s\n", mp,
			yaffsfs_GetError(), yaffs_error_str());
	return retval;
}


int cmd_yaffs_umount(char *mp)
{
	int retval = yaffs_unmount(mp);

	if (retval < 0)
		printf("Error umounting %s, return value: %d, %s\n", mp,
			yaffsfs_GetError(), yaffs_error_str());
	return retval;
}

int cmd_yaffs_write_file(char *yaffsName, char bval, int sizeOfFile)
{
	return make_a_file(yaffsName, bval, sizeOfFile);
}


int cmd_yaffs_read_file(char *fn)
{
	return read_a_file(fn);
}


int cmd_yaffs_mread_file(char *fn, char *addr)
{
	int h;
	int retval = 0;
	struct yaffs_stat s;
	int read_size;

	yaffs_stat(fn, &s);

	printf("Copy %s to 0x%p... ", fn, addr);
	h = yaffs_open(fn, O_RDWR, 0);
	if (h < 0) {
		printf("File not found\n");
		retval = -1;
		read_size = 0;
	} else {
		read_size = (int) s.st_size;
		yaffs_read(h, addr, read_size);
		printf("\t[DONE]\n");

		yaffs_close(h);
	}

	return retval;
}


int cmd_yaffs_mwrite_file(char *fn, char *addr, int size)
{
	int outh;
	int wrote;

	outh = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (outh < 0) {
		printf("Error opening file: %d, %s\n", outh, yaffs_error_str());
		return -1;
	}

	wrote = yaffs_write(outh, addr, size);
	yaffs_close(outh);

	if(wrote != size) {
		printf("only wrote %d (0x%x) bytes\n", wrote, wrote);
		return -1;
	}
	return 0;


}


int cmd_yaffs_ls(const char *mountpt, int longlist)
{
	int i;
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat stat;
	char tempstr[255];

	d = yaffs_opendir(mountpt);

	if (!d) {
		printf("opendir failed, %s\n", yaffs_error_str());
		return -1;
	}

	for (i = 0; (de = yaffs_readdir(d)) != NULL; i++) {
		if (longlist) {
			sprintf(tempstr, "%s/%s", mountpt, de->d_name);
			yaffs_lstat(tempstr, &stat);
			printf("%-25s\t%7ld",
					de->d_name,
					(long)stat.st_size);
			printf(" %5d %6o %s\n",
				stat.st_ino, stat.st_mode,
					yaffs_file_type_str(&stat));
		} else {
			printf("%s\n", de->d_name);
		}
	}

	yaffs_closedir(d);

	return 0;
}

int cmd_yaffs_check(const char *fname, const char *type)
{
	int retval = 0;
	int ret;
	struct yaffs_stat stat;

	ret = yaffs_stat(fname, &stat);
	if (ret < 0) {
		printf("%s not found\n", fname);
		return -1;
	}

	printf("%s is a %s\n", fname, yaffs_file_type_str(&stat));

	if (strcmp(type, "REG") == 0 &&
	    (stat.st_mode & S_IFMT) != S_IFREG)
		retval = -1;

	if (strcmp(type, "DIR") == 0 &&
	    (stat.st_mode & S_IFMT) != S_IFDIR)
		retval = -1;

	if (retval == 0)
		printf("check ok\n");
	else
		printf("check failed\n");

	return retval;
}


int cmd_yaffs_mkdir(const char *dir)
{
	int retval = yaffs_mkdir(dir, 0);

	if (retval < 0) {
		printf("yaffs_mkdir returning error: %d, %s\n",
			retval, yaffs_error_str());
		return -1;
	}
	return 0;
}

int cmd_yaffs_rmdir(const char *dir)
{
	int retval = yaffs_rmdir(dir);

	if (retval < 0) {
		printf("yaffs_rmdir returning error: %d, %s\n",
			retval, yaffs_error_str());
		return -1;
	}
	return 0;
}

int cmd_yaffs_rm(const char *path)
{
	int retval = yaffs_unlink(path);

	if (retval < 0) {
		printf("yaffs_unlink returning error: %d, %s\n",
			retval, yaffs_error_str());
		return -1;
	}

	return 0;
}

int cmd_yaffs_mv(const char *oldPath, const char *newPath)
{
	int retval = yaffs_rename(newPath, oldPath);

	if (retval < 0) {
		printf("yaffs_unlink returning error: %d, %s\n",
			retval, yaffs_error_str());
		return -1;
	}

	return 0;
}

int cmd_yaffs_chmod(const char *path, int mode)
{
	int retval = yaffs_chmod(path, mode);

	if (retval < 0) {
		printf("yaffs_chmod returning error: %d, %s\n",
			retval, yaffs_error_str());
		return -1;
	}

	return 0;
}

int yaffs_start_up(struct mtd_info *mtd)
{
	struct yaffs_dev *dev = NULL;
	
	// Stuff to configure YAFFS
	// Stuff to initialise anything special (eg lock semaphore).
	yaffsfs_OSInitialisation();
	
	dev = yaffs_devconfig("/yaffs2", mtd, 0, 0);
	
	if(dev == NULL)
	{
		printf("yramsim_CreateRamSim() error.\n");
	}
	
	yaffs_mount((char *)dev->param.name);
	
	return 0;
}

static int dump_mem(unsigned char *buf, int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		if((i&0xf)==0) printf("\n%08x/%08x: ", buf +i, i);
		printf("%02x ",buf[i]);
	}
	printf("\n");
	return 0;
}

int cmd_rnand(int argc, char **argv)
{
	struct yaffs_dev *dev;
	struct mtd_info *mtd;
	struct nand_chip *chip;
	int page;
	yaffs_dev_rewind();
	dev = yaffs_next_dev();
	mtd = dev->driver_context;
	chip = mtd->priv;
	page = strtoul(argv[1], 0, 0);
	unsigned char buf[mtd->writesize+mtd->oobsize];
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);
	
	//ynand_rd_chunk (dev, page, buf, mtd->writesize, buf + mtd->writesize, mtd->oobsize, NULL);

	nand_generic_read(mtd, buf, mtd->writesize, 0, buf + mtd->writesize, mtd->oobsize, 0, 0);
	dump_mem(buf, mtd->writesize + mtd->oobsize);
	return 0;
}

int cmd_wnand(int argc, char **argv)
{
	struct yaffs_dev *dev;
	struct mtd_info *mtd;
	struct nand_chip *chip;
	int page;
	yaffs_dev_rewind();
	dev = yaffs_next_dev();
	mtd = dev->driver_context;
	chip = mtd->priv;
	page = strtoul(argv[1], 0, 0);
	unsigned char *buf = strtoul(argv[2], 0, 0);
	
	//ynand_wr_chunk (dev, page, buf, mtd->writesize, buf + mtd->writesize, mtd->oobsize);
	
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);
	nand_generic_write(mtd, buf, mtd->writesize, 0, buf + mtd->writesize, mtd->oobsize, 0, 0);
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	chip->waitfunc(mtd, chip);
	return 0;
}

int cmd_enand(int argc, char **argv)
{
	struct yaffs_dev *dev;
	struct mtd_info *mtd;
	int page;
	yaffs_dev_rewind();
	dev = yaffs_next_dev();
	mtd = dev->driver_context;
	page = strtoul(argv[1], 0, 0);
	
	ynand_erase(dev, page);
	return 0;
}


int cmd_yaffs(int argc, char **argv)
{
	char *usages =  "devls\n"
			"write file char size\n"
			"read file\n"
			"mread file addr\n"
			"mwrite file addr size\n"
			"ls dir [0|1]\n"
			"check file type\n"
			"mkdir dir\n"
			"rmdir dir\n"
			"rm file\n"
			"mv old new\n"
			"chmod file mode\n"
			"mount path\n"
			"unmount path\n";

	if (argc >= 2 && !strcmp(argv[1], "devls"))
		cmd_yaffs_dev_ls();
	else if (argc >= 5 && !strcmp(argv[1], "write"))
		cmd_yaffs_write_file(argv[2], argv[3][0], strtoul(argv[4], 0, 0));
	else if (argc >= 3 && !strcmp(argv[1], "read"))
		cmd_yaffs_read_file(argv[2]);
	else if (argc >= 4 && !strcmp(argv[1], "mread"))
		cmd_yaffs_mread_file(argv[2], strtoul(argv[3], 0, 0));
	else if (argc >= 5 && !strcmp(argv[1], "mwrite"))
		cmd_yaffs_mwrite_file(argv[2], strtoul(argv[3], 0, 0), strtoul(argv[4], 0, 0));
	else if (argc >= 3 && !strcmp(argv[1], "ls"))
		cmd_yaffs_ls(argv[2], argc > 3 ? strtoul(argv[3], 0, 0) : 1);
	else if (argc >= 4 && !strcmp(argv[1], "check"))
		cmd_yaffs_check(argv[2], argv[3]);
	else if (argc >= 3 && !strcmp(argv[1], "mkdir"))
		cmd_yaffs_mkdir(argv[2]);
	else if (argc >= 3 && !strcmp(argv[1], "rmdir"))
		cmd_yaffs_rmdir(argv[2]);
	else if (argc >= 3 && !strcmp(argv[1], "rm"))
		cmd_yaffs_rm(argv[2]);
	else if (argc >= 4 && !strcmp(argv[1], "mv"))
		cmd_yaffs_mv(argv[2], argv[3]);
	else if (argc >= 4 && !strcmp(argv[1], "chmod"))
		cmd_yaffs_chmod(argv[2], strtoul(argv[3], 0, 0));
	else if (argc >= 3 && !strcmp(argv[1], "mount"))
		cmd_yaffs_mount(argv[2]);
	else if (argc >= 3 && !strcmp(argv[1], "unmount"))
		cmd_yaffs_umount(argv[2]);
	else
		printf(usages);

	return 0;
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
        {"yaffs","[devls|ls|write|read|mread|mwrite|check|mdir|rmdir|rm|mv|mount|unmount]",  0,  "yaffs cmds", cmd_yaffs, 0, 99, CMD_REPEAT},
        {"rnand", "page", 0, "read nand", cmd_rnand, 0, 99, CMD_REPEAT},
	{"wnand", "page mem len", 0, "write nand", cmd_wnand, 0, 99, CMD_REPEAT},
	{"enand", "page", 0, "erase nand", cmd_enand,  0, 99, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

int pmon_yaffs_open(int fd,const char *path, int oflag, int mode)
{
	int handle = -1;
	int i, fd_mtd;
        mtdpriv *priv;
        mtdfile *info;
	struct mtd_info *mtd;
        char mp[YAFFS_MAX_NAME_LENGTH]={0};
	struct yaffs_dev *dev;

        path += 15;
        if(!path[0])
            return -1;
        for(i=0;i < YAFFS_MAX_NAME_LENGTH;i++){
            if(!path[i] || (path[i] == '/' && i ) )
                break;
            mp[i] = path[i];
        }
        //mp[i]='/';
        mp[i]=0;

        fd_mtd = open((char *)mp,0);
        if(fd_mtd == -1)
            return -1;
        priv = _file[fd_mtd].data;

	info = priv->file;
	mtd = info->mtd;

	yaffsfs_OSInitialisation();

	if (!yaffs_getdev(mp)) {
		dev = yaffs_devconfig(mp, mtd, (info->part_offset + priv->open_offset) / mtd->erasesize,
				(info->part_offset + priv->open_offset + priv->open_size_real)  / mtd->erasesize - 1);
		close(fd_mtd);
		if (dev == NULL)
			return -1;
		yaffs_mount((char *)dev->param.name);
	}

	if (path[strlen(path) - 1] == '/') {
		cmd_yaffs_ls(path, 1);
		return -1;
	}

	handle = yaffs_open(path, oflag,  S_IREAD | S_IWRITE);
        if(handle >= 0){
       		_file[fd].valid = 1;
		_file[fd].data = (void *)handle;
        }
	return fd;
}

int pmon_yaffs_read(int fd, void *buf, unsigned int nbyte)
{
	int handle = (int)_file[fd].data;

	return yaffs_read(handle, buf, nbyte);
}


int pmon_yaffs_write(int fd, const void *buf, unsigned int nbyte)
{
	int handle = (int)_file[fd].data;
	return  yaffs_write(handle, buf, nbyte);
}

off_t pmon_yaffs_lseek(int fd, off_t offset, int whence)
{
	int handle = (int)_file[fd].data;
	return yaffs_lseek(handle, offset, whence);
}

int pmon_yaffs_close(int fd)
{
	int handle = (int)_file[fd].data;
	return yaffs_close(handle);
}

static FileSystem pmon_yaffsfs = {
	"fs/yaffs2",FS_FILE,
	pmon_yaffs_open,
	pmon_yaffs_read,
	pmon_yaffs_write,
	pmon_yaffs_lseek,
	yaffs_close,
	NULL
};

static void init_fs(void) __attribute__ ((constructor));

static void init_fs()
{
	filefs_init(&pmon_yaffsfs);
}
