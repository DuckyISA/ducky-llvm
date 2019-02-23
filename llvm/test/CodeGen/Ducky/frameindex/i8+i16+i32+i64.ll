; RUN: llc < %s | FileCheck %s

define i8* @foo() {
  %addr1 = alloca i8
  %addr2 = alloca i16
  %addr3 = alloca i32
  %addr4 = alloca i64
  ret i8* %addr1
}

; CHECK-LABEL: foo
; CHECK: sub sp, 0x10
; CHECK: mov r0, sp
; CHECK: add sp, 0x10
; CHECK: ret
