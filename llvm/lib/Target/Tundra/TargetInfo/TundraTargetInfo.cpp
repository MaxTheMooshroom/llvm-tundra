//===-- TundraTargetInfo.h - Tundra Target Implementation -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/TundraTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheTundraTarget() {
  static Target TheTundraTarget;
  return TheTundraTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTundraTargetInfo() {
  RegisterTarget<Triple::tundra>
      X(getTheTundraTarget(), "tundra", "Tundra (16-bit little endian)", "Tundra");
}
