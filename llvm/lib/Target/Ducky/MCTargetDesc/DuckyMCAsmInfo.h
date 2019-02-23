//===-- DuckyMCAsmInfo.h - Ducky asm properties --------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the DuckyMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef DuckyTARGETASMINFO_H
#define DuckyTARGETASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
class StringRef;
class Target;
class Triple;

class DuckyMCAsmInfo : public MCAsmInfo {
  virtual void anchor();

public:
  explicit DuckyMCAsmInfo(const Triple &TT);
};

} // namespace llvm

#endif
