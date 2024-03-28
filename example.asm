mov rs,0xfe00000
jmp start


clearScreen:
mov r0,rv0
mov r1,0xff00000
mov r2,180
mul r2,180
mul r2,4
add r2,r1

.loop:
jge r1,r2,.loopend
mov [r1],r0
add r1,4
jmp .loop
.loopend:

ret


programTest1:
mov r0,0
.loop:
mov rv0,r0
call clearScreen
draw
rdclk r0
mov r1,0x10
div r0,r1
jmp .loop


programTest2:
mov r10,850
mov r11,850
mov r12,100
mov r13,100
rdclk r20

call setInitialAudioNextSampleToWrite

.mainLoop:
mov r1,0xff00000
mov r2,180
mov r4,0

rdclk r21
mov r22,r21
sub r21,r20
mov r20,r22


rdbtn r0
mov r30,r0
shr r30,2
and r30,1
mov r31,r0
and r31,1
sub r30,r31
mul r30,r21
add r11,r30

mov r0,0
jges r11,r0,.skipCollisionA
mov r11,r0
.skipCollisionA:
mov r0,1800
sub r0,r13
jges r0,r11,.skipCollisionB
mov r11,r0
.skipCollisionB:

.loopy:
jge r4,r2,.loopyEnd

mov r3,0

.loopx:
jge r3,r2,.loopxEnd

mov r5,r2
mul r5,r4
add r5,r3
mul r5,4
add r5,r1

mov r0,r3
and r0,0xff
mov r6,r4
and r6,0xff
shl r0,8
or r0,r6
shl r0,8
or r0,0xff
mov [r5],r0

add r3,1
jmp .loopx
.loopxEnd:

add r4,1
jmp .loopy
.loopyEnd:

mov r40,r10
mov r41,r11
mov r42,r12
mov r43,r13
divs r40,10
divs r41,10
divs r42,10
divs r43,10
mov r9,0xffffff

mov r6,0
.loopy2:
jge r6,r43,.loopyEnd2

mov r5,0

.loopx2:
jge r5,r42,.loopxEnd2

mov r3,r5
add r3,r40
mov r4,r6
add r4,r41

mov r0,0
jgs r0,r3,.skipPixel
jgs r0,r4,.skipPixel
mov r0,r2
jges r3,r0,.skipPixel
jges r4,r0,.skipPixel

mov r7,r2
mul r7,r4
add r7,r3
mul r7,4
add r7,r1

mov [r7],r9

.skipPixel:

add r5,1
jmp .loopx2
.loopxEnd2:

add r6,1
jmp .loopy2
.loopyEnd2:

draw

call fillAudioSamples

jmp .mainLoop


start:
call programTest2


setInitialAudioNextSampleToWrite:
rdauds r1
add r1,4096
mod r1,240000
mov r0,audioNextSampleToWrite
mov [r0],r1
ret


fillAudioSamples:
rdauds r1
add r1,4096
mod r1,240000
mov r0,audioNextSampleToWrite
mov r0,[r0]

mov r2,r0

.loop:
je r2,r1,.loopEnd

mov r3,r2
mul r3,4
add r3,0xfe00000

mov r11,r2
mod r11,48000
div r11,48
mod r11,2

mul r11,0xffffffff
sub r11,0x80000000

divs r11,10

mov r10,r11

mov [r3],r10

add r2,1
mod r2,240000
jmp .loop
.loopEnd:

mov r0,audioNextSampleToWrite
mov [r0],r1

ret


audioNextSampleToWrite:
0
