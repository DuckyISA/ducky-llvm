//===-- DuckyTargetStreamer.h - Ducky Target Streamer ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Ducky_DuckyTARGETSTREAMER_H
#define LLVM_LIB_TARGET_Ducky_DuckyTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {

class DuckyTargetStreamer : public MCTargetStreamer {
public:
  DuckyTargetStreamer(MCStreamer &S);
};
}
#endif
