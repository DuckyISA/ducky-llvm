//===-- DuckyISelLowering.cpp - Ducky DAG Lowering Implementation ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DuckyTargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "DuckyISelLowering.h"
#include "Ducky.h"
#include "DuckyMachineFunctionInfo.h"
#include "DuckySubtarget.h"
#include "DuckyTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "ducky"
#include "llvm/Support/Debug.h"

using namespace llvm;

/*
 * Returns the name of a target specific DAG node.
 */
const char *DuckyTargetLowering::getTargetNodeName(
    unsigned Opcode) const {

  switch (Opcode) {
    default:                  return NULL;
    case DuckyISD::RET_FLAG:  return "DuckyISD::RET_FLAG";
    case DuckyISD::LOAD_SYM:  return "DuckyISD::LOAD_SYM";
    case DuckyISD::CALL:      return "DuckyISD::CALL";
  }
}


static SDValue getVarArgPtr(SDValue Chain, SelectionDAG &DAG, MVT PtrVT, const SDLoc &dl)
{
  SDValue FrameHeaderSize = DAG.getConstant(DUCKY_FRAME_HEADER_SIZE, dl, MVT::i32);
  SDValue FramePtr = DAG.getCopyFromReg(Chain, dl, Ducky::FP, PtrVT);

  return DAG.getNode(ISD::ADD, dl, PtrVT, FramePtr, FrameHeaderSize);
}


DuckyTargetLowering::DuckyTargetLowering(DuckyTargetMachine &DuckyTM)
    : TargetLowering(DuckyTM), Subtarget(*DuckyTM.getSubtargetImpl()) {

  // Set up the register classes.
  addRegisterClass(MVT::i32, &Ducky::GRRegsRegClass);

  // Compute derived properties from the register classes
  computeRegisterProperties(Subtarget.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(Ducky::SP);

  setSchedulingPreference(Sched::Source);

  // Nodes that require custom lowering
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::BlockAddress,  MVT::i32, Custom);

  setOperationAction(ISD::BRCOND,       MVT::Other, Expand);
  setOperationAction(ISD::BR_JT,        MVT::Other, Expand);
  setOperationAction(ISD::BRIND,        MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
  setOperationAction(ISD::STACKSAVE,    MVT::Other, Expand);

  setOperationAction(ISD::VASTART,      MVT::Other, Custom);
  setOperationAction(ISD::VAARG,        MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,       MVT::Other, Expand);
  setOperationAction(ISD::VAEND,        MVT::Other, Expand);

//  for (MVT VT : MVT::integer_valuetypes())
//    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Expand);

  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1,   Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i8,   Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i16,  Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i64,  Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i128, Expand);
  }

  // Expand some operations Ducky does not support
  for (MVT Ty : {MVT::i1, MVT::i8, MVT::i16, MVT::i32, MVT::i64, MVT::i128, MVT::Other}) {
    setOperationAction(ISD::BSWAP,              Ty, Expand);
    setOperationAction(ISD::SETCC,              Ty, Expand);
    setOperationAction(ISD::SELECT,             Ty, Expand);
    setOperationAction(ISD::SIGN_EXTEND_INREG,  Ty, Expand);
    setOperationAction(ISD::ADDC,               Ty, Expand);
    setOperationAction(ISD::ADDE,               Ty, Expand);
    setOperationAction(ISD::SUBC,               Ty, Expand);
    setOperationAction(ISD::SUBE,               Ty, Expand);
    setOperationAction(ISD::MULHS,              Ty, Expand);
    setOperationAction(ISD::MULHU,              Ty, Expand);
    setOperationAction(ISD::SMUL_LOHI,          Ty, Expand);
    setOperationAction(ISD::UMUL_LOHI,          Ty, Expand);
    setOperationAction(ISD::UREM,               Ty, Expand);
    setOperationAction(ISD::SDIVREM,            Ty, Expand);
    setOperationAction(ISD::UDIVREM,            Ty, Expand);
    setOperationAction(ISD::DYNAMIC_STACKALLOC, Ty, Expand);
    setOperationAction(ISD::SHL_PARTS,          Ty, Expand);
    setOperationAction(ISD::SRA_PARTS,          Ty, Expand);
    setOperationAction(ISD::SRL_PARTS,          Ty, Expand);
    setOperationAction(ISD::ROTL,               Ty, Expand);
    setOperationAction(ISD::ROTR,               Ty, Expand);
    setOperationAction(ISD::CTPOP,              Ty, Expand);
    setOperationAction(ISD::CTLZ,               Ty, Expand);
    setOperationAction(ISD::CTTZ,               Ty, Expand);

    setOperationAction(ISD::ATOMIC_CMP_SWAP,    Ty, Expand);
    setOperationAction(ISD::ATOMIC_SWAP,        Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD,        Ty, Expand);
    setOperationAction(ISD::ATOMIC_STORE,       Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_ADD,    Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_SUB,    Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_AND,    Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_OR,     Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_XOR,    Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_NAND,   Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_MIN,    Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_MAX,    Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_UMIN,   Ty, Expand);
    setOperationAction(ISD::ATOMIC_LOAD_UMAX,   Ty, Expand);
  }
}


