//===-- DuckyInstrInfo.cpp - Ducky Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Ducky implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "DuckyInstrInfo.h"
#include "Ducky.h"
#include "DuckyMachineFunctionInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "DuckyGenInstrInfo.inc"

#define DEBUG_TYPE "ducky"

using namespace llvm;

// Pin the vtable to this file.
void DuckyInstrInfo::anchor() {}

DuckyInstrInfo::DuckyInstrInfo()
  : DuckyGenInstrInfo(Ducky::ADJCALLSTACKDOWN, Ducky::ADJCALLSTACKUP),
    RI() {
}

/*
 * "Must return 0 if the instruction has any other effect other than loading
 * from the stack slot" - it does, LW may modify Z flag.
 */
unsigned DuckyInstrInfo::isLoadFromStackSlot(const MachineInstr &MI, int &FrameIndex) const
{
  return 0;
}


unsigned DuckyInstrInfo::isStoreToStackSlot(const MachineInstr &MI, int &FrameIndex) const
{
  return 0;
}


/*
 * Create instruction to copy value between two registers.
 */
void DuckyInstrInfo::copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
    const DebugLoc &DL, unsigned DestReg, unsigned SrcReg, bool KillSrc) const
{
  BuildMI(MBB, I, DL, get(Ducky::MOVrr), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc));
}


/*
 * Create instruction to store register value on stack.
 */
void DuckyInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
    unsigned SrcReg, bool isKill, int FrameIndex, const TargetRegisterClass *RC, const TargetRegisterInfo *TRI) const
{
  LLVM_DEBUG(dbgs() << "storeRegToStackSlot: reg=" << SrcReg << ", frame-index=" << FrameIndex << '\n');

  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  BuildMI(MBB, I, DL, get(Ducky::STW)).addReg(SrcReg, getKillRegState(isKill)).addFrameIndex(FrameIndex).addImm(0);
}


/*
 * Create instruction to load register value from stack.
 */
void DuckyInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
    unsigned DestReg, int FrameIndex, const TargetRegisterClass *RC, const TargetRegisterInfo *TRI) const
{
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  BuildMI(MBB, I, DL, get(Ducky::LW), DestReg).addFrameIndex(FrameIndex).addImm(0);
}


static bool isCompareInstr(MachineInstr &MI)
{
  switch(MI.getOpcode()) {
    default:
      return false;

    case Ducky::CMPrr:
    case Ducky::CMPri:
    case Ducky::CMPUrr:
    case Ducky::CMPUri:
      return true;
  }
}

static bool isSelectInstr(MachineInstr &MI)
{
  switch(MI.getOpcode()) {
    default:
      return false;

    case Ducky::SELErr:
    case Ducky::SELEri:
    case Ducky::SELNErr:
    case Ducky::SELNEri:
    case Ducky::SELZrr:
    case Ducky::SELZri:
    case Ducky::SELNZrr:
    case Ducky::SELNZri:
    case Ducky::SELOrr:
    case Ducky::SELOri:
    case Ducky::SELNOrr:
    case Ducky::SELNOri:
    case Ducky::SELSrr:
    case Ducky::SELSri:
    case Ducky::SELNSrr:
    case Ducky::SELNSri:
    case Ducky::SELGrr:
    case Ducky::SELGri:
    case Ducky::SELGErr:
    case Ducky::SELGEri:
    case Ducky::SELLrr:
    case Ducky::SELLri:
    case Ducky::SELLErr:
    case Ducky::SELLEri:
      return true;
  }
}

static bool isBranchingInstr(MachineInstr &MI)
{
  switch(MI.getOpcode()) {
    default:
      return false;

    case Ducky::BE:
    case Ducky::BNE:
    case Ducky::BZ:
    case Ducky::BNZ:
    case Ducky::BO:
    case Ducky::BNO:
    case Ducky::BS:
    case Ducky::BNS:
    case Ducky::BG:
    case Ducky::BGE:
    case Ducky::BL:
    case Ducky::BLE:
      return true;

    case Ducky::Ji:
      return true;

    case Ducky::PseudoCondBranch:
      return true;
  }
}

static bool isPseudoCondBranchingInstr(MachineInstr &MI)
{
  return MI.getOpcode() == Ducky::PseudoCondBranch;
}

static bool isDirectBranchingInstr(MachineInstr &MI)
{
  return (MI.getOpcode() == Ducky::Ji);
}

static bool isCondBranchingInstr(MachineInstr &MI)
{
  switch(MI.getOpcode()) {
    default:
      return false;

    case Ducky::BE:
    case Ducky::BNE:
    case Ducky::BZ:
    case Ducky::BNZ:
    case Ducky::BO:
    case Ducky::BNO:
    case Ducky::BS:
    case Ducky::BNS:
    case Ducky::BG:
    case Ducky::BGE:
    case Ducky::BL:
    case Ducky::BLE:
      return true;
  }
}

