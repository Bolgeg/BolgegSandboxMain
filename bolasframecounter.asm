mov rs,0xfe00000
jmp start


screenClear:
mov r1,0xff00000
mov r2,180
mul r2,180
mul r2,4
add r2,r1
.loop:
mov [r1],rv0
add r1,4
jne r1,r2,.loop
ret

screenInside:
mov r0,0
mov r1,179
jgs r0,rv1,.returnFalse
jgs rv1,r1,.returnFalse
mov rv0,1
ret
.returnFalse:
mov rv0,0
ret

screenRangeWrap:
mov r0,0
mov r1,179
jgs rv1,rv2,.returnNone
jgs rv1,r1,.returnNone
jgs r0,rv2,.returnNone
jgs rv1,r0,.skip1
mov rv1,r0
.skip1:
jgs r1,rv2,.skip2
mov rv2,r1
.skip2:
mov rv0,1
ret
.returnNone:
mov rv0,0
ret

screenHlineNoTest:
mov r1,rv3
mul r1,180
mul r1,4
add r1,0xff00000
mov r2,rv2
add r2,1
mul r2,4
add r2,r1
mov r3,rv1
mul r3,4
add r1,r3
.loop:
mov [r1],rv0
add r1,4
jne r1,r2,.loop
ret

screenHline:
mov r0,0
mov r10,rv0
mov r11,rv1
mov r12,rv2
mov r13,rv3
mov rv1,r13
call screenInside
je rv0,r0,.return
mov rv1,r11
mov rv2,r12
call screenRangeWrap
je rv0,r0,.return
mov rv0,r10
mov rv3,r13
call screenHlineNoTest
.return:
ret

screenRectfillNoTest:
mov r1,rv1
mul r1,4
mov r2,rv2
add r2,1
mul r2,4
mov r3,rv3
mul r3,180
mul r3,4
add r3,0xff00000
mov r4,rv4
add r4,1
mul r4,180
mul r4,4
add r4,0xff00000
mov r6,r3
.loopy:
mov r5,r1
mov r7,r6
add r7,r1
.loopx:
mov [r7],rv0
add r7,4
add r5,4
jne r5,r2,.loopx
add r6,720
jne r6,r4,.loopy
ret

screenRectfill:
mov r0,0
mov r10,rv0
mov r11,rv1
mov r12,rv2
mov r13,rv3
mov r14,rv4
mov rv1,r11
mov rv2,r12
call screenRangeWrap
je rv0,r0,.return
mov r11,rv1
mov r12,rv2
mov rv1,r13
mov rv2,r14
call screenRangeWrap
je rv0,r0,.return
mov r13,rv1
mov r14,rv2
mov rv0,r10
mov rv1,r11
mov rv2,r12
mov rv3,r13
mov rv4,r14
call screenRectfillNoTest
.return:
ret

screenPrint8x8Symbol:
mov r0,0
mov r1,179
mov r10,rv0
mov r11,rv1
mov r12,rv2
mov r13,rv3
mov r8,8
mov r22,0
.loopy:
mov r21,0
.loopx:
mov r31,r21
mov r32,r22
add r31,r11
add r32,r12
jgs r0,r31,.skipPixel
jgs r31,r1,.skipPixel
jgs r0,r32,.skipPixel
jgs r32,r1,.skipPixel
mov r40,r22
mul r40,8
add r40,r21
mul r40,4
add r40,r13
mov r40,[r40]
je r40,r0,.skipPixel
mov r41,r32
mul r41,180
add r41,r31
mul r41,4
add r41,0xff00000
mov [r41],r10
.skipPixel:
add r21,1
jne r21,r8,.loopx
add r22,1
jne r22,r8,.loopy
ret

screenPrintNumberDigit:
mul rv3,256
add rv3,digits
call screenPrint8x8Symbol
ret

screenPrintNumberDigits:
mov r0,0
mov r10,rv0
mov r11,rv1
mov r12,rv2
mov r13,rv3
mov r14,rv4
mov r1,r14
mul r1,8
add r11,r1
.loop:
sub r11,8
sub r14,1
mov r1,r13
mod r1,10
div r13,10
mov rv0,r10
mov rv1,r11
mov rv2,r12
mov rv3,r1
call screenPrintNumberDigit
jne r14,r0,.loop
ret




start:



jmp mainLoop


mainLoop:

mov r0,points
add [r0],1

mov rv0,0x000000
call screenClear

mov rv0,0x0000ff
mov rv1,0
mov rv2,179
mov rv3,0
mov rv4,18
call screenRectfill

mov rv0,0xffffff
mov rv1,8
mov rv2,6
mov r0,points
mov rv3,[r0]
mov rv4,6
call screenPrintNumberDigits

mov rv0,0xffffff
mov rv1,0
mov rv2,179
mov rv3,19
call screenHline

draw

jmp mainLoop


points:
0


digits:
0 0 0 0 0 0 0 0
0 0 1 1 1 0 0 0
0 1 0 0 0 1 0 0
0 1 0 0 0 1 0 0
0 1 0 0 0 1 0 0
0 1 0 0 0 1 0 0
0 0 1 1 1 0 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 0 0 1 1 0 0 0
0 0 1 0 1 0 0 0
0 1 0 0 1 0 0 0
0 0 0 0 1 0 0 0
0 0 0 0 1 0 0 0
0 0 0 0 1 0 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 0 1 1 1 0 0 0
0 1 0 0 0 1 0 0
0 0 0 0 0 1 0 0
0 0 1 1 1 0 0 0
0 1 0 0 0 0 0 0
0 1 1 1 1 1 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 0 1 1 1 0 0 0
0 1 0 0 0 1 0 0
0 0 0 1 1 0 0 0
0 0 0 0 0 1 0 0
0 1 0 0 0 1 0 0
0 0 1 1 1 0 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 0 0 1 1 0 0 0
0 0 1 0 1 0 0 0
0 1 0 0 1 0 0 0
0 1 1 1 1 1 0 0
0 0 0 0 1 0 0 0
0 0 0 0 1 0 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 1 1 1 1 1 0 0
0 1 0 0 0 0 0 0
0 1 1 1 1 0 0 0
0 0 0 0 0 1 0 0
0 1 0 0 0 1 0 0
0 0 1 1 1 0 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 0 1 1 1 0 0 0
0 1 0 0 0 0 0 0
0 1 1 1 1 0 0 0
0 1 0 0 0 1 0 0
0 1 0 0 0 1 0 0
0 0 1 1 1 0 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 1 1 1 1 1 0 0
0 0 0 0 0 1 0 0
0 0 0 0 0 1 0 0
0 0 0 0 1 0 0 0
0 0 0 1 0 0 0 0
0 0 1 0 0 0 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 0 1 1 1 0 0 0
0 1 0 0 0 1 0 0
0 0 1 1 1 0 0 0
0 1 0 0 0 1 0 0
0 1 0 0 0 1 0 0
0 0 1 1 1 0 0 0
0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0
0 0 1 1 1 0 0 0
0 1 0 0 0 1 0 0
0 0 1 1 1 1 0 0
0 0 0 0 0 1 0 0
0 0 0 0 1 0 0 0
0 0 1 1 0 0 0 0
0 0 0 0 0 0 0 0

