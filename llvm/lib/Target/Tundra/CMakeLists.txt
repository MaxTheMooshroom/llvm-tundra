
set(LLVM_TARGET_DEFINITIONS Tundra.td)

tablegen(LLVM TundraGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TundraGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TundraGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM TundraGenExegesis.inc -gen-exegesis)

add_public_tablegen_target(TundraCommonTableGen)

add_llvm_target(TundraCodeGen
  TundraInstrInfo.cpp
  TundraRegisterInfo.cpp
  Tundra.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Target
  TargetParser

  ADD_TO_COMPONENT
  Tundra
)
