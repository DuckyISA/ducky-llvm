//=== Ducky.h - Declare Ducky target feature support *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares Ducky TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_WEBASSEMBLY_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_WEBASSEMBLY_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY DuckyTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

public:
  explicit DuckyTargetInfo(const llvm::Triple &T, const TargetOptions &)
      : TargetInfo(T) {

        resetDataLayout("e-S32-m:e-p:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-a:0:32-n32");

        BigEndian = false;
        TLSSupported = false;
        NoAsmVariants = true;
        HasFloat128 = false;
        PointerWidth = PointerAlign = 32;
        BoolWidth = BoolAlign = 8;
        IntWidth = IntAlign = 32;
        HalfWidth = HalfAlign = 16;
        LongLongWidth = LongLongAlign = 64;
        SuitableAlign = 32;
        DefaultAlignForAttributeAligned = 32;
        MinGlobalAlign = 0;

        SizeType = UnsignedInt;
        IntMaxType = SignedLongLong;;
        PtrDiffType = SignedInt;
        IntPtrType = SignedInt;
        WCharType = UnsignedChar;
        WIntType = UnsignedInt;

        UseZeroLengthBitfieldAlignment = true;
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

private:
  bool
  initFeatureMap(llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override {
    return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
  }

  bool hasFeature(StringRef Feature) const final { return false; };

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) final;

  bool isValidCPUName(StringRef Name) const final;

  bool setCPU(const std::string &Name) final { return isValidCPUName(Name); }

  ArrayRef<Builtin::Info> getTargetBuiltins() const final;

  BuiltinVaListKind getBuiltinVaListKind() const final {
    return VoidPtrBuiltinVaList;
  }

  ArrayRef<const char *> getGCCRegNames() const final { return None; }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const final {
    return None;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const final {
    return false;
  }

  const char *getClobbers() const final { return ""; }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override;
};
} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_WEBASSEMBLY_H
