//===-- TundraSubtarget.h - Define Subtarget for the Tundra -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Tundra specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TUNDRA_TUNDRASUBTARGET_H
#define LLVM_LIB_TARGET_TUNDRA_TUNDRASUBTARGET_H

#include "TundraInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_SUBTARGETINFO_HEADER
#include "TundraGenSubtargetInfo.inc"

namespace llvm {
class TundraSubtarget : public TundraGenSubtargetInfo {
public:
  bool enablePostRAScheduler() const override { return false; }
  void getCriticalPathRCs(RegClassVector &CriticalPathRCs) const override {}
  CodeGenOptLevel getOptLevelToEnablePostRAScheduler() const override { return CodeGenOpt_LevelNone; }

  TundraSubtarget(const Triple &TT, const string &CPU, const string &FS,
                  const TargetMachine &TM);
  ~TundraSubtarget() override;

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override { return &TII; };

  const CallLowering *getCallLowering() const override;
  const LegalizerInfo *getLegalizerInfo() const override;
  const RegisterBankInfo *getRegBankInfo() const override;
  InstructionSelector *getInstructionSelector() const override;
}
}

#endif
