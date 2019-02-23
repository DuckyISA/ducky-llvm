; RUN: not llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: FileCheck --check-prefix=CHECK-ERROR < %t %s

foo:
  cmpu r0, r0
  cmpu r1, fp
  cmpu sp, r29
  cmpu r1, 0x79
  cmpu r1, 0x7fff
  cmpu r1, 0xffff
  cmpu r1, 0xdeadbeef

; CHECK: cmpu r0, r0 ; encoding: [0x30,0x00,0x00,0x00]
; CHECK: cmpu r1, fp ; encoding: [0x70,0xf0,0x00,0x00]
; CHECK: cmpu sp, r29 ; encoding: [0xf0,0xef,0x00,0x00]
; CHECK: cmpu r1, 0x79 ; encoding: [0x70,0x00,0xf3,0x00]
; CHECK: cmpu r1, 0x7fff ; encoding: [0x70,0x00,0xff,0xff]

; CHECK-ERROR: <stdin>:10:12: error: immediate must fit into 15 bits
; CHECK-ERROR:   cmpu r1, 0xffff

; CHECK-ERROR: <stdin>:11:12: error: immediate must fit into 15 bits
; CHECK-ERROR:   cmpu r1, 0xdeadbeef
