//===-- TundraRegisterInfo.td - Tundra Register Definitions -*- tablegen -*-===//

include "llvm/Target/TargetRegisterInfo.td"

class Reg<encoding> : Register {
    let Namespace="Tundra";
    let Encoding = encoding;
    let Size=16;
    let IsAllocatable=1;
}

def A   : Reg<0> {}
def B   : Reg<1> {}
def C   : Reg<2> {}

def PC  : Reg<3> { let IsAllocatable=0; }

def GPR16 : RegisterClass<"Tundra", [i16], 16> {
  let Members = [A, B, C];
}

def PCRegClass : RegisterClass<"Tundra", [i16], 16> {
  let Members = [PC];
}
