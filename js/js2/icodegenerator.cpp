// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
//
// The contents of this file are subject to the Netscape Public
// License Version 1.1 (the "License"); you may not use this file
// except in compliance with the License. You may obtain a copy of
// the License at http://www.mozilla.org/NPL/
//
// Software distributed under the License is distributed on an "AS
// IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
// implied. See the License for the specific language governing
// rights and limitations under the License.
//
// The Original Code is the JavaScript 2 Prototype.
//
// The Initial Developer of the Original Code is Netscape
// Communications Corporation.  Portions created by Netscape are
// Copyright (C) 1998 Netscape Communications Corporation. All
// Rights Reserved.

#include "numerics.h"
#include "world.h"
#include "utilities.h"
#include "icodegenerator.h"

#include <iomanip>
#include <stdexcept>


namespace JavaScript {

    std::ostream & operator<<(std::ostream &s, ICodeGenerator &i)
    {
        return i.print(s);
    }

    //
    // ICodeGenerator
    //

    ICodeModule *ICodeGenerator::complete()
    {
    #ifdef DEBUG
        ASSERT(stitcher.empty());
        for (LabelIterator i = labels.begin(); i != labels.end(); i++) {
            ASSERT((*i)->itsBase == iCode);
            ASSERT((*i)->itsOffset <= iCode->size());
        }
    #endif

        for (InstructionIterator ii = iCode->begin(); ii != iCode->end(); ii++) {            
            if ((*ii)->itsOp == BRANCH) {
                Instruction *t = *ii;
                *ii = new ResolvedBranch(static_cast<Branch *>(*ii)->itsOperand1->itsOffset);
                delete t;
            }
            else 
                if ((*ii)->itsOp >= BRANCH_LT && (*ii)->itsOp <= BRANCH_GT) {
                    Instruction *t = *ii;
                    *ii = new ResolvedBranchCond((*ii)->itsOp, 
                                        static_cast<BranchCond *>(*ii)->itsOperand1->itsOffset,
                                            static_cast<BranchCond *>(*ii)->itsOperand2);
                    delete t;
                }
        }
        markMaxRegister();

        return new ICodeModule(iCode, maxRegister, maxVariable);
    }
    
    /***********************************************************************************************/

    Register ICodeGenerator::loadVariable(uint32 frameIndex)
    {
        markMaxVariable(frameIndex);
        Register dest = getRegister();
        LoadVar *instr = new LoadVar(LOAD_VAR, dest, frameIndex);
        iCode->push_back(instr);
        return dest;
    }

    void ICodeGenerator::saveVariable(uint32 frameIndex, Register value)
    {
        markMaxVariable(frameIndex);
        SaveVar *instr = new SaveVar(SAVE_VAR, frameIndex, value);
        iCode->push_back(instr);
    }

    Register ICodeGenerator::loadImmediate(double value)
    {
        Register dest = getRegister();
        LoadImmediate *instr = new LoadImmediate(LOAD_IMMEDIATE, dest, value);
        iCode->push_back(instr);
        return dest;
    }

    Register ICodeGenerator::newObject()
    {
        Register dest = getRegister();
        NewObject *instr = new NewObject(dest);
        iCode->push_back(instr);
        return dest;
    }

    Register ICodeGenerator::newArray()
    {
        Register dest = getRegister();
        NewArray *instr = new NewArray(dest);
        iCode->push_back(instr);
        return dest;
    }

    Register ICodeGenerator::loadName(StringAtom &name)
    {
        Register dest = getRegister();
        LoadName *instr = new LoadName(LOAD_NAME, dest, &name);
        iCode->push_back(instr);
        return dest;
    }
    
    void ICodeGenerator::saveName(StringAtom &name, Register value)
    {
        SaveName *instr = new SaveName(SAVE_NAME, &name, value);
        iCode->push_back(instr);
    }

    Register ICodeGenerator::getProperty(Register base, StringAtom &name)
    {
        Register dest = getRegister();
        GetProp *instr = new GetProp(GET_PROP, dest, base, &name);
        iCode->push_back(instr);
        return dest;
    }

    void ICodeGenerator::setProperty(Register base, StringAtom &name, Register value)
    {
        SetProp *instr = new SetProp(SET_PROP, base, &name, value);
        iCode->push_back(instr);
    }

    Register ICodeGenerator::getElement(Register base, Register index)
    {
        Register dest = getRegister();
        GetElement *instr = new GetElement(GET_ELEMENT, dest, base, index);
        iCode->push_back(instr);
        return dest;
    }

