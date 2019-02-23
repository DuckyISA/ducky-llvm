//===---- DuckyAsmParser.cpp - Parse Ducky assembly to MCInst instructions ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Ducky.h"
#include "DuckyRegisterInfo.h"
//#include "MCTargetDesc/DuckyMCELFStreamer.h"
//#include "MCTargetDesc/DuckyMCExpr.h"
#include "MCTargetDesc/DuckyMCTargetDesc.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"

#include <sstream>

#define DEBUG_TYPE "ducky-asm-parser"

namespace llvm {

// Parses Ducky assembly from a stream.
class DuckyAsmParser : public MCTargetAsmParser {
  SMLoc getLoc() const { return getParser().getTok().getLoc(); }

#define GET_ASSEMBLER_HEADER
#include "DuckyGenAsmMatcher.inc"

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool ParseDirective(AsmToken DirectiveID) override;

  bool parseOperand(OperandVector &Operands, StringRef Name);

  bool emit(MCInst &Instruction, SMLoc const &Loc, MCStreamer &Out) const;
  bool invalidOperand(SMLoc const &Loc, OperandVector const &Operands,
                      uint64_t const &ErrorInfo);

  OperandMatchResultTy doParseRegister(unsigned &RegNo);
  OperandMatchResultTy doParseImmediate(const MCExpr* &Val);

  OperandMatchResultTy parseRegister(OperandVector &Operands);
  OperandMatchResultTy parseImmediate(OperandVector &Operands);
  OperandMatchResultTy parseCondBranchTarget(OperandVector &Operands);
  OperandMatchResultTy parseMemoryTarget(OperandVector &Operands);

public:
  enum DuckyMatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "DuckyGenAsmMatcher.inc"
#undef GET_OPERAND_DIAGNOSTIC_TYPES
};

  DuckyAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                  const MCInstrInfo &MII, const MCTargetOptions &Options)
    : MCTargetAsmParser(Options, STI, MII) {
      setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }
};

struct DuckyOperand : public MCParsedAsmOperand {
  typedef MCParsedAsmOperand Base;

  enum KindTy {
    Token,
    Immediate,
    Register,
    CondBranchTarget,
    MemoryTarget
  } Kind;

  struct RegOp {
    unsigned RegNo;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  struct CondBranchTargetOp {
    unsigned RegNo;
    const MCExpr *Val;
  };

  struct MemoryTargetOp {
    unsigned RegNo;
    const MCExpr *Val;
  };

  union {
    StringRef Tok;
    RegOp Reg;
    ImmOp Imm;
    CondBranchTargetOp CBTarget;
    MemoryTargetOp MemTarget;
  };

  SMLoc StartLoc, EndLoc;

  DuckyOperand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}

public:
  DuckyOperand(const DuckyOperand &o) : MCParsedAsmOperand() {
    Kind = o.Kind;

    StartLoc = o.StartLoc;
    EndLoc = o.EndLoc;

    switch (Kind) {
      case Register:
        Reg = o.Reg;
        break;
      case Immediate:
        Imm = o.Imm;
        break;
      case Token:
        Tok = o.Tok;
        break;
      case CondBranchTarget:
        CBTarget = o.CBTarget;
        break;
      case MemoryTarget:
        MemTarget = o.MemTarget;
        break;
    }
  }

  bool __isImmediate15(const MCExpr *Val) const {
    const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Val);

    if (!CE)
      return false;

    uint64_t Value = (uint64_t)CE->getValue();
    uint64_t SignedMask = ~0x7FFF;

    LLVM_DEBUG(dbgs() << "__isImmediate15: " << formatv("{0:X+}", Value) << '\n');

