#include "interrupts.h"


#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20
void pic_remap(void) {
	uint8 a1 = inb(PIC1_DATA);
	uint8 a2 = inb(PIC2_DATA);
	outb(PIC1_COMMAND, 0x11);
	outb(PIC2_COMMAND, 0x11);
	outb(PIC1_DATA, 0x20); // master offset = 32
	outb(PIC2_DATA, 0x28); // slave offset = 40
	outb(PIC1_DATA, 0x04); // slave on IRQ2
	outb(PIC2_DATA, 0x02);
	outb(PIC1_DATA, 0x01);
	outb(PIC2_DATA, 0x01);
	outb(PIC1_DATA, a1);
	outb(PIC2_DATA, a2);
}

typedef struct{
	uint16 offset_low;
	uint16 selector;
	uint8 zero;
	uint8 flags;
	/* Flags:
	 * Bit 7: "Interrupt is present"
	 * Bits 6-5: Privilege level of caller (0=kernel..3=user)
	 * Bit 4: Set to 0 for interrupt gates
	 * Bits 3-0: bits 1110 = decimal 14 = "32 bit interrupt gate" */
	uint16 offset_high;
} __attribute__((packed)) idt_entry_t;
typedef struct {
	uint16 limit;
	uint32 base;
} __attribute__((packed)) idt_ptr_t;

idt_entry_t idt[256];
idt_ptr_t idt_ptr;
irq_handler_t handler_functions[256];

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);
extern void isr48(void);
extern void isr49(void);
extern void isr50(void);
extern void isr51(void);
extern void isr52(void);
extern void isr53(void);
extern void isr54(void);
extern void isr55(void);
extern void isr56(void);
extern void isr57(void);
extern void isr58(void);
extern void isr59(void);
extern void isr60(void);
extern void isr61(void);
extern void isr62(void);
extern void isr63(void);
extern void isr64(void);
extern void isr65(void);
extern void isr66(void);
extern void isr67(void);
extern void isr68(void);
extern void isr69(void);
extern void isr70(void);
extern void isr71(void);
extern void isr72(void);
extern void isr73(void);
extern void isr74(void);
extern void isr75(void);
extern void isr76(void);
extern void isr77(void);
extern void isr78(void);
extern void isr79(void);
extern void isr80(void);
extern void isr81(void);
extern void isr82(void);
extern void isr83(void);
extern void isr84(void);
extern void isr85(void);
extern void isr86(void);
extern void isr87(void);
extern void isr88(void);
extern void isr89(void);
extern void isr90(void);
extern void isr91(void);
extern void isr92(void);
extern void isr93(void);
extern void isr94(void);
extern void isr95(void);
extern void isr96(void);
extern void isr97(void);
extern void isr98(void);
extern void isr99(void);
extern void isr100(void);
extern void isr101(void);
extern void isr102(void);
extern void isr103(void);
extern void isr104(void);
extern void isr105(void);
extern void isr106(void);
extern void isr107(void);
extern void isr108(void);
extern void isr109(void);
extern void isr110(void);
extern void isr111(void);
extern void isr112(void);
extern void isr113(void);
extern void isr114(void);
extern void isr115(void);
extern void isr116(void);
extern void isr117(void);
extern void isr118(void);
extern void isr119(void);
extern void isr120(void);
extern void isr121(void);
extern void isr122(void);
extern void isr123(void);
extern void isr124(void);
extern void isr125(void);
extern void isr126(void);
extern void isr127(void);
extern void isr128(void);
extern void isr129(void);
extern void isr130(void);
extern void isr131(void);
extern void isr132(void);
extern void isr133(void);
extern void isr134(void);
extern void isr135(void);
extern void isr136(void);
extern void isr137(void);
extern void isr138(void);
extern void isr139(void);
extern void isr140(void);
extern void isr141(void);
extern void isr142(void);
extern void isr143(void);
extern void isr144(void);
extern void isr145(void);
extern void isr146(void);
extern void isr147(void);
extern void isr148(void);
extern void isr149(void);
extern void isr150(void);
extern void isr151(void);
extern void isr152(void);
extern void isr153(void);
extern void isr154(void);
extern void isr155(void);
extern void isr156(void);
extern void isr157(void);
extern void isr158(void);
extern void isr159(void);
extern void isr160(void);
extern void isr161(void);
extern void isr162(void);
extern void isr163(void);
extern void isr164(void);
extern void isr165(void);
extern void isr166(void);
extern void isr167(void);
extern void isr168(void);
extern void isr169(void);
extern void isr170(void);
extern void isr171(void);
extern void isr172(void);
extern void isr173(void);
extern void isr174(void);
extern void isr175(void);
extern void isr176(void);
extern void isr177(void);
extern void isr178(void);
extern void isr179(void);
extern void isr180(void);
extern void isr181(void);
extern void isr182(void);
extern void isr183(void);
extern void isr184(void);
extern void isr185(void);
extern void isr186(void);
extern void isr187(void);
extern void isr188(void);
extern void isr189(void);
extern void isr190(void);
extern void isr191(void);
extern void isr192(void);
extern void isr193(void);
extern void isr194(void);
extern void isr195(void);
extern void isr196(void);
extern void isr197(void);
extern void isr198(void);
extern void isr199(void);
extern void isr200(void);
extern void isr201(void);
extern void isr202(void);
extern void isr203(void);
extern void isr204(void);
extern void isr205(void);
extern void isr206(void);
extern void isr207(void);
extern void isr208(void);
extern void isr209(void);
extern void isr210(void);
extern void isr211(void);
extern void isr212(void);
extern void isr213(void);
extern void isr214(void);
extern void isr215(void);
extern void isr216(void);
extern void isr217(void);
extern void isr218(void);
extern void isr219(void);
extern void isr220(void);
extern void isr221(void);
extern void isr222(void);
extern void isr223(void);
extern void isr224(void);
extern void isr225(void);
extern void isr226(void);
extern void isr227(void);
extern void isr228(void);
extern void isr229(void);
extern void isr230(void);
extern void isr231(void);
extern void isr232(void);
extern void isr233(void);
extern void isr234(void);
extern void isr235(void);
extern void isr236(void);
extern void isr237(void);
extern void isr238(void);
extern void isr239(void);
extern void isr240(void);
extern void isr241(void);
extern void isr242(void);
extern void isr243(void);
extern void isr244(void);
extern void isr245(void);
extern void isr246(void);
extern void isr247(void);
extern void isr248(void);
extern void isr249(void);
extern void isr250(void);
extern void isr251(void);
extern void isr252(void);
extern void isr253(void);
extern void isr254(void);
extern void isr255(void);

