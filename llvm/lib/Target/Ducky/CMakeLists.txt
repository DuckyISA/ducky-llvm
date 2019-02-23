set(LLVM_TARGET_DEFINITIONS Ducky.td)

tablegen(LLVM DuckyGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM DuckyGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM DuckyGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM DuckyGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM DuckyGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM DuckyGenCallingConv.inc -gen-callingconv)
tablegen(LLVM DuckyGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM DuckyGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM DuckyGenDisassemblerTables.inc -gen-disassembler)

add_public_tablegen_target(DuckyCommonTableGen)

add_llvm_target(DuckyCodeGen
  DuckySubtarget.cpp
  DuckyTargetMachine.cpp
  DuckyRegisterInfo.cpp
  DuckyInstrInfo.cpp
  DuckyFrameLowering.cpp
  DuckyISelLowering.cpp
  DuckySelectionDAGInfo.cpp
  DuckyISelDAGToDAG.cpp
  DuckyAsmPrinter.cpp
  DuckyMCInstLower.cpp
  DuckyTargetObjectFile.cpp
  DuckyMachineFunctionInfo.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