    return Value <= 0x7FFF || (Value & 0x4000 && (Value & SignedMask) == SignedMask);
  }

  bool isImmediate15() const {
    if (!isImm())
      return false;

    return __isImmediate15(getImm());
  }

  bool isImmediate20() const {
    if (!isImm())
      return false;

    if (isa<MCSymbolRefExpr>(Imm.Val))
      return true;

    const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(getImm());

    if (!CE)
      return false;

    uint64_t Value = (uint64_t)CE->getValue();

    return Value <= 0xFFFFF;
  }

  bool isImmediateUpper16() const {
    if (!isImm())
      return false;

    const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(getImm());

    if (!CE)
      return false;

    uint64_t Value = (uint64_t)CE->getValue();

    return (Value & 0xFFFF0000) == 0;
  }

  bool isCondBranchTarget() const {
    if (!isCBTarget())
      return false;

    return true;
  }

  bool isMemoryTarget() const {
    if (!isMem())
      return false;

    return __isImmediate15(getMemoryTargetOffset());
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(Kind == Register && "Unexpected operand kind");
    assert(N == 1 && "Invalid number of operands!");

    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    // Add as immediate when possible
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(Kind == Immediate && "Unexpected operand kind");
    assert(N == 1 && "Invalid number of operands!");

    const MCExpr *Expr = getImm();
    addExpr(Inst, Expr);
  }

  void addCondBranchTargetOperands(MCInst &Inst, unsigned N) const {
    assert(Kind == CondBranchTarget && "Unexpected operand kind");

    LLVM_DEBUG(dbgs() << "DuckyOperand::addCondBranchTargetOperands: Inst=" << Inst << '\n');

    const MCExpr *Val = getCondBranchTargetExpr();

    if (Val != NULL) {
      addExpr(Inst, Val);
      return;
    }

    Inst.addOperand(MCOperand::createReg(getCondBranchTargetReg()));
  }

  void addMemoryTargetOperands(MCInst &Inst, unsigned N) const {
    assert(Kind == MemoryTarget && "Unexpected operand kind");

    LLVM_DEBUG(dbgs() << "DuckyOperand::addMemoryTargetOperands: Inst=" << Inst << '\n');

    Inst.addOperand(MCOperand::createReg(getMemoryTargetBase()));
    addExpr(Inst, getMemoryTargetOffset());
  }

  bool isReg() const override { return Kind == Register; }
  bool isImm() const override { return Kind == Immediate; }
  bool isToken() const override { return Kind == Token; }
  bool isCBTarget() const { return Kind == CondBranchTarget; }
  bool isMem() const override { return Kind == MemoryTarget; }

  StringRef getToken() const {
    assert(Kind == Token && "Invalid access!");

    return Tok;
  }

  unsigned getReg() const {
    assert((Kind == Register) && "Invalid access!");

    return Reg.RegNo;
  }

  const MCExpr *getImm() const {
    assert((Kind == Immediate) && "Invalid access!");

    return Imm.Val;
  }

  unsigned getCondBranchTargetReg() const {
    assert((Kind == CondBranchTarget) && "Invalid access!");

    return CBTarget.RegNo;
  }

  const MCExpr *getCondBranchTargetExpr() const {
    assert((Kind == CondBranchTarget) && "Invalid access!");

    return CBTarget.Val;
  }

  unsigned getMemoryTargetBase() const {
    assert((Kind == MemoryTarget) && "Invalid access!");

    return MemTarget.RegNo;
  }

  const MCExpr *getMemoryTargetOffset() const {
    assert((Kind == MemoryTarget) && "Invalid access!");

    return MemTarget.Val;
  }

  SMLoc getStartLoc() const { return StartLoc; }
  SMLoc getEndLoc() const { return EndLoc; }

  virtual void print(raw_ostream &O) const {
    switch (Kind) {
    case Token:
      O << "Token: \"" << getToken() << "\"";
      break;
    case Register:
      O << "Register: " << getReg();
      break;
    case Immediate:
      O << "Immediate: \"" << *getImm() << "\"";
      break;
    case CondBranchTarget:
      if (getCondBranchTargetExpr() != NULL) {
        O << "CondBranchTarget: " << *getCondBranchTargetExpr() << '\n';
      } else {
        O << "CondBranchtarget: " << getCondBranchTargetReg() << '\n';
      }
      break;
    case MemoryTarget:
      O << "MemoryTarget: " << getMemoryTargetBase() << "[" << *getMemoryTargetOffset() << "]";
      break;
    }
    //O << "\n";
  }

  static std::unique_ptr<DuckyOperand> createToken(StringRef Str, SMLoc S) {
    auto Op = make_unique<DuckyOperand>(Token);

    Op->Tok = Str;
    Op->StartLoc = S;
    Op->EndLoc = S;

    return Op;
  }

  static std::unique_ptr<DuckyOperand> createReg(unsigned RegNo, SMLoc S, SMLoc E) {
    auto Op = make_unique<DuckyOperand>(Register);

    Op->Reg.RegNo = RegNo;
    Op->StartLoc = S;
    Op->EndLoc = E;

    return Op;
  }

  static std::unique_ptr<DuckyOperand> createImm(const MCExpr *Val, SMLoc S, SMLoc E) {
    auto Op = make_unique<DuckyOperand>(Immediate);

    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;

    return Op;
  }

  static std::unique_ptr<DuckyOperand> createCBTarget(unsigned RegNo, const MCExpr *Val, SMLoc S, SMLoc E) {
    auto Op = make_unique<DuckyOperand>(CondBranchTarget);

    Op->CBTarget.RegNo = RegNo;
    Op->CBTarget.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;

    return Op;
  }

  static std::unique_ptr<DuckyOperand> createMemTarget(unsigned RegNo, const MCExpr *Val, SMLoc S, SMLoc E) {
    auto Op = make_unique<DuckyOperand>(MemoryTarget);

    Op->MemTarget.RegNo = RegNo;
    Op->MemTarget.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;

    return Op;
  }
};

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "DuckyGenAsmMatcher.inc"

