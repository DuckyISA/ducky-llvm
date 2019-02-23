; RUN: not llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: FileCheck --check-prefix=CHECK-ERROR < %t %s

foo:
  add r0, r1
  add r1, sp
  add fp, 0x79
  add r0, 0x7fff
  add r0, 0xffff
  add r0, 0xdeadbeef

; CHECK: add r0, r1 ; encoding: [0x1c,0x08,0x00,0x00]
; CHECK: add r1, sp ; encoding: [0x5c,0xf8,0x00,0x00]
; CHECK: add fp, 0x79 ; encoding: [0x9c,0x07,0xf3,0x00]
; CHECK: add r0, 0x7fff ; encoding: [0x1c,0x00,0xff,0xff]

; CHECK-ERROR: <stdin>:9:11: error: immediate must fit into 15 bits
; CHECK-ERROR:   add r0, 0xffff
; CHECK-ERROR:   ^

; CHECK-ERROR: <stdin>:10:11: error: immediate must fit into 15 bits
; CHECK-ERROR:   add r0, 0xdeadbeef
; CHECK-ERROR:   ^
