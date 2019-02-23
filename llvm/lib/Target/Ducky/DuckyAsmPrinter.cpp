//===-- DuckyAsmPrinter.cpp - Ducky LLVM assembly writer ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from internal representation
// of machine-dependent LLVM code to the Ducky assembly language.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "Ducky.h"
#include "InstPrinter/DuckyInstPrinter.h"
#include "DuckyInstrInfo.h"
#include "DuckyMCInstLower.h"
#include "DuckySubtarget.h"
#include "DuckyTargetMachine.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include <algorithm>
#include <cctype>
using namespace llvm;

namespace {
class DuckyAsmPrinter : public AsmPrinter {
  DuckyMCInstLower MCInstLowering;

public:
  explicit DuckyAsmPrinter(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), MCInstLowering(OutContext, *this) {}

  virtual StringRef getPassName() const { return "Ducky Assembly Printer"; }

  void EmitFunctionEntryLabel();
  void EmitInstruction(const MachineInstr *MI);
//  void EmitFunctionBodyStart();
//  void EmitGlobalVariable(const GlobalVariable *GV);

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNum,
                       const char *ExtraCode, raw_ostream &OS) override;

private:
  void printOperand(const MachineInstr *MI, unsigned OpNo, raw_ostream &O,
                          const char *Modifier = 0);


//  void emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute);
//  void emitAlignment(unsigned const NumBits) const;
//  void emitSwitchSection(MCSection *Section) const;
//  void emitType(StringRef Name, const Constant *CV, uint64_t Size) const;
};
} // end of anonymous namespace

/*
void DuckyAsmPrinter::EmitFunctionBodyStart() {
  //MCInstLowering.Initialize(&MF->getContext());
}
*/

void DuckyAsmPrinter::EmitFunctionEntryLabel() {
  OutStreamer->EmitLabel(CurrentFnSym);
}

void DuckyAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);

  EmitToStreamer(*OutStreamer, TmpInst);
}

void DuckyAsmPrinter::printOperand(const MachineInstr *MI, unsigned OpNo,
                                   raw_ostream &O, const char *Modifier) {
  const MachineOperand &MO = MI->getOperand(OpNo);

  LLVM_DEBUG(dbgs() << "DuckyAsmPrinter::printOperand: MO=" << MO << '\n');

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << DuckyInstPrinter::getRegisterName(MO.getReg());
    //O << AVRInstPrinter::getPrettyRegisterName(MO.getReg(), MRI);
    break;
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;
  case MachineOperand::MO_GlobalAddress:
    O << getSymbol(MO.getGlobal());
    break;
  case MachineOperand::MO_ExternalSymbol:
    O << *GetExternalSymbolSymbol(MO.getSymbolName());
    break;
  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    break;
  default:
    llvm_unreachable("Not implemented yet!");
  }
}

bool DuckyAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNum,
                                      const char *ExtraCode, raw_ostream &O)
{
  LLVM_DEBUG(dbgs() << "DuckyAsmPrinter::PrintAsmOperand: MI=" << *MI << ", OpNum=" << OpNum << '\n');

  bool Error = AsmPrinter::PrintAsmOperand(MI, OpNum, ExtraCode, O);

  if (Error && ExtraCode && ExtraCode[0]) {
    LLVM_DEBUG(dbgs() << "  ExtraCode[0]=" << ExtraCode[0] << '\n');
  }

  if (Error)
    printOperand(MI, OpNum, O);

  return false;
}


/*
static unsigned getGVAlignmentLog2(const GlobalValue *GV, const DataLayout &DL,
                                   unsigned InBits = 0)
{
  unsigned NumBits = 0;

  if (const GlobalVariable *GVar = dyn_cast<GlobalVariable>(GV))
    NumBits = DL.getPreferredAlignmentLog(GVar);

  if (InBits > NumBits)
    NumBits = InBits;

  if (GV->getAlignment() == 0)
    return NumBits;

  unsigned GVAlign = Log2_32(GV->getAlignment());

  if (GVAlign > NumBits || GV->hasSection())
    NumBits = GVAlign;

  return NumBits;
}
*/

/*
 * Greatly simplified version of AsmStreamer:EmitSymbolAttribute
 */
/*
void DuckyAsmPrinter::emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute)
{
  if (Attribute == MCSA_Invalid)
    llvm_unreachable("Invalid symbol attribute");

  if (Attribute == MCSA_Global) {
    OutStreamer->EmitRawText(Twine(MAI->getGlobalDirective()) + Twine(Symbol->getName()));
    return;
  }
}
*/

