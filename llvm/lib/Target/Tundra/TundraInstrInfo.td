//===-- TundraInstrInfo.td - Tundra Instruction Definitions -*- tablegen -*-===//

include "llvm/Target/TargetInstrFormats.td"
include "TundraRegisterInfo.td"

// a helper class that takes an outs list and an ins list
class Inst<outs, ins> : Instruction {
    let Namespace       = "Tundra";
    let OutOperandList  = outs;
    let InOperandList   = ins;
}

class MemInst<outs, ins> : Instruction {
    let Namespace         = "Tundra";
    let OutOperandList    = outs;
    let InOperandList     = ins;
    let MemoryOperandList = [(LoadMem, 0)];
}

def INST_MOV : Inst<[(outs GPR16:$dst)], [(ins GPR16:$src)]> {
    let AsmString = "MOV $dst, $src";
}

def INST_MOV_DEREF : MemInst<[(outs GPR16:$dst)], [(ins Mem:$src)]> {
    let AsmString = "MOV $dst, *$src";
}

def INST_STO : Inst<[], [(ins Mem:$addr, GPR16:$src)]> {
    let AsmString = "STO $addr, $src";
    let MemoryOperandList = [(StoreMem, 0)];
}

def INST_STO_DEREF : Inst<[], [(ins Mem:$addr, Mem:$src)]> {
    let AsmString = "STO $addr, *$src";
    let MemoryOperandList = [(StoreMem, 0), (LoadMem, 1)];
}

def INST_ADD : Inst<[(outs GPR16:$dst)], [(ins GPR16:$src)]> {
    let AsmString = "ADD $dst, $src";
}

def INST_ADD_DEREF : MemInst<[(outs GPR16:$dst)], [(ins Mem:$src)]> {
    let AsmString = "ADD $dst, *$src";
}

def INST_CMP : Inst<[], [(ins GPR16:$lhs, GPR16:$rhs)]> {
    let AsmString = "CMP $lhs, $rhs";
}

def INST_CMP_DEREF : MemInst<[], [(ins GPR16:$lhs, Mem:$rhs)]> {
    let AsmString = "CMP $lhs, *$rhs";
}

def INST_ROT : Inst<[(outs GPR16:$dst)], [(ins GPR16:$dst, GPR16:$src)]> {
    let AsmString = "ROT $dst, $src";
}

def INST_ROT_DEREF : MemInst<[(outs GPR16:$dst)], [(ins GPR16:$dst, Mem:$src)]> {
    let AsmString = "ROT $dst, *$src";
}

def INST_AND : Inst<[(outs GPR16:$dst)], [(ins GPR16:$dst, GPR16:$src)]> {
    let AsmString = "AND $dst, $src";
}

def INST_AND_DEREF : MemInst<[(outs GPR16:$dst)], [(ins GPR16:$dst, Mem:$src)]> {
    let AsmString = "AND $dst, *$src";
}

def INST_NOR : Inst<[(outs GPR16:$dst)], [(ins GPR16:$dst, GPR16:$src)]> {
    let AsmString = "NOR $dst, $src";
}

def INST_NOR_DEREF : MemInst<[(outs GPR16:$dst)], [(ins GPR16:$dst, Mem:$src)]> {
    let AsmString = "NOR $dst, *$src";
}

def INST_XOR : Inst<[(outs GPR16:$dst)], [(ins GPR16:$dst, GPR16:$src)]> {
    let AsmString = "XOR $dst, $src";
}

def INST_XOR_DEREF : MemInst<[(outs GPR16:$dst)], [(ins GPR16:$dst, Mem:$src)]> {
    let AsmString = "XOR $dst, *$src";
}
