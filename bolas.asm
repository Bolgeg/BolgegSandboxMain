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

screenCirclefill:
mov r0,0
mov r10,rv0
mov r11,rv1
mov r12,rv2
mov r13,rv3
mov r20,r11
mov r21,r11
sub r20,r13
add r21,r13
mov r22,r12
mov r23,r12
sub r22,r13
add r23,r13
mov rv1,r20
mov rv2,r21
call screenRangeWrap
je rv0,r0,.return
mov r20,rv1
mov r21,rv2
mov rv1,r22
mov rv2,r23
call screenRangeWrap
je rv0,r0,.return
mov r22,rv1
mov r23,rv2
add r21,1
add r23,1
mov r24,r13
mul r24,r24
mov r31,r22
.loopy:
mov r30,r20
.loopx:
mov r32,r30
mov r33,r31
sub r32,r11
sub r33,r12
mul r32,r32
mul r33,r33
add r32,r33
jg r32,r24,.skipPixel
mov r40,r31
mul r40,180
add r40,r30
mul r40,4
add r40,0xff00000
mov [r40],r10
.skipPixel:
add r30,1
jne r30,r21,.loopx
add r31,1
jne r31,r23,.loopy
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


srand:
mov r0,randNext
mov [r0],rv0
ret

rand0x7fff:
mov r0,randNext
mov r1,[r0]
mul r1,1103515245
add r1,12345
mov [r0],r1
shr r1,16
and r1,0x7fff
mov rv0,r1
ret

rand:
call rand0x7fff
mov r0,rv0
shl r0,15
call rand0x7fff
or r0,rv0
shl r0,15
call rand0x7fff
or r0,rv0
mov rv0,r0
ret


seedSrand:
mov r0,0
mov r1,1000
.loop:
mov r2,555
mul r2,333
div r2,111
add r0,1
jne r0,r1,.loop
rdclk r0
mul r0,123456
rdauds r1
mul r1,654321
add r0,r1
mov rv0,r0
call srand
ret


updateGameTime:
mov r0,previousFrameClk
rdclk r1
mov r2,r1
sub r1,[r0]
mov [r0],r2
mov r0,gameDeltaTime
mov [r0],r1
mov r0,gameTime
add [r0],r1
mov r1,[r0]
add r1,60000
div r1,60
mov r0,gameSpeed
mov [r0],r1
ret


drawSquare:
mov r0,squarePositionX
mov r1,[r0]
mov r0,squarePositionY
mov r3,[r0]
mov r0,squareSize
mov r5,[r0]
mov r2,r1
add r2,r5
mov r4,r3
add r4,r5
divs r1,1000
divs r2,1000
divs r3,1000
divs r4,1000
add r3,20
add r4,20
mov r0,squareColor
mov rv0,[r0]
mov rv1,r1
mov rv2,r2
sub rv2,1
mov rv3,r3
mov rv4,r4
sub rv4,1
call screenRectfill
ret


controlSquare:
mov r0,0
mov r1,160000
mov r10,squareSize
sub r1,[r10]
rdbtn r2
mov r3,r2
shr r3,2
and r3,1
mov r4,r3
mov r3,r2
and r3,1
sub r4,r3
mov r10,squareSpeed
mul r4,[r10]
mov r10,gameSpeed
mul r4,[r10]
divs r4,1000
mov r10,gameDeltaTime
mul r4,[r10]
divs r4,1000
mov r10,squarePositionY
mov r5,[r10]
add r5,r4
jges r5,r0,.skip1
mov r5,r0
.skip1:
jges r1,r5,.skip2
mov r5,r1
.skip2:
mov [r10],r5
ret


updateSquareColor:
mov r0,0
mov r10,squareColor
mov [r10],0xff0000
mov r1,squareTimeRed
mov r1,[r1]
mov r2,gameDeltaTime
mov r2,[r2]
sub r1,r2
jgs r1,r0,.skipBlue
mov r1,r0
mov [r10],0x0000ff
.skipBlue:
mov r2,squareTimeRed
mov [r2],r1
ret


getNumberOfBalls:
mov r0,0xfc00000
mov rv0,[r0]
ret

deleteBalls:
mov r0,0xfc00000
mov [r0],0
ret

addBall:
mov r0,0xfc00000
mov r1,[r0]
add [r0],1
mov r0,0xfd00000
mul r1,12
add r0,r1
mov [r0],0
add r0,4
mov [r0],0
add r0,4
mov [r0],0
ret

getBall:
mov r0,0xfd00000
mov r1,rv0
mul r1,12
add r0,r1
mov rv1,[r0]
add r0,4
mov rv2,[r0]
add r0,4
mov rv3,[r0]
ret

