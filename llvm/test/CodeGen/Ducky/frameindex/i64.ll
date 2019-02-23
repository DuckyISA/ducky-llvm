; RUN: llc < %s | FileCheck %s

define i64* @foo() {
  %addr = alloca i64
  ret i64* %addr
}

; CHECK-LABEL: foo
; CHECK: sub sp, 0x8
; CHECK: mov r0, sp
; CHECK: add sp, 0x8
; CHECK: ret
