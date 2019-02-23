; RUN: llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: not test -s %t

test:
  shiftl r0, r1
  shiftl r1, sp
  shiftl fp, 0x79
  shiftl r0, 0x7fff

; CHECK: shiftl r0, r1 ; encoding: [0x26,0x08,0x00,0x00]
; CHECK: shiftl r1, sp ; encoding: [0x66,0xf8,0x00,0x00]
; CHECK: shiftl fp, 0x79 ; encoding: [0xa6,0x07,0xf3,0x00]
; CHECK: shiftl r0, 0x7fff ; encoding: [0x26,0x00,0xff,0xff]
