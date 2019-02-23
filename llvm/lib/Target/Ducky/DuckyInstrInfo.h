//===-- DuckyInstrInfo.h - Ducky Instruction Information --------*- C++ -*-===//
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

#ifndef DuckyINSTRUCTIONINFO_H
#define DuckyINSTRUCTIONINFO_H

#include "DuckyRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "DuckyGenInstrInfo.inc"

// See DuckyInstrInfo.td, CondBranch class: (ins StatusRegs:$flags, brtarget:$addr)
#define BRCC_BLOCK_OPERAND_INDEX 0

namespace llvm {

class DuckyInstrInfo : public DuckyGenInstrInfo {
  const DuckyRegisterInfo RI;
  virtual void anchor();

public:
  DuckyInstrInfo();

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const DuckyRegisterInfo &getRegisterInfo() const { return RI; }

  unsigned isLoadFromStackSlot(const MachineInstr &MI, int &FrameIndex) const override;
  unsigned isStoreToStackSlot(const MachineInstr &MI, int &FrameIndex) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
      MachineBasicBlock *&FBB, SmallVectorImpl<MachineOperand> &Cond, bool AllowModify) const override;
  unsigned removeBranch(MachineBasicBlock &MBB, int *BytesRemoved = nullptr) const override;
  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
      MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
      const DebugLoc &DL,
      int *BytesAdded = nullptr) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
      const DebugLoc &DL, unsigned DestReg, unsigned SrcReg,
      bool KillSrc) const override;

  virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MI,
                                   unsigned SrcReg, bool isKill, int FrameIndex,
                                   const TargetRegisterClass *RC,
                                   const TargetRegisterInfo *TRI) const
      override;

  virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MI,
                                    unsigned DestReg, int FrameIndex,
                                    const TargetRegisterClass *RC,
                                    const TargetRegisterInfo *TRI) const
      override;

  bool FoldImmediate(MachineInstr &UseMI, MachineInstr &DefMI, unsigned Reg, MachineRegisterInfo *MRI) const override;
};

union __attribute__ ((packed)) DuckyBranchTargetEncoding {
  struct {
    uint64_t immediate_flag:1;
    uint64_t reg:5;
    uint64_t immediate:58;
  } split;
  uint64_t overall;
};

}

#endif
