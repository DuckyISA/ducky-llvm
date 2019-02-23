//===-- DuckyMCCodeEmitter.cpp - Convert Ducky code to machine code -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DuckyMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "DuckyInstrInfo.h"
//#include "MCTargetDesc/DuckyBaseInfo.h"
#include "MCTargetDesc/DuckyFixupKinds.h"
//#include "MCTargetDesc/DuckyMCExpr.h"
#include "MCTargetDesc/DuckyMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormatVariadic.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

//STATISTIC(MCNumEmitted, "Number of MC instructions emitted");
//STATISTIC(MCNumFixups, "Number of MC fixups created");

namespace {
class DuckyMCCodeEmitter : public MCCodeEmitter {
  DuckyMCCodeEmitter(const DuckyMCCodeEmitter &) = delete;
  void operator=(const DuckyMCCodeEmitter &) = delete;
  MCContext &Ctx;
  MCInstrInfo const &MCII;
  const MCRegisterInfo &MRI;

public:
  DuckyMCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII, const MCRegisterInfo &mri)
      : Ctx(ctx), MCII(MCII), MRI(mri) {}

  ~DuckyMCCodeEmitter() override {}

  uint64_t encodeMemoryTarget(const MCInst &MI, unsigned Op,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  uint64_t doEncodeBranchTarget(const MCInst &MI, unsigned Op,
                                SmallVectorImpl<MCFixup> &Fixups,
                                const MCSubtargetInfo &STI,
                                unsigned ImmediateLimit, Ducky::Fixups DuckyFixupKind) const;

  uint64_t encodeBranchTarget(const MCInst &MI, unsigned Op,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const;

  uint64_t encodeCondBranchTarget(const MCInst &MI, unsigned Op,
                                  SmallVectorImpl<MCFixup> &Fixups,
                                  const MCSubtargetInfo &STI) const;

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  /// TableGen'erated function for getting the binary encoding for an
  /// instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Return binary encoding of operand. If the machine operand requires
  /// relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;
};
} // end anonymous namespace

MCCodeEmitter *llvm::createDuckyMCCodeEmitter(const MCInstrInfo &MCII,
                                              const MCRegisterInfo &MRI,
                                              MCContext &Ctx) {
  return new DuckyMCCodeEmitter(Ctx, MCII, MRI);
}

/*
 * {4-0}:  base register
 * {5}:    1 if there is an offset
 * {20-7}: offset
 */
uint64_t DuckyMCCodeEmitter::encodeMemoryTarget(const MCInst &MI, unsigned Op,
                                                SmallVectorImpl<MCFixup> &Fixups,
                                                const MCSubtargetInfo &STI) const {
  LLVM_DEBUG(dbgs() << "DuckyMCCodeEmitter::encodeMemoryTarget: MI=" << MI << ", Op=" << Op << '\n');

  MCOperand MO = MI.getOperand(1);
  LLVM_DEBUG(dbgs() << "DuckyMCCodeEmitter::encodeMemoryTarget: MO=" << MO << '\n');

  assert(MO.isReg() && "First operand is not register.");

  uint64_t Encoding = MRI.getEncodingValue(MO.getReg());

  MO = MI.getOperand(2);
  assert(MO.isImm() && "Second operand is not immediate.");

  if (MO.getImm() == 0)
    return Encoding;

  Encoding |= (1 << 5);
  Encoding |= (MO.getImm() << 6);

  return Encoding;
}

uint64_t DuckyMCCodeEmitter::doEncodeBranchTarget(const MCInst &MI, unsigned Op,
                                                  SmallVectorImpl<MCFixup> &Fixups,
                                                  const MCSubtargetInfo &STI,
                                                  unsigned ImmediateLimit, Ducky::Fixups DuckyFixupKind) const {
  MCOperand MO = MI.getOperand(Op);

  LLVM_DEBUG(dbgs() << "DuckyMCCodeEmitter::doEncodeBranchTarget: MI=" << MI << ", Op=" << Op << ", MO=" << MO << '\n');

  if (MO.isExpr()) {
    MCFixupKind FixupKind = static_cast<MCFixupKind>(DuckyFixupKind);
    Fixups.push_back(MCFixup::create(0, MO.getExpr(), FixupKind, MI.getLoc()));

    return 0;
  }

  DuckyBranchTargetEncoding Encoding;
  Encoding.overall = 0;

  if (MO.isReg()) {
    Encoding.split.reg = MRI.getEncodingValue(MO.getReg());

    return Encoding.overall;
  }

  if (MO.isImm()) {
    Encoding.split.immediate_flag = 1;

    if (MO.getImm() == 0)
      return Encoding.overall;

    Encoding.split.immediate = MO.getImm();

    if (Encoding.split.immediate > (uint64_t)(1 << ImmediateLimit))
      llvm_unreachable("Immediate is too big...");

    return Encoding.overall;
  }

  llvm_unreachable("Unhandled branch target type");
}

uint64_t DuckyMCCodeEmitter::encodeBranchTarget(const MCInst &MI, unsigned Op,
                                                SmallVectorImpl<MCFixup> &Fixups,
                                                const MCSubtargetInfo &STI) const {
  return doEncodeBranchTarget(MI, Op, Fixups, STI, 20, Ducky::Fixups::fixup_Ducky_PA_20);
}

uint64_t DuckyMCCodeEmitter::encodeCondBranchTarget(const MCInst &MI, unsigned Op,
                                                    SmallVectorImpl<MCFixup> &Fixups,
                                                    const MCSubtargetInfo &STI) const {
  return doEncodeBranchTarget(MI, Op, Fixups, STI, 16, Ducky::Fixups::fixup_Ducky_PA_16);
}

void DuckyMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  // Get byte count of instruction.
  unsigned Size = Desc.getSize();

