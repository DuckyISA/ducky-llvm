; RUN: llc -mtriple=ducky < %s | FileCheck %s

; CHECK-LABEL: test_asm:
; CHECK: APP
; CHECK: mov r0, r1
; CHECK: NO_APP

define void @test_asm() {
  call void asm sideeffect "mov r0, r1", ""()
  ret void
}