void set_idt_gate(int n, uint32 handler, uint8 flags){
	idt[n].offset_low  = handler & 0xFFFF;
	idt[n].selector    = 0x08;
	idt[n].zero        = 0;
	idt[n].flags       = flags;
	idt[n].offset_high = (handler >> 16) & 0xFFFF;
}
void isr_install(){
	set_idt_gate(0, (uint32)isr0,systemFlags);
	set_idt_gate(1, (uint32)isr1,systemFlags);
	set_idt_gate(2, (uint32)isr2,systemFlags);
	set_idt_gate(3, (uint32)isr3,systemFlags);
	set_idt_gate(4, (uint32)isr4,systemFlags);
	set_idt_gate(5, (uint32)isr5,systemFlags);
	set_idt_gate(6, (uint32)isr6,systemFlags);
	set_idt_gate(7, (uint32)isr7,systemFlags);
	set_idt_gate(8, (uint32)isr8,systemFlags);
	set_idt_gate(9, (uint32)isr9,systemFlags);
	set_idt_gate(10, (uint32)isr10,systemFlags);
	set_idt_gate(11, (uint32)isr11,systemFlags);
	set_idt_gate(12, (uint32)isr12,systemFlags);
	set_idt_gate(13, (uint32)isr13,systemFlags);
	set_idt_gate(14, (uint32)isr14,systemFlags);
	set_idt_gate(15, (uint32)isr15,systemFlags);
	set_idt_gate(16, (uint32)isr16,systemFlags);
	set_idt_gate(17, (uint32)isr17,systemFlags);
	set_idt_gate(18, (uint32)isr18,systemFlags);
	set_idt_gate(19, (uint32)isr19,systemFlags);
	set_idt_gate(20, (uint32)isr20,systemFlags);
	set_idt_gate(21, (uint32)isr21,systemFlags);
	set_idt_gate(22, (uint32)isr22,systemFlags);
	set_idt_gate(23, (uint32)isr23,systemFlags);
	set_idt_gate(24, (uint32)isr24,systemFlags);
	set_idt_gate(25, (uint32)isr25,systemFlags);
	set_idt_gate(26, (uint32)isr26,systemFlags);
	set_idt_gate(27, (uint32)isr27,systemFlags);
	set_idt_gate(28, (uint32)isr28,systemFlags);
	set_idt_gate(29, (uint32)isr29,systemFlags);
	set_idt_gate(30, (uint32)isr30,systemFlags);
	set_idt_gate(31, (uint32)isr31,systemFlags);
}
void irq_install(){
	set_idt_gate(32, (uint32)irq0,systemFlags);
	set_idt_gate(33, (uint32)irq1,systemFlags);
	set_idt_gate(34, (uint32)irq2,systemFlags);
	set_idt_gate(35, (uint32)irq3,systemFlags);
	set_idt_gate(36, (uint32)irq4,systemFlags);
	set_idt_gate(37, (uint32)irq5,systemFlags);
	set_idt_gate(38, (uint32)irq6,systemFlags);
	set_idt_gate(39, (uint32)irq7,systemFlags);
	set_idt_gate(40, (uint32)irq8,systemFlags);
	set_idt_gate(41, (uint32)irq9,systemFlags);
	set_idt_gate(42, (uint32)irq10,systemFlags);
	set_idt_gate(43, (uint32)irq11,systemFlags);
	set_idt_gate(44, (uint32)irq12,systemFlags);
	set_idt_gate(45, (uint32)irq13,systemFlags);
	set_idt_gate(46, (uint32)irq14,systemFlags);
	set_idt_gate(47, (uint32)irq15,systemFlags);
}
void install_extra_isr(){
	set_idt_gate(48, (uint32)isr48,systemFlags);
	set_idt_gate(49,  (uint32)isr49,systemFlags);
	set_idt_gate(50,  (uint32)isr50,systemFlags);
	set_idt_gate(51,  (uint32)isr51,systemFlags);
	set_idt_gate(52,  (uint32)isr52,systemFlags);
	set_idt_gate(53,  (uint32)isr53,systemFlags);
	set_idt_gate(54,  (uint32)isr54,systemFlags);
	set_idt_gate(55,  (uint32)isr55,systemFlags);
	set_idt_gate(56,  (uint32)isr56,systemFlags);
	set_idt_gate(57,  (uint32)isr57,systemFlags);
	set_idt_gate(58,  (uint32)isr58,systemFlags);
	set_idt_gate(59,  (uint32)isr59,systemFlags);
	set_idt_gate(60,  (uint32)isr60,systemFlags);
	set_idt_gate(61,  (uint32)isr61,systemFlags);
	set_idt_gate(62,  (uint32)isr62,systemFlags);
	set_idt_gate(63,  (uint32)isr63,systemFlags);
	set_idt_gate(64,  (uint32)isr64,systemFlags);
	set_idt_gate(65,  (uint32)isr65,systemFlags);
	set_idt_gate(66,  (uint32)isr66,systemFlags);
	set_idt_gate(67,  (uint32)isr67,systemFlags);
	set_idt_gate(68,  (uint32)isr68,systemFlags);
	set_idt_gate(69,  (uint32)isr69,systemFlags);
	set_idt_gate(70,  (uint32)isr70,systemFlags);
	set_idt_gate(71,  (uint32)isr71,systemFlags);
	set_idt_gate(72,  (uint32)isr72,systemFlags);
	set_idt_gate(73,  (uint32)isr73,systemFlags);
	set_idt_gate(74,  (uint32)isr74,systemFlags);
	set_idt_gate(75,  (uint32)isr75,systemFlags);
	set_idt_gate(76,  (uint32)isr76,systemFlags);
	set_idt_gate(77,  (uint32)isr77,systemFlags);
	set_idt_gate(78,  (uint32)isr78,systemFlags);
	set_idt_gate(79,  (uint32)isr79,systemFlags);
	set_idt_gate(80,  (uint32)isr80,systemFlags);
	set_idt_gate(81,  (uint32)isr81,systemFlags);
	set_idt_gate(82,  (uint32)isr82,systemFlags);
	set_idt_gate(83,  (uint32)isr83,systemFlags);
	set_idt_gate(84,  (uint32)isr84,systemFlags);
	set_idt_gate(85,  (uint32)isr85,systemFlags);
	set_idt_gate(86,  (uint32)isr86,systemFlags);
	set_idt_gate(87,  (uint32)isr87,systemFlags);
	set_idt_gate(88,  (uint32)isr88,systemFlags);
	set_idt_gate(89,  (uint32)isr89,systemFlags);
	set_idt_gate(90,  (uint32)isr90,systemFlags);
	set_idt_gate(91,  (uint32)isr91,systemFlags);
	set_idt_gate(92,  (uint32)isr92,systemFlags);
	set_idt_gate(93,  (uint32)isr93,systemFlags);
	set_idt_gate(94,  (uint32)isr94,systemFlags);
	set_idt_gate(95,  (uint32)isr95,systemFlags);
	set_idt_gate(96,  (uint32)isr96,systemFlags);
	set_idt_gate(97,  (uint32)isr97,systemFlags);
	set_idt_gate(98,  (uint32)isr98,systemFlags);
	set_idt_gate(99,  (uint32)isr99,systemFlags);
	set_idt_gate(100, (uint32)isr100,systemFlags);
	set_idt_gate(101, (uint32)isr101,systemFlags);
	set_idt_gate(102, (uint32)isr102,systemFlags);
	set_idt_gate(103, (uint32)isr103,systemFlags);
	set_idt_gate(104, (uint32)isr104,systemFlags);
	set_idt_gate(105, (uint32)isr105,systemFlags);
	set_idt_gate(106, (uint32)isr106,systemFlags);
	set_idt_gate(107, (uint32)isr107,systemFlags);
	set_idt_gate(108, (uint32)isr108,systemFlags);
	set_idt_gate(109, (uint32)isr109,systemFlags);
	set_idt_gate(110, (uint32)isr110,systemFlags);
	set_idt_gate(111, (uint32)isr111,systemFlags);
	set_idt_gate(112, (uint32)isr112,systemFlags);
	set_idt_gate(113, (uint32)isr113,systemFlags);
	set_idt_gate(114, (uint32)isr114,systemFlags);
	set_idt_gate(115, (uint32)isr115,systemFlags);
	set_idt_gate(116, (uint32)isr116,systemFlags);
	set_idt_gate(117, (uint32)isr117,systemFlags);
	set_idt_gate(118, (uint32)isr118,systemFlags);
	set_idt_gate(119, (uint32)isr119,systemFlags);
	set_idt_gate(120, (uint32)isr120,systemFlags);
	set_idt_gate(121, (uint32)isr121,systemFlags);
	set_idt_gate(122, (uint32)isr122,systemFlags);
	set_idt_gate(123, (uint32)isr123,systemFlags);
	set_idt_gate(124, (uint32)isr124,systemFlags);
	set_idt_gate(125, (uint32)isr125,systemFlags);
	set_idt_gate(126, (uint32)isr126,systemFlags);
	set_idt_gate(127, (uint32)isr127,systemFlags);
	set_idt_gate(128, (uint32)isr128,systemFlags);
	set_idt_gate(129, (uint32)isr129,systemFlags);
	set_idt_gate(130, (uint32)isr130,systemFlags);
	set_idt_gate(131, (uint32)isr131,systemFlags);
	set_idt_gate(132, (uint32)isr132,systemFlags);
	set_idt_gate(133, (uint32)isr133,systemFlags);
	set_idt_gate(134, (uint32)isr134,systemFlags);
	set_idt_gate(135, (uint32)isr135,systemFlags);
	set_idt_gate(136, (uint32)isr136,systemFlags);
	set_idt_gate(137, (uint32)isr137,systemFlags);
	set_idt_gate(138, (uint32)isr138,systemFlags);
	set_idt_gate(139, (uint32)isr139,systemFlags);
	set_idt_gate(140, (uint32)isr140,systemFlags);
	set_idt_gate(141, (uint32)isr141,systemFlags);
	set_idt_gate(142, (uint32)isr142,systemFlags);
	set_idt_gate(143, (uint32)isr143,systemFlags);
	set_idt_gate(144, (uint32)isr144,systemFlags);
	set_idt_gate(145, (uint32)isr145,systemFlags);
	set_idt_gate(146, (uint32)isr146,systemFlags);
	set_idt_gate(147, (uint32)isr147,systemFlags);
	set_idt_gate(148, (uint32)isr148,systemFlags);
	set_idt_gate(149, (uint32)isr149,systemFlags);
	set_idt_gate(150, (uint32)isr150,systemFlags);
	set_idt_gate(151, (uint32)isr151,systemFlags);
	set_idt_gate(152, (uint32)isr152,systemFlags);
	set_idt_gate(153, (uint32)isr153,systemFlags);
	set_idt_gate(154, (uint32)isr154,systemFlags);
	set_idt_gate(155, (uint32)isr155,systemFlags);
	set_idt_gate(156, (uint32)isr156,systemFlags);
	set_idt_gate(157, (uint32)isr157,systemFlags);
	set_idt_gate(158, (uint32)isr158,systemFlags);
	set_idt_gate(159, (uint32)isr159,systemFlags);
	set_idt_gate(160, (uint32)isr160,systemFlags);
	set_idt_gate(161, (uint32)isr161,systemFlags);
	set_idt_gate(162, (uint32)isr162,systemFlags);
	set_idt_gate(163, (uint32)isr163,systemFlags);
	set_idt_gate(164, (uint32)isr164,systemFlags);
	set_idt_gate(165, (uint32)isr165,systemFlags);
	set_idt_gate(166, (uint32)isr166,systemFlags);
	set_idt_gate(167, (uint32)isr167,systemFlags);
	set_idt_gate(168, (uint32)isr168,systemFlags);
	set_idt_gate(169, (uint32)isr169,systemFlags);
	set_idt_gate(170, (uint32)isr170,systemFlags);
	set_idt_gate(171, (uint32)isr171,systemFlags);
	set_idt_gate(172, (uint32)isr172,systemFlags);
	set_idt_gate(173, (uint32)isr173,systemFlags);
	set_idt_gate(174, (uint32)isr174,systemFlags);
	set_idt_gate(175, (uint32)isr175,systemFlags);
	set_idt_gate(176, (uint32)isr176,systemFlags);
	set_idt_gate(177, (uint32)isr177,systemFlags);
	set_idt_gate(178, (uint32)isr178,systemFlags);
	set_idt_gate(179, (uint32)isr179,systemFlags);
	set_idt_gate(180, (uint32)isr180,systemFlags);
	set_idt_gate(181, (uint32)isr181,systemFlags);
	set_idt_gate(182, (uint32)isr182,systemFlags);
	set_idt_gate(183, (uint32)isr183,systemFlags);
	set_idt_gate(184, (uint32)isr184,systemFlags);
	set_idt_gate(185, (uint32)isr185,systemFlags);
	set_idt_gate(186, (uint32)isr186,systemFlags);
	set_idt_gate(187, (uint32)isr187,systemFlags);
	set_idt_gate(188, (uint32)isr188,systemFlags);
	set_idt_gate(189, (uint32)isr189,systemFlags);
	set_idt_gate(190, (uint32)isr190,systemFlags);
	set_idt_gate(191, (uint32)isr191,systemFlags);
	set_idt_gate(192, (uint32)isr192,systemFlags);
	set_idt_gate(193, (uint32)isr193,systemFlags);
	set_idt_gate(194, (uint32)isr194,systemFlags);
	set_idt_gate(195, (uint32)isr195,systemFlags);
	set_idt_gate(196, (uint32)isr196,systemFlags);
	set_idt_gate(197, (uint32)isr197,systemFlags);
	set_idt_gate(198, (uint32)isr198,systemFlags);
	set_idt_gate(199, (uint32)isr199,systemFlags);
	set_idt_gate(200, (uint32)isr200,systemFlags);
	set_idt_gate(201, (uint32)isr201,systemFlags);
	set_idt_gate(202, (uint32)isr202,systemFlags);
	set_idt_gate(203, (uint32)isr203,systemFlags);
	set_idt_gate(204, (uint32)isr204,systemFlags);
	set_idt_gate(205, (uint32)isr205,systemFlags);
	set_idt_gate(206, (uint32)isr206,systemFlags);
	set_idt_gate(207, (uint32)isr207,systemFlags);
	set_idt_gate(208, (uint32)isr208,systemFlags);
	set_idt_gate(209, (uint32)isr209,systemFlags);
	set_idt_gate(210, (uint32)isr210,systemFlags);
	set_idt_gate(211, (uint32)isr211,systemFlags);
	set_idt_gate(212, (uint32)isr212,systemFlags);
	set_idt_gate(213, (uint32)isr213,systemFlags);
	set_idt_gate(214, (uint32)isr214,systemFlags);
	set_idt_gate(215, (uint32)isr215,systemFlags);
	set_idt_gate(216, (uint32)isr216,systemFlags);
	set_idt_gate(217, (uint32)isr217,systemFlags);
	set_idt_gate(218, (uint32)isr218,systemFlags);
	set_idt_gate(219, (uint32)isr219,systemFlags);
	set_idt_gate(220, (uint32)isr220,systemFlags);
	set_idt_gate(221, (uint32)isr221,systemFlags);
	set_idt_gate(222, (uint32)isr222,systemFlags);
	set_idt_gate(223, (uint32)isr223,systemFlags);
	set_idt_gate(224, (uint32)isr224,systemFlags);
	set_idt_gate(225, (uint32)isr225,systemFlags);
	set_idt_gate(226, (uint32)isr226,systemFlags);
	set_idt_gate(227, (uint32)isr227,systemFlags);
	set_idt_gate(228, (uint32)isr228,systemFlags);
	set_idt_gate(229, (uint32)isr229,systemFlags);
	set_idt_gate(230, (uint32)isr230,systemFlags);
	set_idt_gate(231, (uint32)isr231,systemFlags);
	set_idt_gate(232, (uint32)isr232,systemFlags);
	set_idt_gate(233, (uint32)isr233,systemFlags);
	set_idt_gate(234, (uint32)isr234,systemFlags);
	set_idt_gate(235, (uint32)isr235,systemFlags);
	set_idt_gate(236, (uint32)isr236,systemFlags);
	set_idt_gate(237, (uint32)isr237,systemFlags);
	set_idt_gate(238, (uint32)isr238,systemFlags);
	set_idt_gate(239, (uint32)isr239,systemFlags);
	set_idt_gate(240, (uint32)isr240,systemFlags);
	set_idt_gate(241, (uint32)isr241,systemFlags);
	set_idt_gate(242, (uint32)isr242,systemFlags);
	set_idt_gate(243, (uint32)isr243,systemFlags);
	set_idt_gate(244, (uint32)isr244,systemFlags);
	set_idt_gate(245, (uint32)isr245,systemFlags);
	set_idt_gate(246, (uint32)isr246,systemFlags);
	set_idt_gate(247, (uint32)isr247,systemFlags);
	set_idt_gate(248, (uint32)isr248,systemFlags);
	set_idt_gate(249, (uint32)isr249,systemFlags);
	set_idt_gate(250, (uint32)isr250,systemFlags);
	set_idt_gate(251, (uint32)isr251,systemFlags);
	set_idt_gate(252, (uint32)isr252,systemFlags);
	set_idt_gate(253, (uint32)isr253,systemFlags);
	set_idt_gate(254, (uint32)isr254,systemFlags);
	set_idt_gate(255, (uint32)isr255,systemFlags);
}
void pic_mask_all(void){
	outb(0x21, 0xFF); // master
	outb(0xA1, 0xFF); // slave
}
void pic_unmask_irq(uint8 irq){
	uint16 port = (irq < 8) ? 0x21 : 0xA1;
	uint8  mask = inb(port) & ~(1 << (irq % 8));
	outb(port, mask);
}
void disable_interrupts(){
	asm volatile("cli");
}
void enable_interrupts(){
	asm volatile("sti");
}
uint8 interrupts_enabled(){
	uint32 eflags;
	asm volatile("pushf\npop %0": "=r"(eflags));
	return (eflags & (1 << 9)) != 0;
}
void idt_init(){
	disable_interrupts();
	
	for(int i = 0; i < 256; i++){
		idt[i] = (idt_entry_t){0};
		handler_functions[i] = 0;
	}
	
	isr_install();
	pic_remap();
	irq_install();
	install_extra_isr();
	
	idt_ptr.base = (uint32) &idt;
	idt_ptr.limit = 256 * sizeof(idt_entry_t) - 1;
	asm volatile("lidt (%0)" : : "r" (&idt_ptr));
	
	pic_mask_all();
}

