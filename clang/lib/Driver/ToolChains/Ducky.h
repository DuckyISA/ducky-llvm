//===--- Ducky.h - Ducky ToolChain Implementations --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WEBASSEMBLY_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WEBASSEMBLY_H

#include "Gnu.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"


namespace clang {
namespace driver {
namespace tools {

namespace Ducky {
class LLVM_LIBRARY_VISIBILITY Assembler : public GnuTool {
  public:
    Assembler(const ToolChain &TC)
      : GnuTool("Ducky::Assembler", "assembler", TC) {}

    bool hasIntegratedCPP() const override { return false; }

 void ConstructJob(Compilation &C, const JobAction &JA,
     const InputInfo &Output, const InputInfoList &Inputs,
     const llvm::opt::ArgList &TCArgs,
     const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker : public GnuTool {
  public:
    Linker(const ToolChain &TC)
      : GnuTool("Ducky::Linker", "ld.lld", TC) {}

    bool isLinkJob() const override;
    bool hasIntegratedCPP() const override;
    void ConstructJob(Compilation &C, const JobAction &JA,
        const InputInfo &Output, const InputInfoList &Inputs,
        const llvm::opt::ArgList &TCArgs,
        const char *LinkingOutput) const override;
};

} // end namespace Ducky
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY DuckyToolChain final : public ToolChain {
public:
  DuckyToolChain(const Driver &D, const llvm::Triple &Triple,
                 const llvm::opt::ArgList &Args);

private:
  bool IsMathErrnoDefault() const override;
  bool IsObjCNonFragileABIDefault() const override;
  bool UseObjCMixedDispatch() const override;
  bool isPICDefault() const override;
  bool isPIEDefault() const override;
  bool isPICDefaultForced() const override;
  bool IsIntegratedAssemblerDefault() const override;
  bool hasBlocksRuntime() const override;
  bool SupportsProfiling() const override;
  bool HasNativeLLVMSupport() const override;
  RuntimeLibType GetDefaultRuntimeLibType() const override;
  CXXStdlibType GetCXXStdlibType(const llvm::opt::ArgList &Args) const override;

  const char *getDefaultLinker() const override {
    return "ld.lld";
  }

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WEBASSEMBLY_H
