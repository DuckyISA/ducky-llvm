; RUN: not llvm-mc -triple ducky < %s 2> %t
; RUN: FileCheck --check-prefix=CHECK-ERROR %s < %t

test:
  lw r2[0x00], r3

; CHECK-ERROR: <stdin>:5:8: error: unexpected token
; CHECK-ERROR:   lw r2[0x00], r3