static bool isArithBinOpInstr(MachineInstr &MI)
{
  switch(MI.getOpcode()) {
    default:
      return false;

    case Ducky::ADDrr:
    case Ducky::ADDri:
    case Ducky::SUBrr:
    case Ducky::SUBri:
    case Ducky::ANDrr:
    case Ducky::ANDri:
    case Ducky::XORrr:
    case Ducky::XORri:
    case Ducky::ORrr:
    case Ducky::ORri:
    case Ducky::SHLrr:
    case Ducky::SHLri:
    case Ducky::SHRrr:
    case Ducky::SHRri:
    case Ducky::SHRArr:
    case Ducky::SHRAri:
    case Ducky::MULrr:
    case Ducky::MULri:
    case Ducky::SDIVrr:
    case Ducky::SDIVri:
    case Ducky::UDIVrr:
    case Ducky::UDIVri:
    case Ducky::SMODrr:
    case Ducky::SMODri:
      return true;
  }
}

static unsigned int getRIVariant(MachineInstr &MI)
{
  switch(MI.getOpcode()) {
    default:
      llvm_unreachable("Unhandled opcode");

    case Ducky::CMPrr:
      return Ducky::CMPri;
    case Ducky::CMPUrr:
      return Ducky::CMPUri;
    case Ducky::ADDrr:
      return Ducky::ADDri;
    case Ducky::SUBrr:
      return Ducky::SUBri;
    case Ducky::ANDrr:
      return Ducky::ANDri;
    case Ducky::XORrr:
      return Ducky::XORri;
    case Ducky::ORrr:
      return Ducky::ORri;
    case Ducky::SHLrr:
      return Ducky::SHLri;
    case Ducky::SHRrr:
      return Ducky::SHRri;
    case Ducky::SHRArr:
      return Ducky::SHRAri;
    case Ducky::MULrr:
      return Ducky::MULri;
    case Ducky::SDIVrr:
      return Ducky::SDIVri;
    case Ducky::UDIVrr:
      return Ducky::UDIVri;
    case Ducky::SMODrr:
      return Ducky::SMODri;
    case Ducky::SELErr:
      return Ducky::SELEri;
    case Ducky::SELNErr:
      return Ducky::SELNEri;
    case Ducky::SELZrr:
      return Ducky::SELZri;
    case Ducky::SELNZrr:
      return Ducky::SELNZri;
    case Ducky::SELOrr:
      return Ducky::SELOri;
    case Ducky::SELNOrr:
      return Ducky::SELNOri;
    case Ducky::SELSrr:
      return Ducky::SELSri;
    case Ducky::SELNSrr:
      return Ducky::SELNSri;
    case Ducky::SELGrr:
      return Ducky::SELGri;
    case Ducky::SELGErr:
      return Ducky::SELGEri;
    case Ducky::SELLrr:
      return Ducky::SELLri;
    case Ducky::SELLErr:
      return Ducky::SELLEri;
  }
}

/*
 * Analyze branch instruction at the end of a given basic block.
 */
static void packCondBranchInstr(MachineInstr &MI, MachineBasicBlock *&TBB, SmallVectorImpl<MachineOperand> &Cond)
{
  TBB = MI.getOperand(2).getMBB();

  Cond.push_back(MachineOperand::CreateImm(0)); // not a "real" instruction
  Cond.push_back(MachineOperand::CreateImm(MI.getOpcode()));
  Cond.push_back(MI.getOperand(0));
  Cond.push_back(MI.getOperand(1));
  Cond.push_back(MI.getOperand(2));
  Cond.push_back(MI.getOperand(3));
  Cond.push_back(MI.getOperand(4));
  Cond.push_back(MI.getOperand(5));
}

static void packBranchInstr(MachineInstr &MI, MachineBasicBlock *&TBB, SmallVectorImpl<MachineOperand> &Cond)
{
  TBB = MI.getOperand(0).getMBB();

  Cond.push_back(MachineOperand::CreateImm(1)); // a "real" instruction
  Cond.push_back(MachineOperand::CreateImm(MI.getOpcode()));
  Cond.push_back(MI.getOperand(0));
}

#define BEFORE() do { LLVM_DEBUG(dbgs() << "--- Before\n"); LLVM_DEBUG(MBB.dump()); LLVM_DEBUG(dbgs() << "---\n"); } while(0)
#define AFTER()  do { LLVM_DEBUG(dbgs() << "--- After\n"); LLVM_DEBUG(MBB.dump()); LLVM_DEBUG(dbgs() << "---\n"); } while(0)

