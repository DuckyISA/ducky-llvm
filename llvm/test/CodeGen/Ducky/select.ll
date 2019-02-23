; RUN: llc -relocation-model=static < %s | FileCheck %s

; Function Attrs: nounwind
define i32 @test1(i32 %i) #0 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %cmp = icmp eq i32 %0, 0
  %cond = select i1 %cmp, i32 11, i32 12
  ret i32 %cond
; CHECK-LABEL: entry
; CHECK: sub sp, 0x4
; CHECK: stw sp, r0
; CHECK: li r1, 0xb
; CHECK: cmp r0, 0x0
; CHECK: sele r1, 0xc
; CHECK: mov r0, r1
; CHECK: add sp, 0x4
; CHECK: ret
}