    void ICodeGenerator::setElement(Register base, Register index, Register value)
    {
        SetElement *instr = new SetElement(SET_ELEMENT, base, index, value);
        iCode->push_back(instr);
    }

    Register ICodeGenerator::op(ICodeOp op, Register source)
    {
        Register dest = getRegister();
        Move *instr = new Move(op, dest, source);
        iCode->push_back(instr);
        return dest;
    }

    Register ICodeGenerator::op(ICodeOp op, Register source1, Register source2)
    {
        Register dest = getRegister();
        Arithmetic *instr = new Arithmetic(op, dest, source1, source2);
        iCode->push_back(instr);
        return dest;
    }

    Register ICodeGenerator::call(Register target, RegisterList args)
    {
        Register dest = getRegister();
        Call *instr = new Call(dest, target, args);
        iCode->push_back(instr);
        return dest;
    }

    void ICodeGenerator::branch(Label *label)
    {
        Branch *instr = new Branch(BRANCH, label);
        iCode->push_back(instr);
    }

    void ICodeGenerator::branchConditional(Label *label, Register condition)
    {
        ICodeOp branchOp = getBranchOp();
        if (branchOp == NOP) {
            // XXXX emit convert to boolean / Test / ...
            branchOp = BRANCH_NE;
        }
        BranchCond *instr = new BranchCond(branchOp, label, condition);
        iCode->push_back(instr);
    }


    /***********************************************************************************************/

    Label *ICodeGenerator::getLabel()
    {
        labels.push_back(new Label(NULL));
        return labels.back();
    }

    void ICodeGenerator::setLabel(Label *l)
    {
        l->itsBase = iCode;
        l->itsOffset = static_cast<int32>(iCode->size());
    }

    void ICodeGenerator::setLabel(InstructionStream *stream, Label *l)
    {
        l->itsBase = stream;
        l->itsOffset = static_cast<int32>(stream->size());
    }

    /***********************************************************************************************/

    void ICodeGenerator::mergeStream(InstructionStream *sideStream)
    {
        // change InstructionStream to be a class that also remembers
        // if it contains any labels (maybe even remembers the labels
        // themselves?) in order to avoid running this loop unnecessarily.
        for (LabelList::iterator i = labels.begin(); i != labels.end(); i++) {
            if ((*i)->itsBase == sideStream) {
                (*i)->itsBase = iCode;
                (*i)->itsOffset += iCode->size();
            }
        }

        for (InstructionIterator ii = sideStream->begin(); ii != sideStream->end(); ii++) {
            iCode->push_back(*ii);
        }

    }

    /***********************************************************************************************/

    void ICodeGenerator::beginWhileStatement(uint32)
    {
        resetTopRegister();
  
        // insert a branch to the while condition, which we're 
        // moving to follow the while block
        Label *whileConditionTop = getLabel();
        Label *whileBlockStart = getLabel();
        branch(whileConditionTop);
    
        // save off the current stream while we gen code for the condition
        stitcher.push_back(new WhileCodeState(whileConditionTop, whileBlockStart, this));

        iCode = new InstructionStream();
    }

    void ICodeGenerator::endWhileExpression(Register condition)
    {
        WhileCodeState *ics = static_cast<WhileCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == While_state);

