//===-- DuckyELFStreamer.cpp - Ducky ELF Target Streamer Methods ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Ducky specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "DuckyELFStreamer.h"
#include "DuckyMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCSubtargetInfo.h"

using namespace llvm;

// This part is for ELF object output.
DuckyTargetELFStreamer::DuckyTargetELFStreamer(MCStreamer &S,
                                               const MCSubtargetInfo &STI)
    : DuckyTargetStreamer(S) {
  //MCAssembler &MCA = getStreamer().getAssembler();
}

MCELFStreamer &DuckyTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
