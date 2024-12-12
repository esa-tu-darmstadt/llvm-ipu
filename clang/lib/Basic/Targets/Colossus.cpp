//===--- Colossus.cpp - Implement Colossus target feature support --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Colossus TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Colossus.h"
#include "clang/Basic/MacroBuilder.h"

using namespace clang;

static constexpr int NumBuiltins =
    clang::Colossus::LastTSBuiltin - Builtin::FirstTSBuiltin;

static constexpr auto BuiltinStorage = Builtin::Storage<NumBuiltins>::Make(
#define BUILTIN CLANG_BUILTIN_STR_TABLE
#define TARGET_BUILTIN CLANG_TARGET_BUILTIN_STR_TABLE
#include "clang/Basic/BuiltinsColossus.def"
    , {
#define BUILTIN CLANG_BUILTIN_ENTRY
#define LIBBUILTIN CLANG_LIBBUILTIN_ENTRY
#define TARGET_BUILTIN CLANG_TARGET_BUILTIN_ENTRY
#include "clang/Basic/BuiltinsColossus.def"
      });

std::pair<const char *, ArrayRef<Builtin::Info>>
ColossusTargetInfo::getTargetBuiltinStorage() const {
  return {BuiltinStorage.StringTable, BuiltinStorage.Infos};
}

const char *const ColossusTargetInfo::GCCRegNames[] = {
    "$m0",  "$m1",  "$m2",  "$m3",  "$m4",  "$m5",   "$m6",   "$m7",   "$m8",
    "$m9",  "$m10", "$m11", "$m12", "$m13", "$m14",  "$m15",  "$a0",   "$a1",
    "$a2",  "$a3",  "$a4",  "$a5",  "$a6",  "$a7",   "$a8",   "$a9",   "$a10",
    "$a11", "$a12", "$a13", "$a14", "$a15", "$a0:1", "$a2:3", "$a4:5", "$a6:7",
};

const TargetInfo::GCCRegAlias ColossusTargetInfo::GCCRegAliases[] = {
    {{"$bp"}, "$m8"},
    {{"$fp"}, "$m9"},
    {{"$lr"}, "$m10"},
    {{"$sp"}, "$m11"},
    {{"$mworker_base"}, "$m12"},
    {{"$mvertex_base"}, "$m13"},
    {{"$azero"}, "$a15"},
    {{"$mzero"}, "$m15"},
};

ArrayRef<const char *> ColossusTargetInfo::getGCCRegNames() const {
  return llvm::ArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> ColossusTargetInfo::getGCCRegAliases() const {
  return llvm::ArrayRef(GCCRegAliases);
}

bool ColossusTargetInfo::initFeatureMap(
    llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags, StringRef CPU,
    const std::vector<std::string> &FeaturesVec) const {
  // This keeps track of "-msupervisor" which resolves to "-worker".
  // We override this if any explicit target attribute is set in the
  // function (this is to be changed later).
  bool hasImplicitSupervisorTarget = false;

  StringRef TargetFeat;
  std::vector<std::string> UpdatedFeaturesVec;
  for (const std::string &F : FeaturesVec) {
    if (F == "-worker") {
      hasImplicitSupervisorTarget = true;
    } else if (F == "+worker" || F == "+supervisor" || F == "+both") {
      // Sets the explicit target feature found in function attributes.
      // This must not be set twice as they are mutually exclusive.
      if (!TargetFeat.empty()) {
        Diags.Report(diag::err_opt_not_valid_with_opt) << TargetFeat << F;
        return false;
      }
      TargetFeat = F;
    } else {
      UpdatedFeaturesVec.push_back(F);
    }
  }

  // Either have the explicit target attribute or the implicit from
  // "-msupervisor" option. If neither, we default to "+worker".
  if (!TargetFeat.empty())
    UpdatedFeaturesVec.push_back(TargetFeat.str());
  else if (hasImplicitSupervisorTarget)
    UpdatedFeaturesVec.push_back("+supervisor");
  else
    UpdatedFeaturesVec.push_back("+worker");

  return TargetInfo::initFeatureMap(Features, Diags, CPU, UpdatedFeaturesVec);
}

bool ColossusTargetInfo::handleTargetFeatures(
    std::vector<std::string> &Features, DiagnosticsEngine &Diags) {

  for (const auto &Feature : Features) {
    if (Feature == "+worker") {
      HalfArgsAndReturns = true;
      HasLegalHalfType = true;
      HasFloat16 = true;
      Worker = true;
      llvm::outs() << "Compiling for worker\n";
    }
    if (Feature == "+supervisor") {
      llvm::outs() << "Compiling for supervisor\n";
      Supervisor = true;
    }
  }

  return true;
}

void ColossusTargetInfo::getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const {
    Builder.defineMacro("__IPU__");
    if (CPU == "ipu1") {
      Builder.defineMacro("__IPU_ARCH_VERSION__", "1");
      Builder.defineMacro("__IPU_REPEAT_COUNT_SIZE__", "12");
    } else if (CPU == "ipu2") {
      Builder.defineMacro("__IPU_ARCH_VERSION__", "2");
      Builder.defineMacro("__IPU_REPEAT_COUNT_SIZE__", "16");
#ifdef IPU21
    } else if (CPU == "ipu21") {
      Builder.defineMacro("__IPU_ARCH_VERSION__", "21");
      Builder.defineMacro("__IPU_REPEAT_COUNT_SIZE__", "16");
#endif
    } else {
      llvm::errs() << "Unsupported CPU " << CPU << "\n";
    }

    if (Supervisor) {
      Builder.defineMacro("__SUPERVISOR__");
    }
  }