bool DuckyInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const
{
  TBB = nullptr;
  FBB = nullptr;

  LLVM_DEBUG(dbgs() << "AnalyzeBranch:\n");
  BEFORE();

  // If the block has no terminators, it just falls into the block after it.
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end()) {
    LLVM_DEBUG(dbgs() << "  no terminator, fall through\n");
    return false;
  }

  if (!isUnpredicatedTerminator(*I)) {
    LLVM_DEBUG(dbgs() << "  not an unpredicated terminator\n");
    return false;
  }

  // Last instruction of the block
  MachineInstr &LastInstr = *I;

  LLVM_DEBUG(dbgs() << "  last: " << LastInstr);

  // If there is only one terminator instruction, process it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
    LLVM_DEBUG(dbgs() << "  one terminator only\n");

    if (isDirectBranchingInstr(LastInstr)) {
      LLVM_DEBUG(dbgs() << "  jump instruction, setting TBB\n");

      TBB = LastInstr.getOperand(0).getMBB();
      AFTER();
      return false;
    }

    if (isPseudoCondBranchingInstr(LastInstr)) {
      LLVM_DEBUG(dbgs() << "    pseudo cond branch, setting TBB and cond\n");

      packCondBranchInstr(LastInstr, TBB, Cond);
      AFTER();
      return false;
    }

    if (isCondBranchingInstr(LastInstr)) {
      LLVM_DEBUG(dbgs() << "    cond branch, setting TBB and cond\n");

      packBranchInstr(LastInstr, TBB, Cond);
      AFTER();
      return false;
    }

    // "Don't understand it" sentinel.
    LLVM_DEBUG(dbgs() << "  don't understand it\n");
    AFTER();
    return true;
  }

  // Get the instruction before it if it's a terminator.
  MachineInstr &SecondLastInstr = *I;

  LLVM_DEBUG(dbgs() << "  second to last: " << SecondLastInstr);

  // If there are three terminators, we don't know what sort of block this is.
  if (I != MBB.begin() && isUnpredicatedTerminator(*--I)) {
    LLVM_DEBUG(dbgs() << "  three terminators?\n");
    AFTER();
    return true;
  }

  if (isDirectBranchingInstr(SecondLastInstr)) {
    LLVM_DEBUG(dbgs() << "  second to last is jump instruction - re-setting TBB\n");

    TBB = SecondLastInstr.getOperand(0).getMBB();

    if (AllowModify) {
      LLVM_DEBUG(dbgs() << "  removing the last instruction\n");
      LastInstr.eraseFromParent();
    }

    AFTER();
    return false;
  }

  if (isDirectBranchingInstr(LastInstr)) {
    if (isPseudoCondBranchingInstr(SecondLastInstr)) {
      LLVM_DEBUG(dbgs() << "  second to last is pseudo cond and last is jump - setting TBB, FBB and cond\n");

      packCondBranchInstr(SecondLastInstr, TBB, Cond);
      FBB = LastInstr.getOperand(0).getMBB();

      AFTER();
      return false;
    }

    if (isCondBranchingInstr(SecondLastInstr)) {
      LLVM_DEBUG(dbgs() << "  second to last is cond and last is jump - setting TBB, FBB and cond\n");

      packBranchInstr(SecondLastInstr, TBB, Cond);
      FBB = LastInstr.getOperand(0).getMBB();

      AFTER();
      return false;
    }
  }

  LLVM_DEBUG(dbgs() << "  don't undestand this\n");
  AFTER();
  return true;
}

/*
 * Remove the branching code at the end of a given basic block.
 */
unsigned DuckyInstrInfo::removeBranch(MachineBasicBlock &MBB, int *BytesRemoved) const
{
  assert(!BytesRemoved && "code size not handled");

  if (MBB.empty())
    return 0;

  LLVM_DEBUG(dbgs() << "RemoveBranch:\n");
  BEFORE();

  unsigned NumRemoved = 0;
  auto I = MBB.end();
  do {
    --I;
    LLVM_DEBUG(dbgs() << "  inst: "; I->dump());

    if (!isBranchingInstr(*I)) {
      LLVM_DEBUG(dbgs() << "      not a branching inst, leave\n");
      break;
    }

    LLVM_DEBUG(dbgs() << "      removing\n");
    I->eraseFromParent();
    I = MBB.end();
    NumRemoved++;
  } while (I != MBB.begin());

  LLVM_DEBUG(dbgs() << "--- After\n");
  LLVM_DEBUG(MBB.dump());
  LLVM_DEBUG(dbgs() << "---\n");

  AFTER();
  return NumRemoved;
}