/*
void DuckyAsmPrinter::emitAlignment(unsigned const NumBits) const
{
  SmallString<128> Str;
  raw_svector_ostream O(Str);

  O << "\t.align\t" << ((int)pow(2.0, NumBits));
  OutStreamer->EmitRawText(O.str());
}
*/

/*
void DuckyAsmPrinter::emitSwitchSection(MCSection *Section) const
{
  const MCSectionELF *ELFSection = ((const MCSectionELF*)Section);
  const SectionKind K = Section->getKind();
  StringRef Name = ELFSection->getSectionName(), Flags = StringRef("l");

  // For some reason, .data has ReadOnly Kind - don't know why (yet) :/
  if (Name.equals(StringRef(".data"))) {
    Flags = StringRef("rwl");
  } else if (Name.equals(StringRef(".bss"))) {
    Flags = StringRef("rwlb");
  } else {
    if (K.isText())
      Flags = StringRef("rxl");

    else if (K.isReadOnly())
      Flags = StringRef("rl");

    else
      Flags = StringRef("rwl");
  }

  OutStreamer->EmitRawText(StringRef("\t.section\t") + ELFSection->getSectionName() + ", \"" + Flags + "\"");
}
*/

/*
void DuckyAsmPrinter::emitType(StringRef Name, const Constant *CV, uint64_t Size) const
{
#define doEmitType(_type) OutStreamer->EmitRawText("\t.type\t" + Twine(Name) + ", " + _type)

  if (dyn_cast<ConstantInt>(CV)) {
    switch(Size) {
      case 1:
        doEmitType("byte");
        break;
      case 2:
        doEmitType("short");
        break;
      case 4:
        doEmitType("word");
        break;
      default:
        llvm_unreachable("Unhandled integer size");
    }
    return;
  }

  if (const ConstantDataSequential *CDS = dyn_cast<ConstantDataSequential>(CV)) {
    if (CDS->isString()) {
      if (CDS->getAsString().back() == 0) {
        doEmitType("string");
        return;
      }

      doEmitType("ascii");
      return;
    }

    llvm_unreachable("Unhandled constant data sequential type variant");
  }

  if (dyn_cast<ConstantAggregateZero>(CV)) {
    doEmitType("space");
    return;
  }

  if (dyn_cast<ConstantPointerNull>(CV)) {
    doEmitType("word");
    return;
  }

#undef doEmitType

  OutStreamer->EmitRawText(Twine(Name) + ":");
}
*/
/*
void DuckyAsmPrinter::EmitGlobalVariable(const GlobalVariable *GV)
{
  MCSymbol *GVSym = getSymbol(GV);

  LLVM_DEBUG(dbgs() << "#DB.EmitGlobalVariable: GV=" << GVSym->getName() << '\n');

  // If symbol has no initializer, there's nothing else to do
  if (!GV->hasInitializer()) {
    LLVM_DEBUG(dbgs() << "#DB.EmitGlobalVariable:   no initializer\n");
    return;
  }

  const DataLayout &DL = GV->getParent()->getDataLayout();
  SectionKind GVKind = TargetLoweringObjectFile::getKindForGlobal(GV, TM);
  MCSection *Section = getObjFileLowering().SectionForGlobal(GV, GVKind, TM);
  const Constant *CV = GV->getInitializer();
  //uint64_t Size = DL.getTypeAllocSize(CV->getType());

  // Switch to the correct section
  emitSwitchSection(Section);

  // Emit attributes, like .global
  emitSymbolAttribute(GVSym, MCSA_Global);

  // Don't redefine symbols
  GVSym->redefineIfPossible();
  if (GVSym->isDefined() || GVSym->isVariable())
    report_fatal_error("symbol '" + Twine(GVSym->getName()) + "' is already defined");

  // Force alignment
  emitAlignment(getGVAlignmentLog2(GV, DL));

  // Emit .type <name>, <type> directive
  // emitType(GVSym->getName(), CV, Size);

  // Emit label
  OutStreamer->EmitRawText(Twine(GVSym->getName()) + ":");

  // Emit initializer
  EmitGlobalConstant(DL, CV);

  OutStreamer->AddBlankLine();
}
*/

// Force static initialization.
extern "C" void LLVMInitializeDuckyAsmPrinter() {
  RegisterAsmPrinter<DuckyAsmPrinter> X(getTheDuckyTarget());
}
