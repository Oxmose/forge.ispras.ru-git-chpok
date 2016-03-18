/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#include <config.h>

#include <errno.h>
#include <arch.h>
#include <core/debug.h>
#include <bsp_common.h>
#include "space.h"
#include <cons.h>

#include "devtree.h"

pok_bsp_t pok_bsp = {
    .ccsrbar_size = 0x1000000ULL,
    .ccsrbar_base = 0x0FE000000ULL,
    .ccsrbar_base_phys = 0x0FE000000ULL,
    .dcfg_offset = 0xE0000ULL,
    .serial0_regs_offset = 0x11C500ULL,
    /* U-Boot claims that CCB freq (equal to platform clock freq) = 606.061 MHz
     * Timebase freq = platform_clock/32 */
    .timebase_freq = 18939406,
    .pci_bridge = {
        .cfg_addr = 0xfe200000,
        .cfg_data = 0xfe200004,
        .iorange =  0xf8000000
    }
};

int pok_bsp_init (void)
{
   pok_cons_init ();

   //devtree_dummy_dump();

#ifdef POK_NEEDS_PCI
   pok_pci_init();
#endif

   return (POK_ERRNO_OK);
}


extern char _end[];

static char *heap_end = _end;

void *pok_bsp_mem_alloc (size_t sz)
{
  char *res;

  res = (char *)(((unsigned int)heap_end + 4095) & ~4095);
  heap_end = res + sz;
  return res;
}

void * pok_bsp_alloc_partition(size_t size)
{
    static uintptr_t last_base             = 0x4000000ULL; // 64 mebibytes
    static const size_t max_partition_size = POK_PARTITION_MEMORY_SIZE; // 16 mebibytes
    uintptr_t res;
    
    // TODO it should be more flexible than this
    if (size > max_partition_size) pok_fatal("partition size is too big");

    res = last_base;
    last_base += max_partition_size;

    // TODO check that we aren't out of RAM bounds

    return (void *) res;
}

//This is only used by virtio where alignment=4096
void *pok_bsp_mem_alloc_aligned(size_t mem_size, size_t alignment) 
{
    if (alignment == 4096)
        return pok_bsp_mem_alloc(mem_size);

    pok_fatal("unimplemented!");
}

void pok_bsp_get_info(void *addr) {
    pok_bsp_t *data = addr;
    *data = pok_bsp;
}

