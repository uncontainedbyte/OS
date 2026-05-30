#pragma once
#include "int.h"
#include "display.h"
#include "interrupts.h"

#define KEY_ASCII(k)    ((k) & 0xFF)
#define KEY_SCANCODE(k) (((k) >> 8) & 0xFF)
#define KEY_MODS(k)     (((k) >> 16) & 0xFF)
#define KEY_FLAGS(k)    (((k) >> 24) & 0xFF)

#define KMOD_SHIFT      (1 << 0)
#define KMOD_CTRL       (1 << 1)
#define KMOD_ALT        (1 << 2)
#define KMOD_SUPER      (1 << 3)

#define KFLAG_RELEASE   (1 << 0)
#define KFLAG_EXTENDED  (1 << 1)
#define KFLAG_REPEAT    (1 << 2)
#define KFLAG_PRINTABLE (1 << 3)

#define KSTATE_CAPS     (1 << 0)
#define KSTATE_NUM      (1 << 1)
#define KSTATE_SCROLL   (1 << 2)

uint8 keyboard_get_state();
uint32 keyboard_read();
void keyboard_init();
