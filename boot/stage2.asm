



[bits 16]
[org 0x1000]

_start:;{
	mov [BOOT_DRIVE], al
	
	mov si,spacer_msg
	call print_string
	mov si,loading_kernel_msg
	call print_string
	
	call load_kernel
	mov dword[0x7FA], KERNEL_LOAD_ADDRESS
	mov word[0x7F8], KERNEL_SECTORS_TO_LOAD
	
	mov si,ld_switch_msg
	call print_string
	
	call scan_hardware
	
	cli; Disable interrupts
	
	lgdt [GDT_POINTER]; Load the GDT
	
	; Set the PE (Protected Mode Enable) bit in CR0
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	
	jmp CODE_SEG:mode32b_start; Far jump
	;jmp $
;}



scan_hardware:;{
	
	call scan_ram
	
	
	
	
	
	
	
	
	
	ret
;}

scan_ram:;{
	
	; Initialize loop counter and buffer pointer
	xor ebx, ebx            ; EBX must be 0 for the first call
	mov di, 0x800         ; Start of the buffer to store memory regions
	mov ax, ds
	mov es, ax              ; ES must point to the segment of EDI
	mov word [0x7FE], 0
	
.get_next_region:
	; Set up registers for the BIOS call
	mov eax, 0xE820
	mov edx, 0x534D4150     ; "SMAP" signature
	mov ecx, 24             ; Size of the buffer to fill
	
	mov dword [es:di + 20], 1
	
	; Call the interrupt. The memory region data is stored at ES:DI
	int 0x15
	
	cmp eax, 0x534D4150
	jne .end_loop
	
	; Check the carry flag for errors or end of list
	jc .end_loop
	
	; The returned EBX is the continuation value.
	; If EBX is 0, we're done.
	cmp ebx, 0
	je .end_loop
	
	add word [0x7FE], 1
	
	; The BIOS call already wrote the 24 bytes to ES:DI.
	; We just need to advance our buffer pointer for the next entry.
	add di, 24
	
	; Loop again
	jmp .get_next_region
	
.end_loop:
	
	
	ret
;}








load_kernel:
	mov si, DAP
	; destination memory
	mov word [DAP + 4], 0x0000
	mov word [DAP + 6], KERNEL_LOAD_ADDRESS >> 4
	; starting LBA
	mov dword [DAP + 8], KERNEL_LBA_START
	mov dword [DAP + 12], 0
	mov cx, KERNEL_SECTORS_TO_LOAD
.load_loop:
	mov ah, 0x42
	mov dl, [BOOT_DRIVE]
	int 0x13
	jc .load_error
	; advance destination memory
	add word [DAP + 4], 512
	jnc .no_segment_adjust
	add word [DAP + 6], 0x1000
.no_segment_adjust:
	; next LBA
	inc dword [DAP + 8]
	loop .load_loop
	ret
.load_error:
	mov si, loading_error_msg
	call print_string
	jmp $
.partal_load_error:
	mov si, partal_loading_error_msg
	call print_string
	jmp $
;}
; Data for the error message
loading_error_msg db "Error: Failed to load kernel!", 10, 13, 0
partal_loading_error_msg db "Error: Failed to load entire kernel!", 10, 13, 0


spacer_msg db "----------------------------------------------------",10,13, 0
ld_switch_msg db "Switching To 32-Bit Mode...",10,13, 0
loading_kernel_msg db "Loading Kernel...",10,13, 0

print_string:;{
	mov ah, 0x0E      ; Teletype output function
	cld
.next_char:
	lodsb             ; Load byte at DS:SI into AL and increment SI
	cmp al, 0        ; Check if it's the null terminator
	je .done         ; If so, jump to done
	int 0x10         ; Call BIOS interrupt to print character in AL
	jmp .next_char   ; Repeat for the next character
.done:
	ret               ; Return from function
;}

DAP:
	db 0x10         ; size of packet
	db 0            ; reserved
	dw 1            ; sectors to read
	dw 0            ; offset
	dw 0            ; segment
	dq 0            ; starting LBA







[bits 32]
mode32b_start:;{
	; Reload all segment registers with the new 32-bit data segment
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	; Set up a stack
	mov esp, 0x90000          ; A common stack location
	
	cld
	call KERNEL_LOAD_ADDRESS
.hang:
	hlt
	jmp .hang
;}








GDT_START:
  GDT_NULL: db 0, 0, 0, 0, 0, 0, 0, 0
  GDT_CODE: db 0xFF, 0xFF, 0x00, 0x00, 0x00, 10011010b, 11001111b, 0x00
  GDT_DATA: db 0xFF, 0xFF, 0x00, 0x00, 0x00, 10010010b, 11001111b, 0x00
GDT_END:
GDT_POINTER:
  dw GDT_END - GDT_START - 1
  dd GDT_START

CODE_SEG equ GDT_CODE - GDT_START
DATA_SEG equ GDT_DATA - GDT_START



KERNEL_LOAD_ADDRESS    equ 0x10000
KERNEL_LBA_START    equ 20         ; LBA address of the first sector of kernel binary
KERNEL_SECTORS_TO_LOAD equ 128        ; Number of sectors to load
BOOT_DRIVE db 0





;times 2048 db 0
;times 2048 db 0
;times 2048 db 0
;times 2048 db 0
;times 2048 db 0
