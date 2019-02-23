//===-- DuckyELFStreamer.h - Ducky ELF Target Streamer ---------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Ducky_DuckyELFSTREAMER_H
#define LLVM_LIB_TARGET_Ducky_DuckyELFSTREAMER_H

#include "DuckyTargetStreamer.h"
#include "llvm/MC/MCELFStreamer.h"

namespace llvm {

class DuckyTargetELFStreamer : public DuckyTargetStreamer {
public:
  MCELFStreamer &getStreamer();
  DuckyTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
};
}
#endif
