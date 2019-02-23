//===-- DuckyTargetMachine.h - Define TargetMachine for Ducky ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Ducky specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef DuckyTARGETMACHINE_H
#define DuckyTARGETMACHINE_H

#include "Ducky.h"
#include "DuckyFrameLowering.h"
#include "DuckyISelLowering.h"
#include "DuckyInstrInfo.h"
#include "DuckySelectionDAGInfo.h"
#include "DuckySubtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/TargetPassConfig.h"

namespace llvm {

class DuckyTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  DuckySubtarget Subtarget;

public:
  DuckyTargetMachine(const Target &T, const Triple &TT,
                     StringRef CPU, StringRef FS,
                     const TargetOptions &Options,
                     Optional<Reloc::Model> RM,
                     Optional<CodeModel::Model> CM,
                     CodeGenOpt::Level OL, bool JIT);
  
  const DuckySubtarget * getSubtargetImpl() const {
    return &Subtarget;
  }
  
  virtual const TargetSubtargetInfo *
  getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // end namespace llvm

#endif
