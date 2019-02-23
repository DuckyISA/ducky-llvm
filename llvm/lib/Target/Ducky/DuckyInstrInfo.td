//===-- DuckyInstrInfo.td - Target Description for Ducky ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the Ducky instructions in TableGen format.
//
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Instruction operands
//===----------------------------------------------------------------------===//

class ImmAsmOperand<int Bits> : AsmOperandClass {
  let PredicateMethod = "isImmediate" # Bits;
  let RenderMethod = "addImmOperands";
  let DiagnosticString = "immediate must fit into " # Bits # " bits";
}

def Imm15AsmOperand : ImmAsmOperand<15> {
  let Name = "Imm15";
}

def Imm20AsmOperand : ImmAsmOperand<20> {
  let Name = "Imm20";
}

def ImmUpper16AsmOperand : ImmAsmOperand<16> {
  let Name = "ImmUpper16";
  let PredicateMethod = "isImmediateUpper16";
  let DiagnosticString = "immediate must fit into upper 16 bits bits";
}

def CondBranchTargetAsmOperand : AsmOperandClass {
  let Name = "CondBranchTarget";
  let ParserMethod = "parseCondBranchTarget";
  let RenderMethod = "addCondBranchTargetOperands";
}

def MemoryTargetAsmOperand : AsmOperandClass {
  let Name = "MemoryTarget";
  let ParserMethod = "parseMemoryTarget";
  let RenderMethod = "addMemoryTargetOperands";
}

def ducky_imm_zero : Operand<i32>, ImmLeaf<i32, [{ return Imm ==  0; }]>;
def ducky_imm_one  : Operand<i32>, ImmLeaf<i32, [{ return Imm ==  1; }]>;
def ducky_imm_mone : Operand<i32>, ImmLeaf<i32, [{ return Imm == -1; }]>;

def i15            : Operand<i32>, ImmLeaf<i32, [{ return (Imm & 0x000007FFF) == Imm; }]> {
  let ParserMatchClass = Imm15AsmOperand;
}

def i20            : Operand<i32>, ImmLeaf<i32, [{ return (Imm & 0x000FFFFF) == Imm; }]> {
  let ParserMatchClass = Imm20AsmOperand;
}

def i16_upper      : Operand<i32>, ImmLeaf<i32, [{ return (Imm & 0xFFFF0000) == Imm; }]> {
  let ParserMatchClass = ImmUpper16AsmOperand;
}

def LO16 : SDNodeXForm<imm, [{ return getI32Imm((unsigned short)N->getZExtValue(), SDLoc(N)); }]>;
def HI16 : SDNodeXForm<imm, [{ return getI32Imm((unsigned)N->getZExtValue() >> 16, SDLoc(N)); }]>;

def brtarget : Operand<OtherVT> {
  let OperandType = "OPERAND_PCREL";
  let EncoderMethod = "encodeBranchTarget";
  let DecoderMethod = "decodeBranchTarget";
  let PrintMethod = "printBranchTarget";
}

def condbrtarget : Operand<OtherVT> {
  let OperandType = "OPERAND_PCREL";
  let EncoderMethod = "encodeCondBranchTarget";
  let DecoderMethod = "decodeCondBranchTarget";
  let PrintMethod = "printCondBranchTarget";

  let ParserMatchClass = CondBranchTargetAsmOperand;
}

def MEMbo : Operand<iPTR> {
  let EncoderMethod = "encodeMemoryTarget";
  let DecoderMethod = "decodeMemoryTarget";
  let PrintMethod = "printMemoryTarget";
  let MIOperandInfo = (ops GRRegs:$base, i15:$offset);

  let ParserMatchClass = MemoryTargetAsmOperand;
}

def ADDRbo : ComplexPattern<iPTR, 2, "SelectMEMBo", []>;

def SDT_DuckyCall         : SDTypeProfile<0, -1, [SDTCisPtrTy<0>]>;
def SDT_DuckyCallSeqStart : SDCallSeqStart<[ SDTCisVT<0, i32>, SDTCisVT<1, i32> ]>;
def SDT_DuckyCallSeqEnd   : SDCallSeqEnd<[ SDTCisVT<0, i32>, SDTCisVT<1, i32> ]>;

def ducky_load_sym      : SDNode<"DuckyISD::LOAD_SYM",  SDTIntUnaryOp>;
def ducky_callseq_start : SDNode<"ISD::CALLSEQ_START",  SDT_DuckyCallSeqStart, [SDNPHasChain, SDNPOutGlue]>;
def ducky_callseq_end   : SDNode<"ISD::CALLSEQ_END",    SDT_DuckyCallSeqEnd, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def ducky_call          : SDNode<"DuckyISD::CALL",      SDT_DuckyCall, [ SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic ]>;
def ducky_ret_flag      : SDNode<"DuckyISD::RET_FLAG",  SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;


