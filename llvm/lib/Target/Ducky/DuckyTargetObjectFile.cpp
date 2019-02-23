//===-- DuckyTargetObjectFile.cpp - Ducky object files --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DuckyTargetObjectFile.h"
#include "DuckySubtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;


void DuckyTargetObjectFile::Initialize(MCContext &Ctx, const TargetMachine &TM){
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
 }


MCSection * DuckyTargetObjectFile::SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const
{
  if (Kind.isCommon())
    return BSSSection;

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, Kind, TM);
}
