//===-- DuckyDisassembler.cpp - Disassembler for Ducky --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DuckyDisassembler class.
//
//===----------------------------------------------------------------------===//

#include "DuckyInstrInfo.h"
#include "MCTargetDesc/DuckyMCTargetDesc.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "riscv-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {
class DuckyDisassembler : public MCDisassembler {

public:
  DuckyDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &VStream,
                              raw_ostream &CStream) const override;
};
} // end anonymous namespace

static MCDisassembler *createDuckyDisassembler(const Target &T,
                                               const MCSubtargetInfo &STI,
                                               MCContext &Ctx) {
  return new DuckyDisassembler(STI, Ctx);
}

extern "C" void LLVMInitializeDuckyDisassembler() {
  // Register the disassembler for each target.
  TargetRegistry::RegisterMCDisassembler(getTheDuckyTarget(),
                                         createDuckyDisassembler);
}

static const unsigned GPRDecoderTable[] = {
  Ducky::R0,  Ducky::R1,  Ducky::R2,  Ducky::R3,
  Ducky::R4,  Ducky::R5,  Ducky::R6,  Ducky::R7,
  Ducky::R8,  Ducky::R9,  Ducky::R10, Ducky::R11,
  Ducky::R12, Ducky::R13, Ducky::R14, Ducky::R15,
  Ducky::R16, Ducky::R17, Ducky::R18, Ducky::R19,
  Ducky::R20, Ducky::R21, Ducky::R22, Ducky::R23,
  Ducky::R24, Ducky::R25, Ducky::R26, Ducky::R27,
  Ducky::R28, Ducky::R29, Ducky::FP,  Ducky::SP
};

static DecodeStatus DecodeGRRegsRegisterClass(MCInst &Inst, uint64_t RegNo,
                                           uint64_t Address,
                                           const void *Decoder) {
  //LLVM_DEBUG(dbgs() << "DecodeGRRegsRegisterClass: Inst=" << Inst << ", RegNo=" << RegNo << '\n');

  if (RegNo > sizeof(GPRDecoderTable))
    return MCDisassembler::Fail;

  // We must define our own mapping from RegNo to register identifier.
  // Accessing index RegNo in the register class will work in the case that
  // registers were added in ascending order, but not in general.
  unsigned Reg = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));

  LLVM_DEBUG(dbgs() << "    Reg=" << Reg << '\n');

  return MCDisassembler::Success;
}

#if 0
static const unsigned FPR32DecoderTable[] = {
  Ducky::F0_32,  Ducky::F1_32,  Ducky::F2_32,  Ducky::F3_32,
  Ducky::F4_32,  Ducky::F5_32,  Ducky::F6_32,  Ducky::F7_32,
  Ducky::F8_32,  Ducky::F9_32,  Ducky::F10_32, Ducky::F11_32,
  Ducky::F12_32, Ducky::F13_32, Ducky::F14_32, Ducky::F15_32,
  Ducky::F16_32, Ducky::F17_32, Ducky::F18_32, Ducky::F19_32,
  Ducky::F20_32, Ducky::F21_32, Ducky::F22_32, Ducky::F23_32,
  Ducky::F24_32, Ducky::F25_32, Ducky::F26_32, Ducky::F27_32,
  Ducky::F28_32, Ducky::F29_32, Ducky::F30_32, Ducky::F31_32
};