SDValue DuckyTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
  switch (Op.getOpcode()) {
  default:
    LLVM_DEBUG(errs() << "LowerOperation: Op="; Op.dump());

    DAG.setGraphColor(Op.getNode(), "green");
    DAG.viewGraph();

    llvm_unreachable("Unimplemented operand");

  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);

  case ISD::BlockAddress:
    return LowerBlockAddress(Op, DAG);

  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  }
}

SDValue DuckyTargetLowering::LowerGlobalAddress(SDValue Op, SelectionDAG& DAG) const
{
  LLVM_DEBUG(dbgs() << "#DB.LowerGlobalAddress: Op="; Op.dump());

  GlobalAddressSDNode *GA = cast<GlobalAddressSDNode>(Op.getNode());
  EVT VT = Op.getValueType();
  SDLoc DL(GA);

  SDValue TargetAddr = DAG.getTargetGlobalAddress(GA->getGlobal(), DL, VT);
  SDValue Load = DAG.getNode(DuckyISD::LOAD_SYM, Op, VT, TargetAddr);

  if (GA->getOffset() == 0)
    return Load;

  SDValue Offset = DAG.getConstant(GA->getOffset(), DL, VT);

  return DAG.getNode(ISD::ADD, DL, VT, Load, Offset);
}

SDValue DuckyTargetLowering::LowerBlockAddress(SDValue Op, SelectionDAG& DAG) const
{
  LLVM_DEBUG(dbgs() << "#DB.LowerBlockAddress: Op="; Op.dump());

  BlockAddressSDNode *N = cast<BlockAddressSDNode>(Op.getNode());
  const BlockAddress *BA = N->getBlockAddress();
  EVT VT = Op.getValueType();
  SDLoc DL(N);
  int64_t Offset = N->getOffset();

  SDValue TargetAddr = DAG.getTargetBlockAddress(BA, VT, Offset);

  return SDValue(DAG.getMachineNode(Ducky::LAi, DL, VT, TargetAddr), 0);
}

SDValue DuckyTargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const
{
  LLVM_DEBUG(dbgs() << "#DB: LowerVASTART\n"; Op.dump());

  MachineFunction &MF = DAG.getMachineFunction();
  auto *DuckyMFI = MF.getInfo<DuckyMachineFunctionInfo>();
  SDLoc DL(Op);

  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  SDValue VarArgPtr = getVarArgPtr(Op.getOperand(0), DAG, getPointerTy(DAG.getDataLayout()), DL);
  SDValue FrameIndex = DAG.getConstant(DuckyMFI->getVarArgFrameIndex(), DL, MVT::i32);
  SDValue FramePtr   = DAG.getNode(ISD::ADD, DL, VarArgPtr.getValueType(), VarArgPtr, FrameIndex);

  LLVM_DEBUG(dbgs() << "  value:       "; SV->dump());
  LLVM_DEBUG(dbgs() << "  vararg-ptr:  "; VarArgPtr.dump());
  LLVM_DEBUG(dbgs() << "  frame-index: "; FrameIndex.dump());
  LLVM_DEBUG(dbgs() << "  frame-ptr:   "; FramePtr.dump());

  // VASTART stores the address of the first vararg the memory location argument
  return DAG.getStore(Op.getOperand(0), DL, FramePtr, Op.getOperand(1), MachinePointerInfo(SV));
}

