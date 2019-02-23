//===-- DuckyELFObjectWriter.cpp - Ducky ELF Writer -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCValue.h"
#include "MCTargetDesc/DuckyFixupKinds.h"
#include "MCTargetDesc/DuckyMCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "ducky-elf-object-writer"

using namespace llvm;

namespace {
class DuckyELFObjectWriter : public MCELFObjectTargetWriter {
public:
  DuckyELFObjectWriter(uint8_t OSABI);

  ~DuckyELFObjectWriter() override;

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
}

DuckyELFObjectWriter::DuckyELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_DUCKY, /*HasRelocationAddend*/ true) {}

DuckyELFObjectWriter::~DuckyELFObjectWriter() {}

unsigned DuckyELFObjectWriter::getRelocType(MCContext &Ctx,
                                            const MCValue &Target,
                                            const MCFixup &Fixup,
                                            bool IsPCRel) const {
  LLVM_DEBUG(dbgs() << "DuckyELFObjectWriter::getRelocType: Target="; Target.print(dbgs()); dbgs() << ", Kind=" << Fixup.getKind() << '\n');

  // Determine the type of the relocation
  switch ((unsigned)Fixup.getKind()) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case FK_Data_4:
    return ELF::R_DUCKY_Sys_Data_4;
  case Ducky::fixup_Ducky_PI_20:
    return ELF::R_DUCKY_PI_20;
  case Ducky::fixup_Ducky_PA_20:
    return ELF::R_DUCKY_PA_20;
  case Ducky::fixup_Ducky_PA_16:
    return ELF::R_DUCKY_PA_16;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createDuckyELFObjectWriter(uint8_t OSABI) {
  return llvm::make_unique<DuckyELFObjectWriter>(OSABI);
}
