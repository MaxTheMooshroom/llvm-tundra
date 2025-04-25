//===- llvm/unittests/Target/DirectX/ResourceBindingAnalysisTests.cpp -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Analysis/DXILResource.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/DXILABI.h"
#include "gtest/gtest.h"
#include <cstdint>

using namespace llvm;
using namespace llvm::dxil;

namespace {
class ResourceBindingAnalysisTest : public testing::Test {
protected:
  PassBuilder *PB;
  ModuleAnalysisManager *MAM;
  LLVMContext *Context;

  virtual void SetUp() {
    PB = new PassBuilder();
    MAM = new ModuleAnalysisManager();
    Context = new LLVMContext();
    PB->registerModuleAnalyses(*MAM);
    MAM->registerPass([&] { return DXILResourceBindingAnalysis(); });
  }

  std::unique_ptr<Module> parseAsm(StringRef Asm) {
    SMDiagnostic Error;
    std::unique_ptr<Module> M = parseAssemblyString(Asm, Error, *Context);
    EXPECT_TRUE(M) << "Bad assembly?: " << Error.getMessage();
    return M;
  }

  virtual void TearDown() {
    delete PB;
    delete MAM;
    delete Context;
  }

  void checkExpectedSpaceAndFreeRanges(
      DXILResourceBindingInfo::RegisterSpace &RegSpace, uint32_t ExpSpace,
      ArrayRef<uint32_t> ExpValues) {
    EXPECT_EQ(RegSpace.Space, ExpSpace);
    EXPECT_EQ(RegSpace.FreeRanges.size() * 2, ExpValues.size());
    unsigned I = 0;
    for (auto &R : RegSpace.FreeRanges) {
      EXPECT_EQ(R.LowerBound, ExpValues[I]);
      EXPECT_EQ(R.UpperBound, ExpValues[I + 1]);
      I += 2;
    }
  }
};

TEST_F(ResourceBindingAnalysisTest, TestTrivialCase) {
  // RWBuffer<float> Buf : register(u5);
  StringRef Assembly = R"(
define void @main() {
entry:
  %handle = call target("dx.TypedBuffer", float, 1, 0, 0) @llvm.dx.resource.handlefrombinding(i32 0, i32 5, i32 1, i32 0, i1 false)
  
  call void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handle)
  ret void
}

declare void @a.func(target("dx.RawBuffer", float, 1, 0) %handle)
  )";

  auto M = parseAsm(Assembly);

  DXILResourceBindingInfo &DRBI =
      MAM->getResult<DXILResourceBindingAnalysis>(*M);

  EXPECT_EQ(false, DRBI.containsImplicitBinding());
  EXPECT_EQ(false, DRBI.containsOverlappingBinding());

  // check that UAV has exactly one gap
  DXILResourceBindingInfo::BindingSpaces &UAVSpaces =
      DRBI.getBindingSpaces(ResourceClass::UAV);
  EXPECT_EQ(UAVSpaces.ResClass, ResourceClass::UAV);
  EXPECT_EQ(UAVSpaces.Spaces.size(), 1u);
  checkExpectedSpaceAndFreeRanges(UAVSpaces.Spaces[0], 0,
                                  {0, 4, 6, UINT32_MAX});

  // check that other kinds of register spaces are all available
  for (auto RC :
       {ResourceClass::SRV, ResourceClass::CBuffer, ResourceClass::Sampler}) {
    DXILResourceBindingInfo::BindingSpaces &Spaces = DRBI.getBindingSpaces(RC);
    EXPECT_EQ(Spaces.ResClass, RC);
    EXPECT_EQ(Spaces.Spaces.size(), 1u);
    checkExpectedSpaceAndFreeRanges(Spaces.Spaces[0], 0, {0, UINT32_MAX});
  }
}

