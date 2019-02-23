//===-- DuckyMCInstLower.cpp - Convert Ducky MachineInstr to MCInst -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file contains code to lower Ducky MachineInstrs to their
/// corresponding MCInst records.
///
//===----------------------------------------------------------------------===//
#include "DuckyMCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/Debug.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define DEBUG_TYPE "ducky"

using namespace llvm;

DuckyMCInstLower::DuckyMCInstLower(MCContext &ctx, AsmPrinter &asmprinter)
    : Ctx(ctx), Printer(asmprinter) {}

MCOperand DuckyMCInstLower::LowerSymbolOperand(const MachineOperand &MO) const
{
  const MCSymbol *Symbol;
  unsigned Offset = 0;

  LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: MO="; MO.print(dbgs(), MO.getParent()->getParent()->getParent()->getSubtarget().getRegisterInfo()); dbgs() << '\n');

  switch (MO.getType()) {
  case MachineOperand::MO_MachineBasicBlock:
    LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: Basic block\n");
    Symbol = MO.getMBB()->getSymbol();
    break;
  case MachineOperand::MO_GlobalAddress:
    LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: Global address\n");
    Symbol = Printer.getSymbol(MO.getGlobal());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_BlockAddress:
    LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: Block address\n");
    Symbol = Printer.GetBlockAddressSymbol(MO.getBlockAddress());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_ExternalSymbol:
    LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: External symbol\n");
    Symbol = Printer.GetExternalSymbolSymbol(MO.getSymbolName());
    Offset += MO.getOffset();
    break;
  case MachineOperand::MO_JumpTableIndex:
    LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: Jump table index\n");
    Symbol = Printer.GetJTISymbol(MO.getIndex());
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: Constant pool index\n");
    Symbol = Printer.GetCPISymbol(MO.getIndex());
    Offset += MO.getOffset();
    break;
  default:
    llvm_unreachable("<unknown operand type>");
  }

  LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: Symbol="; dbgs() << Symbol; dbgs() << ", Offset=" << Offset; dbgs() << '\n');

  const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::create(Symbol, MCSymbolRefExpr::VK_None, Ctx);

  LLVM_DEBUG(dbgs() << "DB.LowerOperandSymbol: MCSym="; dbgs() << MCSym; dbgs() << '\n');

  if (!Offset)
    return MCOperand::createExpr(MCSym);

  // Assume offset is never negative.
  assert(Offset > 0);

  const MCConstantExpr *OffsetExpr = MCConstantExpr::create(Offset, Ctx);
  const MCBinaryExpr *Add = MCBinaryExpr::createAdd(MCSym, OffsetExpr, Ctx);
  return MCOperand::createExpr(Add);
}


MCOperand DuckyMCInstLower::LowerOperand(const MachineOperand &MO, unsigned offset) const
{
  //MachineOperandType MOTy = MO.getType();

  LLVM_DEBUG(dbgs() << "DB.LowerOperand: MO="; MO.print(dbgs(), MO.getParent()->getParent()->getParent()->getSubtarget().getRegisterInfo()); dbgs() << '\n');

  switch (MO.getType()) {
    default:
      llvm_unreachable("unknown operand type");

    case MachineOperand::MO_Register:
      // Ignore all implicit register operands
      if (MO.isImplicit())
        break;

      return MCOperand::createReg(MO.getReg());

    case MachineOperand::MO_Immediate:
      return MCOperand::createImm(MO.getImm() + offset);

    case MachineOperand::MO_MachineBasicBlock:
    case MachineOperand::MO_GlobalAddress:
    case MachineOperand::MO_ExternalSymbol:
    case MachineOperand::MO_JumpTableIndex:
    case MachineOperand::MO_ConstantPoolIndex:
    case MachineOperand::MO_BlockAddress:
      return LowerSymbolOperand(MO);

    case MachineOperand::MO_RegisterMask:
      break;
  }

  return MCOperand();
}


void DuckyMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  LLVM_DEBUG(dbgs() << "#-----------\n");
  LLVM_DEBUG(dbgs() << "DB.Lower: MI=" << *MI);

  OutMI.setOpcode(MI->getOpcode());

  for (auto &MO : MI->operands()) {
    const MCOperand MCOp = LowerOperand(MO);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }

  LLVM_DEBUG(dbgs() << '\n');
}
