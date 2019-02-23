//===-- DuckyMCAsmInfo.cpp - Ducky asm properties -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DuckyMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

void DuckyMCAsmInfo::anchor() {}

DuckyMCAsmInfo::DuckyMCAsmInfo(const Triple &TT) {
  PrivateGlobalPrefix = ".L";
  PrivateLabelPrefix = ".L";

  // Data16bitsDirective = "\t.short\t";
  // Data32bitsDirective = "\t.word\t";
  Data64bitsDirective = 0;
  CommentString = ";";
  //ZeroDirective = "\t.space\t";
  //GlobalDirective = "\t.globl\t";
  //AscizDirective = "\t.string\t";
  WeakRefDirective = "\t.weak\t";

  SupportsDebugInformation = true;
  HasSingleParameterDotFile = true;
  HasDotTypeDotSizeDirective = true;

  HiddenVisibilityAttr = MCSA_Invalid;
  HiddenDeclarationVisibilityAttr = MCSA_Invalid;
  ProtectedVisibilityAttr = MCSA_Invalid;
}
