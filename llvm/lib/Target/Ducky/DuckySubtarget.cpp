//===-- DuckySubtarget.cpp - Ducky Subtarget Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Ducky specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "DuckySubtarget.h"
#include "Ducky.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "ducky-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "DuckyGenSubtargetInfo.inc"

using namespace llvm;

void DuckySubtarget::anchor() {}

DuckySubtarget::DuckySubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                           DuckyTargetMachine &TM)
    : DuckyGenSubtargetInfo(TT, CPU, FS),
      DL("e-S32-m:e-p:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-a:0:32-n32"),
      InstrInfo(), TLInfo(TM), TSInfo(), FrameLowering() {}
