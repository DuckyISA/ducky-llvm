; RUN: llc < %s | FileCheck %s

define i16* @foo() {
  %addr = alloca i16
  ret i16* %addr
}

; CHECK-LABEL: foo
; CHECK: sub sp, 0x4
; CHECK: mov r0, sp
; CHECK: add sp, 0x4
; CHECK: ret
