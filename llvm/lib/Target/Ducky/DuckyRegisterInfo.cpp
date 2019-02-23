//===-- DuckyRegisterInfo.cpp - Ducky Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Ducky implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "DuckyRegisterInfo.h"
#include "Ducky.h"
#include "DuckyFrameLowering.h"
#include "DuckyInstrInfo.h"
#include "DuckyMachineFunctionInfo.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/Debug.h"

#define GET_REGINFO_TARGET_DESC
#include "DuckyGenRegisterInfo.inc"

#define DEBUG_TYPE "ducky"


using namespace llvm;

DuckyRegisterInfo::DuckyRegisterInfo() : DuckyGenRegisterInfo(Ducky::FP) {}

static const uint16_t CalleeSavedRegs_Ducky[] = {
  Ducky::R8,  Ducky::R9,  Ducky::R10, Ducky::R11, Ducky::R12, Ducky::R13, Ducky::R14, Ducky::R15,
  Ducky::R16, Ducky::R17, Ducky::R18, Ducky::R19, Ducky::R20, Ducky::R21, Ducky::R22, Ducky::R23,
  Ducky::R24, Ducky::R25, Ducky::R26, Ducky::R27, Ducky::R28, Ducky::R29,
  0
};

static const uint16_t CalleeSavedRegs_PreserveAll[] = {
  Ducky::R0,  Ducky::R1,  Ducky::R2,  Ducky::R3,  Ducky::R4,  Ducky::R5,  Ducky::R6,  Ducky::R7,
  Ducky::R8,  Ducky::R9,  Ducky::R10, Ducky::R11, Ducky::R12, Ducky::R13, Ducky::R14, Ducky::R15,
  Ducky::R16, Ducky::R17, Ducky::R18, Ducky::R19, Ducky::R20, Ducky::R21, Ducky::R22, Ducky::R23,
  Ducky::R24, Ducky::R25, Ducky::R26, Ducky::R27, Ducky::R28, Ducky::R29,
  0
};

static const uint16_t CalleeSavedRegs_PreserveMost[] = {
  Ducky::R0,  Ducky::R1,  Ducky::R2,  Ducky::R3,  Ducky::R4,  Ducky::R5,  Ducky::R6,  Ducky::R7,
  Ducky::R8,  Ducky::R9,  Ducky::R10, Ducky::R11, Ducky::R12, Ducky::R13, Ducky::R14, Ducky::R15,
  Ducky::R16, Ducky::R17, Ducky::R18, Ducky::R19, Ducky::R20, Ducky::R21, Ducky::R22, Ducky::R23,
  Ducky::R24, Ducky::R25, Ducky::R26, Ducky::R27, Ducky::R28, Ducky::R29,
  0
};

const uint16_t *
DuckyRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  switch (MF->getFunction().getCallingConv()) {
    default:
      return CalleeSavedRegs_Ducky;
    case CallingConv::PreserveMost:
      return CalleeSavedRegs_PreserveMost;
    case CallingConv::PreserveAll:
      return CalleeSavedRegs_PreserveAll;
  }
}

BitVector DuckyRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(Ducky::SP);
  Reserved.set(Ducky::FP);
  return Reserved;
}

const uint32_t *DuckyRegisterInfo::getCallPreservedMask(const MachineFunction &MF, CallingConv::ID CC) const
{
  switch (CC) {
    default:
      return CC_Save_Ducky_RegMask;
    case CallingConv::PreserveMost:
      return CC_Save_Most_RegMask;
    case CallingConv::PreserveAll:
      return CC_Save_All_RegMask;
  }
}

bool
DuckyRegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const {
  return true;
}

bool
DuckyRegisterInfo::trackLivenessAfterRegAlloc(const MachineFunction &MF) const {
  return true;
}

bool DuckyRegisterInfo::useFPForScavengingIndex(const MachineFunction &MF) const {
  return false;
}


static int computeFrameIndexOffset(MachineInstr &MI, const MachineFunction &MF, const MachineOperand &FIOperand, int Offset = 0)
{
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  int FrameIndex = FIOperand.getIndex();
  int ObjectOffset = MFI.getObjectOffset(FrameIndex);
  int StackSize = MFI.getStackSize();

  LLVM_DEBUG(dbgs() << "#DB.computeFrameIndexOffset: FrameIndex=" << FrameIndex << ", ObjectOffset=" << ObjectOffset << ", StackSize=" << StackSize << ", Offset=" << Offset << '\n');

  return ObjectOffset + StackSize + Offset;
}

void DuckyRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  MachineInstr &MI = *II;
  const MachineFunction &MF = *MI.getParent()->getParent();

  LLVM_DEBUG(dbgs() << "#DB.eliminateFrameIndex:         MI:"; MI.dump());

  if (MI.getOpcode() == Ducky::ADDri) {
    /*
     * ADDri <tfi>, <offset> comes from DAG-to-DAG's SelectFrameIndex,
     * and basically means "load effective address of frame index #FI
     * to a register". This requires sequence of two instructions,
     * `mov $dst, sp; add sp, #FI`.
     */
    LLVM_DEBUG(dbgs() << "#DB.eliminateFrameIndex: load effective address\n");

    const MachineFunction &MF = *MI.getParent()->getParent();
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    DebugLoc dl = MI.getDebugLoc();

    int Offset = computeFrameIndexOffset(MI, MF, MI.getOperand(FIOperandNum));
    LLVM_DEBUG(dbgs() << "#DB.eliminateFrameIndex: offset=" << Offset << '\n');

    MI.setDesc(TII->get(Ducky::MOVrr));
    MI.getOperand(FIOperandNum).ChangeToRegister(Ducky::SP, false);

    LLVM_DEBUG(dbgs() << "#DB.eliminateFrameIndex: updated MI:"; MI.dump());

    if (Offset == 0)
      return;

    MachineBasicBlock &MBB = *MI.getParent();
    unsigned DstReg = MI.getOperand(0).getReg();

    MachineInstr *LEA = nullptr;

    if (Offset < 0) {
      LEA = BuildMI(MBB, std::next(II), dl, TII->get(Ducky::SUBri), DstReg).addReg(DstReg).addImm(-Offset).getInstr();
    } else {
      LEA = BuildMI(MBB, std::next(II), dl, TII->get(Ducky::ADDri), DstReg).addReg(DstReg).addImm(Offset).getInstr();
    }

    assert(LEA != nullptr && "LEA cannot be NULL");
    LLVM_DEBUG(dbgs() << "#DB.eliminateFrameIndex:        LEA:"; LEA->dump(); dbgs() << '\n');

    return;
  }

  switch (MI.getOpcode()) {
    default:
      llvm_unreachable("#DB.eliminateFrameIndex: not supported");

    case Ducky::LW:
    case Ducky::LS:
    case Ducky::LB:
    case Ducky::STW:
    case Ducky::STS:
    case Ducky::STB:
      break;
  }

  int Offset = computeFrameIndexOffset(MI, MF, MI.getOperand(FIOperandNum), MI.getOperand(FIOperandNum + 1).getImm());

  MI.getOperand(FIOperandNum).ChangeToRegister(Ducky::SP, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);

  LLVM_DEBUG(dbgs() << "#DB.eliminateFrameIndex: updated MI:"; MI.dump(); dbgs() << '\n');
}

Register DuckyRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Ducky::SP;
}
