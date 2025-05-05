//===-- TundraRegisterInfo.td - Tundra Register Definitions -*- tablegen -*-===//

include "llvm/Target/TargetRegisterInfo.td"

def A   : Register { let Namespace="Tundra"; let Encoding=0; let Size=16; }
def B   : Register { let Namespace="Tundra"; let Encoding=1; let Size=16; }
def C   : Register { let Namespace="Tundra"; let Encoding=2; let Size=16; }

def PC  : Register { let Namespace="Tundra"; let Encoding=3; let Size=16; let IsAllocatable=0; }

def GPR16 : RegisterClass<"Tundra", [i16], 16> {
  let Members = [A, B, C];
}

def PCRegClass : RegisterClass<"Tundra", [i16], 16> {
  let Members = [PC];
}
