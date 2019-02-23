//===-- DuckyFrameLowering.h - Frame info for Ducky Target ------*- C++ -*-===//
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

#ifndef DuckyFRAMEINFO_H
#define DuckyFRAMEINFO_H

#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class DuckySubtarget;

class DuckyFrameLowering : public TargetFrameLowering {
public:
  DuckyFrameLowering();

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs, RegScavenger *RS = NULL) const override;

  bool hasFP(const MachineFunction &MF) const;

  MachineBasicBlock::iterator eliminateCallFramePseudoInstr(
      MachineFunction &MF,
      MachineBasicBlock &MBB,
      MachineBasicBlock::iterator I) const override;

  static int stackSlotSize() { return 4; }

private:
  uint64_t computeStackSize(MachineFunction &MF) const;
};
}

#endif // DuckyFRAMEINFO_H

