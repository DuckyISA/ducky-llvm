; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  shiftrs r0, 0xffff

; CHECK-ERROR: <stdin>:5:15: error: immediate must fit into 15 bits
; CHECK-ERROR:   shiftrs r0, 0xffff