static MachineBasicBlock *emitPseudoSel(MachineInstr &MI, MachineBasicBlock *BB)
{
  MachineFunction *F = BB->getParent();
  MachineRegisterInfo &RegInfo = F->getRegInfo();
  const DebugLoc dl = MI.getDebugLoc();
  const TargetInstrInfo *TII = F->getSubtarget().getInstrInfo();

  unsigned DstReg  = MI.getOperand(0).getReg();
  unsigned LHSReg  = MI.getOperand(1).getReg();
  unsigned RHSReg  = MI.getOperand(2).getReg();
  unsigned TValReg = MI.getOperand(3).getReg();
  unsigned FValReg = MI.getOperand(4).getReg();
  unsigned Flag    = MI.getOperand(5).getImm();
  unsigned Value   = MI.getOperand(6).getImm();
  unsigned Signed  = MI.getOperand(7).getImm();

  LLVM_DEBUG(dbgs() << "emitPseudoSel: unpack pseudo-select instruction:\n");
  LLVM_DEBUG(dbgs() << "  " << MI.getOperand(0) << " = (" << MI.getOperand(1) << " [" << Flag << ":" << Value << "] " << MI.getOperand(2) << " ? " << MI.getOperand(3) << " : " << MI.getOperand(4) << ")\n");

  const unsigned CMPDstReg = RegInfo.createVirtualRegister(&Ducky::GRRegsRegClass);

  auto CMPOpcode = (Signed ? Ducky::CMPrr : Ducky::CMPUrr);
  auto SELOpcode = Ducky::SELErr;

  switch(Flag) {
    default:
      llvm_unreachable("Invalid PseudoSel flag");

    case 0:
      SELOpcode = (Value ? Ducky::SELErr : Ducky::SELNErr);
      break;
    case 4:
      SELOpcode = (Value ? Ducky::SELLrr : Ducky::SELGErr);
      break;
    case 5:
      SELOpcode = (Value ? Ducky::SELGrr : Ducky::SELLErr);
      break;
  }

  BuildMI(*BB, &MI, dl, TII->get(CMPOpcode), CMPDstReg).addReg(LHSReg).addReg(RHSReg);
  BuildMI(*BB, &MI, dl, TII->get(SELOpcode), DstReg).addReg(TValReg).addReg(FValReg);

  MI.eraseFromParent();
  return BB;
}

static MachineBasicBlock *emitPseudoCondBranch(MachineInstr &MI, MachineBasicBlock *BB)
{
  MachineFunction *F = BB->getParent();
  MachineRegisterInfo &RegInfo = F->getRegInfo();
  const DebugLoc dl = MI.getDebugLoc();
  const TargetInstrInfo *TII = F->getSubtarget().getInstrInfo();

  unsigned LHSReg         = MI.getOperand(0).getReg();
  unsigned RHSReg         = MI.getOperand(1).getReg();
  MachineBasicBlock *Addr = MI.getOperand(2).getMBB();
  unsigned Flag           = MI.getOperand(3).getImm();
  unsigned Value          = MI.getOperand(4).getImm();
  unsigned Signed         = MI.getOperand(5).getImm();

  LLVM_DEBUG(dbgs() << "emitPseudoCondBranch: unpack pseudo-condbr instruction:\n");
  LLVM_DEBUG(dbgs() << "  " << MI.getOperand(0) << " (" << Flag << ":" << Value << " S? " << Signed << ") " << RHSReg << " ? " << Addr << '\n');

  const unsigned CMPDstReg = RegInfo.createVirtualRegister(&Ducky::GRRegsRegClass);

  auto CMPOpcode = (Signed ? Ducky::CMPrr : Ducky::CMPUrr);
  auto BROpcode = Ducky::BE;

  switch(Flag) {
    default:
      llvm_unreachable("Invalid PseudoCondBranch flag");

    case 0:
      BROpcode = (Value ? Ducky::BE : Ducky::BNE);
      break;
    case 4:
      BROpcode = (Value ? Ducky::BL : Ducky::BGE);
      break;
    case 5:
      BROpcode = (Value ? Ducky::BG : Ducky::BLE);
      break;
  }

  BuildMI(*BB, &MI, dl, TII->get(CMPOpcode), CMPDstReg).addReg(LHSReg).addReg(RHSReg);
  BuildMI(*BB, &MI, dl, TII->get(BROpcode)).addMBB(Addr);

  MI.eraseFromParent();
  return BB;
}

