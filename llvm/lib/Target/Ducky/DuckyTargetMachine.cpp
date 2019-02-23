//===-- DuckyTargetMachine.cpp - Define TargetMachine for Ducky -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "DuckyTargetMachine.h"
#include "Ducky.h"
#include "DuckyFrameLowering.h"
#include "DuckyInstrInfo.h"
#include "DuckyISelLowering.h"
#include "DuckySelectionDAGInfo.h"
#include "DuckyTargetObjectFile.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

static std::string computeDataLayout(const Triple &TT, StringRef CPU, StringRef FS) {
  // XXX Build the triple from the arguments.
  // This is hard-coded for now for this example target.
  return "e-S32-m:e-p:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-a:0:32-n32";
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

DuckyTargetMachine::DuckyTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Optional<Reloc::Model> RM,
                                       Optional<CodeModel::Model> CM,
                                       CodeGenOpt::Level OL, bool JIT)
  : LLVMTargetMachine(
      T, computeDataLayout(TT, CPU, FS), TT, CPU, FS, Options,
      getEffectiveRelocModel(RM),
      getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

namespace {
  class DuckyPassConfig : public TargetPassConfig {
    public:
      DuckyPassConfig(DuckyTargetMachine &TM, PassManagerBase &PM)
        : TargetPassConfig(TM, PM) {}

      DuckyTargetMachine &getDuckyTargetMachine() const {
        return getTM<DuckyTargetMachine>();
      }

      bool addPreISel() override;
      bool addInstSelector() override;
      void addPreEmitPass() override;
  };
} // namespace

TargetPassConfig *DuckyTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new DuckyPassConfig(*this, PM);
}

bool DuckyPassConfig::addPreISel() { return false; }

bool DuckyPassConfig::addInstSelector() {
  addPass(createDuckyISelDag(getDuckyTargetMachine(), getOptLevel()));
  return false;
}

void DuckyPassConfig::addPreEmitPass() {}

// Force static initialization.
extern "C" void LLVMInitializeDuckyTarget() {
  RegisterTargetMachine<DuckyTargetMachine> X(getTheDuckyTarget());
}