bool DuckyAsmParser::invalidOperand(SMLoc const &Loc,
                                  OperandVector const &Operands,
                                  uint64_t const &ErrorInfo) {
  SMLoc ErrorLoc = Loc;
  char const *Diag = 0;

  if (ErrorInfo != ~0U) {
    if (ErrorInfo >= Operands.size()) {
      Diag = "too few operands for instruction.";
    } else {
      DuckyOperand const &Op = (DuckyOperand const &)*Operands[ErrorInfo];

      if (Op.getStartLoc() != SMLoc())
        ErrorLoc = Op.getStartLoc();
    }
  }

  if (!Diag)
    Diag = "invalid operand for instruction";

  return Error(ErrorLoc, Diag);
}

bool DuckyAsmParser::emit(MCInst &Inst, SMLoc const &Loc, MCStreamer &Out) const {
  Inst.setLoc(Loc);
  Out.EmitInstruction(Inst, getSTI());

  return false;
}

bool DuckyAsmParser::MatchAndEmitInstruction(SMLoc Loc, unsigned &Opcode,
                                           OperandVector &Operands,
                                           MCStreamer &Out, uint64_t &ErrorInfo,
                                           bool MatchingInlineAsm) {
  MCInst Inst;

  LLVM_DEBUG(dbgs() << "DuckyAsmParser::MatchAndEmitInstruction:\n");

  unsigned MatchResult = MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);

  switch (MatchResult) {
  case Match_Success:
    return emit(Inst, Loc, Out);

  case Match_MissingFeature:
    return Error(Loc, "missing feature");

  case Match_InvalidOperand:
    return Error(Loc, "invalid operand");

  case Match_MnemonicFail:
    return Error(Loc, "invalid instruction");

  case Match_Imm15:
  case Match_ImmUpper16:
  case Match_Imm20: {
    auto &Operand = Operands[ErrorInfo];

    return Error(Operand->getStartLoc(), getMatchKindDiag((DuckyMatchResultTy)MatchResult));
  }

  default:
    LLVM_DEBUG(dbgs() << "DuckyAsmParser::MatchAndEmitInstruction: MatchResult=" << MatchResult << '\n');
    llvm_unreachable("unhandled match result");
  }
}

