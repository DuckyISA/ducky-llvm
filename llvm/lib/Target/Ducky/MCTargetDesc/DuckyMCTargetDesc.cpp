//===-- DuckyMCTargetDesc.cpp - Ducky Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Ducky specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "DuckyMCTargetDesc.h"
#include "InstPrinter/DuckyInstPrinter.h"
#include "DuckyMCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "MCTargetDesc/DuckyELFStreamer.h"

#define GET_INSTRINFO_MC_DESC
#include "DuckyGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "DuckyGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "DuckyGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createDuckyMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitDuckyMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createDuckyMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitDuckyMCRegisterInfo(X, Ducky::FP);
  return X;
}

static MCSubtargetInfo *createDuckyMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU,
                                                 StringRef FS) {
  return createDuckyMCSubtargetInfoImpl(TT, CPU, FS);
}

static MCAsmInfo *createDuckyMCAsmInfo(const MCRegisterInfo &MRI,
                                     const Triple &TT) {
  return new DuckyMCAsmInfo(TT);
}

/*
static void adjustCodeGenOpts(const Triple &TT, Reloc::Model RM, CodeModel::Model &CM)
{
  CM = CodeModel::Small;
}
*/

static MCInstPrinter *
createDuckyMCInstPrinter(const Triple &TT, unsigned SyntaxVariant,
                       const MCAsmInfo &MAI, const MCInstrInfo &MII,
                       const MCRegisterInfo &MRI) {
  return new DuckyInstPrinter(MAI, MII, MRI);
}

static MCTargetStreamer *
createDuckyObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();

  if (TT.isOSBinFormatELF())
    return new DuckyTargetELFStreamer(S, STI);

  return new DuckyTargetStreamer(S);
}

// Force static initialization.
extern "C" void LLVMInitializeDuckyTargetMC() {
  TargetRegistry::RegisterMCAsmInfo(getTheDuckyTarget(), createDuckyMCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(getTheDuckyTarget(), createDuckyMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(getTheDuckyTarget(), createDuckyMCRegisterInfo);
  TargetRegistry::RegisterMCAsmBackend(getTheDuckyTarget(), createDuckyAsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(getTheDuckyTarget(), createDuckyMCCodeEmitter);
  TargetRegistry::RegisterMCInstPrinter(getTheDuckyTarget(), createDuckyMCInstPrinter);
  TargetRegistry::RegisterMCSubtargetInfo(getTheDuckyTarget(), createDuckyMCSubtargetInfo);
  TargetRegistry::RegisterObjectTargetStreamer(getTheDuckyTarget(), createDuckyObjectTargetStreamer);
}
