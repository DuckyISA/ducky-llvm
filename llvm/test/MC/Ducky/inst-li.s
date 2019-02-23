; RUN: not llvm-mc -triple ducky -show-encoding < %s 2> %t | FileCheck %s
; RUN: FileCheck --check-prefix=CHECK-ERROR < %t %s

foo:
  li r0, 0x00
  li fp, 0x79
  li sp, 0xfffff
  li r0, 0x1fffff
  li r0, 0xdeadbeef

; CHECK: li r0, 0x0                     ; encoding: [0x09,0x08,0x00,0x00]
; CHECK: li fp, 0x79                    ; encoding: [0x89,0x9f,0x07,0x00]
; CHECK: li sp, 0xfffff                 ; encoding: [0xc9,0xff,0xff,0xff]

; CHECK-ERROR: <stdin>:8:10: error: immediate must fit into 20 bits
; CHECK-ERROR:   li r0, 0x1fffff

; CHECK-ERROR: <stdin>:9:10: error: immediate must fit into 20 bits
; CHECK-ERROR:   li r0, 0xdeadbeef
