#include "pci.h"



uint32 pci_read32(uint8 bus,uint8 slot,uint8 func,uint8 offset){
	uint32 address = (1u << 31) | ((uint32)bus  << 16) | ((uint32)slot << 11) | ((uint32)func << 8) | (offset & 0xFC);
	outl(0xCF8, address);
	return inl(0xCFC);
}

void PCI_devices(){
	for(uint16 bus = 0; bus < 256; bus++){
		for(uint8 slot = 0; slot < 32; slot++){
			uint32 id = pci_read32(bus, slot, 0, 0);
			
			uint16 vendor = id & 0xFFFF;
			uint16 device = id >> 16;
			
			if(vendor == 0xFFFF) continue;
			
			uint32 classReg = pci_read32(bus, slot, 0, 0x08);
			
			uint8 classCode = classReg >> 24;
			uint8 subclass  = classReg >> 16;
			uint8 progIF    = classReg >> 8;
			
			printf("bus: %i : slot: %i : ",bus,slot);
			printf("vendor: %i : device: %i : ",vendor,device);
			printf("class: %i : sub: %i : ",classCode,subclass);
			printf("IF: %i\n",progIF);
		}
	}
}
uint32 find_device_PCI(int16 CLASS,uint16 SUB,uint16 IF){
	for(uint16 bus = 0; bus < 256; bus++){
		for(uint8 slot = 0; slot < 32; slot++){
			uint32 id = pci_read32(bus, slot, 0, 0);
			
			uint16 vendor = id & 0xFFFF;
			uint16 device = id >> 16;
			
			if(vendor == 0xFFFF) continue;
			
			uint32 classReg = pci_read32(bus, slot, 0, 0x08);
			
			uint8 classCode = classReg >> 24;
			uint8 subclass  = classReg >> 16;
			uint8 progIF    = classReg >> 8;
			
			if(classCode==CLASS||CLASS==-1){
				if(subclass==SUB||SUB==-1){
					if(progIF==IF||IF==-1){
						uint32 pack=0;
						pack |= bus;
						pack |= (slot<<16);
						return pack;
					}
				}
			}
		}
	}
	return 0;
}
uint32 hunt_AHCI(){
	return find_device_PCI(0x1,0x6,0x1);
}

#define HBA_PxCMD_ST   (1 << 0)
#define HBA_PxCMD_FRE  (1 << 4)
#define HBA_PxCMD_FR   (1 << 14)
#define HBA_PxCMD_CR   (1 << 15)
#define FIS_TYPE_REG_H2D 0x27
#define ATA_CMD_IDENTIFY_DEVICE 0xEC
#define AHCI_WORKSPACE_PAGES 4

typedef volatile struct{
	uint32 cap;
	uint32 ghc;
	uint32 is;
	uint32 pi;
} HBA_MEM_HEAD;
typedef volatile struct{
	uint32 clb;
	uint32 clbu;
	uint32 fb;
	uint32 fbu;
	uint32 is;
	uint32 ie;
	uint32 cmd;
	uint32 reserved0;
	uint32 tfd;
	uint32 sig;
	uint32 ssts;
	uint32 sctl;
	uint32 serr;
	uint32 sact;
	uint32 ci;
	uint32 sntf;
	uint32 fbs;
	uint32 reserved1[11];
	uint32 vendor[4];
} HBA_PORT;
typedef volatile struct{
	uint32 cap;
	uint32 ghc;
	uint32 is;
	uint32 pi;
	uint32 vs;
	
	uint32 ccc_ctl;
	uint32 ccc_pts;
	uint32 em_loc;
	uint32 em_ctl;
	uint32 cap2;
	uint32 bohc;
	
	uint8 reserved[0xA0 - 0x2C];
	uint8 vendor[0x100 - 0xA0];
	
	HBA_PORT ports[32];
} HBA_MEM;
typedef struct{
	HBA_PORT* port;
	
	uint64 sector_count;
} SATA_DRIVE;
typedef struct{
	uint8 cfl:5;
	uint8 a:1;
	uint8 w:1;
	uint8 p:1;
	
	uint8 r:1;
	uint8 b:1;
	uint8 c:1;
	uint8 reserved0:1;
	uint8 pmp:4;
	
	uint16 prdtl;
	
	volatile uint32 prdbc;
	
	uint32 ctba;
	uint32 ctbau;
	
	uint32 reserved1[4];
} __attribute__((packed)) HBA_CMD_HEADER;
typedef struct{
	uint32 dba;
	uint32 dbau;
	uint32 reserved0;
	uint32 dbc : 22;
	uint32 reserved1 : 9;
	uint32 i : 1;
} __attribute__((packed)) HBA_PRDT_ENTRY;
typedef struct{
	uint8 cfis[64];
	uint8 acmd[16];
	uint8 reserved[48];
	HBA_PRDT_ENTRY prdt_entry[1];
} __attribute__((packed)) HBA_CMD_TBL;
typedef struct{
	uint8 fis_type;
	uint8 pmport : 4;
	uint8 reserved0 : 3;
	uint8 c : 1;
	uint8 command;
	uint8 featurel;
	uint8 lba0;
	uint8 lba1;
	uint8 lba2;
	uint8 device;
	uint8 lba3;
	uint8 lba4;
	uint8 lba5;
	uint8 featureh;
	uint8 countl;
	uint8 counth;
	uint8 icc;
	uint8 control;
	uint8 reserved1[4];
} __attribute__((packed)) FIS_REG_H2D;



