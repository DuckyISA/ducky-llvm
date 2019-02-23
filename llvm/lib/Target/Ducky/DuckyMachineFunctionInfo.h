#ifndef DuckyMACHINEFUNCTIONINFO_H
#define DuckyMACHINEFUNCTIONINFO_H

#include "MCTargetDesc/DuckyMCTargetDesc.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

namespace llvm {

class DuckyMachineFunctionInfo final : public MachineFunctionInfo {
  MachineFunction &MF;

  unsigned VarArgFrameIndex = 0;

public:
  explicit DuckyMachineFunctionInfo(MachineFunction &MF) : MF(MF) {}
  ~DuckyMachineFunctionInfo() override;

  unsigned getVarArgFrameIndex() const
  {
    return VarArgFrameIndex;
  }

  void setVarArgFrameIndex(unsigned FI)
  {
    VarArgFrameIndex = FI;
  }
};

} // end namespace llvm

#endif