static DecodeStatus DecodeFPR32RegisterClass(MCInst &Inst, uint64_t RegNo,
                                             uint64_t Address,
                                             const void *Decoder) {
  if (RegNo > sizeof(FPR32DecoderTable))
    return MCDisassembler::Fail;

  // We must define our own mapping from RegNo to register identifier.
  // Accessing index RegNo in the register class will work in the case that
  // registers were added in ascending order, but not in general.
  unsigned Reg = FPR32DecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeFPR32CRegisterClass(MCInst &Inst, uint64_t RegNo,
                                              uint64_t Address,
                                              const void *Decoder) {
  if (RegNo > 8) {
    return MCDisassembler::Fail;
  }
  unsigned Reg = FPR32DecoderTable[RegNo + 8];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static const unsigned FPR64DecoderTable[] = {
  Ducky::F0_64,  Ducky::F1_64,  Ducky::F2_64,  Ducky::F3_64,
  Ducky::F4_64,  Ducky::F5_64,  Ducky::F6_64,  Ducky::F7_64,
  Ducky::F8_64,  Ducky::F9_64,  Ducky::F10_64, Ducky::F11_64,
  Ducky::F12_64, Ducky::F13_64, Ducky::F14_64, Ducky::F15_64,
  Ducky::F16_64, Ducky::F17_64, Ducky::F18_64, Ducky::F19_64,
  Ducky::F20_64, Ducky::F21_64, Ducky::F22_64, Ducky::F23_64,
  Ducky::F24_64, Ducky::F25_64, Ducky::F26_64, Ducky::F27_64,
  Ducky::F28_64, Ducky::F29_64, Ducky::F30_64, Ducky::F31_64
};

static DecodeStatus DecodeFPR64RegisterClass(MCInst &Inst, uint64_t RegNo,
                                             uint64_t Address,
                                             const void *Decoder) {
  if (RegNo > sizeof(FPR64DecoderTable))
    return MCDisassembler::Fail;

  // We must define our own mapping from RegNo to register identifier.
  // Accessing index RegNo in the register class will work in the case that
  // registers were added in ascending order, but not in general.
  unsigned Reg = FPR64DecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeFPR64CRegisterClass(MCInst &Inst, uint64_t RegNo,
                                              uint64_t Address,
                                              const void *Decoder) {
  if (RegNo > 8) {
    return MCDisassembler::Fail;
  }
  unsigned Reg = FPR64DecoderTable[RegNo + 8];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeGPRNoX0RegisterClass(MCInst &Inst, uint64_t RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo == 0) {
    return MCDisassembler::Fail;
  }

  return DecodeGPRRegisterClass(Inst, RegNo, Address, Decoder);
}

static DecodeStatus DecodeGPRNoX0X2RegisterClass(MCInst &Inst, uint64_t RegNo,
                                                 uint64_t Address,
                                                 const void *Decoder) {
  if (RegNo == 2) {
    return MCDisassembler::Fail;
  }

  return DecodeGPRNoX0RegisterClass(Inst, RegNo, Address, Decoder);
}

static DecodeStatus DecodeGPRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  if (RegNo > 8)
    return MCDisassembler::Fail;

  unsigned Reg = GPRDecoderTable[RegNo + 8];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

// Add implied SP operand for instructions *SP compressed instructions. The SP
// operand isn't explicitly encoded in the instruction.
static void addImplySP(MCInst &Inst, int64_t Address, const void *Decoder) {
  if (Inst.getOpcode() == Ducky::C_LWSP || Inst.getOpcode() == Ducky::C_SWSP ||
      Inst.getOpcode() == Ducky::C_LDSP || Inst.getOpcode() == Ducky::C_SDSP ||
      Inst.getOpcode() == Ducky::C_FLWSP ||
      Inst.getOpcode() == Ducky::C_FSWSP ||
      Inst.getOpcode() == Ducky::C_FLDSP ||
      Inst.getOpcode() == Ducky::C_FSDSP ||
      Inst.getOpcode() == Ducky::C_ADDI4SPN) {
    DecodeGPRRegisterClass(Inst, 2, Address, Decoder);
  }
  if (Inst.getOpcode() == Ducky::C_ADDI16SP) {
    DecodeGPRRegisterClass(Inst, 2, Address, Decoder);
    DecodeGPRRegisterClass(Inst, 2, Address, Decoder);
  }
}

template <unsigned N>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t Imm,
                                      int64_t Address, const void *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  addImplySP(Inst, Address, Decoder);
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint64_t Imm,
                                      int64_t Address, const void *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  addImplySP(Inst, Address, Decoder);
  // Sign-extend the number in the bottom N bits of Imm
  Inst.addOperand(MCOperand::createImm(SignExtend64<N>(Imm)));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeSImmOperandAndLsl1(MCInst &Inst, uint64_t Imm,
                                             int64_t Address,
                                             const void *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  // Sign-extend the number in the bottom N bits of Imm after accounting for
  // the fact that the N bit immediate is stored in N-1 bits (the LSB is
  // always zero)
  Inst.addOperand(MCOperand::createImm(SignExtend64<N>(Imm << 1)));
  return MCDisassembler::Success;
}

static DecodeStatus decodeCLUIImmOperand(MCInst &Inst, uint64_t Imm,
                                         int64_t Address,
                                         const void *Decoder) {
  assert(isUInt<6>(Imm) && "Invalid immediate");
  if (Imm > 31) {
    Imm = (SignExtend64<6>(Imm) & 0xfffff);
  }
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

#endif

static DecodeStatus decodeMemoryTarget(MCInst &Inst, unsigned Insn,
                                       uint64_t Address, const void *Decoder) {
  LLVM_DEBUG(dbgs() << "decodeMemoryTarget: Inst=" << Inst << ", Insn=" << formatv("{0:X}", Insn) << '\n');

  unsigned RawBase = Insn & 0x1F;
  unsigned RawOffset = Insn >> 6;

  LLVM_DEBUG(dbgs() << "    RawBase=" << formatv("{0:X}", RawBase) << ", RawOffset=" << formatv("{0:X}", RawOffset) << '\n');

  DecodeGRRegsRegisterClass(Inst, RawBase, Address, Decoder);
  Inst.addOperand(MCOperand::createImm(RawOffset));

  LLVM_DEBUG(dbgs() << "    Inst=" << Inst << '\n');

  return MCDisassembler::Success;
}

static DecodeStatus doDecodeBranchTarget(MCInst &Inst, unsigned Insn,
                                         uint64_t Address, const void *Decoder) {
  LLVM_DEBUG(dbgs() << "doDecodeBranchTarget: Inst=" << Inst << ", Insn=" << formatv("{0:X}", Insn) << '\n');

  DuckyBranchTargetEncoding Encoding;
  Encoding.overall = Insn;

  LLVM_DEBUG(dbgs() << "    IF=" << (unsigned)Encoding.split.immediate_flag << ", Register=" << Encoding.split.reg << ", Immediate=" << formatv("{0:X}", (uint64_t)Encoding.split.immediate) << '\n');

  if (Encoding.split.immediate_flag == 1) {
    Inst.addOperand(MCOperand::createImm(Encoding.split.immediate));
  } else {
    DecodeGRRegsRegisterClass(Inst, Encoding.split.reg, Address, Decoder);
  }

  LLVM_DEBUG(dbgs() << "    Inst=" << Inst << '\n');

  return MCDisassembler::Success;
}

static DecodeStatus decodeBranchTarget(MCInst &Inst, unsigned Insn,
                                       uint64_t Address, const void *Decoder) {
  return doDecodeBranchTarget(Inst, Insn, Address, Decoder);
}

static DecodeStatus decodeCondBranchTarget(MCInst &Inst, unsigned Insn,
                                           uint64_t Address, const void *Decoder) {
  return doDecodeBranchTarget(Inst, Insn, Address, Decoder);
}

#include "DuckyGenDisassemblerTables.inc"

DecodeStatus DuckyDisassembler::getInstruction(MCInst &MI, uint64_t &Size,
                                               ArrayRef<uint8_t> Bytes,
                                               uint64_t Address,
                                               raw_ostream &OS,
                                               raw_ostream &CS) const {
  // TODO: This will need modification when supporting instruction set
  // extensions with instructions > 32-bits (up to 176 bits wide).
  uint32_t Insn;
  DecodeStatus Result;


  Insn = support::endian::read32le(Bytes.data());

  LLVM_DEBUG(dbgs() << "Trying Ducky32 table :\n");

  Result = decodeInstruction(DecoderTable32, MI, Insn, Address, this, STI);
  Size = 4;

  return Result;

#if 0
  // It's a 32 bit instruction if bit 0 and 1 are 1.
  if ((Bytes[0] & 0x3) == 0x3) {
    Insn = support::endian::read32le(Bytes.data());
    LLVM_DEBUG(dbgs() << "Trying Ducky32 table :\n");
    Result = decodeInstruction(DecoderTable32, MI, Insn, Address, this, STI);
    Size = 4;
  } else {
    Insn = support::endian::read16le(Bytes.data());

    if (!STI.getFeatureBits()[Ducky::Feature64Bit]) {
      LLVM_DEBUG(dbgs() << "Trying Ducky32Only_16 table (16-bit Instruction):\n");
      // Calling the auto-generated decoder function.
      Result = decodeInstruction(DecoderTableDucky32Only_16, MI, Insn, Address,
                                 this, STI);
      if (Result != MCDisassembler::Fail) {
        Size = 2;
        return Result;
      }
    }

    LLVM_DEBUG(dbgs() << "Trying Ducky_C table (16-bit Instruction):\n");
    // Calling the auto-generated decoder function.
    Result = decodeInstruction(DecoderTable16, MI, Insn, Address, this, STI);
    Size = 2;
  }

  return Result;
#endif
}
