; RUN: not llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: FileCheck --check-prefix=CHECK-ERROR < %t %s

foo:
  shiftl r0, r1
  shiftl r1, sp
  shiftl fp, 0x79
  shiftl r0, 0x7fff
  shiftl r0, 0xffff
  shiftl r0, 0xdeadbeef

; CHECK: shiftl r0, r1 ; encoding: [0x26,0x08,0x00,0x00]
; CHECK: shiftl r1, sp ; encoding: [0x66,0xf8,0x00,0x00]
; CHECK: shiftl fp, 0x79 ; encoding: [0xa6,0x07,0xf3,0x00]
; CHECK: shiftl r0, 0x7fff ; encoding: [0x26,0x00,0xff,0xff]

; CHECK-ERROR: <stdin>:9:14: error: immediate must fit into 15 bits
; CHECK-ERROR:   shiftl r0, 0xffff

; CHECK-ERROR: <stdin>:10:14: error: immediate must fit into 15 bits
; CHECK-ERROR:   shiftl r0, 0xdeadbeef