//===----------------------------------------------------------------------===//
// Custom SDNodes
//===----------------------------------------------------------------------===//

def LoadImm32Ty : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisInt<0>]>;


//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "DuckyInstrFormats.td"


//===----------------------------------------------------------------------===//
// ALU Instructions
//===----------------------------------------------------------------------===//

let Constraints = "$dst = $src", Defs = [FLAGS] in {
  def INCr : InstDucky_Rr<0b011010, "inc $dst", (outs GRRegs:$dst), (ins GRRegs:$src, ducky_imm_one:$imm), [(set i32:$dst, (add i32:$src, ducky_imm_one:$imm))]>;
  def DECr : InstDucky_Rr<0b011011, "dec $dst", (outs GRRegs:$dst), (ins GRRegs:$src, ducky_imm_mone:$imm), [(set i32:$dst, (add i32:$src, ducky_imm_mone:$imm))]>;
}

multiclass ArithBinOp<bits<6> opcode, string opstr, SDNode opnode> {
  let Constraints = "$dst = $src1", Defs = [FLAGS] in {
    def rr : InstDucky_Rrr<opcode, opstr, (outs GRRegs:$dst), (ins GRRegs:$src1, GRRegs:$src2), [(set i32:$dst, (opnode i32:$src1, i32:$src2))]>;
    def ri : InstDucky_Rri<opcode, opstr, (outs GRRegs:$dst), (ins GRRegs:$src1, i15:$imm),  [(set i32:$dst, (opnode i32:$src1, i15:$imm))]>;
  }
}

defm ADD    : ArithBinOp<0b011100, "add",     add>;
defm SUB    : ArithBinOp<0b011101, "sub",     sub>;
defm AND    : ArithBinOp<0b100010, "and",     and>;
defm XOR    : ArithBinOp<0b100100, "xor",     xor>;
defm OR     : ArithBinOp<0b100011, "or",      or>;
defm SHL    : ArithBinOp<0b100110, "shiftl",  shl>;
defm SHR    : ArithBinOp<0b100111, "shiftr",  srl>;
defm SHRA   : ArithBinOp<0b101000, "shiftrs", sra>;
defm MUL    : ArithBinOp<0b011110, "mul",     mul>;
defm SDIV   : ArithBinOp<0b011111, "div",     sdiv>;
defm UDIV   : ArithBinOp<0b100000, "udiv",    udiv>;
defm SMOD   : ArithBinOp<0b100001, "mod",     srem>;

let Defs = [FLAGS] in  {
  def NOTr : InstDucky_Rr<0b100101, "not $dst", (outs), (ins GRRegs:$dst), []>;
}

//===----------------------------------------------------------------------===//
// Mov Instructions
//===----------------------------------------------------------------------===//
let isAsCheapAsAMove = 1, isMoveImm = 1, hasSideEffects = 0 in {
  def MOVrr : InstDucky_Rrrx<0b001011, "mov", (outs GRRegs:$dst), (ins GRRegs:$src), []>;
}


//===----------------------------------------------------------------------===//
// Load Immediate Instructions
//===----------------------------------------------------------------------===//

let Defs = [FLAGS], isReMaterializable = 1, isAsCheapAsAMove = 1, isMoveImm = 1, hasSideEffects = 0 in {
  class LoadImmediate<bits<6> opcode, string opstr, dag ins>
    : InstDucky_Iri<opcode, opstr, (outs GRRegs:$dst), ins, []>;

  class LoadSymbol<bits<6> opcode, string opstr, dag ins>
    : InstDucky_Iri<opcode, opstr, (outs GRRegs:$dst), ins, []>;
}

def LIi20  : LoadImmediate<0b001001, "li",  (ins i20:$imm)>;
def LIUi16 : LoadImmediate<0b001010, "liu", (ins i32imm:$imm)>;
let Constraints = "$dst = $src" in {
  def LIUi32 : LoadImmediate<0b001010, "liu", (ins GRRegs:$src, i32imm:$imm)>;
}

def LAi    : LoadSymbol<0b001000, "la", (ins i20:$imm)>;

def : Pat<(i32 i20:$imm),       (LIi20  i20:$imm)>;
def : Pat<(i32 i16_upper:$imm), (LIUi16 (HI16 i16_upper:$imm))>;
def : Pat<(i32 imm:$imm),       (LIUi32 (LIi20 (LO16 imm:$imm)), (HI16 imm:$imm))>;