static void* ahci_workspace;
HBA_MEM* g_ahci;
SATA_DRIVE sata0;
static uint16 identify_data[256];

void stop_cmd(HBA_PORT* port){
	port->cmd &= ~HBA_PxCMD_ST;
	port->cmd &= ~HBA_PxCMD_FRE;
	while (port->cmd & HBA_PxCMD_FR);
	while(port->cmd & HBA_PxCMD_CR);
}
void start_cmd(HBA_PORT* port){
	while (port->cmd & HBA_PxCMD_CR);
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST;
}
void ahci_port_init(HBA_PORT* port){
	stop_cmd(port);
	
	ahci_workspace = (void*)alloc_pages(AHCI_WORKSPACE_PAGES);
	
	memset(ahci_workspace,0,AHCI_WORKSPACE_PAGES*4096);
	
	uint8* base = (uint8*)ahci_workspace;
	
	port->clb  = (uint32)(base);
	port->clbu = 0;
	port->fb   = (uint32)(base + 1024);
	port->fbu  = 0;
	
	HBA_CMD_HEADER* headers = (HBA_CMD_HEADER*)base;
	
	for(int i=0;i<32;i++){
		headers[i].prdtl = 1;
		headers[i].ctba = (uint32)(base + 0x1000 + (i * 0x100));
		headers[i].ctbau = 0;
	}
	
	port->serr = 0xFFFFFFFF;
	port->is   = 0xFFFFFFFF;
	
	start_cmd(port);
}
int find_cmdslot(HBA_PORT* port){
	uint32 slots = port->sact | port->ci;
	
	for(int i = 0; i < 32; i++){
		if((slots & (1 << i)) == 0) return i;
	}
	
	return -1;
}
void PCI_init(){
	
	uint32 AHCI = hunt_AHCI();
	if(!AHCI){
		printf("No AHCI\n");
	}
	uint32 bar5_low = pci_read32(AHCI&0xFFFF, (AHCI>>16)&0xFF, 0, 0x24);
	uint32 abar = bar5_low & 0xFFFFFFF0;
	
	uint32 virt_adr = alloc_pages(1);
	map_page(virt_adr,abar,0);
	
	HBA_MEM* hba = (HBA_MEM*)virt_adr;
	g_ahci = hba;
	
	for(int i = 0; i < 32; i++){
		if(!(hba->pi & (1 << i))) continue;
		
		HBA_PORT* port = &hba->ports[i];
		
		uint32 ssts = port->ssts;
		
		uint8 det = ssts & 0x0F;
		uint8 ipm = (ssts >> 8) & 0x0F;
		
		if(det == 3 && ipm == 1){
			if(port->sig == 0x00000101){
				sata0.port = port;
			}
		}
		
	}
	
	ahci_port_init(sata0.port);
	
}





