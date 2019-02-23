//===-- DuckyRegisterInfo.td - Ducky Register defs -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the Ducky register file
//===----------------------------------------------------------------------===//
class DuckyReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Ducky";
}


//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//

foreach i = 0-29 in {
  def R#i : DuckyReg<i, "r"#i>;
} 

def FP  : DuckyReg< 30, "fp" >;
def SP  : DuckyReg< 31, "sp" >;
def FLAGS : DuckyReg< 32, "flags" >;


//===----------------------------------------------------------------------===//
//  Register classes
//===----------------------------------------------------------------------===//

def GRRegs : RegisterClass<"Ducky", [i32, i16, i8], 32,
  (add (sequence "R%i", 0, 29), SP, FP)>;

def StatusRegs : RegisterClass<"Ducky", [i32], 32, (add FLAGS)> {
  let CopyCost = -1;
  let isAllocatable = 0;
}
