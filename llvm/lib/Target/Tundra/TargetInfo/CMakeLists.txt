
add_llvm_component_library(LLVMTundraInfo
  TundraTargetInfo.cpp

  LINK_COMPONENTS
  MC

  ADD_TO_COMPONENT
  Tundra
)
