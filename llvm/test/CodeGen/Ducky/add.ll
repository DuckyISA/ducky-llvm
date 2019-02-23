; RUN: llc < %s | FileCheck %s

define i32 @test1(i32 %i) #0 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %add = add nsw i32 %0, 2
  ret i32 %add
; CHECK-LABEL: entry
; CHECK: sub sp, 0x4
; CHECK: stw sp, r0
; CHECK: add r0, 0x2
; CHECK: add sp, 0x4
; CHECK: ret
}

define i32 @test2(i32 %i) #0 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %add = add nsw i32 %0, 32767
  ret i32 %add
; CHECK-LABEL: entry
; CHECK: sub sp, 0x4
; CHECK: stw sp, r0
; CHECK: add r0, 0x7fff
; CHECK: add sp, 0x4
; CHECK: ret
}

define i32 @test3(i32 %i) #0 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %add = add nsw i32 %0, 65536
  ret i32 %add
; CHECK-LABEL: entry
; CHECK: sub sp, 0x4
; CHECK: stw sp, r0
; CHECK: li r1, 0x10000
; CHECK: add r0, r1
; CHECK: add sp, 0x4
; CHECK: ret
}

define i32 @test4(i32 %i) #0 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %add = add nsw i32 %0, 1
  ret i32 %add
; CHECK-LABEL: entry
; CHECK: sub sp, 0x4
; CHECK: stw sp, r0
; CHECK: inc r0
; CHECK: add sp, 0x4
; CHECK: ret
}

