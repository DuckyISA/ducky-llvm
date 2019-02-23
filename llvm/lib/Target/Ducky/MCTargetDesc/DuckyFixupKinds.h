//===-- DuckyFixupKinds.h - Ducky Specific Fixup Entries --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DUCKY_MCTARGETDESC_DUCKYFIXUPKINDS_H
#define LLVM_LIB_TARGET_DUCKY_MCTARGETDESC_DUCKYFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace Ducky {

// This table *must* be in the save order of
// MCFixupKindInfo Infos[Ducky::NumTargetFixupKinds]
// in DuckyAsmBackend.cpp.
enum Fixups {
  // High 20 bits, PC-relative, inaligned => R_DUCKY_PI_20
  fixup_Ducky_PI_20 = FirstTargetFixupKind,

  // High 20 bits, PC-relative, 4 byte aligned => R_DUCKY_PA_20
  fixup_Ducky_PA_20,

  // High 16 bits, PC-relative, 4 byte aligned => R_DUCKY_PA_PC16
  fixup_Ducky_PA_16,

  // Marker
  fixup_Ducky_invalid,
  NumTargetFixupKinds = fixup_Ducky_invalid - FirstTargetFixupKind
};
} // namespace Ducky
} // namespace llvm

#endif // LLVM_DUCKY_DUCKYFIXUPKINDS_H
