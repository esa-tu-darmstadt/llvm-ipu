//===--- Colossus.h - Declare Colossus target feature support ----- C++ -*-===//
//    Copyright (c) 2023 Graphcore Ltd. All Rights Reserved.
//     Licensed under the Apache License, Version 2.0 (the "License");
//     you may not use this file except in compliance with the License.
//     You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//     Unless required by applicable law or agreed to in writing, software
//     distributed under the License is distributed on an "AS IS" BASIS,
//     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//     See the License for the specific language governing permissions and
//     limitations under the License.
// --- LLVM Exceptions to the Apache 2.0 License ----
//
// As an exception, if, as a result of your compiling your source code, portions
// of this Software are embedded into an Object form of such source code, you
// may redistribute such embedded portions in such Object form without complying
// with the conditions of Sections 4(a), 4(b) and 4(d) of the License.
//
// In addition, if you combine or link compiled forms of this Software with
// software that is licensed under the GPLv2 ("Combined Software") and if a
// court of competent jurisdiction determines that the patent provision (Section
// 3), the indemnity provision (Section 9) or other Section of the License
// conflicts with the conditions of the GPLv2, you may retroactively and
// prospectively choose to deem waived or otherwise exclude such Section(s) of
// the License, but only in their entirety and only with respect to the Combined
// Software.
//
//===----------------------------------------------------------------------===//
//
// This file declares Colossus TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_COLOSSUS_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_COLOSSUS_H

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"

namespace clang {
namespace targets {}
class LLVM_LIBRARY_VISIBILITY ColossusTargetInfo : public TargetInfo {
public:
  static const char *const GCCRegNames[];
  static const TargetInfo::GCCRegAlias GCCRegAliases[];

  std::string CPU;
  bool Supervisor = false;
  bool Worker = false;

  ColossusTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : TargetInfo(Triple) {
    BigEndian = false;
    NoAsmVariants = true;
    LongAlign = 32;
    LongWidth = 32;
    LongLongAlign = 64;
    LongLongWidth = 64;
    SuitableAlign = 32;
    DoubleAlign = LongDoubleAlign = 32;
    MaxVectorAlign = 64;
    WIntType = UnsignedInt;
    UseZeroLengthBitfieldAlignment = true;
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 32;
    PtrDiffType = IntPtrType = SignedInt;
    SizeType = UnsignedInt;
    HasStrictFP = true;
    CPU = Opts.CPU;
    resetDataLayout("e-m:e-p:32:32-i1:8:32-i8:8:32-i16:16:32-i64:32-i128:64"
                    "-f64:32-f128:64-v128:64-a:0:32-n32");
  }
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  std::pair<const char *, ArrayRef<Builtin::Info>>
  getTargetBuiltinStorage() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  bool isValidCPUName(StringRef Name) const override {
    return Name == "ipu1" || Name == "ipu2" || Name == "ipu21";
  }

  bool setCPU(const std::string &Name) override {
    if (isValidCPUName(Name)) {
      CPU = Name;
      return true;
    }
    return false;
  }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

  bool
  initFeatureMap(llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override;

  bool hasBitIntType() const override { return true; }

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;
};
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_COLOSSUS_H
