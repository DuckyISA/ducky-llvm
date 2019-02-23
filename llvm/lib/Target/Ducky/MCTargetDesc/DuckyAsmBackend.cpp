//===-- DuckyAsmBackend.cpp - Ducky Asm Backend  --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DuckyAsmBackend class.
//
//===----------------------------------------------------------------------===//
//

#include "MCTargetDesc/DuckyFixupKinds.h"
#include "MCTargetDesc/DuckyMCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "MCTargetDesc/DuckyFixupKinds.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"

#define DEBUG_TYPE "ducky-asm-backend"

using namespace llvm;

namespace {

class DuckyAsmBackend : public MCAsmBackend {
  Triple::OSType OSType;
  uint8_t OSABI;

public:
  DuckyAsmBackend(const Target &T, Triple::OSType OSType, uint8_t OSABI)
      : MCAsmBackend(support::little), OSType(OSType), OSABI(OSABI) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  bool writeNopData(raw_ostream &OS, uint64_t Count) const override;

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  Optional<MCFixupKind> getFixupKind(StringRef Name) const override;
  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

  unsigned getNumFixupKinds() const override {
    return Ducky::NumTargetFixupKinds;
  }

  /// MayNeedRelaxation - Check whether the given instruction may need
  /// relaxation.
  ///
  /// \param Inst - The instruction to test.
  bool mayNeedRelaxation(const MCInst &Inst,
                         const MCSubtargetInfo &STI) const override { return false; }

  /// fixupNeedsRelaxation - Target specific predicate for whether a given
  /// fixup requires the associated instruction to be relaxed.
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    // FIXME.
    llvm_unreachable("RelaxInstruction() unimplemented");
    return false;
  }

  /// RelaxInstruction - Relax the instruction in the given fragment
  /// to the next wider instruction.
  ///
  /// \param Inst - The instruction to relax, which may be the same
  /// as the output.
  /// \param [out] Res On return, the relaxed instruction.
  void relaxInstruction(const MCInst &Inst, const MCSubtargetInfo &STI,
                        MCInst &Res) const override {}

}; // class DuckyAsmBackend

} // end anonymous namespace

// Prepare value for the target space for it
static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value, MCContext &Ctx) {

  unsigned Kind = Fixup.getKind();

  LLVM_DEBUG(dbgs() << "adjustFixupValue: Value=" << formatv("{0:X}", Value) << '\n');
  LLVM_DEBUG(dbgs() << "adjustFixupValue: " << Value << " " << (int64_t)Value << '\n');

  switch (Kind) {
    default:
      return 0;

    case Ducky::fixup_Ducky_PI_20:
      Value -= 4;

      if (!isInt<20>((int64_t)Value)) {
          Ctx.reportError(Fixup.getLoc(), "out of range PI_20 fixup");
          return 0;
      }

      Value = (Value << 1) | 0x1;
      break;

    case Ducky::fixup_Ducky_PA_20:
      Value = ((int64_t)Value >> 2) - 1;

      if (!isInt<20>((int64_t)Value)) {
          Ctx.reportError(Fixup.getLoc(), "out of range PA_20 fixup");
          return 0;
      }

      Value = (Value << 1) | 0x1;
      break;

    case Ducky::fixup_Ducky_PA_16:
      Value = ((int64_t)Value >> 2) - 1;

      if (!isInt<20>((int64_t)Value)) {
        Ctx.reportError(Fixup.getLoc(), "out of range PA_16 fixup");
        return 0;
      }

      Value = (Value << 1) | 0x1;
      break;
  }

  return Value;
}

