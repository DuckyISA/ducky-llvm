; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  udiv r0, 0xdeadbeef

; CHECK-ERROR: <stdin>:5:12: error: immediate must fit into 15 bits
; CHECK-ERROR:   udiv r0, 0xdeadbeef
