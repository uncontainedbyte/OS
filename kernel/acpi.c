#include "acpi.h"





struct RSDP_t{
	char Signature[8];
	uint8 Checksum;
	char OEMID[6];
	uint8 Revision;
	uint32 RsdtAddress;
} __attribute__ ((packed));
struct SDT_header{
	char Signature[4];
	uint32 Length;
	uint8 Revision;
	uint8 Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32 OEMRevision;
	uint32 CreatorID;
	uint32 CreatorRevision;
} __attribute__ ((packed));
struct RSDT_t{
	struct SDT_header header;
	uint32* entries;
} __attribute__ ((packed));

uint32 hunt_RSDP(){
	
	int s=0x000E0000;
	while(true){
		if(((uint64*)s)[0]==0x2052545020445352){
			break;
		}
		s+=16;
		if(s>=0x000FFFFF) return 0;
	}
	
	return s;
}
uint8 rsdp_checksum(uint8 *rsdp){
	uint8 sum = 0;
	for(int i = 0; i < 20; i++){
		sum += rsdp[i];
	}
	return sum == 0;
}
uint8 rsdt_checksum(struct SDT_header *header){
	uint8 sum = 0;
	for(int i = 0; i < header->Length; i++){
		sum += ((uint8 *) header)[i];
	}
	return sum == 0;
}


void ACPI_init(){
	struct RSDP_t* RSDP = (struct RSDP_t*)hunt_RSDP();
	printf("%p\n",RSDP);
	
	if(!rsdp_checksum((uint8*)RSDP)){
		printf("%#40! RSDP checksum failed !");
	}
	printf("%p\n",RSDP->RsdtAddress);
	
	struct RSDT_t* RSDT = (struct RSDT_t*)RSDP->RsdtAddress;
	int RSDT_count = (RSDT->header.Length - sizeof(ACPISDTHeader)) / 4;
	
	if(!rsdt_checksum((struct SDT_header*) RSDT)){
		printf("%#40! RSDP checksum failed !");
	}
	
	
	
	
	
	
}


































