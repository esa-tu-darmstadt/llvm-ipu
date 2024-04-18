//===- Colossus.cpp
//------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Colossus ABI Implementation.
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"

#include "clang/Basic/TargetBuiltins.h"
#include "llvm/IR/IntrinsicsColossus.h"

using namespace clang;
using namespace clang::CodeGen;

namespace {

class ColossusABIInfo : public DefaultABIInfo {
public:
  ColossusABIInfo(CodeGen::CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}
  Address EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                    QualType Ty) const override;
};

class ColossusTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  ColossusTargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<ColossusABIInfo>(CGT)) {}

  void setTargetAttributes(const Decl *D, llvm::GlobalValue *GV,
                           CodeGen::CodeGenModule &CGM) const override {
    if (GV->isDeclaration())
      return;
    // Set the vertex calling convention on functions that have been
    // annotated with the colossus vertex attribute in the source.
    if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
      if (FD->getAttr<ColossusVertexAttr>()) {
        llvm::Function *Fn = cast<llvm::Function>(GV);
        Fn->setCallingConv(llvm::CallingConv::Colossus_Vertex);
      }
    }
  }

  llvm::Value *testFPKind(llvm::Value *V, unsigned BuiltinID,
                          CGBuilderTy &Builder,
                          CodeGenModule &CGM) const override {
    assert(V->getType()->isFloatingPointTy() && "V should have an FP type.");

    llvm::Type *Ty = V->getType();
    if (Ty->isFloatTy()) {
      llvm::Module &M = CGM.getModule();
      llvm::Function *ClassFunc = llvm::Intrinsic::getDeclaration(
          &M, llvm::Intrinsic::colossus_f32class);
      llvm::Value *Class = Builder.CreateCall(ClassFunc, {V});
      switch (BuiltinID) {
      case Builtin::BI__builtin_isnan:
      case Colossus::BI__builtin_ipu_isnan_f32: {
        llvm::ConstantInt *Cst2 = llvm::ConstantInt::get(CGM.Int32Ty, 2);
        llvm::Value *BoolRes = Builder.CreateICmpULE(Class, Cst2);
        return Builder.CreateZExt(BoolRes, CGM.Int32Ty);
      }
      case Builtin::BI__builtin_isinf:
      case Colossus::BI__builtin_ipu_isinf_f32: {
        llvm::ConstantInt *Cst3 = llvm::ConstantInt::get(CGM.Int32Ty, 3);
        llvm::ConstantInt *Cst4 = llvm::ConstantInt::get(CGM.Int32Ty, 4);
        llvm::Value *Eq3 = Builder.CreateICmpEQ(Class, Cst3);
        llvm::Value *Eq4 = Builder.CreateICmpEQ(Class, Cst4);
        llvm::Value *BoolRes = Builder.CreateOr(Eq3, Eq4);
        return Builder.CreateZExt(BoolRes, CGM.Int32Ty);
      }
      case Builtin::BIfinite:
      case Builtin::BI__finite:
      case Builtin::BIfinitef:
      case Builtin::BI__finitef:
      case Builtin::BIfinitel:
      case Builtin::BI__finitel:
      case Builtin::BI__builtin_isfinite:
      case Colossus::BI__builtin_ipu_isfinite_f32: {
        llvm::ConstantInt *Cst5 = llvm::ConstantInt::get(CGM.Int32Ty, 5);
        llvm::Value *BoolRes = Builder.CreateICmpUGE(Class, Cst5);
        return Builder.CreateZExt(BoolRes, CGM.Int32Ty);
      }
      default:
        llvm_unreachable("Unsupported FP test builtin");
        break;
      }
    }
    return nullptr;
  }
};

} // End anonymous namespace.
Address ColossusABIInfo::EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                                   QualType Ty) const {
  CGBuilderTy &Builder = CGF.Builder;

  // Get the VAList.
  CharUnits SlotSize = CharUnits::fromQuantity(4);
  llvm::Value *VAList = Builder.CreateLoad(VAListAddr);
  Address AP(VAList, getVAListElementType(CGF), SlotSize);

  // Handle the argument.
  ABIArgInfo AI = classifyArgumentType(Ty);
  CharUnits TypeAlign = getContext().getTypeAlignInChars(Ty);
  llvm::Type *ArgTy = CGT.ConvertType(Ty);
  if (AI.canHaveCoerceToType() && !AI.getCoerceToType())
    AI.setCoerceToType(ArgTy);
  llvm::Type *ArgPtrTy = llvm::PointerType::getUnqual(ArgTy);

  Address Val = Address::invalid();
  CharUnits ArgSize = CharUnits::Zero();
  switch (AI.getKind()) {
  case ABIArgInfo::Expand:
  case ABIArgInfo::InAlloca:
  case ABIArgInfo::IndirectAliased:
  case ABIArgInfo::CoerceAndExpand:
    llvm_unreachable("Unsupported ABI kind for va_arg");
  case ABIArgInfo::Ignore:
    Val = Address(llvm::UndefValue::get(ArgPtrTy), ArgPtrTy, TypeAlign);
    ArgSize = CharUnits::Zero();
    break;
  case ABIArgInfo::Extend:
  case ABIArgInfo::Direct:
    Val = AP.withElementType(ArgTy);
    ArgSize = CharUnits::fromQuantity(
        getDataLayout().getTypeAllocSize(AI.getCoerceToType()));
    ArgSize = ArgSize.alignTo(SlotSize);
    break;
  case ABIArgInfo::Indirect: {
    Val = AP.withElementType(ArgPtrTy);
    Val = Address(Builder.CreateLoad(Val), ArgTy, TypeAlign);
    ArgSize = SlotSize;
    break;
  }
  }

  // Increment the VAList.
  if (!ArgSize.isZero()) {
    Address APN = Builder.CreateConstInBoundsByteGEP(AP, ArgSize);
    Builder.CreateStore(APN.getBasePointer(), VAListAddr);
  }

  return Val;
}