OperandMatchResultTy DuckyAsmParser::doParseRegister(unsigned &RegNo) {
  auto tokenKind = getLexer().getKind();

  LLVM_DEBUG(dbgs() << "DuckyAsmParser::doParseRegister: tokenKind=" << tokenKind << '\n');

  if (tokenKind != AsmToken::Identifier) {
    LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseRegister: token is not identifier\n");
    return MatchOperand_NoMatch;
  }

  StringRef Name = getLexer().getTok().getIdentifier();

  RegNo = MatchRegisterName(Name);

  if (!RegNo) {
    RegNo = MatchRegisterAltName(Name);

    if (!RegNo) {
      LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseRegister: register name does not match\n");
      return MatchOperand_NoMatch;
    }
  }

  getLexer().Lex();

  return MatchOperand_Success;
}

OperandMatchResultTy DuckyAsmParser::doParseImmediate(const MCExpr* &Val) {
  LLVM_DEBUG(dbgs() << "DuckyAsmParser::doParseImmediate\n");

  switch (getLexer().getKind()) {
    default:
      return MatchOperand_NoMatch;

    case AsmToken::LParen:
    case AsmToken::Minus:
    case AsmToken::Plus:
    case AsmToken::Integer:
    case AsmToken::String:
      if (getParser().parseExpression(Val))
        return MatchOperand_ParseFail;
      return MatchOperand_Success;

    case AsmToken::Identifier: {
      StringRef Identifier;

      if (getParser().parseIdentifier(Identifier))
        return MatchOperand_ParseFail;

      MCSymbol *Sym = getContext().getOrCreateSymbol(Identifier);
      Val = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, getContext());
      return MatchOperand_Success;
    }
  }
}


OperandMatchResultTy DuckyAsmParser::parseRegister(OperandVector &Operands) {
  unsigned RegNo;

  OperandMatchResultTy Result = doParseRegister(RegNo);

  if (Result != MatchOperand_Success)
    return Result;

  SMLoc S = getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  Operands.push_back(DuckyOperand::createReg(RegNo, S, E));

  LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseRegister: success\n");
  return MatchOperand_Success;
}

OperandMatchResultTy DuckyAsmParser::parseImmediate(OperandVector &Operands) {
  LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseImmediate\n");

  const MCExpr *Val;

  SMLoc S = getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  OperandMatchResultTy Result = doParseImmediate(Val);

  if (Result != MatchOperand_Success)
    return Result;

  Operands.push_back(DuckyOperand::createImm(Val, S, E));

  LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseImmediate: success\n");
  return MatchOperand_Success;

  /*
  SMLoc S = getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);
  const MCExpr *Res;

  switch (getLexer().getKind()) {
    default:
      return MatchOperand_NoMatch;

    case AsmToken::LParen:
    case AsmToken::Minus:
    case AsmToken::Plus:
    case AsmToken::Integer:
    case AsmToken::String:
      if (getParser().parseExpression(Res))
        return MatchOperand_ParseFail;

      break;

    case AsmToken::Identifier: {
      StringRef Identifier;

      if (getParser().parseIdentifier(Identifier))
        return MatchOperand_ParseFail;

      MCSymbol *Sym = getContext().getOrCreateSymbol(Identifier);
      Res = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, getContext());
      break;
    }
  }

  Operands.push_back(DuckyOperand::createImm(Res, S, E));
  return MatchOperand_Success;
  */
}

OperandMatchResultTy DuckyAsmParser::parseCondBranchTarget(OperandVector &Operands) {
  LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseCondBranch\n");

  unsigned RegNo;

  OperandMatchResultTy Result = doParseRegister(RegNo);

  if (Result == MatchOperand_Success) {
    SMLoc S = getLoc(), E = SMLoc::getFromPointer(S.getPointer() - 1);

    Operands.push_back(DuckyOperand::createCBTarget(RegNo, nullptr, S, E));

    LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseCondBranchTarget: success, register\n");
    return MatchOperand_Success;
  }

  const MCExpr *Val;

  Result = doParseImmediate(Val);

  if (Result != MatchOperand_Success)
    return Result;

  SMLoc S = getLoc(), E = SMLoc::getFromPointer(S.getPointer() - 1);

  Operands.push_back(DuckyOperand::createCBTarget(RegNo, Val, S, E));

  return MatchOperand_Success;
}

