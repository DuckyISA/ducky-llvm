; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  or 0x79

; CHECK-ERROR: <stdin>:5:3: error: invalid operand
; CHECK-ERROR:   or 0x79
