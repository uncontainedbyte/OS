
[BITS 16]
[org 0x7C00]



_start:;{
	mov [BOOT_DRIVE], dl ; the drive id that was booted from
	
	mov bp, 0x9000
	mov sp, bp
	
	xor ax, ax
	mov ds, ax
	mov es, ax
	
	;## load stage 2 bootloader section ##;{
	mov si,loading_stage2_msg
	call print_string
	
	call load_stage2
	
	mov si,stage2_loaded_msg
	call print_string
	;}## load stage2 section end ##
	
	xor ax,ax
	mov al,[BOOT_DRIVE]
	
	jmp 0x0000:STAGE2_OFFSET
;}

load_stage2:;{
	mov bx, STAGE2_OFFSET
	mov ax, 20; loads 20 512B sectors of stage2 bootloader(10KB)
	push ax
	mov dl, [BOOT_DRIVE]
	mov cl, 0x2
	mov ah, 0x2
	mov ch, 0
	mov dh, 0
	int 0x13
	
	jc .stage2_load_error
	
	pop dx
	cmp al,dl
	jne .stage2_partal_load_error
	
	ret
	
.stage2_load_error:;{
	mov si, stage2_load_error_msg
	call print_string
	jmp $;}
.stage2_partal_load_error:;{
	mov si, stage2_partal_load_error_msg
	call print_string
	jmp $;}
;}

print_string:;{
	mov ah, 0x0E      ; Teletype output function
.next_char:
	cld
	lodsb             ; Load byte at DS:SI into AL and increment SI
	cmp al, 0        ; Check if it's the null terminator
	je .done         ; If so, jump to done
	int 0x10         ; Call BIOS interrupt to print character in AL
	jmp .next_char   ; Repeat for the next character
.done:
	ret               ; Return from function
;}


; data section
STAGE2_OFFSET equ 0x1000
BOOT_DRIVE db 0

stage2_load_error_msg db "Error: Failed to load Stage2!", 0
stage2_partal_load_error_msg db "Error: Failed to load entire Stage2!", 0
loading_stage2_msg db "Loading Stage2...", 10, 13, 0
stage2_loaded_msg db "Stage2 Loaded!", 10, 13, 0



; the bootloader needs to be 512B, this fills that
times 510 - ($ - $$) db 0  ; Fill the rest with zeros
dw 0xAA55                  ; Boot signature

