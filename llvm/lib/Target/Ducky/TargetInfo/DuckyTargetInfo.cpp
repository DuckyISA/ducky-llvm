//===-- DuckyTargetInfo.cpp - Ducky Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Ducky.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheDuckyTarget() {
  static Target TheDuckyTarget;
  return TheDuckyTarget;
}

extern "C" void LLVMInitializeDuckyTargetInfo() {
  RegisterTarget<Triple::ducky, /*HasJIT=*/false> X(getTheDuckyTarget(), "ducky", "Ducky", "Ducky");
}
