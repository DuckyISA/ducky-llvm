//===- DuckyCallingConv.td - Calling Conventions for Ducky -*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Ducky architecture.
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Ducky Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_Ducky : CallingConv<[
  // i32 are returned in registers R0, R1, R2, R3
  CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3]>>,

  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;


//===----------------------------------------------------------------------===//
// Ducky Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_Ducky : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  CCIfNotVarArg<CCIfType<[i32], CCAssignToReg<[ R0, R1, R2, R3, R4, R5, R6, R7 ]>>>,

  CCAssignToStack<4, 4>
]>;


//===----------------------------------------------------------------------===//
// Callee saved registers
//===----------------------------------------------------------------------===//

// Most common
def CC_Save_Ducky : CalleeSavedRegs<(add (sequence "R%u", 29, 8))>;
def CC_Save_All   : CalleeSavedRegs<(add (sequence "R%u", 29, 0))>;
def CC_Save_Most  : CalleeSavedRegs<(add (sequence "R%u", 29, 0))>;
