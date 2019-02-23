//===-- DuckyFrameLowering.cpp - Frame info for Ducky Target --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains Ducky frame information that doesn't fit anywhere else
// cleanly...
//
//===----------------------------------------------------------------------===//

#include "DuckyFrameLowering.h"
#include "Ducky.h"
#include "DuckyInstrInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Debug.h"
#include <algorithm> // std::sort

#define DEBUG_TYPE "Ducky"

using namespace llvm;

//===----------------------------------------------------------------------===//
// DuckyFrameLowering:
//===----------------------------------------------------------------------===//

DuckyFrameLowering::DuckyFrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 4, 0) {
}

bool DuckyFrameLowering::hasFP(const MachineFunction &MF) const
{
  // We always have FP
  return true;
}

/*
 * Compute size of space required on stack for the function, and align it.
 */
uint64_t DuckyFrameLowering::computeStackSize(MachineFunction &MF) const
{
  MachineFrameInfo &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  unsigned StackAlign = getStackAlignment();

  return (StackAlign > 0 ? alignTo(StackSize, StackAlign) : StackSize);
}


/*
 * Materialize an offset for a ADD/SUB stack operation.
 * Return zero if the offset fits into the instruction as an immediate,
 * or the number of the register where the offset is materialized.
 */
static unsigned materializeOffset(
    MachineFunction &MF,
    MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MBBI,
    unsigned Offset) {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // ADD/SUB are "R" encoding, i.e. 15-bit immediate
  if (Offset <= 0x7FFF)
    return 0;

  unsigned OffsetReg = Ducky::R8;

  if ((Offset & 0x000FFFFF) == Offset) {
    // Offset can fit into one LI instruction
    BuildMI(MBB, MBBI, dl, TII.get(Ducky::LIi20), OffsetReg).addImm(Offset & 0x000FFFFF).setMIFlag(MachineInstr::FrameSetup);

  } else if ((Offset & 0xFFFF0000) == Offset) {
    // Offset can fit into LIU instruction
    BuildMI(MBB, MBBI, dl, TII.get(Ducky::LIUi16), OffsetReg).addImm(Offset & 0xFFFF0000).setMIFlag(MachineInstr::FrameSetup);

  } else {
    // Offset is so weirdly shaped we have to use both LI and LIU
    // to materialize it
    BuildMI(MBB, MBBI, dl, TII.get(Ducky::LIi20),  OffsetReg).addImm(Offset & 0x0000FFFF).setMIFlag(MachineInstr::FrameSetup);
    BuildMI(MBB, MBBI, dl, TII.get(Ducky::LIUi16), OffsetReg).addReg(OffsetReg).addImm(Offset >> 16).setMIFlag(MachineInstr::FrameSetup);
  }

  return OffsetReg;
}


/*
 * Adjust SP to gain (or release) necessary space. Used by ADJUST* pseudo
 * instructions to manipulate space for vararg buffer.
 */
static void emitSPAdjustment(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, uint64_t Bytes, bool reserve)
{
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  unsigned StackReg = Ducky::SP;
  unsigned OffsetReg = materializeOffset(MF, MBB, MBBI, Bytes);
  unsigned Opcode;

  if (reserve) {
    Opcode = (OffsetReg ? Ducky::SUBrr : Ducky::SUBri);
  } else {
    Opcode = (OffsetReg ? Ducky::ADDrr : Ducky::ADDri);
  }

  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  if (OffsetReg) {
    BuildMI(MBB, MBBI, dl, TII.get(Opcode), Ducky::SP)
      .addReg(StackReg)
      .addReg(OffsetReg)
      .setMIFlag(MachineInstr::FrameSetup);
  } else {
    BuildMI(MBB, MBBI, dl, TII.get(Opcode), StackReg)
        .addReg(StackReg)
        .addImm(Bytes)
        .setMIFlag(MachineInstr::FrameSetup);
  }
}


/*
 * If function requires some stack space, we have to adjust SP in its prolog.
 * Emit necessary instructions.
 */
void DuckyFrameLowering::emitPrologue(
    MachineFunction &MF,
    MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  uint64_t StackSize = computeStackSize(MF);

  // If required stack size is zero, we don't have to adjust SP at all
  if (!StackSize)
    return;

  emitSPAdjustment(MF, MBB, MBBI, (unsigned)StackSize, true);
}

/*
 * If function requires some stack space, we have to adjust SP in its epilog.
 * Emit necessary instructions.
 */
void DuckyFrameLowering::emitEpilogue(
    MachineFunction &MF,
    MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc dl = MBBI->getDebugLoc();
  uint64_t StackSize = computeStackSize(MF);

  if (!StackSize)
    return;

  emitSPAdjustment(MF, MBB, MBBI, (unsigned)StackSize, false);
}

/*
 * Eliminate remaining pseudo instructions.
 */
MachineBasicBlock::iterator DuckyFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF,
    MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const
{
  MachineInstr &MI = *I;

  if (MI.getOpcode() == Ducky::ADJCALLSTACKDOWN || MI.getOpcode() == Ducky::ADJCALLSTACKUP) {
    int Size = MI.getOperand(0).getImm();
    if (Size) {
      if (MI.getOpcode() == Ducky::ADJCALLSTACKDOWN)
        emitSPAdjustment(MF, MBB, I, (unsigned)Size, true);
      else
        emitSPAdjustment(MF, MBB, I, (unsigned)Size, false);
    }

    return MBB.erase(I);
  }

  llvm_unreachable("Unknown pseudo instruction");
}


/*
 * Determine which - if any - of callee saved registers needs actual spill.
 */
void DuckyFrameLowering::determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs, RegScavenger *RS) const
{
  LLVM_DEBUG(dbgs() << "determineCalleeSaves: MF=" << MF.getName() << '\n');

  SavedRegs.clear();
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);

  if (MF.getFunction().getCallingConv() == CallingConv::C) {
    LLVM_DEBUG(dbgs() << "  default CC, let it go\n");
    return;
  }

  const FunctionType *FT = MF.getFunction().getFunctionType();

  LLVM_DEBUG(dbgs() << "  num params: " << FT->getNumParams() << '\n');
  for (unsigned i = 0, reg = Ducky::R0; i < FT->getNumParams(); i++, reg++) {
    SavedRegs.reset(reg);
  }

  if (!FT->getReturnType()->isVoidTy()) {
    LLVM_DEBUG(dbgs() << "  return type is not void, avoid restoring R0\n");
    SavedRegs.reset(Ducky::R0);
  }
}
