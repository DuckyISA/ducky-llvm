//===-- DuckyInstPrinter.cpp - Convert Ducky MCInst to assembly syntax ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an Ducky MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "DuckyInstPrinter.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"

using namespace llvm;

#include "DuckyGenAsmWriter.inc"


DuckyInstPrinter::DuckyInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII, const MCRegisterInfo &MRI)
  : MCInstPrinter(MAI, MII, MRI) {}


/*
 * Print a register name, properly formatted.
 */
void DuckyInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const
{
  OS << StringRef(getRegisterName(RegNo)).lower();
}

void DuckyInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                               StringRef Annot, const MCSubtargetInfo &STI) {
  LLVM_DEBUG(dbgs() << "DB.printInst: MI=" << *MI << '\n');

  printInstruction(MI, O);
  printAnnotation(O, Annot);
}


/*
 * Print "expression" operand - so far it looks like the only possible one
 * in Ducky world is branch target in a form of symbol name.
 */
/*
static void printExpr(const MCAsmInfo &MAI, const MCExpr *Expr, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "#DB.printExpr: Expr='" << *Expr; dbgs() << "'\n");

  Expr->print(OS, &MAI);
  return;

  const MCSymbolRefExpr *SRE = dyn_cast<MCSymbolRefExpr>(Expr);
  assert(SRE && "Unexpected MCExpr type");

  const MCSymbolRefExpr::VariantKind Kind = SRE->getKind();
  assert(Kind == MCSymbolRefExpr::VK_None && "Unexpected MCSymbolRef kind.");

  OS << SRE->getSymbol();
}
*/


/*
 * Print a memory target - (register, offset) pair.
 */
void DuckyInstPrinter::printMemoryTarget(const MCInst *MI, unsigned OpNum, raw_ostream &O)
{
  const MCOperand &Op1 = MI->getOperand(OpNum);
  const MCOperand &Op2 = MI->getOperand(OpNum + 1);

  LLVM_DEBUG(dbgs() << "DB.printMemorytarget: MI=" << *MI << ", Reg=" << Op1 << ", Offset=" << Op2 << '\n');

  printRegName(O, Op1.getReg());

  int64_t Offset = Op2.getImm();
  if (Offset) {
    O << "[";
    O << formatHex(Offset);
    O << "]";
  }
}


/*
 * Print a branch target - register, or an symbol name.
 */
void DuckyInstPrinter::printBranchTarget(const MCInst *MI, unsigned OpNum, raw_ostream &O)
{
  const MCOperand &Op = MI->getOperand(OpNum);

  LLVM_DEBUG(dbgs() << "DuckyInstPrinter::printBranchTarget: MI=" << *MI << ", OpNum=" << OpNum << '\n');

  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isExpr()) {
    Op.getExpr()->print(O, &MAI);
    return;
  }

  if (Op.isImm()) {
    O << formatv("{0:X}", Op.getImm());
    return;
  }

  llvm_unreachable("Unhandled branch target type");
}

void DuckyInstPrinter::printCondBranchTarget(const MCInst *MI, unsigned OpNum, raw_ostream &O)
{
  const MCOperand &Op = MI->getOperand(OpNum);

  LLVM_DEBUG(dbgs() << "DuckyInstPrinter::printBranchTarget: MI=" << *MI << ", OpNum=" << OpNum << '\n');

  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isExpr()) {
    Op.getExpr()->print(O, &MAI);
    return;
  }

  if (Op.isImm()) {
    O << formatv("{0:X}", Op.getImm());
    return;
  }

  llvm_unreachable("Unhandled branch target type");
}

/*
 * Generic method, handling printing of any type of operands.
 */
void DuckyInstPrinter::printOperand(const MCInst *MI, unsigned OpNum, raw_ostream &O)
{
  LLVM_DEBUG(dbgs() << "DuckyInstPrinter::printOperand: MI=" << *MI; dbgs() << ", OpNum=" << OpNum << " of " << MI->getNumOperands() << '\n');

  switch(MI->getOpcode()) {
    default:
      break;
    case Ducky::CMPrr:
    case Ducky::CMPri:
    case Ducky::CMPUrr:
    case Ducky::CMPUri:
      // when it comes from encoding
      if (MI->getNumOperands() == 2)
        OpNum -= 1;
      break;
  }

  const MCOperand &Op = MI->getOperand(OpNum);

  LLVM_DEBUG(dbgs() << "DuckyInstPrinter::printOperand: Op=" << Op << '\n');

  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
    switch(MI->getOpcode()) {
      case Ducky::LIi20:
      case Ducky::LIUi16:
        O << formatHex((uint64_t)Op.getImm() & 0xFFFFFFFF);
        break;
      default:
        O << formatHex(Op.getImm());
        break;
    }
    return;
  }

  if (Op.isExpr()) {
    Op.getExpr()->print(O, &MAI);
    return;
  }

  llvm_unreachable("Unknown operand kind in printOperand");
}
