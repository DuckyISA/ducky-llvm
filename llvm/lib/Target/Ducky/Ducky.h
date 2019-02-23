//===-- Ducky.h - Top-level interface for Ducky representation --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// Ducky back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_Ducky_H
#define TARGET_Ducky_H

#include "MCTargetDesc/DuckyMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class TargetMachine;
class DuckyTargetMachine;

FunctionPass *createDuckyISelDag(DuckyTargetMachine &TM,
                               CodeGenOpt::Level OptLevel);
} // end namespace llvm;

#endif
