// I have this file hardlinked to llvm-project/llvm/lib/Transforms/Utils/HelloWorld.cpp

//===-- HelloWorld.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/HelloWorld.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

PreservedAnalyses HelloWorldPass::run(Function &F,
                                      FunctionAnalysisManager &AM)
{
  errs() << F.getName() << "\n";
  errs() << "safe geps" << "\n";
  auto context = std::make_unique<LLVMContext>();
  for (BasicBlock &b : F)
  {
    // Get all Allocas in the block
    for (Instruction &i : b)
    {
      if (i.getOpcode() == Instruction::Alloca)
      {
        // get all uses
        for (Use &u : i.uses())
        {
          // check if the use is a getelementptr
          if (GetElementPtrInst *gep =
                  dyn_cast<GetElementPtrInst>(u.getUser()))
          {
            if (!gep->hasAllConstantIndices())
            {
              continue;
            }
            // check that the gep result is only use in loads and stores
            bool safe_gep = true;
            for (Use &u2 : gep->uses())
            {
              if (!(dyn_cast<LoadInst>(u2.getUser()) ||
                    dyn_cast<StoreInst>(u2.getUser())))
              {
                safe_gep = false;
              }
            }
            if (!safe_gep)
            {
              continue;
            }
            errs() << gep << "\n";
          }
        }
      }
    }
  }

  return PreservedAnalyses::none();
}
