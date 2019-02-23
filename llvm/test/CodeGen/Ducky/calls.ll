; RUN: llc -mtriple=ducky -verify-machineinstrs < %s | FileCheck -check-prefix=DUCKY %s

declare i32 @external_function(i32)

define i32 @test_call_external(i32 %a) nounwind {
; DUCKY-LABEL: test_call_external:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:          call external_function
; DUCKY-NEXT:          ret
  %1 = call i32 @external_function(i32 %a)
  ret i32 %1
}

declare i64 @external_function_ll(i64)

define i64 @test_call_external_ll(i64 %a) nounwind {
; DUCKY-LABEL: test_call_external_ll:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:          call external_function_ll
; DUCKY-NEXT:          ret
  %1 = call i64 @external_function_ll(i64 %a)
  ret i64 %1
}

define i32 @defined_function(i32 %a) nounwind {
; DUCKY-LABEL: defined_function:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:          inc r0
; DUCKY-NEXT:          ret
  %1 = add i32 %a, 1
  ret i32 %1
}

define i32 @test_call_defined(i32 %a) nounwind {
; DUCKY-LABEL: test_call_defined:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:          call defined_function
; DUCKY-NEXT:          ret
  %1 = call i32 @defined_function(i32 %a)
  ret i32 %1
}

define i64 @defined_function_ll(i64 %a) nounwind {
; DUCKY-LABEL: defined_function_ll:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:         mov r2, r0
; DUCKY-NEXT:         inc r2
; DUCKY-NEXT:         li r3, 0x0
; DUCKY-NEXT:         li r4, 0x1
; DUCKY-NEXT:         cmpu r2, r0
; DUCKY-NEXT:         mov r0, r4
; DUCKY-NEXT:         sell r0, r3
; DUCKY-NEXT:         cmp r2, r3
; DUCKY-NEXT:         sele r4, r0
; DUCKY-NEXT:         add r1, r4
; DUCKY-NEXT:         mov r0, r2
; DUCKY-NEXT:         ret
  %1 = add i64 %a, 1
  ret i64 %1
}

define i64 @test_call_defined_ll(i64 %a) nounwind {
; DUCKY-LABEL: test_call_defined_ll:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:        sub sp, 0x8
; DUCKY-NEXT:        stw sp[0x4], r8
; DUCKY-NEXT:        stw sp, r9
; DUCKY-NEXT:        mov r8, r1
; DUCKY-NEXT:        mov r9, r0
; DUCKY-NEXT:        call defined_function_ll
; DUCKY-NEXT:        mov r0, r9
; DUCKY-NEXT:        mov r1, r8
; DUCKY-NEXT:        lw r9, sp
; DUCKY-NEXT:        lw r8, sp[0x4]
; DUCKY-NEXT:        add sp, 0x8
; DUCKY-NEXT:        ret
  %1 = call i64 @defined_function_ll(i64 %a)
  ret i64 %a
}

define i32 @test_call_indirect(i32 (i32)* %a, i32 %b) nounwind {
; DUCKY-LABEL: test_call_indirect:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:          mov r2, r0
; DUCKY-NEXT:          mov r0, r1
; DUCKY-NEXT:          call r2
; DUCKY-NEXT:          ret
  %1 = call i32 %a(i32 %b)
  ret i32 %1
}

declare i32 @external_many_args(i32, i32, i32, i32, i32, i32, i32, i32, i32, i32) nounwind

define i32 @test_call_external_many_args(i32 %a) nounwind {
; DUCKY-LABEL: test_call_external_many_args:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:          sub sp, 0x4
; DUCKY-NEXT:          stw sp, r8
; DUCKY-NEXT:          mov r8, r0
; DUCKY-NEXT:          sub sp, 0x8
; DUCKY-NEXT:          stw sp, r8
; DUCKY-NEXT:          mov r0, sp
; DUCKY-NEXT:          add r0, 0x4
; DUCKY-NEXT:          stw r0, r8
; DUCKY-NEXT:          mov r0, r8
; DUCKY-NEXT:          mov r1, r8
; DUCKY-NEXT:          mov r2, r8
; DUCKY-NEXT:          mov r3, r8
; DUCKY-NEXT:          mov r4, r8
; DUCKY-NEXT:          mov r5, r8
; DUCKY-NEXT:          mov r6, r8
; DUCKY-NEXT:          mov r7, r8
; DUCKY-NEXT:          call external_many_args
; DUCKY-NEXT:          add sp, 0x8
; DUCKY-NEXT:          mov r0, r8
; DUCKY-NEXT:          lw r8, sp
; DUCKY-NEXT:          add sp, 0x4
; DUCKY-NEXT:          ret
  %1 = call i32 @external_many_args(i32 %a, i32 %a, i32 %a, i32 %a, i32 %a,
                                    i32 %a, i32 %a, i32 %a, i32 %a, i32 %a)
  ret i32 %a
}

define i32 @defined_many_args(i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 %j) nounwind {
; DUCKY-LABEL: defined_many_args:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:    mov r0, fp
; DUCKY-NEXT:    add r0, 0xc
; DUCKY-NEXT:    lw r0, r0
; DUCKY-NEXT:    inc r0
; DUCKY-NEXT:    ret
  %added = add i32 %j, 1
  ret i32 %added
}

define i32 @test_call_defined_many_args(i32 %a) nounwind {
; DUCKY-LABEL: test_call_defined_many_args:
; DUCKY:       ; %bb.0:
; DUCKY-NEXT:    sub sp, 0x8
; DUCKY-NEXT:    stw sp, r0
; DUCKY-NEXT:    mov r1, sp
; DUCKY-NEXT:    add r1, 0x4
; DUCKY-NEXT:    stw r1, r0
; DUCKY-NEXT:    mov r1, r0
; DUCKY-NEXT:    mov r2, r0
; DUCKY-NEXT:    mov r3, r0
; DUCKY-NEXT:    mov r4, r0
; DUCKY-NEXT:    mov r5, r0
; DUCKY-NEXT:    mov r6, r0
; DUCKY-NEXT:    mov r7, r0
; DUCKY-NEXT:    call defined_many_args
; DUCKY-NEXT:    add sp, 0x8
; DUCKY-NEXT:    ret
  %1 = call i32 @defined_many_args(i32 %a, i32 %a, i32 %a, i32 %a, i32 %a,
                                   i32 %a, i32 %a, i32 %a, i32 %a, i32 %a)
  ret i32 %1
}

