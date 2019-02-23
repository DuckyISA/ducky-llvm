; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  lw r2, r3[0xffff]

; CHECK-ERROR: <stdin>:5:3: error: invalid operand
; CHECK-ERROR:   lw r2, r3[0xffff]
