//===--- Ducky.cpp - Ducky ToolChain Implementation -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Ducky.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "ducky-toolchain"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

void Ducky::Assembler::ConstructJob(Compilation &C, const JobAction &JA,
                                    const InputInfo &Output,
                                    const InputInfoList &Inputs,
                                    const ArgList &Args,
                                    const char *LinkingOutput) const {
  claimNoWarnArgs(Args);

  ArgStringList CmdArgs;

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  CmdArgs.push_back("-f");

  if (Args.hasArg(options::OPT_v))
    CmdArgs.push_back("-v");

  Args.AddAllArgValues(CmdArgs, options::OPT_Wa_COMMA, options::OPT_Xassembler);

  for (const auto &II : Inputs) {
    CmdArgs.push_back("-i");
    CmdArgs.push_back(II.getFilename());
  }

  const char *Exec = Args.MakeArgString(getToolChain().GetProgramPath("ducky-as"));
  C.addCommand(llvm::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
}

bool Ducky::Linker::isLinkJob() const {
  return true;
}

bool Ducky::Linker::hasIntegratedCPP() const {
  return false;
}

void Ducky::Linker::ConstructJob(Compilation &C, const JobAction &JA,
    const InputInfo &Output,
    const InputInfoList &Inputs,
    const ArgList &Args,
    const char *LinkingOutput) const {

  const ToolChain &ToolChain = getToolChain();
  const char *Linker = Args.MakeArgString(ToolChain.GetLinkerPath());

  ArgStringList CmdArgs;
  CmdArgs.push_back("--no-threads");

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  Args.AddAllArgs(CmdArgs, options::OPT_u);

//  Args.AddAllArgValues(CmdArgs, options::OPT_Wl_COMMA, options::OPT_Xlinker);

  ToolChain.AddFilePathLibArgs(Args, CmdArgs);

  AddLinkerInputs(ToolChain, Inputs, Args, CmdArgs, JA);

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  C.addCommand(llvm::make_unique<Command>(JA, *this, Linker, CmdArgs, Inputs));
}

DuckyToolChain::DuckyToolChain(const Driver &D, const llvm::Triple &Triple,
                               const llvm::opt::ArgList &Args)
  : ToolChain(D, Triple, Args) {
    // ProgramPaths are found via 'PATH' environment variable.
}

bool DuckyToolChain::IsMathErrnoDefault() const { return false; }

bool DuckyToolChain::IsObjCNonFragileABIDefault() const { return true; }

bool DuckyToolChain::UseObjCMixedDispatch() const { return true; }

bool DuckyToolChain::isPICDefault() const { return false; }

bool DuckyToolChain::isPIEDefault() const { return false; }

bool DuckyToolChain::isPICDefaultForced() const { return false; }

bool DuckyToolChain::IsIntegratedAssemblerDefault() const { return true; }

bool DuckyToolChain::hasBlocksRuntime() const { return false; }

// TODO: Support profiling.
bool DuckyToolChain::SupportsProfiling() const { return false; }

bool DuckyToolChain::HasNativeLLVMSupport() const { return true; }

/*
void DuckyToolChain::addClangTargetOptions(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args,
                                        Action::OffloadKind) const {
  if (DriverArgs.hasFlag(clang::driver::options::OPT_fuse_init_array,
                         options::OPT_fno_use_init_array, true))
    CC1Args.push_back("-fuse-init-array");
}
*/

ToolChain::RuntimeLibType DuckyToolChain::GetDefaultRuntimeLibType() const {
  return ToolChain::RLT_CompilerRT;
}

ToolChain::CXXStdlibType DuckyToolChain::GetCXXStdlibType(const ArgList &Args) const {
  return ToolChain::CST_Libcxx;
}

Tool *DuckyToolChain::buildAssembler() const {
  return new tools::Ducky::Assembler(*this);
}

Tool *DuckyToolChain::buildLinker() const {
  return new tools::Ducky::Linker(*this);
}
