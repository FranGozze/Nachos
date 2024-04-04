#include "sys_info.hh"
#include "copyright.h"
#include "filesys/directory_entry.hh"
#include "filesys/file_system.hh"
#include "filesys/raw_file_header.hh"

#include <stdio.h>

#include "machine/disk.hh"
#include "machine/mmu.hh"

static const unsigned MAGIC_SIZE = sizeof(int);

static const unsigned DISK_SIZE = MAGIC_SIZE + NUM_SECTORS * SECTOR_SIZE;
// #include "machine/statistics.hh"
// extern Statistics *stats; ///< Performance metrics.

#ifdef USER_PROGRAM
#include "machine/machine.hh"
extern Machine *machine; // User program memory and registers.
#endif
static const int numPhysicalPages =
#ifdef USER_PROGRAM
    machine->GetNumPhysicalPages()
#endif
#ifndef USER_PROGRAM
        DEFAULT_NUM_PHYS_PAGES
#endif
    ;

void SysInfo()
{
  (void)COPYRIGHT; // Prevent warning about unused variable.

  const char *OPTIONS =
#ifdef THREADS
      "THREADS "
#endif
#ifdef USER_PROGRAM
      "USER_PROGRAM "
#endif
#ifdef VMEM
      "VMEM "
#endif
#ifdef FILESYS
      "FILESYS "
#endif
#ifdef FILESYS_STUB
      "FILESYS_STUB "
#endif
#ifdef USE_TLB
      "USE_TLB "
#endif
#ifdef FILESYS_NEEDED
      "FILESYS_NEEDED "
#endif
#ifdef DFS_TICKS_FIX
      "DFS_TICKS_FIX "
#endif
      ;

  printf("System information.\n");
  printf("\n\
General:\n\
  Nachos release: %s %s.\n\
  Option definitions: %s\n",
         PROGRAM, VERSION, OPTIONS);
  printf("\n\
Memory:\n\
  Page size: %u bytes.\n\
  Number of pages: %u.\n\
  Number of TLB entries: %u.\n\
  Memory size: %u bytes.\n",
         PAGE_SIZE,
         numPhysicalPages,
         TLB_SIZE,
         numPhysicalPages * PAGE_SIZE);
  printf("\n\
Disk:\n\
  Sector size: %u bytes.\n\
  Sectors per track: %u.\n\
  Number of tracks: %u.\n\
  Number of sectors: %u.\n\
  Disk size: %u bytes.\n",
         SECTOR_SIZE, SECTORS_PER_TRACK, NUM_TRACKS, NUM_SECTORS, DISK_SIZE);
  printf("\n\
Filesystem:\n\
  Sectors per header: %u.\n\
  Maximum file size: %u bytes.\n\
  File name maximum length: %u.\n\
  Free sectors map size: %u bytes.\n\
  Maximum number of dir-entries: %u.\n\
  Directory file size: %u bytes.\n",
         NUM_DIRECT, MAX_FILE_SIZE, FILE_NAME_MAX_LEN,
         FREE_MAP_FILE_SIZE, NUM_DIR_ENTRIES, DIRECTORY_FILE_SIZE);
}