OperandMatchResultTy DuckyAsmParser::parseMemoryTarget(OperandVector &Operands) {
  LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseMemoryTarget\n");

  /* first, there must be a register */
  unsigned RegNo;

  OperandMatchResultTy Result = doParseRegister(RegNo);

  if (Result != MatchOperand_Success)
    return Result;

  // if there's no left bracket, it's just a base register
  auto tokenKind = getLexer().getKind();

  if (tokenKind != AsmToken::LBrac) {
    SMLoc S = getLoc(), E = SMLoc::getFromPointer(S.getPointer() - 1);

    Operands.push_back(DuckyOperand::createMemTarget(RegNo, MCConstantExpr::create(0, getContext()), S, E));

    LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseMemoryTarget: success, just base\n");
    return MatchOperand_Success;
  }

  // eat LBRAC
  getLexer().Lex();

  const MCExpr *Val;

  Result = doParseImmediate(Val);

  if (Result != MatchOperand_Success)
    return Result;

  SMLoc S = getLoc(), E = SMLoc::getFromPointer(S.getPointer() - 1);

  Operands.push_back(DuckyOperand::createMemTarget(RegNo, Val, S, E));

  if (tokenKind != AsmToken::LBrac) {
    Error(getLoc(), "missing ']'");
    return MatchOperand_ParseFail;
  }

  // eat RBRAC
  getLexer().Lex();

  return MatchOperand_Success;
}

bool DuckyAsmParser::parseOperand(OperandVector &Operands, StringRef Name) {
  LLVM_DEBUG(dbgs() << "DuckyAsmParser::parseOperand: Name=" << Name << '\n');

  auto MatchResult = MatchOperandParserImpl(Operands, Name);

  if (MatchResult == MatchOperand_Success)
    return false;

  if (MatchResult == MatchOperand_ParseFail) {
    SMLoc Loc = getLexer().getLoc();
    getParser().eatToEndOfStatement();

    return Error(Loc, "failed to parse");
  }

  if (parseRegister(Operands) == MatchOperand_Success)
    return false;

  if (parseImmediate(Operands) == MatchOperand_Success)
    return false;

  return Error(getLoc(), "unknown operand");
}

bool DuckyAsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) {
  LLVM_DEBUG(dbgs() << "DuckyAsmParser::ParseRegister\n");

  const AsmToken &Tok = getParser().getTok();

  StartLoc = Tok.getLoc();
  EndLoc = Tok.getLoc();

  StringRef Name = getLexer().getTok().getIdentifier();

  if (!MatchRegisterName(Name) || !MatchRegisterAltName(Name)) {
    getParser().Lex();
    return false;
  }

  return Error(StartLoc, "invalid register name");
}

bool DuckyAsmParser::ParseInstruction(ParseInstructionInfo &Info,
                                    StringRef Name, SMLoc NameLoc,
                                    OperandVector &Operands) {
  LLVM_DEBUG(dbgs() << '\n');
  LLVM_DEBUG(dbgs() << "##### DuckyAsmParser::ParseInstruction\n");

  // First operand is token for instruction
  Operands.push_back(DuckyOperand::createToken(Name, NameLoc));

  //LLVM_DEBUG(dbgs() << "DuckyAsmParser::ParseInstruction: instruction token " << InstrToken->getToken()<< '\n');

  // If there are no more operands, then finish
  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  // Parse first operand
  if (parseOperand(Operands, Name))
    return true;

  // Parse until end of statement, consuming commas between operands
  while (getLexer().is(AsmToken::Comma)) {
    // Consume comma token
    getLexer().Lex();

    // Parse next operand
    if (parseOperand(Operands, Name))
      return true;
  }

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    getParser().eatToEndOfStatement();
    return Error(Loc, "unexpected token");
  }

  getParser().Lex(); // Consume the EndOfStatement.
  return false;
}

bool DuckyAsmParser::ParseDirective(AsmToken DirectiveID) { return true; }

extern "C" void LLVMInitializeDuckyAsmParser() {
  RegisterMCAsmParser<DuckyAsmParser> X(getTheDuckyTarget());
}

} // end of namespace llvm
