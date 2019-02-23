

class InstDucky<bits<6> opcode, string asmstr, dag outs, dag ins, list<dag> pattern>
  : Instruction {
  field bits<32> Inst;

  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<32> SoftFail = 0;

  let Namespace = "Ducky";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let Pattern = pattern;

  let AsmString = asmstr;
  let Size = 4;

  let Inst{5-0} = opcode;
}

// "R" encoding, generic variant
class InstDucky_R<bits<6> opcode, string asmstr, dag outs, dag ins, list<dag> pattern>
  : InstDucky<opcode, asmstr, outs, ins, pattern>;

let Constraints = "$dst = $src1" in {
  // "R" encoding, R-R variant
  class InstDucky_Rrr<bits<6> opcode, string opstr, dag outs, dag ins, list<dag> pattern>
    : InstDucky_R<opcode, !strconcat(opstr, " $dst, $src2"), outs, ins, pattern> {
    bits<5> dst;
    bits<5> src2;

    let Inst{10-6} = dst;
    let Inst{15-11} = src2;
    let Inst{16} = 0b0;
    let Inst{31-17} = 0b000000000000000;
  }

  // "R" encoding, R-I variant
  class InstDucky_Rri<bits<6> opcode, string opstr, dag outs, dag ins, list<dag> pattern>
    : InstDucky_R<opcode, !strconcat(opstr, " $dst, $imm"), outs, ins, pattern> {
    bits<5> dst;
    bits<15> imm;

    let Inst{10-6} = dst;
    let Inst{15-11} = 0b00000;
    let Inst{16} = 0b1;
    let Inst{31-17} = imm;
  }
}

// "R" encoding, R-R variant, just 2 inputs (MOV, SWP)
class InstDucky_Rrrx<bits<6> opcode, string opstr, dag outs, dag ins, list<dag> pattern>
  : InstDucky_R<opcode, !strconcat(opstr, " $dst, $src"), outs, ins, pattern> {
  bits<5> dst;
  bits<5> src;

  let Inst{10-6} = dst;
  let Inst{15-11} = src;
  let Inst{16} = 0b0;
  let Inst{31-17} = 0b000000000000000;
}

class InstDucky_Rr<bits<6> opcode, string asmstr, dag outs, dag ins, list<dag> pattern>
  : InstDucky<opcode, asmstr, outs, ins, pattern> {
  bits<5> dst;

  let Inst{10-6} = dst;
  let Inst{15-11} = 0b00000;
  let Inst{16} = 0b0;
  let Inst{31-17} = 0b000000000000000;
}

// "I" encoding, generic variant
class InstDucky_I<bits<6> opcode, string asmstr, dag outs, dag ins, list<dag> pattern>
  : InstDucky<opcode, asmstr, outs, ins, pattern>;

// "I" encoding, R variant
class InstDucky_Ir<bits<6> opcode, string opstr, dag outs, list<dag> pattern>
  : InstDucky_I<opcode, !strconcat(opstr, " $reg"), outs, (ins GRRegs:$reg), pattern> {
  bits<5> reg;

  let Inst{10-6} = reg;
  let Inst{11} = 0b0;
  let Inst{31-12} = 0b00000000000000000000;
}

// "I" encoding, I variant
class InstDucky_Ii<bits<6> opcode, string opstr, dag outs, list<dag> pattern>
  : InstDucky_I<opcode, !strconcat(opstr, " $imm"), outs, (ins i20:$imm), pattern> {
  bits<20> imm;

  let Inst{10-6} = 0b00000;
  let Inst{11} = 0b1;
  let Inst{31-12} = imm;
}

// "I" encoding, R-I variant
class InstDucky_Iri<bits<6> opcode, string opstr, dag outs, dag ins, list<dag> pattern>
  : InstDucky_I<opcode, !strconcat(opstr, " $dst, $imm"), outs, ins, pattern> {
  bits<5> dst;
  bits<20> imm;

  let Inst{10-6} = dst;
  let Inst{11} = 0b1;
  let Inst{31-12} = imm;
}

class InstDucky_Ip<bits<6> opcode, string opstr, dag outs, dag ins, list<dag> pattern>
  : InstDucky_I<opcode, opstr, outs, ins, pattern> {
  let Inst{10-6} = 0b00000;
  let Inst{11} = 0b1;
  let Inst{31-12} = 0b00000000000000000000;
}

// "C" encoding, generic variant
class InstDucky_C<bits<6> opcode, string opstr, bits<3> flag, bits<1> value, dag outs, dag ins, list<dag> pattern>
  : InstDucky<opcode, !strconcat(opstr, " $addr"), outs, ins, pattern> {
  bits<22> addr;

  let Inst{10-6} = addr{5-1}; // addr.register
  let Inst{13-11} = flag;
  let Inst{14} = value;
  let Inst{15} = addr{0}; // addr.immediate_flag
  let Inst{31-16} = addr{21-6}; // addr.immediate
}

// "S" encoding, generic variant
class InstDucky_S<bits<6> opcode, string opstr, bits<3> flag, bits<1> value, dag outs, dag ins, list<dag> pattern>
  : InstDucky<opcode, !strconcat(opstr, " $tval, $fval"), outs, ins, pattern> {
  bits<5> tval;

  let Inst{10-6} = tval;
  let Inst{18-16} = flag;
  let Inst{19} = value;
}

class InstDucky_Sr<bits<6> opcode, string opstr, bits<3> flag, bits<1> value, dag outs, dag ins, list<dag> pattern>
  : InstDucky_S<opcode, opstr, flag, value, outs, ins, pattern> {
  bits<5> fval;

  let Inst{15-11} = fval;
  let Inst{20} = 0b0;
  let Inst{31-21} = 0b00000000000;
}

class InstDucky_Si<bits<6> opcode, string opstr, bits<3> flag, bits<1> value, dag outs, dag ins, list<dag> pattern>
  : InstDucky_S<opcode, opstr, flag, value, outs, ins, pattern> {
  bits<11> fval;

  let Inst{15-11} = 0b00000;
  let Inst{20} = 0b1;
  let Inst{31-21} = fval;
}

// (Unconditional) "Branch" is a special case of I encoding
let isBranch = 1, isBarrier = 1 in {
  class InstDucky_Branch_i<bits<6> opcode, string opstr, dag outs, dag ins, list<dag> pattern>
    : InstDucky_I<opcode, !strconcat(opstr, " $addr"), outs, ins, pattern> {
    bits<26> addr;

    let Inst{10-6} = addr{5-1}; // addr.register
    let Inst{11} = addr{0}; // addr.immediate_flag
    let Inst{31-12} = addr{25-6}; // addr.immediate
  }

  class InstDucky_Branch_r<bits<6> opcode, string opstr, dag outs, dag ins, list<dag> pattern>
    : InstDucky_I<opcode, !strconcat(opstr, " $dst"), outs, ins, pattern> {
    bits<5> dst;

    let Inst{10-6} = dst;
    let Inst{11} = 0;
    let Inst{31-12} = 0b00000000000000000000;
  }
}

// "A" encoding, generic variant
class InstDucky_Arrr<bits<6> opcode, string opstr, dag outs, dag ins, list<dag> pattern>
  : InstDucky<opcode, !strconcat(opstr, " $mem, $check, $value"), outs, ins, pattern> {
  bits<5> mem;
  bits<5> check;
  bits<5> value;

  let Inst{10-6} = mem;
  let Inst{15-11} = check;
  let Inst{20-16} = value;
  let Inst{31-17} = 0b000000000000000;
}
