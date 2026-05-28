OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
	. = 0x00010000;
	
	kernel_start = .;
	
	.text ALIGN(4) :
	{
		*(.text)
	}
	
	.rodata ALIGN(4) :
	{
		*(.rodata)
	}
	
	.data ALIGN(4) :
	{
		*(.data)
	}
	
	.bss ALIGN(16) :
	{
		*(COMMON)
		*(.bss)
	}
	
	kernel_end = .;
}
