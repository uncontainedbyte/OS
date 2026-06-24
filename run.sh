
DISK_IMG="sata_storage.img"

if [ ! -f "$DISK_IMG" ]; then
	echo "Creating new persistent 32MB SATA disk image..."
	# Creates a 1GB empty file
	dd if=/dev/zero of="$DISK_IMG" bs=1M count=32
else
	echo "Persistent storage '$DISK_IMG' found. Reusing existing drive."
fi

qemu-system-i386 \
		-drive file=floppy.img,format=raw \
		-m 128M \
		-rtc base=localtime,clock=host\
		-smp 2 \
		-cpu core2duo \
		-device ahci,id=ahci0 \
		-drive id=sata_disk,file="$DISK_IMG",format=raw,if=none \
		-device ide-hd,bus=ahci0.0,drive=sata_disk \
		-monitor vc &         # Redirect serial output to terminal

sleep 0.1
vncviewer -geometry 720x400+280+299 localhost:5900


# -d int #qemu debug
