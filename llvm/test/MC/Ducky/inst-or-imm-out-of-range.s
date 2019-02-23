; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  or r0, 0xffff

; CHECK-ERROR: <stdin>:5:10: error: immediate must fit into 15 bits
; CHECK-ERROR:   or r0, 0xffff
