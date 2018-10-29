# NOTE: Assertions have been autogenerated by utils/update_mca_test_checks.py
# RUN: llvm-mca -mtriple=x86_64-unknown-unknown -mcpu=bdver2 -iterations=1500 -timeline -timeline-max-iterations=3 < %s | FileCheck %s

# perf stat reports a throughput of 1.00 IPC for this code snippet.

lzcnt %ax, %bx  ## partial register stall.

# CHECK:      Iterations:        1500
# CHECK-NEXT: Instructions:      1500
# CHECK-NEXT: Total Cycles:      1504
# CHECK-NEXT: Total uOps:        3000

# CHECK:      Dispatch Width:    4
# CHECK-NEXT: uOps Per Cycle:    1.99
# CHECK-NEXT: IPC:               1.00
# CHECK-NEXT: Block RThroughput: 0.5

# CHECK:      Instruction Info:
# CHECK-NEXT: [1]: #uOps
# CHECK-NEXT: [2]: Latency
# CHECK-NEXT: [3]: RThroughput
# CHECK-NEXT: [4]: MayLoad
# CHECK-NEXT: [5]: MayStore
# CHECK-NEXT: [6]: HasSideEffects (U)

# CHECK:      [1]    [2]    [3]    [4]    [5]    [6]    Instructions:
# CHECK-NEXT:  2      2     0.50                        lzcntw	%ax, %bx

# CHECK:      Resources:
# CHECK-NEXT: [0.0] - PdAGLU01
# CHECK-NEXT: [0.1] - PdAGLU01
# CHECK-NEXT: [1]   - PdBranch
# CHECK-NEXT: [2]   - PdCount
# CHECK-NEXT: [3]   - PdDiv
# CHECK-NEXT: [4]   - PdEX0
# CHECK-NEXT: [5]   - PdEX1
# CHECK-NEXT: [6]   - PdFPCVT
# CHECK-NEXT: [7.0] - PdFPFMA
# CHECK-NEXT: [7.1] - PdFPFMA
# CHECK-NEXT: [8.0] - PdFPMAL
# CHECK-NEXT: [8.1] - PdFPMAL
# CHECK-NEXT: [9]   - PdFPMMA
# CHECK-NEXT: [10]  - PdFPSTO
# CHECK-NEXT: [11]  - PdFPU0
# CHECK-NEXT: [12]  - PdFPU1
# CHECK-NEXT: [13]  - PdFPU2
# CHECK-NEXT: [14]  - PdFPU3
# CHECK-NEXT: [15]  - PdFPXBR
# CHECK-NEXT: [16]  - PdMul

# CHECK:      Resource pressure per iteration:
# CHECK-NEXT: [0.0]  [0.1]  [1]    [2]    [3]    [4]    [5]    [6]    [7.0]  [7.1]  [8.0]  [8.1]  [9]    [10]   [11]   [12]   [13]   [14]   [15]   [16]
# CHECK-NEXT:  -      -      -      -      -     0.50   0.50    -      -      -      -      -      -      -      -      -      -      -      -      -

# CHECK:      Resource pressure by instruction:
# CHECK-NEXT: [0.0]  [0.1]  [1]    [2]    [3]    [4]    [5]    [6]    [7.0]  [7.1]  [8.0]  [8.1]  [9]    [10]   [11]   [12]   [13]   [14]   [15]   [16]   Instructions:
# CHECK-NEXT:  -      -      -      -      -     0.50   0.50    -      -      -      -      -      -      -      -      -      -      -      -      -     lzcntw	%ax, %bx

# CHECK:      Timeline view:
# CHECK-NEXT: Index     0123456

# CHECK:      [0,0]     DeeER..   lzcntw	%ax, %bx
# CHECK-NEXT: [1,0]     D=eeER.   lzcntw	%ax, %bx
# CHECK-NEXT: [2,0]     .D=eeER   lzcntw	%ax, %bx

# CHECK:      Average Wait times (based on the timeline view):
# CHECK-NEXT: [0]: Executions
# CHECK-NEXT: [1]: Average time spent waiting in a scheduler's queue
# CHECK-NEXT: [2]: Average time spent waiting in a scheduler's queue while ready
# CHECK-NEXT: [3]: Average time elapsed from WB until retire stage

# CHECK:            [0]    [1]    [2]    [3]
# CHECK-NEXT: 0.     3     1.7    0.3    0.0       lzcntw	%ax, %bx