TEST_F(ResourceBindingAnalysisTest, TestManyBindings) {
  // cbuffer CB                 : register(b3) { int a; }
  // RWBuffer<float4> A[5]      : register(u10, space20);
  // StructuredBuffer<int> B    : register(t5);
  // RWBuffer<float> C          : register(u5);
  // StructuredBuffer<int> D[5] : register(t0);
  // RWBuffer<float> E[2]       : register(u2);
  StringRef Assembly = R"(
%__cblayout_CB = type <{ i32 }>
define void @main() {
entry:
  %handleCB = call target("dx.CBuffer", target("dx.Layout", %__cblayout_CB, 4, 0)) @llvm.dx.resource.handlefrombinding(i32 0, i32 3, i32 1, i32 0, i1 false)
  %handleA = call target("dx.TypedBuffer", float, 1, 0, 0) @llvm.dx.resource.handlefrombinding(i32 20, i32 10, i32 5, i32 0, i1 false)
  %handleB = call target("dx.RawBuffer", i32, 0, 0) @llvm.dx.resource.handlefrombinding(i32 0, i32 5, i32 1, i32 0, i1 false)
  %handleC = call target("dx.TypedBuffer", float, 1, 0, 0) @llvm.dx.resource.handlefrombinding(i32 0, i32 5, i32 1, i32 0, i1 false)
  %handleD = call target("dx.RawBuffer", i32, 0, 0) @llvm.dx.resource.handlefrombinding(i32 0, i32 0, i32 5, i32 4, i1 false)
  %handleE = call target("dx.TypedBuffer", float, 1, 0, 0) @llvm.dx.resource.handlefrombinding(i32 0, i32 2, i32 2, i32 0, i1 false)
  
  call void @a.func(target("dx.CBuffer", target("dx.Layout", %__cblayout_CB, 4, 0)) %handleCB)
  call void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handleA)
  call void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handleC)
  call void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handleE)
  call void @a.func(target("dx.RawBuffer", i32, 0, 0) %handleB)
  call void @a.func(target("dx.RawBuffer", i32, 0, 0) %handleD)
  
  ret void
}

declare void @a.func(target("dx.RawBuffer", float, 1, 0) %handle)
  )";

  auto M = parseAsm(Assembly);

  DXILResourceBindingInfo &DRBI =
      MAM->getResult<DXILResourceBindingAnalysis>(*M);

  EXPECT_EQ(false, DRBI.containsImplicitBinding());
  EXPECT_EQ(false, DRBI.containsOverlappingBinding());

  DXILResourceBindingInfo::BindingSpaces &SRVSpaces =
      DRBI.getBindingSpaces(ResourceClass::SRV);
  EXPECT_EQ(SRVSpaces.ResClass, ResourceClass::SRV);
  EXPECT_EQ(SRVSpaces.Spaces.size(), 1u);
  checkExpectedSpaceAndFreeRanges(SRVSpaces.Spaces[0], 0, {6, UINT32_MAX});

  DXILResourceBindingInfo::BindingSpaces &UAVSpaces =
      DRBI.getBindingSpaces(ResourceClass::UAV);
  EXPECT_EQ(UAVSpaces.ResClass, ResourceClass::UAV);
  EXPECT_EQ(UAVSpaces.Spaces.size(), 2u);
  checkExpectedSpaceAndFreeRanges(UAVSpaces.Spaces[0], 0,
                                  {0, 1, 4, 4, 6, UINT32_MAX});
  checkExpectedSpaceAndFreeRanges(UAVSpaces.Spaces[1], 20,
                                  {0, 9, 15, UINT32_MAX});

  DXILResourceBindingInfo::BindingSpaces &CBufferSpaces =
      DRBI.getBindingSpaces(ResourceClass::CBuffer);
  EXPECT_EQ(CBufferSpaces.ResClass, ResourceClass::CBuffer);
  EXPECT_EQ(CBufferSpaces.Spaces.size(), 1u);
  checkExpectedSpaceAndFreeRanges(CBufferSpaces.Spaces[0], 0,
                                  {0, 2, 4, UINT32_MAX});
}