setBall:
mov r0,0xfd00000
mov r1,rv0
mul r1,12
add r0,r1
mov [r0],rv1
add r0,4
mov [r0],rv2
add r0,4
mov [r0],rv3
ret

addBalls:
mov r1,rv0
mov r2,0
.loop:
call addBall
add r2,1
jne r2,r1,.loop
ret

getNewRandomBall:
call rand
mov r1,rv0
and r1,0x3fffff
add r1,200000
call rand
mov r2,rv0
mov r10,0
mov r11,160000
mov r12,ballRadius
mov r12,[r12]
add r10,r12
sub r11,r12
mov r13,r11
sub r13,r10
mod r2,r13
add r2,r10
call rand
mov r10,rv0
mod r10,100
mov r11,95
jg r11,r10,.skipGreen
mov r3,2
jmp .colorSelected
.skipGreen:
mov r11,85
jg r11,r10,.skipRed
mov r3,0
jmp .colorSelected
.skipRed:
mov r3,1
.colorSelected:
mov rv1,r1
mov rv2,r2
mov rv3,r3
ret


updateBall:
mov r0,rv0
call getBall
mov r1,rv1
mov r2,rv2
mov r3,rv3
mov r10,ballSpeed
mov r10,[r10]
mov r11,gameSpeed
mov r11,[r11]
mul r10,r11
divs r10,1000
mov r11,gameDeltaTime
mov r11,[r11]
mul r10,r11
divs r10,1000
sub r1,r10
mov r20,-50000
jgs r1,r20,.skip
call getNewRandomBall
mov r1,rv1
mov r2,rv2
mov r3,rv3
.skip:
mov rv0,r0
mov rv1,r1
mov rv2,r2
mov rv3,r3
call setBall
ret

updateBalls:
call getNumberOfBalls
mov r1,rv0
mov r0,0
.loop:
mov rv0,r0
call updateBall
add r0,1
jne r0,r1,.loop
ret


drawBall:
call getBall
mov r1,rv1
mov r2,rv2
mov r3,rv3
divs r1,1000
divs r2,1000
add r2,20
mov r4,ballRadius
mov r4,[r4]
divs r4,1000
mov r0,0xff0000
mov r10,1
jne r3,r10,.skipBlue
mov r0,0x0000ff
jmp .colorSet
.skipBlue:
mov r10,2
jne r3,r10,.skipGreen
mov r0,0x00ff00
jmp .colorSet
.skipGreen:
.colorSet:
mov rv0,r0
mov rv1,r1
mov rv2,r2
mov rv3,r4
call screenCirclefill
ret

drawBalls:
call getNumberOfBalls
mov r1,rv0
mov r0,0
.loop:
mov rv0,r0
call drawBall
add r0,1
jne r0,r1,.loop
ret


initializeBalls:
call deleteBalls
mov r1,100
mov rv0,r1
call addBalls
mov r0,0
.loop:
call getNewRandomBall
mov rv0,r0
call setBall
add r0,1
jne r0,r1,.loop
ret


updateBallCollisionsGetIfInRadius:
mov r11,rv1
mov r12,rv2
mov r13,rv3
mov r14,rv4
mov r15,rv5
sub r11,r13
sub r12,r14
divs r11,1000
divs r12,1000
mul r11,r11
mul r12,r12
add r11,r12
divs r15,1000
mul r15,r15
jge r11,r15,.notInRadius
mov rv0,1
ret
.notInRadius:
mov rv0,0
ret

updateBallCollisions:
mov r0,rv0
call getBall
mov r1,rv1
mov r2,rv2
mov r3,rv3
mov r10,squarePositionX
mov r10,[r10]
mov r11,squarePositionY
mov r11,[r11]
sub r1,r10
sub r2,r11
mov r20,0
mov r21,squareSize
mov r21,[r21]
mov r30,ballRadius
mov r30,[r30]
mov r22,r20
sub r22,r30
mov r23,r21
add r23,r30
jgs r22,r1,.noCollision
jgs r1,r23,.noCollision
jgs r22,r2,.noCollision
jgs r2,r23,.noCollision
mov r50,0
jgs r20,r1,.skip1
jgs r1,r21,.skip1
jgs r22,r2,.skip1
jgs r2,r23,.skip1
mov r50,1
jmp .checkEnd
.skip1:
jgs r22,r1,.skip2
jgs r1,r23,.skip2
jgs r20,r2,.skip2
jgs r2,r21,.skip2
mov r50,1
jmp .checkEnd
.skip2:
mov rv1,0
mov rv2,0
mov rv3,r1
mov rv4,r2
mov rv5,r30
call updateBallCollisionsGetIfInRadius
mov rv1,r21
mov rv2,0
mov rv3,r1
mov rv4,r2
mov rv5,r30
call updateBallCollisionsGetIfInRadius
mov rv1,0
mov rv2,r21
mov rv3,r1
mov rv4,r2
mov rv5,r30
call updateBallCollisionsGetIfInRadius
mov rv1,r21
mov rv2,r21
mov rv3,r1
mov rv4,r2
mov rv5,r30
call updateBallCollisionsGetIfInRadius
.checkEnd:
mov r51,0
je r50,r51,.noCollision
mov r10,r3
call getNewRandomBall
mov rv0,r0
call setBall
mov rv0,r10
call squareBallCollisionEffect
.noCollision:
ret