/// ApplyFixup - Apply the \p Value for given \p Fixup into the provided
/// data fragment, at the offset specified by the fixup and following the
/// fixup kind as appropriate.
void DuckyAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                 const MCValue &Target,
                                 MutableArrayRef<char> Data, uint64_t Value,
                                 bool IsResolved,
                                 const MCSubtargetInfo *STI) const {
  MCFixupKind Kind = Fixup.getKind();
  MCContext &Ctx = Asm.getContext();

  LLVM_DEBUG(dbgs() << "DuckyAsmBackend::applyFixup: Value=" << Value << ", Fixup.Kind=" << Kind << ", Expr=" << *Fixup.getValue() << '\n');

  Value = adjustFixupValue(Fixup, Value, Ctx);

  LLVM_DEBUG(dbgs() << "DuckyAsmBackend::applyFixup: adjusted Value=" << Value << '\n');

  if (!Value) {
    LLVM_DEBUG(dbgs() << '\n');
    return; // Doesn't change encoding.
  }

  MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());

  unsigned Offset = Fixup.getOffset();
  auto NumBits = Info.TargetSize + Info.TargetOffset;
  auto NumBytes = (NumBits / 8) + ((NumBits % 8) == 0 ? 0 : 1);

  LLVM_DEBUG(dbgs() << formatv("DuckyAsmBackend::agplyFixup: Offset={0:X}, TargetOffset={1}, TargetSize={2} NumBits={3}, NumBytes={4}\n", Offset, Info.TargetOffset, Info.TargetSize, NumBits, NumBytes));

  uint32_t CurVal = 0;

  for (unsigned i = 0; i != NumBytes; ++i)
    CurVal |= (uint32_t)((uint8_t)Data[Offset + i] << (i * 8));

  LLVM_DEBUG(dbgs() << formatv("DuckyAsmBackend::applyFixup: CurVal={0:X}\n", CurVal));

  Value = (Value << Info.TargetOffset) | (CurVal & ~(1 << Info.TargetOffset));
  LLVM_DEBUG(dbgs() << formatv("DuckyAsmBackend::applyFixup: Value={0:X}\n", Value));

  for (unsigned i = 0; i < NumBytes; ++i)
    Data[Offset + i] = (Value >> (i * 8)) & 0xFF;

  LLVM_DEBUG(dbgs() << '\n');
}

Optional<MCFixupKind> DuckyAsmBackend::getFixupKind(StringRef Name) const {
  return StringSwitch<Optional<MCFixupKind>>(Name)
      .Case("R_DUCKY_Sys_Data_4", (MCFixupKind)FK_Data_4)
      .Case("R_DUCKY_PI_20", (MCFixupKind)Ducky::fixup_Ducky_PI_20)
      .Case("R_DUCKY_PA_20", (MCFixupKind)Ducky::fixup_Ducky_PA_20)
      .Case("R_DUCKY_PA_16", (MCFixupKind)Ducky::fixup_Ducky_PA_16)
      .Default(MCAsmBackend::getFixupKind(Name));
}

//@getFixupKindInfo {
const MCFixupKindInfo &
DuckyAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[Ducky::NumTargetFixupKinds] = {
      // This table *must* be in same the order of fixup_* kinds in
      // DuckyFixupKinds.h.
      //
      // name                        offset  bits  flags
      // the actual value is 20 bits, adding immediate flag 0x1 at the beginning
      {"fixup_Ducky_PI_20",          11,     21,   MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_Ducky_PA_20",          11,     21,   MCFixupKindInfo::FKF_IsPCRel},
      // the actual value is 16 bits, adding immediate flag 0x1 at the beginning
      {"fixup_Ducky_PA_16",          15,     17,   MCFixupKindInfo::FKF_IsPCRel}
  };

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

std::unique_ptr<MCObjectTargetWriter>
DuckyAsmBackend::createObjectTargetWriter() const {
  return createDuckyELFObjectWriter(OSABI);
}

bool DuckyAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count) const {
  return true;
}

// MCAsmBackend
MCAsmBackend *llvm::createDuckyAsmBackend(const Target &T,
                                          const MCSubtargetInfo &STI,
                                          const MCRegisterInfo & /*MRI*/,
                                          const MCTargetOptions & /*Options*/) {
  const Triple &TT = STI.getTargetTriple();
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  return new DuckyAsmBackend(T, TT.getOS(), OSABI);
}