/*
 * Insert branching code at the end of a given basic block.
 */
unsigned DuckyInstrInfo::insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
    MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const
{
  unsigned NumInserted = 0;

  LLVM_DEBUG(dbgs() << "InsertBranch:\n");
  BEFORE();

  if (Cond.size() > 0) {
    MachineInstr *MI = NULL;

    if (Cond[0].getImm() == 1) {
      LLVM_DEBUG(dbgs() << "  inserting cond branch, opcode=" << Cond[1] << ", block=" << Cond[2] << '\n');

      MI = BuildMI(MBB, MBB.end(), DL, get(Cond[1].getImm())).add(Cond[2]).getInstr();
    } else {
      LLVM_DEBUG(dbgs() << "  inserting pseudo cond branch, opcode=" << Cond[1] << ", lhs=" << Cond[2] << ", rhs=" << Cond[3] << ", block=" << Cond[4] << ", flag=" << Cond[5] << ", value=" << Cond[6] << ", signed=" << Cond[7] << '\n');

      MI = BuildMI(MBB, MBB.end(), DL, get(Cond[1].getImm())) \
           .addReg(Cond[2].getReg()) \
           .addReg(Cond[3].getReg()) \
           .addMBB(Cond[4].getMBB()) \
           .addImm(Cond[5].getImm()) \
           .addImm(Cond[6].getImm()) \
           .addImm(Cond[7].getImm()) \
           .getInstr();
    }

    assert(MI != NULL);

    NumInserted++;
    LLVM_DEBUG(dbgs() << "  inserted cond branch: " << *MI);
  }

  if (Cond.empty() || FBB) {
    MachineInstr *MI = BuildMI(MBB, MBB.end(), DL, get(Ducky::Ji)).addMBB(Cond.empty() ? TBB : FBB).getInstr();
    NumInserted++;
    LLVM_DEBUG(dbgs() << "  inserted branch: " << *MI);
  }

  LLVM_DEBUG(dbgs() << "  successors: "; for (MachineBasicBlock *SuccBB : MBB.successors()) dbgs() << " " << SuccBB->getName(); dbgs() << '\n');

  if (BytesAdded)
    *BytesAdded = NumInserted * 4;

  AFTER();

  return NumInserted;
}


/*
 * Immediate folding
 *
 * Some instructions can take as their second operand register as well as an
 * immediate value. Handle such optimization opportunities for known cases.
 */
bool DuckyInstrInfo::FoldImmediate(MachineInstr &UseMI, MachineInstr &DefMI, unsigned Reg, MachineRegisterInfo *MRI) const
{
  LLVM_DEBUG(dbgs() << "FoldImmediate:\n  DefMI=" << DefMI << "  UseMI=" << UseMI << "  Reg=" << Reg << '\n');

  if (DefMI.getOpcode() != Ducky::LIi20) {
    LLVM_DEBUG(dbgs() << "    DefMI is not LIi20 - cannot fold anything else yet\n");
    return false;
  }

  if (!MRI->hasOneNonDBGUse(Reg)) {
    LLVM_DEBUG(dbgs() << "    Reg " << Reg << " does not have one non-dbg use\n");
    return false;
  }

  unsigned Imm = DefMI.getOperand(1).getImm();

  if (isCompareInstr(UseMI)) {
    if ((Imm & 0x000007FFF) != Imm) {
      LLVM_DEBUG(dbgs() << "  Immediate is too large to fold into CMP*\n");
      return false;
    }

  } else if (isSelectInstr(UseMI)) {
    if ((Imm & 0x000007FF) != Imm) {
      LLVM_DEBUG(dbgs() << "  Immediate is too large to fold into SEL*\n");
      return false;
    }

    if (UseMI.getOperand(1).getReg() == Reg) {
      LLVM_DEBUG(dbgs() << "  Investigating $src1 reg of SEL*, tied to $dst\n");
      return false;
    }

  } else if (isArithBinOpInstr(UseMI)) {
    if ((Imm & 0x00007FFF) != Imm) {
      LLVM_DEBUG(dbgs() << "  Immediate is too large to fold into binary op\n");
      return false;
    }

    if (UseMI.getOperand(1).getReg() == Reg) {
      LLVM_DEBUG(dbgs() << "  Investigating $src1 reg of binary op, tied to $dst\n");
      return false;
    }

  } else {
    LLVM_DEBUG(dbgs() << "    Dont know how to fold\n");
    return false;
  }

  UseMI.setDesc(get(getRIVariant(UseMI)));
  UseMI.getOperand(2).ChangeToImmediate(Imm);

  LLVM_DEBUG(dbgs() << "  Folded: " << UseMI);
  return true;
}
