; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  or r1

; CHECK-ERROR: <stdin>:5:3: error: invalid operand
; CHECK-ERROR:   or r1
