//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/DebugInfo.h"
#include <map>
using namespace llvm;

#define DEBUG_TYPE "hello"

//STATISTIC(FuncCounter, "Counts number of functions greeted");

typedef struct myfunctionnode
{
  int functionCalls;
  int functionparams;
  Type* paramTypes[50];
}myfunctionNode; 


void paramtoString(Type *obj, char str[])
{
      if(obj->isVoidTy())
        strcpy(str,"void");

      else if(obj->isIntegerTy(8))
        strcpy(str,"char");
      
      else if(obj->isIntegerTy(32))
        strcpy(str,"int");
      
      else if(obj->isFloatTy())
        strcpy(str,"float");
      
      else if(obj->isDoubleTy())
        strcpy(str,"double");
      
      else if(obj->isPointerTy())
        strcpy(str,"pointer type");
      
      else
        strcpy(str,"UndefinedType");
  
}


namespace {
  //declare a function map table with name and ptr to myfunction node
  std::map<std::string, myfunctionNode*> funcmap;
  // inherit the modulePass class
  struct FuncPass : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    FuncPass() : ModulePass(ID) {}

    //iterate through the module -> function -> basic block - >instructions->callinstructions
    bool runOnModule(Module &M) override {

      for (Module::iterator F = M.begin(), E = M.end(); F != E; F++)
      {
        for(Function::iterator bb = F->begin(), e = F->end(); bb != e; bb++)
        {
          for(BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; i++)
          {
                MDNode *md = i->getMetadata("dbg");
                DILocation location = DILocation(md);
                int lineNumber = location.getLineNumber();

                //errs() <<"\nLine Number: " << lineNumber;

                //Check whether the instruction is a function call
                if(isa<CallInst>(i))
                {

                    //Typecast to call instruction
                    CallInst &callinstr = static_cast<CallInst&>(*i);
                    
                    //obtain a pointer to the the function definition 
                    const Function* funcdefn = dyn_cast<const Function>(callinstr.getCalledValue()->stripPointerCasts());
                     
                     //if function already exists in the map
                     if(funcmap[funcdefn->getName()])
                     {
                        myfunctionnode *fptr=funcmap[funcdefn->getName()];
                        fptr->functionCalls++;
                        funcmap[funcdefn->getName()]= fptr;
                     }       

                     //add the function to the map
                     else
                     {
                        myfunctionnode *fptr=new myfunctionnode();
                        fptr->functionCalls=1;
                        int noofparams=funcdefn->getFunctionType()->getNumParams();
                        fptr->functionparams=noofparams;

                        //display all function calls made
                        //errs() <<"\nFunction called: " << funcdefn->getName();
                        //errs() <<"\nFunction params: " << fptr->functionparams;

                        for(int i = 0; i < noofparams; i++)
                        {
                          Type *typeObj = funcdefn->getFunctionType()->getParamType(i);
                          fptr->paramTypes[i] = typeObj; 
                        }
                        funcmap[funcdefn->getName()] = fptr;
                     }

                     //check if functiuon declaration
                     if(!(funcdefn->isDeclaration()))
                      {
                                //errs() << "\n" << funcdefn->getName() << "+++Function Name\n";
                              
                                int actualArgs = callinstr.getNumArgOperands();
                                int formalArgs = funcdefn->getFunctionType()->getNumParams();
                                
                                if( actualArgs != formalArgs )
                                {
                                    errs() << "\nFunction '" << funcdefn->getName() << "' call on line " << lineNumber 
                                    << ": Expected " << formalArgs << " arguments but " << actualArgs << " are present";
  
                                } 
                                else
                                {

                                    //errs() << "Inside arguments length same";                                
                                    for(int j = 0; j<formalArgs; j++)
                                    {
                                        //errs() << "\nCall Argument : " << *ci.getArgOperand(i)->getType();
                                        //errs() << "\nFunction Argument : " << *function->getFunctionType()->getParamType(i);
                                        
                                        //compare parameter type of func defn. and func call
                                        int flag=0;

                                        Type *param1=callinstr.getArgOperand(j)->getType();
                                        Type *param2=funcdefn->getFunctionType()->getParamType(j);

                                        if(param1->isVoidTy() && param2->isVoidTy())
                                            flag=1;

                                        else if(param1->isIntegerTy(8) && param2->isIntegerTy(8))
                                            flag=1;

                                        else if(param1->isIntegerTy(32) && param2->isIntegerTy(32))
                                            flag=1;
                                        
                                        else if(param1->isFloatTy() && param2->isFloatTy())
                                            flag=1;

                                        else if(param1->isDoubleTy() && param2->isDoubleTy())
                                            flag=1;

                                        else if(param1->isPointerTy() && param2->isPointerTy())
                                            flag=1;

                                        else
                                            flag=0;

                                        if(flag==0)
                                        {
                                            char expectedtype[50];
                                            char istype[50];
                                            paramtoString(funcdefn->getFunctionType()->getParamType(j),expectedtype);
                                            paramtoString(callinstr.getArgOperand(j)->getType(),istype);
                                            errs() << "\nFunction '" << funcdefn->getName() << "' call on line " << lineNumber<< ": argument type mismatch. Expected '" 
                                            << expectedtype<<"' but argument is of type '"<< istype <<"'";
                                        }
                                    }    
                                }
                      }
                    }
                  }
                }
              }
            
                    errs() << "\n\nList of function calls:\n";  
                    for(std::map<std::string,myfunctionnode*>::iterator itr = funcmap.begin(); itr!=funcmap.end(); ++itr)
                    {
        
                        myfunctionNode *fptr = itr->second;
                        errs() << "\n" << itr->first <<"(";

                        for(int j=0;j<fptr->functionparams;j++)
                        {
                            char str[100];
                            paramtoString(fptr->paramTypes[j], str);
                            if(j == 0)
                                errs() << str;
                            else
                                errs() << "," << str;  
                        }
                        errs() << "):" << fptr->functionCalls;
                    }    

    errs() <<"\n\n";            
          
        

      
      //Function:iterator=M.begin();
      
      //++HelloCounter;
      //errs() << "Function names: ";
      //errs().write_escaped(F.getName()) << '\n';
      return false;
    }
  };
}

char FuncPass::ID = 0;
static RegisterPass<FuncPass> X("funcpass", "Function Pass",false,false);

/*namespace {
  // Hello2 - The second implementation with getAnalysisUsage implemented.
  struct Hello2 : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello2() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char Hello2::ID = 0;
static RegisterPass<Hello2>
Y("hello2", "Hello World Pass (with getAnalysisUsage implemented)");*/