def : Pat<(i32 (ducky_load_sym tglobaladdr:$addr)), (LAi $addr)>;



//===----------------------------------------------------------------------===//
// Load/Store Instructions
//===----------------------------------------------------------------------===//

class LoadMemory<bits<6> opcode, string opstr>
    : InstDucky_R<opcode, !strconcat(opstr, " $dst, $addr"), (outs GRRegs:$dst), (ins MEMbo:$addr), []> {
    let Defs = [FLAGS];

    bits<5> dst;
    bits<21> addr;

    let Inst{10-6} = dst;
    let Inst{15-11} = addr{4-0}; // addr.base
    let Inst{16} = addr{5}; // addr.immediate_flag
    let Inst{31-17} = addr{20-6}; // addr.immediate
}

class StoreMemory<bits<6> opcode, string opstr, PatFrag opnode>
    : InstDucky_R<opcode, !strconcat(opstr, " $addr, $src"), (outs), (ins GRRegs:$src, MEMbo:$addr), [(opnode i32:$src, ADDRbo:$addr)]> {
    bits<5> src;
    bits<21> addr;

    let Inst{10-6} = src;
    let Inst{15-11} = addr{4-0}; // addr.base
    let Inst{16} = addr{5}; // addr.immediate_flag
    let Inst{31-17} = addr{20-6}; // addr.immediate
}

def LB : LoadMemory<0b000011, "lb">;
def LS : LoadMemory<0b000010, "ls">;
def LW : LoadMemory<0b000001, "lw">;

//def : Pat<(set i32:$dst, (sextloadi8 ADDRbo:$addr)),
//          (LB $dst, $addr)>;
def : Pat<(i32 (zextloadi8 ADDRbo:$addr)),
          (LB ADDRbo:$addr)>;
def : Pat<(i32 (extloadi8 ADDRbo:$addr)),
          (LB ADDRbo:$addr)>;

def : Pat<(i32 (zextloadi1 ADDRbo:$addr)),
          (LB ADDRbo:$addr)>;
def : Pat<(i32 (extloadi1 ADDRbo:$addr)),
          (LB ADDRbo:$addr)>;

def : Pat<(i32 (zextloadi16 ADDRbo:$addr)),
          (LS ADDRbo:$addr)>;
def : Pat<(i32 (extloadi16 ADDRbo:$addr)),
          (LS ADDRbo:$addr)>;

def : Pat<(i32 (load ADDRbo:$addr)),
          (LW ADDRbo:$addr)>;

//def LSs : LoadMemory<0b000010, "ls", sextloadi16>;
//def LSz : LoadMemory<0b000010, "ls", zextloadi16>;
//def LS  : LoadMemory<0b000010, "ls", extloadi16>;
//def LBs : LoadMemory<0b000011, "lb", sextloadi8>;
//def LBz : LoadMemory<0b000011, "lb", zextloadi8>;
//def LB  : LoadMemory<0b000011, "lb", extloadi8>;
//def LT  : LoadMemory<0b000011, "lb", extloadi1>;
//def LTz : LoadMemory<0b000011, "lb", zextloadi1>;

def STW : StoreMemory<0b000100, "stw", store>;
def STS : StoreMemory<0b000101, "sts", truncstorei16>;
def STB : StoreMemory<0b000110, "stb", truncstorei8>;


//===----------------------------------------------------------------------===//
// Branching Instructions
//===----------------------------------------------------------------------===//

let isCall = 1, Defs = [FP], Uses = [SP] in {
//  def CALLi : InstDucky_Branch_i<0b001111, "call", (outs), (ins i32imm:$addr), [(ducky_call i32imm:$addr)]>;
  def CALLr : InstDucky_Branch_r<0b001111, "call", (outs), (ins GRRegs:$dst), [(ducky_call GRRegs:$dst)]>;
  def CALLi : InstDucky_Branch_i<0b001111, "call", (outs), (ins i32imm:$addr), [(ducky_call tglobaladdr:$addr)]>;
}

def : Pat<(ducky_call texternalsym:$addr), (CALLi texternalsym:$addr)>;

let isTerminator = 1 in {
  def Ji : InstDucky_Branch_i<0b101110, "j", (outs), (ins brtarget:$addr), [(br bb:$addr)]>;
  def Jr : InstDucky_Branch_r<0b101110, "j", (outs), (ins GRRegs:$dst), []>;
}

//===----------------------------------------------------------------------===//
// Conditional operations
//===----------------------------------------------------------------------===//


