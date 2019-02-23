//===-- DuckyMCTargetDesc.h - Ducky Target Descriptions ---------*- C++ -*-===//
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

#ifndef DuckyMCTARGETDESC_H
#define DuckyMCTARGETDESC_H

#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
class formatted_raw_ostream;
class Target;
class MCInstrInfo;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCContext;
class MCCodeEmitter;
class MCAsmInfo;
class MCCodeGenInfo;
class MCInstPrinter;
class MCObjectWriter;
class MCAsmBackend;
class MCStreamer;
class MCTargetStreamer;

class StringRef;
class raw_ostream;
class raw_pwrite_stream;
class Triple;

Target &getTheDuckyTarget();

MCCodeEmitter *createDuckyMCCodeEmitter(const MCInstrInfo &MCII, const MCRegisterInfo &MRI, MCContext &Ctx);
MCAsmBackend *createDuckyAsmBackend(const Target &T, const MCSubtargetInfo &STI, const MCRegisterInfo &MRI, const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter> createDuckyELFObjectWriter(uint8_t OSABI);

} // End llvm namespace

// Defines symbolic names for Ducky registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "DuckyGenRegisterInfo.inc"

// Defines symbolic names for the Ducky instructions.
//
#define GET_INSTRINFO_ENUM
#include "DuckyGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "DuckyGenSubtargetInfo.inc"

#endif
