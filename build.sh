#!/bin/bash
set -e

LD=i686-elf-ld
AS=nasm
BUILD_DIR=build

mkdir -p $BUILD_DIR

compile_c(){
	local filename=$(basename "$1")
	gcc -m32 -ffreestanding -c "$1" -o "$BUILD_DIR/${filename%.c}.o" -fno-builtin -nostartfiles -fno-pic -fno-stack-protector
	if [ $? -eq 0 ]; then
		echo "Compiled ($1) into ($BUILD_DIR/${filename%.c}.o)"
	else
		echo "Failed to compile ($1)"
	fi
}
compile_16asm() {
	local filename=$(basename "$1")
	nasm "$1" -f bin -o "$BUILD_DIR/${filename%.asm}.bin"
	if [ $? -eq 0 ]; then
		echo "Compiled ($1) into ($BUILD_DIR/${filename%.asm}.bin)"
	else
		echo "Failed To Compile ($1)"
	fi
}
compile_asm() {
	local filename=$(basename "$1")
	nasm "$1" -f elf -o "$BUILD_DIR/${filename%.asm}.o"
	if [ $? -eq 0 ]; then
		echo "Compiled ($1) into ($BUILD_DIR/${filename%.asm}.o)"
	else
		echo "Failed To Compile ($1)"
	fi
}
link_kernel() {
	echo "Linking kernel files into kernel.bin"
	local object_files=()
	for obj_file in "$@"; do
		object_files+=("$BUILD_DIR/$obj_file")
	done
	echo "Objects:"
	printf '  %s\n' "${object_files[@]}"
	ld \
		-m elf_i386 \
		-T linker-script.lds \
		-o "$BUILD_DIR/kernel.bin" \
		"$BUILD_DIR/kernel-entry.o" \
		"${object_files[@]}"
	if [ $? -eq 0 ]; then
		echo "Kernel linked successfully"
	else
		echo "Kernel linking failed"
		exit 1
	fi
}
create_floppy() {
	echo "Creating floppy image"
	
	# Create blank 1.44MB floppy
	dd if=/dev/zero of=floppy.img bs=512 count=2880 status=none
	
	# -----------------------------------------------------------------
	# Layout
	#
	# Sector 0      -> bootloader
	# Sector 1-19   -> stage2
	# Sector 20+    -> kernel
	#
	# -----------------------------------------------------------------
	
	# Bootloader
	dd if="$BUILD_DIR/bootloader.bin" \
	   of=floppy.img \
	   conv=notrunc \
	   bs=512 seek=0 status=none
	
	# Stage2
	dd if="$BUILD_DIR/stage2.bin" \
	   of=floppy.img \
	   conv=notrunc \
	   bs=512 seek=1 status=none
	
	# Kernel
	dd if="$BUILD_DIR/kernel.bin" \
	   of=floppy.img \
	   conv=notrunc \
	   bs=512 seek=20 status=none
	
	echo "Floppy image created"
}

# ===== BUILD =====

compile_16asm "boot/bootloader.asm"
compile_16asm "boot/stage2.asm"
compile_asm "boot/kernel-entry.asm"

compile_c "kernel/display.c"
compile_c "kernel/main.c"

link_kernel "main.o" "display.o"

create_floppy










#link_kernel(){
	#echo "Linking kernel files into kernel.bin"
	#local object_files=()
	#for obj_file in "$@"; do
		## Prepend the 'binarys/' directory to each filename
		#object_files+="binarys/$obj_file "
	#done
	#echo "${object_files[@]}"
	#ld -m elf_i386 -o binarys/kernel.bin -T linker-script.lds binarys/kernel-entry.o ${object_files[@]} --oformat binary
#}
#create_floppy() {
	#echo "Creating floppy.img"
	##dd if=/dev/zero of=blank.bin bs=1024 count=1440
	#cat binarys/bootloader.bin binarys/stage2.bin binarys/kernel.bin bees/*.bee blank.bin> floppy.img
	#cat binarys/bootloader.bin binarys/stage2.bin binarys/kernel.bin bees/*.bee> OS.img
#}


##compile_c "lib/memory_API.c"
##compile_c "kernel/arch/interrupt.c"
##compile_c "kernel/drivers/ports.c"
##compile_c "lib/util.c"
##compile_c "kernel/drivers/display.c"
##compile_asm32 "kernel/arch/asm_interrupt.asm"
##compile_c "kernel/drivers/keyboard.c"
##compile_c "lib/keyboard_API.c"




#compile_bee_v2 "bees/display/"
##compile_bee_v2 "bees/interrupt/"
##compile_bee_v2 "bees/keyboard/"
##compile_bee_v2 "bees/PIT/"
##compile_bee_v2 "bees/memory/"

#compile_bee_v2 "bees/util/"
#compile_bee_v2 "bees/start/"

##compile_asm16 "queen/bootloader.asm"
##compile_asm16 "queen/stage2.asm"
##compile_asm32 "queen/kernel-entry.asm"

#compile_c "queen/main.c"
##compile_asm32 "queen/main.asm"

#link_kernel "main.o" #"memory_API.o" "interrupt.o" "ports.o" "util.o" "display.o" "asm_interrupt.o" "keyboard.o" "keyboard_API.o"


#create_floppy
