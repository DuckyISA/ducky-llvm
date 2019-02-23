; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  shiftr r0, 0xdeadbeef

; CHECK-ERROR: <stdin>:5:14: error: immediate must fit into 15 bits
; CHECK-ERROR:   shiftr r0, 0xdeadbeef
