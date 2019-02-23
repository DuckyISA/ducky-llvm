//===--- Ducky.cpp - Implement Ducky target feature support ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements Ducky TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Ducky.h"
#include "Targets.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;
using namespace clang::targets;

const Builtin::Info DuckyTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS) \
  { #ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr },
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER) \
  { #ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr },
#include "clang/Basic/BuiltinsDucky.def"
};

bool DuckyTargetInfo::isValidCPUName(StringRef Name) const {
  return false;
  /*
  return llvm::StringSwitch<bool>(Name)
      .Case("mvp", true)
      .Case("bleeding-edge", true)
      .Case("generic", true)
      .Default(false);
  */
}

void DuckyTargetInfo::getTargetDefines(const LangOptions &Opts,
                                             MacroBuilder &Builder) const {
  Builder.defineMacro("__DUCKY__");
}

bool DuckyTargetInfo::handleTargetFeatures(
    std::vector<std::string> &Features, DiagnosticsEngine &Diags) {
  for (const auto &Feature : Features) {
    Diags.Report(diag::err_opt_not_valid_with_opt)
        << Feature << "-target-feature";
    return false;
  }
  return true;
}

ArrayRef<Builtin::Info> DuckyTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, clang::Ducky::LastTSBuiltin - Builtin::FirstTSBuiltin);
}

TargetInfo::CallingConvCheckResult DuckyTargetInfo::checkCallingConvention(CallingConv CC) const
{
  switch (CC) {
    default:
      return CCCR_Warning;
    case CC_C:
    case CC_PreserveMost:
    case CC_PreserveAll:
      return CCCR_OK;
  }
}