MachineBasicBlock *DuckyTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI, MachineBasicBlock *BB) const
{
  LLVM_DEBUG(dbgs() << "EmitInstrWithCustomInserter: MI="; MI.dump());

  switch (MI.getOpcode()) {
    default:
      llvm_unreachable("Unexpected instruction for custom inserter!");

    case Ducky::PseudoSel:
      return emitPseudoSel(MI, BB);

    case Ducky::PseudoCondBranch:
      return emitPseudoCondBranch(MI, BB);
  }
}


//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "DuckyGenCallingConv.inc"

static unsigned NumFixedArgs;

static bool CC_Ducky_VarArg(unsigned ValNo, MVT ValVT, MVT LocVT,
    CCValAssign::LocInfo LocInfo,
    ISD::ArgFlagsTy ArgFlags, CCState &State)
{
  if (ValNo < NumFixedArgs)
    return CC_Ducky(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State);

  if (LocVT == MVT::i8 || LocVT == MVT::i16) {
    LocVT = MVT::i32;

    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;

    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;

    else
      LocInfo = CCValAssign::AExt;
  }

  //unsigned Offset = State.AllocateStack(ValVT.getStoreSize(), ArgFlags.getOrigAlign());
  unsigned Offset = State.AllocateStack(4, 4);
  State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));

  return false;
}

