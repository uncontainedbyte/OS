#pragma once

#include "int.h"
#include "display.h"
#include "memory.h"

void PCI_init();

int sata_init(void);
int sata_read(uint64 lba, uint32 count, void* buffer);
int sata_write(uint64 lba, uint32 count, void* buffer);
