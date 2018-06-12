@Assembler assembling this must allow comments. Otherwise remove any line starting with @

@Store address 0x2020 0004 (ldr is used for huge numbers, else use mov)
ldr r0, =0x20200004

@Store 0x2020 0028
ldr r1, =0x20200028

@Store 0x2020 001C
ldr r2, =0x2020001C

@Pin 16 location for 0x2020 0004
ldr r3, =0x40000

@Pin 16 location for set and clear addresses
ldr r4, =0x10000

@Loop value- determines how long LED stays on or off
ldr r8, =0x1000000

mov r5, #0

@Set pin 16 at 0x2020 004
str r3, [r0]

flash:
@Clear pin at 0x2020 0028
str r4, [r1]

str r5, [r2]

mov r6, #0
keepOn:
add r6, r6, #1
cmp r6, r8
bne keepOn

str r4, [r2]

mov r6, #0
keepOff:
add r6, r6, #1
cmp r6, r8
bne keepOff

b flash


andeq r0, r0, r0
