qemu-system-i386 -drive file=floppy.img,format=raw -m 12M -smp 2 -cpu core2duo -monitor vc &         # Redirect serial output to terminal
sleep 0.1
vncviewer -geometry 720x400+880+499 localhost:5900


# -d int #qemu debug
