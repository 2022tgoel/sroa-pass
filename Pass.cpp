// I have this file hardlinked to
// llvm-project/llvm/lib/Transforms/Utils/HelloWorld.cpp

//===-- HelloWorld.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Utils/HelloWorld.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Bitfields.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/iterator.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GEPNoWrapFlags.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/OperandTraits.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/User.h"
#include "llvm/Support/AtomicOrdering.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

PreservedAnalyses HelloWorldPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  errs() << F.getName() << "\n";
  errs() << "safe geps" << "\n";
  auto context = std::make_unique<LLVMContext>();
  for (BasicBlock &b : F) {
    // Get all Allocas in the block
    for (Instruction &i : b) {
      if (AllocaInst *ai = dyn_cast<AllocaInst>(&i)) {
        Type *ty = ai->getAllocatedType();
        if (!ty->isAggregateType())
          continue;
        bool can_rewrite = true;
        // get all uses
        for (Use &u : i.uses()) {
          // check if the use is a getelementptr
          if (GetElementPtrInst *gep =
                  dyn_cast<GetElementPtrInst>(u.getUser())) {
            if (!gep->hasAllConstantIndices()) {
              can_rewrite = false;
              break;
            }
            // check that the gep result is only use in loads and stores
            bool safe_gep = true;
            for (Use &u2 : gep->uses()) {
              if (!(isa<LoadInst>(u2.getUser()) ||
                    isa<StoreInst>(u2.getUser()))) {
                safe_gep = false;
                break;
              }
            }
            if (!safe_gep) {
              can_rewrite = false;
              break;
            }
          } else {
            can_rewrite = false;
            break;
          }
        }
        if (can_rewrite) {
          std::vector<AllocaInst *> allocas;
          for (unsigned int i = 0; i < ty->getNumContainedTypes(); i++) {
            allocas.push_back(new AllocaInst(ty->getContainedType(i), 0,
                                             "split", ai->getIterator()));
          }
          // replace all getelementptrs to use new allocas
          for (Use &u : ai->uses()) {
            GetElementPtrInst *gep = cast<GetElementPtrInst>(u.getUser());
            AllocaInst *which_alloca =
                allocas[cast<ConstantInt>(gep->getOperand(1))->getZExtValue()];
            ReplaceInstWithInst(
                gep->getParent(), gep->getIterator(),
                GetElementPtrInst::Create(
                    which_alloca->getAllocatedType(), which_alloca,
                    ArrayRef(gep->op_begin() + 2, gep->op_end()),
                    gep->getName()));
          }
          // ai->eraseFromParent();
        }
      }
    }
  }

  return PreservedAnalyses::none();
}