updateCollisions:
call getNumberOfBalls
mov r1,rv0
mov r0,0
.loop:
mov rv0,r0
call updateBallCollisions
add r0,1
jne r0,r1,.loop
ret

squareBallCollisionEffectRed:
mov r0,lives
sub [r0],1
mov r0,squareTimeRed
mov [r0],300
ret

squareBallCollisionEffectGreen:
mov r0,lives
add [r0],1
mov r0,points
add [r0],10
ret

squareBallCollisionEffectBlue:
mov r0,points
add [r0],1
ret

squareBallCollisionEffect:
mov r0,rv0
mov r1,0
jne r0,r1,.skipRed
call squareBallCollisionEffectRed
jmp .done
.skipRed:
mov r1,1
jne r0,r1,.skipBlue
call squareBallCollisionEffectBlue
jmp .done
.skipBlue:
call squareBallCollisionEffectGreen
.done:
ret


updateGameState:
mov r0,points
mov r0,[r0]
mov r1,999999
jge r1,r0,.skip1
mov r0,r1
mov r1,points
mov [r1],r0
.skip1:
mov r5,lives
mov r2,[r5]
mov r0,0
mov r1,999
jgs r1,r2,.skip2
mov r2,r1
mov [r5],r2
.skip2:
jges r2,r0,.skipLoose
mov [r5],r0
mov r0,gameOver
mov [r0],1
.skipLoose:
ret


start:

call seedSrand

call initializeGame

jmp mainLoop


initializeGame:
mov r0,lives
mov [r0],10
mov r0,points
mov [r0],0

mov r0,squarePositionX
mov [r0],10000
mov r0,squarePositionY
mov [r0],70000
mov r0,squareSize
mov [r0],20000
mov r0,squareColor
mov [r0],0x0000ff
mov r0,squareTimeRed
mov [r0],0
mov r0,squareSpeed
mov [r0],80000

mov r0,ballRadius
mov [r0],5000
mov r0,ballSpeed
mov [r0],80000

mov r0,gameDeltaTime
mov [r0],0
mov r0,gameTime
mov [r0],0
mov r0,gameSpeed
mov [r0],1000
mov r0,gameOver
mov [r0],0

call initializeBalls

mov r0,previousFrameClk
rdclk r1
mov [r0],r1
ret


gameOverLoop:
mov r0,0
rdbtn r1
shr r1,9
and r1,1
je r1,r0,.skipRestart
jmp restart
.skipRestart:
jmp gameOverLoop

restart:
call initializeGame
jmp mainLoop


mainLoop:

mov r0,0
mov r1,gameOver
mov r1,[r1]
je r1,r0,.skipGameOver
jmp gameOverLoop
.skipGameOver:


call updateGameTime

call controlSquare

call updateBalls

call updateCollisions

call updateSquareColor

call updateGameState


mov rv0,0x000000
call screenClear

call drawBalls

call drawSquare

mov rv0,0x0000ff
mov rv1,0
mov rv2,179
mov rv3,0
mov rv4,18
call screenRectfill

mov rv0,0x00ff00
mov rv1,8
mov rv2,6
mov r0,lives
mov rv3,[r0]
mov rv4,3
call screenPrintNumberDigits

mov rv0,0x00ff00
mov rv1,32
mov rv2,6
mov rv3,symbolHeart
call screenPrint8x8Symbol

mov rv0,0xffffff
mov rv1,124
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


randNext:
0

lives:
0
points:
0

squarePositionX:
0
squarePositionY:
0
squareSize:
0
squareColor:
0x000000
squareTimeRed:
0
squareSpeed:
0

ballRadius:
0
ballSpeed:
0

previousFrameClk:
0
gameDeltaTime:
0
gameTime:
0
gameSpeed:
0
gameOver:
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


symbolHeart:
0 1 0 0 0 1 0 0
1 1 1 0 1 1 1 0
1 1 1 1 1 1 1 0
1 1 1 1 1 1 1 0
0 1 1 1 1 1 0 0
0 0 1 1 1 0 0 0
0 0 0 1 0 0 0 0
0 0 0 0 0 0 0 0
