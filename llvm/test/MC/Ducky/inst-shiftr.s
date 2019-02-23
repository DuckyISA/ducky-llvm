; RUN: llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: not test -s %t

test:
  shiftr r0, r1
  shiftr r1, sp
  shiftr fp, 0x79
  shiftr r0, 0x7fff

; CHECK: shiftr r0, r1 ; encoding: [0x27,0x08,0x00,0x00]
; CHECK: shiftr r1, sp ; encoding: [0x67,0xf8,0x00,0x00]
; CHECK: shiftr fp, 0x79 ; encoding: [0xa7,0x07,0xf3,0x00]
; CHECK: shiftr r0, 0x7fff ; encoding: [0x27,0x00,0xff,0xff]