  switch (Size) {
  default:
    llvm_unreachable("Unhandled encodeInstruction length!");
  case 2: {
    uint16_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write<uint16_t>(OS, Bits, support::little);
    break;
  }
  case 4: {
    uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write(OS, Bits, support::little);
    break;
  }
  }

  //++MCNumEmitted; // Keep track of the # of mi's emitted.
}

unsigned
DuckyMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  LLVM_DEBUG(dbgs() << "DuckyMCCodeEmitter::getMachineOpValue: MI=" << MI << ", MO=" << MO << '\n');

  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  if (MO.isExpr()) {
    const MCExpr *Expr = MO.getExpr();

    if (Expr->getKind() == MCExpr::Binary) {
      const MCBinaryExpr *BinaryExpr = static_cast<const MCBinaryExpr *>(Expr);
      Expr = BinaryExpr->getLHS();
    }

    assert(Expr->getKind() == MCExpr::SymbolRef);

    switch(MI.getOpcode()) {
      case Ducky::LAi:
        Fixups.push_back(MCFixup::create(0, MO.getExpr(), MCFixupKind(Ducky::fixup_Ducky_PI_20)));
        return 0;
      case Ducky::CALLi:
      case Ducky::Ji:
        Fixups.push_back(MCFixup::create(0, MO.getExpr(), MCFixupKind(Ducky::fixup_Ducky_PA_20)));
        return 0;
      case Ducky::BE:
        Fixups.push_back(MCFixup::create(0, MO.getExpr(), MCFixupKind(Ducky::fixup_Ducky_PA_16)));
        return 0;
      default:
        LLVM_DEBUG(dbgs() << "Unhandled opcode " << MI.getOpcode() << '\n');
        llvm_unreachable("Unhandled expression!");
        return 0;
    }
  }

  llvm_unreachable("Unhandled expression!");
  return 0;
}

unsigned DuckyMCCodeEmitter::getImmOpValue(const MCInst &MI, unsigned OpNo,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {

  const MCOperand &MO = MI.getOperand(OpNo);

  //MCInstrDesc const &Desc = MCII.get(MI.getOpcode());
  //unsigned MIFrm = Desc.TSFlags & DuckyII::InstFormatMask;

  LLVM_DEBUG(dbgs() << "DuckyMCCodeEmitter::getImmOpValue: MI=" << MI << ", OpNo=" << OpNo << '\n');

  // If the destination is an immediate, there is nothing to do
  if (MO.isImm())
    return MO.getImm();

  assert(MO.isExpr() && "getImmOpValue expects only expressions or immediates");

  /*
  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();
  Ducky::Fixups FixupKind = Ducky::fixup_riscv_invalid;
  if (Kind == MCExpr::Target) {
    const DuckyMCExpr *RVExpr = cast<DuckyMCExpr>(Expr);

    switch (RVExpr->getKind()) {
    case DuckyMCExpr::VK_Ducky_None:
    case DuckyMCExpr::VK_Ducky_Invalid:
      llvm_unreachable("Unhandled fixup kind!");
    case DuckyMCExpr::VK_Ducky_LO:
      if (MIFrm == DuckyII::InstFormatI)
        FixupKind = Ducky::fixup_riscv_lo12_i;
      else if (MIFrm == DuckyII::InstFormatS)
        FixupKind = Ducky::fixup_riscv_lo12_s;
      else
        llvm_unreachable("VK_Ducky_LO used with unexpected instruction format");
      break;
    case DuckyMCExpr::VK_Ducky_HI:
      FixupKind = Ducky::fixup_riscv_hi20;
      break;
    case DuckyMCExpr::VK_Ducky_PCREL_LO:
      if (MIFrm == DuckyII::InstFormatI)
        FixupKind = Ducky::fixup_riscv_pcrel_lo12_i;
      else if (MIFrm == DuckyII::InstFormatS)
        FixupKind = Ducky::fixup_riscv_pcrel_lo12_s;
      else
        llvm_unreachable(
            "VK_Ducky_PCREL_LO used with unexpected instruction format");
      break;
    case DuckyMCExpr::VK_Ducky_PCREL_HI:
      FixupKind = Ducky::fixup_riscv_pcrel_hi20;
      break;
    }
  } else if (Kind == MCExpr::SymbolRef &&
             cast<MCSymbolRefExpr>(Expr)->getKind() == MCSymbolRefExpr::VK_None) {
    if (Desc.getOpcode() == Ducky::JAL) {
      FixupKind = Ducky::fixup_riscv_jal;
    } else if (MIFrm == DuckyII::InstFormatB) {
      FixupKind = Ducky::fixup_riscv_branch;
    } else if (MIFrm == DuckyII::InstFormatCJ) {
      FixupKind = Ducky::fixup_riscv_rvc_jump;
    } else if (MIFrm == DuckyII::InstFormatCB) {
      FixupKind = Ducky::fixup_riscv_rvc_branch;
    }
  }

  assert(FixupKind != Ducky::fixup_riscv_invalid && "Unhandled expression!");

  Fixups.push_back(
      MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
  ++MCNumFixups;
  */

  return 0;
}

#include "DuckyGenMCCodeEmitter.inc"
