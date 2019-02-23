; RUN: not llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: FileCheck --check-prefix=CHECK-ERROR < %t %s

foo:
  cmp r0, r0
  cmp r1, fp
  cmp sp, r29
  cmp r1, 0x79
  cmp r1, 0x7fff
  cmp r1, 0xffff
  cmp r1, 0xdeadbeef

; CHECK: cmp r0, r0 ; encoding: [0x2f,0x00,0x00,0x00]
; CHECK: cmp r1, fp ; encoding: [0x6f,0xf0,0x00,0x00]
; CHECK: cmp sp, r29 ; encoding: [0xef,0xef,0x00,0x00]
; CHECK: cmp r1, 0x79 ; encoding: [0x6f,0x00,0xf3,0x00]
; CHECK: cmp r1, 0x7fff ; encoding: [0x6f,0x00,0xff,0xff]

; CHECK-ERROR: <stdin>:10:11: error: immediate must fit into 15 bits
; CHECK-ERROR:   cmp r1, 0xffff

; CHECK-ERROR: <stdin>:11:11: error: immediate must fit into 15 bits
; CHECK-ERROR:   cmp r1, 0xdeadbeef
