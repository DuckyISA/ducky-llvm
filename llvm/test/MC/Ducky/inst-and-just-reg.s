; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  and r1

; CHECK-ERROR: <stdin>:5:3: error: invalid operand
; CHECK-ERROR:   and r1
