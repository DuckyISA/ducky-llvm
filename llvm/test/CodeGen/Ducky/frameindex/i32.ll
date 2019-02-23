; RUN: llc < %s | FileCheck %s

define i32* @foo() {
  %addr = alloca i32
  ret i32* %addr
}

; CHECK-LABEL: foo
; CHECK: sub sp, 0x4
; CHECK: mov r0, sp
; CHECK: add sp, 0x4
; CHECK: ret