// Comparison instructions
let Defs = [FLAGS], hasSideEffects = 0, isCompare = 1 in {
  class InstDucky_CMPrr<bits<6> opcode, string opstr>
      : InstDucky_R<opcode, !strconcat(opstr, " $src1, $src2"), (outs GRRegs:$dst), (ins GRRegs:$src1, GRRegs:$src2), []> {
      bits<5> src1;
      bits<5> src2;

      let Inst{10-6} = src1;
      let Inst{15-11} = src2;
      let Inst{16} = 0b0;
      let Inst{31-17} = 0b000000000000000;
  }

  class InstDucky_CMPri<bits<6> opcode, string opstr>
      : InstDucky_R<opcode, !strconcat(opstr, " $src1, $imm"), (outs GRRegs:$dst), (ins GRRegs:$src1, i15:$imm), []> {
      bits<5> src1;
      bits<15> imm;

      let Inst{10-6} = src1;
      let Inst{15-11} = 0b00000;
      let Inst{16} = 0b1;
      let Inst{31-17} = imm;
  }


  def CMPrr  : InstDucky_CMPrr<0b101111, "cmp">;
  def CMPri  : InstDucky_CMPri<0b101111, "cmp">;
  def CMPUrr : InstDucky_CMPrr<0b110000, "cmpu">;
  def CMPUri : InstDucky_CMPri<0b110000, "cmpu">;
}


// SEL* instructions
let Uses = [FLAGS], hasSideEffects = 0, isSelect = 1, Constraints = "$dst = $tval" in {
  multiclass Select<string opstr, bits<3> flag, bits<1> value> {
    def rr : InstDucky_Sr<0b110011, opstr, flag, value, (outs GRRegs:$dst), (ins GRRegs:$tval, GRRegs:$fval), []>;
    def ri : InstDucky_Si<0b110011, opstr, flag, value, (outs GRRegs:$dst), (ins GRRegs:$tval, i32imm:$fval), []>;
  }
}

defm SELE  : Select<"sele",  0b000, 0b1>;
defm SELNE : Select<"selne", 0b000, 0b0>;
defm SELZ  : Select<"selz",  0b001, 0b1>;
defm SELNZ : Select<"selnz", 0b001, 0b0>;
defm SELO  : Select<"selo",  0b010, 0b1>;
defm SELNO : Select<"selno", 0b010, 0b0>;
defm SELS  : Select<"sels",  0b011, 0b1>;
defm SELNS : Select<"selns", 0b011, 0b0>;
defm SELG  : Select<"selg",  0b101, 0b1>;
defm SELGE : Select<"selge", 0b100, 0b0>;
defm SELL  : Select<"sell",  0b100, 0b1>;
defm SELLE : Select<"selle", 0b101, 0b0>;

// Pseudo instructions carrying necessary operands for SEL*
let isPseudo = 1, isSelect = 1, Constraints = "$dst = $tval", usesCustomInserter = 1 in {
  def PseudoSel : InstDucky<0b110011, "pseudo-sel $lhs ( $flag = $value ) $rhs ? $tval : $fval",
                            (outs GRRegs:$dst),
                            (ins GRRegs:$lhs, GRRegs:$rhs, GRRegs:$tval, GRRegs:$fval, i32imm:$flag, i32imm:$value, i32imm:$signed),
                            []>;
}

// Patterns mapping SELECT_CC nodes to a PseudoSel
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETEQ)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 0, 1, 1)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETNE)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 0, 0, 1)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETLT)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 4, 1, 1)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETLE)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 5, 0, 1)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETGT)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 5, 1, 1)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETGE)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 4, 0, 1)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETULT)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 4, 1, 0)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETULE)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 5, 0, 0)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETUGT)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 5, 1, 0)>;
def : Pat<(i32 (selectcc i32:$lhs, i32:$rhs, i32:$tval, i32:$fval, SETUGE)),
          (PseudoSel $lhs, $rhs, $tval, $fval, 4, 0, 0)>;


// Conditional branching Instructions
let Uses = [FLAGS], isTerminator = 1, isBranch = 1, isBarrier = 0, hasSideEffects = 0 in {
  class CondBranch<string opstr, bits<3> flag, bits<1> value>
    : InstDucky_C<0b110010, opstr, flag, value, (outs), (ins condbrtarget:$addr), []>;
}