SDValue DuckyTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI, SmallVectorImpl<SDValue> &InVals) const
{
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &Loc = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;

  MachineFunction &MF = DAG.getMachineFunction();

  const bool IsVarArg = CLI.IsVarArg;

  LLVM_DEBUG(dbgs() << "#DB.LowerCall:\n");

  // Tail call optimization is not supported
  CLI.IsTailCall = false;

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, DUCKY_CC_MAX_ARGS> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());

  NumFixedArgs = 0;

  if (IsVarArg) {
    if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
      const Function *CalleeFn = dyn_cast<Function>(G->getGlobal());

      if (CalleeFn)
        NumFixedArgs = CalleeFn->getFunctionType()->getNumParams();
    }
  }

  if (NumFixedArgs) {
    CCInfo.AnalyzeCallOperands(Outs, CC_Ducky_VarArg);
  } else {
    CCInfo.AnalyzeCallOperands(Outs, CC_Ducky);
  }


  // Get the size of the outgoing arguments stack space requirement.
  const unsigned NumBytes = CCInfo.getNextStackOffset();
  LLVM_DEBUG(dbgs() << "  needs " << NumBytes << " bytes on stack\n");

  auto PtrVT = getPointerTy(DAG.getDataLayout());

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, CLI.DL);

  SmallVector<std::pair<unsigned, SDValue>, DUCKY_CC_MAX_ARGS> RegsToPass;
  SmallVector<SDValue, DUCKY_CC_MAX_ARGS> MemOpChains;
  SDValue StackPtr = DAG.getCopyFromReg(Chain, Loc, Ducky::SP, PtrVT);

  LLVM_DEBUG(dbgs() << "  Walking assignments\n");

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned I = 0, E = ArgLocs.size(); I != E; ++I) {
    LLVM_DEBUG(dbgs() << "  #" << I << '\n');

    CCValAssign &VA = ArgLocs[I];
    SDValue Arg = OutVals[I];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
      case CCValAssign::Full:
        break;
      case CCValAssign::SExt:
        Arg = DAG.getNode(ISD::SIGN_EXTEND, Loc, VA.getLocVT(), Arg);
        break;
      case CCValAssign::ZExt:
        Arg = DAG.getNode(ISD::ZERO_EXTEND, Loc, VA.getLocVT(), Arg);
        break;
      case CCValAssign::AExt:
        Arg = DAG.getNode(ISD::ANY_EXTEND,  Loc, VA.getLocVT(), Arg);
        break;
      default:
        llvm_unreachable("Unknown loc info!");
    }

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) {
      LLVM_DEBUG(dbgs() << "    loc-reg=" << VA.getLocReg() << '\n');

      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

      LLVM_DEBUG(dbgs() << "    loc-mem-offset=" << VA.getLocMemOffset() << '\n');

      SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), Loc);
      LLVM_DEBUG(dbgs() << "    loc-mem-offset="; PtrOff.dump());

      PtrOff = DAG.getNode(ISD::ADD, Loc, PtrVT, StackPtr, PtrOff);
      LLVM_DEBUG(dbgs() << "    loc-mem-offset="; PtrOff.dump());

      SDValue Store = DAG.getStore(Chain, Loc, Arg, PtrOff, MachinePointerInfo());
      //LLVM_DEBUG(dbgs() << "    store="; Store.dump());

      MemOpChains.push_back(Store);
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, Loc, MVT::Other, ArrayRef<SDValue>(&MemOpChains[0], MemOpChains.size()));

  SDValue InFlag;

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  for (unsigned I = 0, E = RegsToPass.size(); I != E; ++I) {
    Chain = DAG.getCopyToReg(Chain, Loc, RegsToPass[I].first, RegsToPass[I].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), Loc, PtrVT, G->getOffset(), 0);

  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), PtrVT, 0);

  // Returns a chain & a flag for retval copy to use.
  SmallVector<SDValue, DUCKY_CC_MAX_ARGS> Ops;

  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add a register mask operand representing the call-preserved registers.
  const uint32_t *Mask;
  const TargetRegisterInfo *TRI = DAG.getSubtarget().getRegisterInfo();
  Mask = TRI->getCallPreservedMask(DAG.getMachineFunction(), CallConv);
  Ops.push_back(DAG.getRegisterMask(Mask));

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  // Returns a chain and a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(DuckyISD::CALL, Loc, NodeTys, ArrayRef<SDValue>(&Ops[0], Ops.size()));
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getConstant(NumBytes, Loc, PtrVT, true), DAG.getConstant(0, Loc, PtrVT, true), InFlag, Loc);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, IsVarArg, Ins, Loc, DAG, InVals);
}

