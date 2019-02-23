; RUN: llc < %s | FileCheck %s

define i8* @foo() {
  %addr = alloca i8
  ret i8* %addr
}

; CHECK-LABEL: foo
; CHECK: sub sp, 0x4
; CHECK: mov r0, sp
; CHECK: add sp, 0x4
; CHECK: ret
