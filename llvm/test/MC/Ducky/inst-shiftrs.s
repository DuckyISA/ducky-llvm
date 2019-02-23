; RUN: llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: not test -s %t

test:
  shiftrs r0, r1
  shiftrs r1, sp
  shiftrs fp, 0x79
  shiftrs r0, 0x7fff

; CHECK: shiftrs r0, r1 ; encoding: [0x28,0x08,0x00,0x00]
; CHECK: shiftrs r1, sp ; encoding: [0x68,0xf8,0x00,0x00]
; CHECK: shiftrs fp, 0x79 ; encoding: [0xa8,0x07,0xf3,0x00]
; CHECK: shiftrs r0, 0x7fff ; encoding: [0x28,0x00,0xff,0xff]