void UNHANDLED_INTERRUPT(int i){
	printf("%#40! Unhandled Interrupt: ");
	if(i==0){
		printf("%#400 !");
	}else if(i<10){
		printf("%#40%c !",i+'0');
	}else if(i<100){
		printf("%#40%c%c !",((i/10)%10)+'0',(i%10)+'0');
	}else{
		printf("%#40%c%c%c !",((i/100)%10)+'0',((i/10)%10)+'0',(i%10)+'0');
	}
	asm volatile("hlt");
}
uint32 isr_handler(registers_t *r){
	irq_handler_t handler = (irq_handler_t)handler_functions[r->int_no];
	if(handler==0){ UNHANDLED_INTERRUPT(r->int_no); }
	
	uint32 ret = handler(r);
	
	if(31 < r->int_no && r->int_no < 48){
		if(r->int_no >= 40) outb(PIC2_COMMAND, PIC_EOI);
		outb(PIC1_COMMAND, PIC_EOI);
	}
	
	return ret;
}

void registerInterrupt(uint32 id,uint8 flags,irq_handler_t handler){
	disable_interrupts();
	
	handler_functions[id] = handler;
	idt[id].flags = flags;
	
	enable_interrupts();
}



uint32 handle_DIVbyZERO(registers_t *r){
	printf("%#40! Division By Zero !");
	asm volatile("hlt");
	while(1);
	return (uint32)r;
}
uint32 handle_InvalidOpcode(registers_t *r){
	printf("%#40! Invalid Opcode !");
	asm volatile("hlt");
	while(1);
	return (uint32)r;
}
uint32 handle_DoubleFault(registers_t *r){
	printf("%#40! Double Fault !");
	asm volatile("hlt");
	while(1);
	return (uint32)r;
}
uint32 handle_GeneralProtectionFault(registers_t *r){
	printf("%#40! General Protection Fault !");
	asm volatile("hlt");
	while(1);
	return (uint32)r;
}
uint32 handle_Breakpoint(registers_t *r){
	printf("%#40! Breakpoint !");
	asm volatile("hlt");
	while(1);
	return (uint32)r;
}
uint32 handle_PageFault(registers_t *r){
	printf("%#40! Page Fault !");
	uint32 addr;
	asm volatile("mov %%cr2, %0": "=r"(addr));
	printf("\nAddress: %p",addr);
	asm volatile("hlt");
	while(1);
	return (uint32)r;
}


void install_Basic_Interrupts(){
	registerInterrupt(0,systemFlags,handle_DIVbyZERO);
	
	
	registerInterrupt(3,systemFlags,handle_Breakpoint);
	
	
	registerInterrupt(6,systemFlags,handle_InvalidOpcode);
	
	registerInterrupt(8,systemFlags,handle_DoubleFault);
	
	
	
	
	registerInterrupt(13,systemFlags,handle_GeneralProtectionFault);
	registerInterrupt(14,systemFlags,handle_PageFault);
}