TEST_F(ResourceBindingAnalysisTest, TestUnboundedAndOverlap) {
  // StructuredBuffer<float> A[]  : register(t5);
  // StructuredBuffer<float> B[3] : register(t0);
  // StructuredBuffer<float> C[]  : register(t0, space2);
  // StructuredBuffer<float> D    : register(t4, space2); /* overlapping */
  StringRef Assembly = R"(
%__cblayout_CB = type <{ i32 }>
define void @main() {
entry:
  %handleA = call target("dx.RawBuffer", float, 0, 0) @llvm.dx.resource.handlefrombinding(i32 0, i32 5, i32 -1, i32 10, i1 false)
  %handleB = call target("dx.RawBuffer", float, 0, 0) @llvm.dx.resource.handlefrombinding(i32 0, i32 0, i32 3, i32 0, i1 false)
  %handleC = call target("dx.RawBuffer", float, 0, 0) @llvm.dx.resource.handlefrombinding(i32 2, i32 0, i32 -1, i32 100, i1 false)
  %handleD = call target("dx.RawBuffer", float, 0, 0) @llvm.dx.resource.handlefrombinding(i32 2, i32 4, i32 1, i32 0, i1 false)
  
  call void @a.func(target("dx.RawBuffer", float, 0, 0) %handleA)
  call void @a.func(target("dx.RawBuffer", float, 0, 0) %handleB)
  call void @a.func(target("dx.RawBuffer", float, 0, 0) %handleC)
  call void @a.func(target("dx.RawBuffer", float, 0, 0) %handleD)
  
  ret void
}

declare void @a.func(target("dx.RawBuffer", float, 0, 0) %handle)
  )";

  auto M = parseAsm(Assembly);

  DXILResourceBindingInfo &DRBI =
      MAM->getResult<DXILResourceBindingAnalysis>(*M);

  EXPECT_EQ(false, DRBI.containsImplicitBinding());
  EXPECT_EQ(true, DRBI.containsOverlappingBinding());

  DXILResourceBindingInfo::BindingSpaces &SRVSpaces =
      DRBI.getBindingSpaces(ResourceClass::SRV);
  EXPECT_EQ(SRVSpaces.ResClass, ResourceClass::SRV);
  EXPECT_EQ(SRVSpaces.Spaces.size(), 2u);
  checkExpectedSpaceAndFreeRanges(SRVSpaces.Spaces[0], 0, {3, 4});
  checkExpectedSpaceAndFreeRanges(SRVSpaces.Spaces[1], 2, {});
}

TEST_F(ResourceBindingAnalysisTest, TestEndOfRange) {
  // RWBuffer<float> A     : register(u4294967295);  /* UINT32_MAX */
  // RWBuffer<float> B[10] : register(u4294967286, space1);
  //                         /* range (UINT32_MAX - 9, UINT32_MAX )*/
  // RWBuffer<float> C[10] : register(u2147483647, space2);
  //                         /* range (INT32_MAX, INT32_MAX + 9) */
  StringRef Assembly = R"(
%__cblayout_CB = type <{ i32 }>
define void @main() {
entry:
  %handleA = call target("dx.TypedBuffer", float, 1, 0, 0) @llvm.dx.resource.handlefrombinding(i32 0, i32 -1, i32 1, i32 0, i1 false)
  %handleB = call target("dx.TypedBuffer", float, 1, 0, 0) @llvm.dx.resource.handlefrombinding(i32 1, i32 -10, i32 10, i32 50, i1 false)
  %handleC = call target("dx.TypedBuffer", float, 1, 0, 0) @llvm.dx.resource.handlefrombinding(i32 2, i32 2147483647, i32 10, i32 100, i1 false)
  
  call void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handleA)
  call void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handleB)
  call void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handleC)
  
  ret void
}

declare void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handle)
  )";

  auto M = parseAsm(Assembly);

  DXILResourceBindingInfo &DRBI =
      MAM->getResult<DXILResourceBindingAnalysis>(*M);

  EXPECT_EQ(false, DRBI.containsImplicitBinding());
  EXPECT_EQ(false, DRBI.containsOverlappingBinding());

  DXILResourceBindingInfo::BindingSpaces &UAVSpaces =
      DRBI.getBindingSpaces(ResourceClass::UAV);
  EXPECT_EQ(UAVSpaces.ResClass, ResourceClass::UAV);
  EXPECT_EQ(UAVSpaces.Spaces.size(), 3u);
  checkExpectedSpaceAndFreeRanges(UAVSpaces.Spaces[0], 0, {0, UINT32_MAX - 1});
  checkExpectedSpaceAndFreeRanges(UAVSpaces.Spaces[1], 1, {0, UINT32_MAX - 10});
  checkExpectedSpaceAndFreeRanges(
      UAVSpaces.Spaces[2], 2,
      {0, (uint32_t)INT32_MAX - 1, (uint32_t)INT32_MAX + 10, UINT32_MAX});
}

TEST_F(ResourceBindingAnalysisTest, TestImplicitFlag) {
  // RWBuffer<float> A;
  StringRef Assembly = R"(
%__cblayout_CB = type <{ i32 }>
define void @main() {
entry:
  %handleA = call target("dx.TypedBuffer", float, 1, 0, 0) @llvm.dx.resource.handlefromimplicitbinding(i32 0, i32 0, i32 1, i32 0)
  call void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handleA)
  ret void
}
declare void @a.func(target("dx.TypedBuffer", float, 1, 0, 0) %handle)
  )";

  auto M = parseAsm(Assembly);

  DXILResourceBindingInfo &DRBI =
      MAM->getResult<DXILResourceBindingAnalysis>(*M);
  EXPECT_EQ(true, DRBI.containsImplicitBinding());
}

} // namespace