        branchConditional(ics->whileBody, condition);
        resetTopRegister();
        // stash away the condition expression and switch 
        // back to the main stream
        iCode = ics->swapStream(iCode);
        setLabel(ics->whileBody);         // mark the start of the while block
    }
  
    void ICodeGenerator::endWhileStatement()
    {
        // recover the while stream
        WhileCodeState *ics = static_cast<WhileCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == While_state);
        stitcher.pop_back();

        // mark the start of the condition code
        setLabel(ics->whileCondition);              // which is where continues will target

        // and re-attach it to the main stream
        mergeStream(ics->whileExpressionStream);

        if (ics->breakLabel != NULL)
            setLabel(ics->breakLabel);

        delete ics;

        resetTopRegister();
    }

    /***********************************************************************************************/

    void ICodeGenerator::beginForStatement(uint32)
    {
        Label *forCondition = getLabel();

        ForCodeState *ics = new ForCodeState(forCondition, getLabel(), this);

        branch(forCondition);

        stitcher.push_back(ics);

        iCode = new InstructionStream();        // begin the stream for collecting the condition expression
        setLabel(forCondition);

        resetTopRegister();
    }

    void ICodeGenerator::forCondition(Register condition)
    {
        ForCodeState *ics = static_cast<ForCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == For_state);

        // finsh off the test expression by adding the branch to the body
        branchConditional(ics->forBody, condition);

        iCode = ics->swapStream(iCode);     // switch back to main stream
        iCode = new InstructionStream();    //  begin the stream for collecting the increment expression

        ics->continueLabel = getLabel();
        setLabel(ics->continueLabel);       // can't lazily insert this since we haven't seen the body yet
                                            // ??? could just remember the offset

        resetTopRegister();
    }

    void ICodeGenerator::forIncrement()
    {
        ForCodeState *ics = static_cast<ForCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == For_state);

        // now switch back to the main stream
        iCode = ics->swapStream2(iCode);
        setLabel(ics->forBody);
    
        resetTopRegister();
    }

    void ICodeGenerator::endForStatement()
    {
        ForCodeState *ics = static_cast<ForCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == For_state);
        stitcher.pop_back();

        mergeStream(ics->forIncrementStream);
        mergeStream(ics->forConditionStream);

        if (ics->breakLabel != NULL)
            setLabel(ics->breakLabel);

        delete ics;
    }

    /***********************************************************************************************/

    void ICodeGenerator::beginDoStatement(uint32)
    {
        resetTopRegister();
  
        // mark the top of the loop body
        // and reserve a label for the condition
        Label *doBlock = getLabel();
        Label *doCondition = getLabel();
        setLabel(doBlock);
    
        stitcher.push_back(new DoCodeState(doBlock, doCondition, this));

        iCode = new InstructionStream();
    }

    void ICodeGenerator::endDoStatement()
    {
        DoCodeState *ics = static_cast<DoCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == Do_state);

        // mark the start of the do conditional
        setLabel(ics->doCondition);
        if (ics->continueLabel != NULL)
            setLabel(ics->continueLabel);

        resetTopRegister();
    }
  
    void ICodeGenerator::endDoExpression(Register condition)
    {
        DoCodeState *ics = static_cast<DoCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == Do_state);
        stitcher.pop_back();

        // add branch to top of do block
        branchConditional(ics->doBody, condition);
        if (ics->breakLabel != NULL)
            setLabel(ics->breakLabel);

        delete ics;

        resetTopRegister();
    }

    /***********************************************************************************************/

    void ICodeGenerator::beginSwitchStatement(uint32, Register expression)
    {
        // stash the control expression value
        resetTopRegister();
        Register control = op(MOVE_TO, expression);
        // build an instruction stream for the case statements, the case
        // expressions are generated into the main stream directly, the
        // case statements are then added back in afterwards.
        InstructionStream *x = new InstructionStream();
        SwitchCodeState *ics = new SwitchCodeState(control, this);
        ics->swapStream(x);
        stitcher.push_back(ics);
    }

    void ICodeGenerator::endCaseCondition(Register expression)
    {
        SwitchCodeState *ics = static_cast<SwitchCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == Switch_state);

        Label *caseLabel = getLabel();
        Register r = op(COMPARE_EQ, expression, ics->controlExpression);
        branchConditional(caseLabel, r);

        setLabel(ics->caseStatementsStream, caseLabel);     // mark the case in the Case Statement stream 
        resetTopRegister();
    }

    void ICodeGenerator::beginCaseStatement()
    {
        SwitchCodeState *ics = static_cast<SwitchCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == Switch_state);
        iCode = ics->swapStream(iCode);                     // switch to Case Statement stream
    }

    void ICodeGenerator::endCaseStatement()
    {
        SwitchCodeState *ics = static_cast<SwitchCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == Switch_state);             // do more to guarantee correct blocking?
        iCode = ics->swapStream(iCode);                     // switch back to Case Conditional stream
        resetTopRegister();
    }

    void ICodeGenerator::beginDefaultStatement()
    {
        SwitchCodeState *ics = static_cast<SwitchCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == Switch_state);
        ASSERT(ics->defaultLabel == NULL);
        ics->defaultLabel = getLabel();
        setLabel(ics->caseStatementsStream, ics->defaultLabel);
        iCode = ics->swapStream(iCode);                    // switch to Case Statement stream
    }

    void ICodeGenerator::endDefaultStatement()
    {
        SwitchCodeState *ics = static_cast<SwitchCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == Switch_state);
        ASSERT(ics->defaultLabel != NULL);          // do more to guarantee correct blocking?
        iCode = ics->swapStream(iCode);             // switch to Case Statement stream
        resetTopRegister();
    }

    void ICodeGenerator::endSwitchStatement()
    {
        SwitchCodeState *ics = static_cast<SwitchCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == Switch_state);
        stitcher.pop_back();

        // ground out the case chain at the default block or fall thru
        // to the break label
        if (ics->defaultLabel != NULL)       
            branch(ics->defaultLabel);
        else {
            if (ics->breakLabel == NULL)
                ics->breakLabel = getLabel();
            branch(ics->breakLabel);
        }
    
        // dump all the case statements into the main stream
        mergeStream(ics->caseStatementsStream);

        if (ics->breakLabel != NULL)
            setLabel(ics->breakLabel);

        delete ics;
    }


    /***********************************************************************************************/

    void ICodeGenerator::beginIfStatement(uint32, Register condition)
    {
        Label *elseLabel = getLabel();
    
        stitcher.push_back(new IfCodeState(elseLabel, NULL, this));

        Register notCond = op(NOT, condition);
        branchConditional(elseLabel, notCond);

        resetTopRegister();
    }

    void ICodeGenerator::beginElseStatement(bool hasElse)
    {
        IfCodeState *ics = static_cast<IfCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == If_state);

        if (hasElse) {
            Label *beyondElse = getLabel();
            ics->beyondElse = beyondElse;
            branch(beyondElse);
        }
        setLabel(ics->elseLabel);
        resetTopRegister();
    }

    void ICodeGenerator::endIfStatement()
    {
        IfCodeState *ics = static_cast<IfCodeState *>(stitcher.back());
        ASSERT(ics->stateKind == If_state);
        stitcher.pop_back();

        if (ics->beyondElse != NULL) {     // had an else
            setLabel(ics->beyondElse);   // the beyond else label
        }
    
        delete ics;
        resetTopRegister();
    }

    /***********************************************************************************************/

    void ICodeGenerator::breakStatement()
    {
        for (std::vector<ICodeState *>::reverse_iterator p = stitcher.rbegin(); p != stitcher.rend(); p++) {
            if ((*p)->breakLabel != NULL) {
                branch((*p)->breakLabel);
                return;
            }
            if (((*p)->stateKind == While_state)
                    || ((*p)->stateKind == Do_state)
                    || ((*p)->stateKind == For_state)
                    || ((*p)->stateKind == Switch_state)) {
                (*p)->breakLabel = getLabel();
                branch((*p)->breakLabel);
                return;
            }
        }
        NOT_REACHED("no break target available");
    }

    void ICodeGenerator::continueStatement()
    {
        for (std::vector<ICodeState *>::reverse_iterator p = stitcher.rbegin(); p != stitcher.rend(); p++) {
            if ((*p)->continueLabel != NULL) {
                branch((*p)->continueLabel);
                return;
            }
            if (((*p)->stateKind == While_state)
                    || ((*p)->stateKind == Do_state)
                    || ((*p)->stateKind == For_state)) {
                (*p)->continueLabel = getLabel();
                branch((*p)->continueLabel);
                return;
            }
        }
        NOT_REACHED("no continue target available");
    }
    
    /***********************************************************************************************/


    char *opcodeName[] = {
            "nop",
            "move_to",
            "load_var",
            "save_var",
            "load_imm",
            "load_name",
            "save_name",
            "new_object",
            "new_array",
            "get_prop",
            "set_prop",
            "get_element",
            "set_element",
            "add",
            "subtract",
            "multiply",
            "divide",
            "compare",
            "compare",
            "compare",
            "compare",
            "compare",
            "compare",
            "not",
            "branch",
            "branch_lt",
            "branch_le",
            "branch_eq",
            "branch_ne",
            "branch_ge",
            "branch_gt",
            "return",
            "call"
    };

    std::ostream &operator<<(std::ostream &s, StringAtom &str)
    {
        for (String::iterator i = str.begin(); i != str.end(); i++)
            s << (char)*i;
        return s;
    }

    std::ostream &ICodeGenerator::print(std::ostream &s)
    {
        s << "ICG! " << iCode->size() << "\n";
        for (InstructionIterator i = iCode->begin(); i != iCode->end(); i++) {

            for (LabelList::iterator k = labels.begin(); k != labels.end(); k++)
                if ((ptrdiff_t)(*k)->itsOffset == (i - iCode->begin())) {
                    //s << "label #" << (k - labels.begin()) << ":\n";
                    s << "#" << (i - iCode->begin());
                    break;
                }
        
            Instruction *instr = *i;
            s  << "\t" << std::setiosflags( std::ostream::left ) << std::setw(16) << opcodeName[instr->itsOp];
            switch (instr->itsOp) {
                case LOAD_NAME :
                    {
                        LoadName *t = static_cast<LoadName * >(instr);
                        s << "R" << t->itsOperand1 << ", \"" << *t->itsOperand2 <<  "\"";
                    }
                    break;
                case SAVE_NAME :
                    {
                        SaveName *t = static_cast<SaveName * >(instr);
                        s << "\"" << *t->itsOperand1 <<  "\", R" << t->itsOperand2;
                    }
                    break;
                case NEW_OBJECT :
                    {
                        NewObject *t = static_cast<NewObject * >(instr);
                        s << "R" << t->itsOperand1;
                    }
                    break;
                case NEW_ARRAY :
                    {
                        NewArray *t = static_cast<NewArray * >(instr);
                        s << "R" << t->itsOperand1;
                    }
                    break;
                case GET_PROP :
                    {
                        GetProp *t = static_cast<GetProp * >(instr);
                        s << "R" << t->itsOperand1 << ", R" << t->itsOperand2 << "[\"" << *t->itsOperand3 << "\"]";
                    }
                    break;
                case SET_PROP :
                    {
                        SetProp *t = static_cast<SetProp * >(instr);
                        s << "R" << t->itsOperand1 << "[\"" << *t->itsOperand2 <<  "\"], R" << t->itsOperand3;
                    }
                    break;
                case GET_ELEMENT :
                    {
                        GetElement *t = static_cast<GetElement * >(instr);
                        s << "R" << t->itsOperand1 << ", R" << t->itsOperand2 << "[R" << t->itsOperand3 << "]";
                    }
                    break;
                case SET_ELEMENT :
                    {
                        SetElement *t = static_cast<SetElement * >(instr);
                        s << "R" << t->itsOperand1 << "[R" << t->itsOperand2 <<  "], R" << t->itsOperand3;
                    }
                    break;
                case LOAD_IMMEDIATE :
                    {
                        LoadImmediate *t = static_cast<LoadImmediate * >(instr);
                        s << "R" << t->itsOperand1 << ", " << t->itsOperand2;
                    }
                    break;
                case LOAD_VAR :
                    {
                        LoadVar *t = static_cast<LoadVar * >(instr);
                        s << "R" << t->itsOperand1 << ", Variable #" << t->itsOperand2;
                    }
                    break;
                case SAVE_VAR :
                    {
                        LoadVar *t = static_cast<LoadVar * >(instr);
                        s << "Variable #" << t->itsOperand1 << ", R" << t->itsOperand2;
                    }
                    break;
                case BRANCH :
                    {
                        ResolvedBranch *t = static_cast<ResolvedBranch * >(instr);
                        s << "instr #" << t->itsOperand1;
                    }
                    break;
                case BRANCH_LT :
                case BRANCH_LE :
                case BRANCH_EQ :
                case BRANCH_NE :
                case BRANCH_GE :
                case BRANCH_GT :
                    {
                        ResolvedBranchCond *t = static_cast<ResolvedBranchCond * >(instr);
                        s << "instr #" << t->itsOperand1 << ", R" << t->itsOperand2;
                    }
                    break;
                case ADD :
                case SUBTRACT :
                case MULTIPLY :
                case DIVIDE :
                case COMPARE_LT :
                case COMPARE_LE :
                case COMPARE_EQ :
                case COMPARE_NE :
                case COMPARE_GT :
                case COMPARE_GE :
                    {
                         Arithmetic *t = static_cast<Arithmetic * >(instr);
                         s << "R" << t->itsOperand1 << ", R" << t->itsOperand2 << ", R" << t->itsOperand3;
                    }
                    break;
                case MOVE_TO :
                case NOT :
                    {
                         Move *t = static_cast<Move * >(instr);
                         s << "R" << t->itsOperand1 << ", R" << t->itsOperand2;
                    }
                    break;
                case CALL :
                    {
                         Call *t = static_cast<Call * >(instr);
                         s << "R" << t->itsOperand1 << ", R" << t->itsOperand2 << "(";
                         RegisterList args = t->itsOperand3;
                         for (RegisterList::iterator r = args.begin(); r != args.end(); r++) {
                             s << "R" << (*r);
                             if ((r + 1) != args.end())
                                 s << ", ";
                         }
                         s << ")";
                    }
                    break;
                case RETURN :
                    {
                         Return *t = static_cast<Return * >(instr);
                         if (t->itsOperand1 != NotARegister)
                            s << "R" << t->itsOperand1;
                    }
                    break;
            }
            s << "\n";
        }
        for (LabelList::iterator k = labels.begin(); k != labels.end(); k++)
            if ((ptrdiff_t)(*k)->itsOffset == (iCode->end() - iCode->begin())) {
//                s << "label #" << (k - labels.begin()) << ":\n";
//                s << "#" << (i - iCode->begin());
            }
        return s;
    }


}   // namespace JavaScript
