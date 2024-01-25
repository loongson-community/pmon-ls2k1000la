#include <machine/cpu.h>
#include "loongarchregs.h"

int md_cachestat()
{
       return 1;
}
void md_cacheon()
{
}

void wbflush()
{
       asm volatile ("\tdbar 0\n"::);
}

void CPU_ConfigCache(void)
{
       unsigned int cpu_prid;
       unsigned long long val;
       unsigned int cache_way, cache_index, cpu_l2_cache_lsize; 
       cpu_prid = read_32bit_csr(0xc0);

       /* L1 - I cache */
       val = read_64bit_csr(0xc8);
       cpu_icache_way = ((val >> 32) & 0xffff) + 1;
       cpu_icache_sets = 1UL << ((val >> 48) & 0xff);
       cpu_icache_linesz = 1UL << ((val >> 56) & 0x7f);
       cpu_icache_size = cpu_icache_way * cpu_icache_sets * cpu_icache_linesz;
       tgt_printf("Icache 0x%x\n", cpu_icache_size);

       /* L1 - D cache */
       val = read_64bit_csr(0xc9);
       cpu_dcache_way = (val & 0xffff) + 1;
       cpu_dcache_sets = 1UL << ((val >> 16) & 0xff);
       cpu_dcache_linesz = 1UL << ((val >> 24) & 0x7f);
       cpu_dcache_size = cpu_dcache_way * cpu_dcache_sets * cpu_dcache_linesz;
       tgt_printf("Dcache 0x%x\n", cpu_dcache_size);

       /* L2 - cache */
       val = read_64bit_csr(0xc9);
       cpu_l2_cache_way = ((val >> 32) & 0xffff) + 1;
       cpu_l2_cache_sets = 1UL << ((val >> 48) & 0x7f);
       cpu_l2_cache_linesz = 1UL << ((val >> 56) & 0xff);
       cpu_l2_cache_size = cpu_l2_cache_way * cpu_l2_cache_sets * cpu_l2_cache_linesz;
       tgt_printf("L2 cache 0x%x\n", cpu_l2_cache_size);

       /* L3 - cache */
       val = read_64bit_csr(0xca);
       cpu_l3_cache_way = (val & 0xffff) + 1;
       cpu_l3_cache_sets = 1UL << ((val >> 16) & 0x7f);
       cpu_l3_cache_linesz = 1UL << ((val >> 24) & 0xff);
       cpu_l3_cache_size = cpu_l3_cache_way * cpu_l3_cache_sets * cpu_l3_cache_linesz;
       tgt_printf("L3 cache 0x%x\n", cpu_l3_cache_size);
}
 
void CPU_FlushCache(void)
{

}

void CPU_FlushICache(unsigned long start, unsigned long len)
{
       asm volatile ("\tibar 0\n"::);
}

void CPU_FlushDCache(unsigned long start, unsigned long len)
{
       asm volatile ("\tdbar 0\n"::);
}

void CPU_HitFlushDCache(unsigned long start, unsigned long len)
{

}

void CPU_HitFlushSCache(unsigned long start, unsigned long len)
{

}

void CPU_IOFlushDCache(unsigned long start, unsigned long len, int flag)
{

}
