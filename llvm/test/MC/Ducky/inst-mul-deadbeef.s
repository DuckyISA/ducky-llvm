; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  mul r0, 0xdeadbeef

; CHECK-ERROR: <stdin>:5:11: error: immediate must fit into 15 bits
; CHECK-ERROR:   mul r0, 0xdeadbeef