def BE  : CondBranch<"be",  0b000, 0b1>;
def BNE : CondBranch<"bne", 0b000, 0b0>;
def BZ  : CondBranch<"bz",  0b001, 0b1>;
def BNZ : CondBranch<"bnz", 0b001, 0b0>;
def BO  : CondBranch<"bo",  0b010, 0b1>;
def BNO : CondBranch<"bno", 0b010, 0b0>;
def BS  : CondBranch<"bs",  0b011, 0b1>;
def BNS : CondBranch<"bns", 0b011, 0b0>;
def BG  : CondBranch<"bg",  0b101, 0b1>;
def BGE : CondBranch<"bge", 0b100, 0b0>;
def BL  : CondBranch<"bl",  0b100, 0b1>;
def BLE : CondBranch<"ble", 0b101, 0b0>;

// Pseudo instructions carrying necessary operands for conditonal branching
let isPseudo = 1, usesCustomInserter = 1, isTerminator = 1, isBranch = 1, isBarrier = 0 in {
  def PseudoCondBranch : InstDucky<0b111111, "pseudo-condbr $lhs ( $flag = $value ) $rhs ? $addr",
                                   (outs),
                                   (ins GRRegs:$lhs, GRRegs:$rhs, condbrtarget:$addr, i32imm:$flag, i32imm:$value, i32imm:$signed),
                                   []>;
}

def : Pat<(brcc SETEQ, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 0, 1, 1)>;
def : Pat<(brcc SETNE, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 0, 0, 1)>;
def : Pat<(brcc SETLT, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 4, 1, 1)>;
def : Pat<(brcc SETLE, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 5, 0, 1)>;
def : Pat<(brcc SETGT, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 5, 1, 1)>;
def : Pat<(brcc SETGE, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 4, 0, 1)>;
def : Pat<(brcc SETULT, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 4, 1, 0)>;
def : Pat<(brcc SETULE, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 5, 0, 0)>;
def : Pat<(brcc SETUGT, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 5, 1, 0)>;
def : Pat<(brcc SETUGE, i32:$lhs, i32:$rhs, bb:$addr),
          (PseudoCondBranch $lhs, $rhs, $addr, 4, 0, 0)>;

//===----------------------------------------------------------------------===//
// Return Instructions
//===----------------------------------------------------------------------===//
def RET : InstDucky_I<0b010000, "ret", (outs), (ins variable_ops), [(ducky_ret_flag)]> {
  let Defs = [SP, FP];
  let isBarrier = 1;
  let isTerminator = 1;
  let isReturn = 1;
}

let isBarrier = 1, isTerminator = 1 in {
  def HLTr : InstDucky_Ir<0b010100, "hlt", (outs), []>;
  def HLTi : InstDucky_Ii<0b010100, "hlt", (outs), []>;
}

def IDLE : InstDucky_Ip<0b010101, "idle", (outs), (ins), []>;
def FPTC : InstDucky_Ip<0b111110, "fptc", (outs), (ins), []>;
def STI : InstDucky_Ip<0b010010, "sti", (outs), (ins), []>;
def CLI : InstDucky_Ip<0b010001, "cli", (outs), (ins), []>;
def RETINT : InstDucky_Ip<0b001110, "retint", (outs), (ins), []>;
def NOP : InstDucky_Ip<0b000000, "nop", (outs), (ins), []>;

def INTr : InstDucky_Ir<0b001101, "int", (outs), []>;
def INTi : InstDucky_Ii<0b001101, "int", (outs), []>;

def PUSHr : InstDucky_Ir<0b011000, "push", (outs), []>;
def PUSHi : InstDucky_Ii<0b011000, "push", (outs), []>;
def POP : InstDucky_Rr<0b011001, "pop $dst", (outs GRRegs:$dst), (ins), []>;

def SWP : InstDucky_Rrrx<0b001100, "swp", (outs), (ins GRRegs:$dst, GRRegs:$src), []>;

def CTR : InstDucky_Rrrx<0b111100, "ctr", (outs), (ins GRRegs:$dst, GRRegs:$src), []>;
def CTW : InstDucky_Rrrx<0b111101, "ctw", (outs), (ins GRRegs:$dst, GRRegs:$src), []>;

def CAS : InstDucky_Arrr<0b000111, "cas", (outs), (ins GRRegs:$mem, GRRegs:$check, GRRegs:$value), []>;

//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//
let Defs = [SP], Uses = [SP] in {
  def ADJCALLSTACKDOWN : InstDucky<0b111111, "# ADJCALLSTACKDOWN $amt1 $amt2", (outs), (ins i32imm:$amt1, i32imm:$amt2),
                                   [(ducky_callseq_start timm:$amt1, timm:$amt2)]>;
  def ADJCALLSTACKUP   : InstDucky<0b111111, "# ADJCALLSTACKUP $amt1 $amt2", (outs), (ins i32imm:$amt1, i32imm:$amt2),
                                   [(ducky_callseq_end timm:$amt1, timm:$amt2)]>;
}
