.data
_g_pi: .float 3.141600
.text
.text
_start_sqr:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_sqr
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x14, [sp, #48]
str x15, [sp, #56]
str s16, [sp, #64]
str s17, [sp, #68]
str s18, [sp, #72]
str s19, [sp, #76]
str s20, [sp, #80]
str s21, [sp, #84]
str s22, [sp, #88]
str s23, [sp, #92]
ldr s16, [x29, #16]
ldr s17, [x29, #16]
fmul s16, s16, s17
fmov s0, s16
b _end_sqr
_end_sqr:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x14, [sp, #48]
ldr x15, [sp, #56]
ldr s16, [sp, #64]
ldr s17, [sp, #68]
ldr s18, [sp, #72]
ldr s19, [sp, #76]
ldr s20, [sp, #80]
ldr s21, [sp, #84]
ldr s22, [sp, #88]
ldr s23, [sp, #92]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_sqr: .word 92
.text
_start_calarea:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_calarea
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x14, [sp, #48]
str x15, [sp, #56]
str s16, [sp, #64]
str s17, [sp, #68]
str s18, [sp, #72]
str s19, [sp, #76]
str s20, [sp, #80]
str s21, [sp, #84]
str s22, [sp, #88]
str s23, [sp, #92]
ldr x14, =_g_pi
ldr s16, [x14, #0]
# caller save
sub sp, sp, 4
ldr w9, [x29, #16]
scvtf s17, w9
str s17, [sp, #8]
bl _start_sqr
# caller restore
add sp, sp, 4
fmov s17, s0
fmul s16, s16, s17
str s16, [x29, #-4]
ldr s16, [x29, #-4]
fmov s0, s16
b _end_calarea
_end_calarea:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x14, [sp, #48]
ldr x15, [sp, #56]
ldr s16, [sp, #64]
ldr s17, [sp, #68]
ldr s18, [sp, #72]
ldr s19, [sp, #76]
ldr s20, [sp, #80]
ldr s21, [sp, #84]
ldr s22, [sp, #88]
ldr s23, [sp, #92]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_calarea: .word 92
.text
_start_floor:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_floor
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x14, [sp, #48]
str x15, [sp, #56]
str s16, [sp, #64]
str s17, [sp, #68]
str s18, [sp, #72]
str s19, [sp, #76]
str s20, [sp, #80]
str s21, [sp, #84]
str s22, [sp, #88]
str s23, [sp, #92]
ldr s16, [x29, #16]
fcvtzs w9, s16
str w9, [x29, #-8]
ldr w9, [x29, #-8]
mov w0, w9
b _end_floor
_end_floor:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x14, [sp, #48]
ldr x15, [sp, #56]
ldr s16, [sp, #64]
ldr s17, [sp, #68]
ldr s18, [sp, #72]
ldr s19, [sp, #76]
ldr s20, [sp, #80]
ldr s21, [sp, #84]
ldr s22, [sp, #88]
ldr s23, [sp, #92]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_floor: .word 100
.text
_start_MAIN:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_MAIN
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x14, [sp, #48]
str x15, [sp, #56]
str s16, [sp, #64]
str s17, [sp, #68]
str s18, [sp, #72]
str s19, [sp, #76]
str s20, [sp, #80]
str s21, [sp, #84]
str s22, [sp, #88]
str s23, [sp, #92]
.data
_CONSTANT_1: .ascii "Enter an Integer :\000"
.align 3
.text
ldr x9, =_CONSTANT_1
mov x0, x9
bl _write_str
bl _read_int
mov w9, w0
str w9, [x29, #-8]
# caller save
sub sp, sp, 8
ldr w9, [x29, #-8]
str w9, [sp, #8]
bl _start_calarea
# caller restore
add sp, sp, 8
fmov s16, s0
str s16, [x29, #-12]
ldr s16, [x29, #-12]
# caller save
sub sp, sp, 4
ldr s17, [x29, #-12]
str s17, [sp, #8]
bl _start_floor
# caller restore
add sp, sp, 4
mov w9, w0
scvtf s17, w9
fsub s16, s16, s17
str s16, [x29, #-16]
ldr s16, [x29, #-12]
fmov s0, s16
bl _write_float
.data
_CONSTANT_2: .ascii " \000"
.align 3
.text
ldr x9, =_CONSTANT_2
mov x0, x9
bl _write_str
ldr s16, [x29, #-16]
fmov s0, s16
bl _write_float
.data
_CONSTANT_3: .ascii "\n\000"
.align 3
.text
ldr x9, =_CONSTANT_3
mov x0, x9
bl _write_str
.data
_CONSTANT_4: .word 0
.align 3
.text
ldr w9, _CONSTANT_4
mov w0, w9
b _end_MAIN
_end_MAIN:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x14, [sp, #48]
ldr x15, [sp, #56]
ldr s16, [sp, #64]
ldr s17, [sp, #68]
ldr s18, [sp, #72]
ldr s19, [sp, #76]
ldr s20, [sp, #80]
ldr s21, [sp, #84]
ldr s22, [sp, #88]
ldr s23, [sp, #92]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_MAIN: .word 108
