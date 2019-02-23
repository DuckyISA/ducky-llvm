//===-- Ducky.td - Describe the Ducky Target Machine -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the top level entry point for the Ducky target.
//
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"


//===----------------------------------------------------------------------===//
// Descriptions
//===----------------------------------------------------------------------===//

include "DuckyRegisterInfo.td"
include "DuckyInstrInfo.td"
include "DuckyCallingConv.td"

def DuckyInstrInfo : InstrInfo;

def DuckyAsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 1;
  let ShouldEmitMatchRegisterAltName = 1;
}

//===----------------------------------------------------------------------===//
// Ducky processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
    : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def Ducky : Target {
  let InstructionSet = DuckyInstrInfo;
  let AssemblyParsers = [DuckyAsmParser];
}