int ahci_identify(SATA_DRIVE* drive){
	HBA_PORT* port = drive->port;
	int slot = find_cmdslot(port);
	
	if(slot < 0) return 0;
	
	HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)(uint32)port->clb;
	
	cmdheader += slot;
	memset(cmdheader,0,sizeof(HBA_CMD_HEADER));
	cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32);
	cmdheader->w = 0;
	cmdheader->prdtl = 1;
	
	HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)(uint32)cmdheader->ctba;
	
	memset(cmdtbl,0,256);
	
	cmdtbl->prdt_entry[0].dba = (uint32)identify_data;
	cmdtbl->prdt_entry[0].dbau = 0;
	cmdtbl->prdt_entry[0].dbc = 511;
	cmdtbl->prdt_entry[0].i = 1;
	
	FIS_REG_H2D* fis = (FIS_REG_H2D*)cmdtbl->cfis;
	
	fis->fis_type = FIS_TYPE_REG_H2D;
	fis->c = 1;
	fis->command = ATA_CMD_IDENTIFY_DEVICE;
	port->is = 0xFFFFFFFF;
	port->ci |= (1u << slot);
	
	while(port->ci & (1u << slot)){}
	
	if(port->is & (1u << 30)) return 0;
	
	uint64 sectors =
		((uint64)identify_data[103] << 48) |
		((uint64)identify_data[102] << 32) |
		((uint64)identify_data[101] << 16) |
		((uint64)identify_data[100]);
	
	drive->sector_count = sectors;
	
	printf("SATA sectors: %u\n", sectors);
	
	return 1;
}
int sata_init(){
	uint32 ahci = hunt_AHCI();
	
	if(!ahci){
		printf("AHCI controller not found\n");
		return 0;
	}
	
	uint8 bus  = ahci & 0xFF;
	uint8 slot = (ahci >> 16) & 0xFF;
	uint32 abar = pci_read32(bus,slot,0,0x24) & 0xFFFFFFF0;
	uint32 virt = alloc_pages(2);
	
	map_page(virt,abar,0);
	map_page(virt+4096,abar+4096,0);
	
	g_ahci = (HBA_MEM*)virt;
	g_ahci->ghc |= (1u << 31);
	sata0.port = 0;
	
	for(int i=0;i<32;i++){
		if(!(g_ahci->pi & (1u<<i))) continue;
		
		HBA_PORT* port = &g_ahci->ports[i];
		
		uint32 ssts = port->ssts;
		
		uint8 det = ssts & 0xF;
		uint8 ipm = (ssts >> 8) & 0xF;
		
		if(det != 3) continue;
		if(ipm != 1) continue;
		
		if(port->sig == 0x00000101){
			sata0.port = port;
			
			break;
		}
	}
	
	if(!sata0.port){
		printf("No SATA drive\n");
		return 0;
	}
	
	ahci_port_init(sata0.port);
	
	if(!ahci_identify(&sata0)) return 0;
	
	return 1;
}
#define ATA_CMD_READ_DMA_EXT   0x25
#define ATA_CMD_WRITE_DMA_EXT  0x35
int ahci_rw(HBA_PORT* port,uint64 lba,uint32 count,void* buffer,int write){
	int slot = find_cmdslot(port);
	if(slot < 0) return 0;
	
	HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)(uint32)port->clb;
	
	cmdheader += slot;
	
	memset(cmdheader,0,sizeof(HBA_CMD_HEADER));
	
	cmdheader->cfl = sizeof(FIS_REG_H2D)/4;
	cmdheader->w = write ? 1 : 0;
	cmdheader->prdtl = 1;
	
	HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)(uint32)cmdheader->ctba;
	
	memset(cmdtbl,0,256);
	
	cmdtbl->prdt_entry[0].dba = (uint32)buffer;
	cmdtbl->prdt_entry[0].dbau = 0;
	cmdtbl->prdt_entry[0].dbc = (count * 512) - 1;
	cmdtbl->prdt_entry[0].i = 1;
	
	FIS_REG_H2D* fis = (FIS_REG_H2D*)cmdtbl->cfis;
	
	fis->fis_type = FIS_TYPE_REG_H2D;
	fis->c = 1;
	fis->command = (write)? ATA_CMD_WRITE_DMA_EXT : ATA_CMD_READ_DMA_EXT;
	
	fis->lba0 = (uint8)(lba);
	fis->lba1 = (uint8)(lba >> 8);
	fis->lba2 = (uint8)(lba >> 16);
	fis->lba3 = (uint8)(lba >> 24);
	fis->lba4 = (uint8)(lba >> 32);
	fis->lba5 = (uint8)(lba >> 40);
	fis->device = 1 << 6;
	fis->countl = count & 0xFF;
	fis->counth = count >> 8;
	port->is = 0xFFFFFFFF;
	port->ci |= (1u << slot);
	
	while(port->ci & (1u << slot)){}
	
	if(port->is & (1u << 30)) return 0;
	
	return 1;
}
int sata_read(uint64 lba,uint32 count,void* buffer){
	return ahci_rw(sata0.port,lba,count,buffer,0);
}
int sata_write(uint64 lba,uint32 count,void* buffer){
	return ahci_rw(sata0.port,lba,count,buffer,1);
}















































