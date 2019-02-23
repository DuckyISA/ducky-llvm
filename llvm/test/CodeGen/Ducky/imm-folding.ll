; RUN: llc < %s | FileCheck %s

define i32 @test1(i32 %a) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %a, 1
  ret i32 %add
; CHECK-LABEL: test1
; CHECK: inc r0
; CHECK: ret
}

; Function Attrs: norecurse nounwind readnone
define i32 @test2(i32 %a) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
; CHECK-LABEL: test2
; CHECK: add r0, 0x2
; CHECK: ret
}

; Function Attrs: norecurse nounwind readnone
define i32 @test3(i32 %a) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %a, 32767
  ret i32 %add
; CHECK-LABEL: test3
; CHECK: add r0, 0x7fff
; CHECK: ret
}

define i32 @test4(i32 %a) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %a, 36863
  ret i32 %add
; CHECK-LABEL: test4
; CHECK: li r1, 0x8fff
; CHECK: add r0, r1
; CHECK: ret
}
