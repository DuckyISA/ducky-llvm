; RUN: not llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: FileCheck --check-prefix=CHECK-ERROR < %t %s

foo:
  hlt r1
  hlt sp
  hlt fp
  hlt 0x79
  hlt 0xfffff
  hlt 0x1fffff
  hlt 0xdeadbeef

; CHECK: hlt r1 ; encoding: [0x54,0x00,0x00,0x00]
; CHECK: hlt sp ; encoding: [0xd4,0x07,0x00,0x00]
; CHECK: hlt fp ; encoding: [0x94,0x07,0x00,0x00]
; CHECK: hlt 0x79 ; encoding: [0x14,0x98,0x07,0x00]
; CHECK: hlt 0xfffff ; encoding: [0x14,0xf8,0xff,0xff]

; CHECK-ERROR: <stdin>:10:7: error: immediate must fit into 20 bits
; CHECK-ERROR:   hlt 0x1fffff

; CHECK-ERROR: <stdin>:11:7: error: immediate must fit into 20 bits
; CHECK-ERROR:   hlt 0xdeadbeef