SDValue DuckyTargetLowering::LowerCallResult(
    SDValue Chain,
    SDValue InGlue,
    CallingConv::ID CallConv,
    bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins,
    SDLoc dl,
    SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const
{
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, DUCKY_CC_MAX_ARGS> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_Ducky);

  // Copy all of the result registers out of their specified physreg.
  for (auto &Loc : RVLocs) {
    Chain = DAG.getCopyFromReg(Chain, dl, Loc.getLocReg(), Loc.getValVT(),
                               InGlue).getValue(1);
    InGlue = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

/*
 * Lower the incoming (formal) arguments, described by the Ins array, into the
 * specified DAG.
 */
SDValue DuckyTargetLowering::LowerFormalArguments(
    SDValue Chain,
    CallingConv::ID CallConv,
    bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins,
    const SDLoc &dl,
    SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {

  LLVM_DEBUG(dbgs() << "#DB.LowerFormalArguments:\n");

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, DUCKY_CC_MAX_ARGS> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_Ducky);

  SDValue VarArgPtr = getVarArgPtr(Chain, DAG, getPointerTy(DAG.getDataLayout()), dl);
  LLVM_DEBUG(dbgs() << "  var-arg-ptr: "; VarArgPtr.dump());

  LLVM_DEBUG(dbgs() << "  Process arguments\n");

  unsigned FixedArgsEnd = 0;

  for(unsigned i = 0; i < ArgLocs.size(); i++) {
    LLVM_DEBUG(dbgs() << "  arg #" << i << '\n');

    auto &VA = ArgLocs[i];

    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();

      if (RegVT.getSimpleVT().SimpleTy != MVT::i32) {
        LLVM_DEBUG(dbgs() << "LowerFormalArguments Unhandled argument type: " << RegVT.getEVTString() << '\n');
        llvm_unreachable("unhandled argument type");
      }

      unsigned VReg = RegInfo.createVirtualRegister(&Ducky::GRRegsRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);

      if (VA.getLocInfo() == CCValAssign::SExt)
        ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue, DAG.getValueType(VA.getValVT()));

      else if (VA.getLocInfo() == CCValAssign::ZExt)
        ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue, DAG.getValueType(VA.getValVT()));

      if (VA.getLocInfo() != CCValAssign::Full)
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);

      InVals.push_back(ArgValue);

    } else {
      assert(VA.isMemLoc() && "If not register, memory is allowed only");

      LLVM_DEBUG(dbgs() << "    memloc\n");

      // Load the argument to a virtual register
      unsigned ObjSize = VA.getLocVT().getSizeInBits() / 8;
      if (ObjSize > 4)
        errs() << "LowerFormalArguments Unhandled argument type: " << EVT(VA.getLocVT()).getEVTString() << '\n';

      LLVM_DEBUG(dbgs() << "    obj-size: " << ObjSize << '\n');
      LLVM_DEBUG(dbgs() << "    mem-offset: " << VA.getLocMemOffset() << '\n');

      SDValue Offset = DAG.getIntPtrConstant(VA.getLocMemOffset(), dl);
      LLVM_DEBUG(dbgs() << "    offset: "; Offset.dump());

      SDValue OffsetPtr = DAG.getNode(ISD::ADD, dl, getPointerTy(DAG.getDataLayout()), VarArgPtr, Offset);
      LLVM_DEBUG(dbgs() << "    offset-ptr: "; OffsetPtr.dump());

      SDValue Load = DAG.getLoad(VA.getLocVT(), dl, Chain, OffsetPtr, MachinePointerInfo());
      LLVM_DEBUG(dbgs() << "    load: "; Load.dump());

      InVals.push_back(Load);

      FixedArgsEnd = alignTo(VA.getLocMemOffset() + ObjSize, DUCKY_STACK_ALIGNMENT);
      LLVM_DEBUG(dbgs() << "    fixed-args-end: " << FixedArgsEnd << '\n');
    }
  }

  if (isVarArg) {
    auto *DuckyMFI = MF.getInfo<DuckyMachineFunctionInfo>();

    LLVM_DEBUG(dbgs() << "  IsVarArg: fixed-args-end: " << FixedArgsEnd << '\n');

    DuckyMFI->setVarArgFrameIndex(FixedArgsEnd); // Add frame header size
  }

  return Chain;
}



//===----------------------------------------------------------------------===//
//             Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

/*
 * Check whether the return values described by the Outs array can fit into the
 * return registers.
 */
bool DuckyTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv,
    MachineFunction &MF,
    bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    LLVMContext &Context) const {

#if 1
    for (auto &Out : Outs)
      if (Out.ArgVT == MVT::i128)
        return false;

    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);

    return CCInfo.CheckReturn(Outs, RetCC_Ducky);
#else
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);

  if (!CCInfo.CheckReturn(Outs, RetCC_Ducky))
    return false;

  if (CCInfo.getNextStackOffset() != 0 && isVarArg)
    return false;

  return true;
#endif
}


/*
 * Lower outgoing return values, described by the Outs array, into the
 * specified DAG.
 */
SDValue DuckyTargetLowering::LowerReturn(
    SDValue Chain,
    CallingConv::ID CallConv,
    bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals,
    const SDLoc &dl,
    SelectionDAG &DAG) const {

  // CCValAssign - represent the assignment of
  // the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_Ducky);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(DuckyISD::RET_FLAG, dl, MVT::Other, RetOps);
}


std::pair<unsigned, const TargetRegisterClass *> DuckyTargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI, StringRef Constraint, MVT VT) const
{
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
      default : break;
      case 'r':
        return std::make_pair(0U, &Ducky::GRRegsRegClass);
    }
  }

  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}
