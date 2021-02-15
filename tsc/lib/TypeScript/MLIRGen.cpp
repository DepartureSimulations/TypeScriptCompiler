#include "TypeScript/MLIRGen.h"
#include "TypeScript/TypeScriptDialect.h"
#include "TypeScript/TypeScriptOps.h"

#include "TypeScript/DOM.h"

#include "mlir/IR/Types.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"

#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/Dialect/SCF/SCF.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/Support/raw_ostream.h"

#include "TypeScriptLexerANTLR.h"
#include "TypeScriptParserANTLR.h"
#include "TypeScript/VisitorAST.h"

#include <numeric>

using namespace mlir::typescript;
using namespace typescript;

using llvm::ArrayRef;
using llvm::cast;
using llvm::dyn_cast;
using llvm::isa;
using llvm::makeArrayRef;
using llvm::ScopedHashTableScope;
using llvm::SmallVector;
using llvm::StringRef;
using llvm::Twine;

namespace
{
    struct GenContext
    {
        bool allowPartialResolve;
    };

    /// Implementation of a simple MLIR emission from the TypeScript AST.
    ///
    /// This will emit operations that are specific to the TypeScript language, preserving
    /// the semantics of the language and (hopefully) allow to perform accurate
    /// analysis and transformation based on these high level semantics.
    class MLIRGenImpl
    {
        using VariablePairT = std::pair<mlir::Value, VariableDeclarationDOM *>;
        using SymbolTableScopeT = llvm::ScopedHashTableScope<StringRef, VariablePairT>;

    public:
        MLIRGenImpl(const mlir::MLIRContext &context) : builder(&const_cast<mlir::MLIRContext &>(context))
        {
            fileName = "<unknown>";
        }

        MLIRGenImpl(const mlir::MLIRContext &context, const llvm::StringRef &fileNameParam) : builder(&const_cast<mlir::MLIRContext &>(context))
        {
            fileName = fileNameParam;
        }

        /// Public API: convert the AST for a TypeScript module (source file) to an MLIR
        /// Module operation.
        mlir::ModuleOp mlirGen(ModuleAST &module)
        {
            // We create an empty MLIR module and codegen functions one at a time and
            // add them to the module.
            theModule = mlir::ModuleOp::create(loc(module.getLoc()), fileName);
            builder.setInsertionPointToStart(theModule.getBody());

            declareAllFunctionDeclarations(module);

            //theModuleDOM.parseTree = module;

            // Process generating here
            GenContext genContext = {0};
            for (auto &statement : module)
            {
                if (failed(mlirGenStatement(*statement.get(), genContext)))
                {
                    return nullptr;
                }
            }

            // Verify the module after we have finished constructing it, this will check
            // the structural properties of the IR and invoke any specific verifiers we
            // have on the TypeScript operations.
            if (failed(mlir::verify(theModule)))
            {
                // TODO: uncomment it
                //theModule.emitError("module verification error");
                //return nullptr;
            }

            return theModule;
        }

        mlir::LogicalResult declareAllFunctionDeclarations(ModuleAST &module)
        {
            // TODO: finish it
            return mlir::success();
        }

        mlir::LogicalResult mlirGenStatement(NodeAST &statementAST, const GenContext &genContext)
        {
            // TODO:
            if (auto *functionDeclarationAST = llvm::dyn_cast<FunctionDeclarationAST>(&statementAST))
            {
                /*
                auto func = mlirGen(*functionDeclarationAST);
                if (!func)
                {
                    return mlir::failed();
                }
                */
            } 
            else 
            {
                llvm_unreachable("unknown statement type");
            }

            return mlir::success();
        }        

        /*
        mlir::LogicalResult declareAllFunctionDeclarations(TypeScriptParserANTLR::MainContext *module)
        {
            auto unresolvedFunctions = -1;

            // VisitorAST
            // TODO: test recursive references
            do
            {
                auto unresolvedFunctionsCurrentRun = 0;
                FilterVisitorAST<TypeScriptParserANTLR::FunctionDeclarationContext> visitorAST(
                    [&](auto *funcDecl) {
                        GenContext genContextDecl = {0};
                        genContextDecl.allowPartialResolve = true;

                        auto funcOpAndFuncProto = mlirGenFunctionPrototype(funcDecl, genContextDecl);
                        auto result = std::get<2>(funcOpAndFuncProto);
                        if (!result)
                        {
                            unresolvedFunctionsCurrentRun++;
                            return;
                        }

                        auto funcOp = std::get<0>(funcOpAndFuncProto);
                        auto &funcProto = std::get<1>(funcOpAndFuncProto);
                        if (auto funcOp = theModule.lookupSymbol<mlir::FuncOp>(funcProto->getName()))
                        {
                            return;
                        }

                        functionMap.insert({funcOp.getName(), funcOp});
                    });
                visitorAST.visit(module);

                if (unresolvedFunctionsCurrentRun == unresolvedFunctions)
                {
                    emitError(loc(module)) << "can't resolve function recursive references '" << fileName << "'";
                    return mlir::failure();
                }

                unresolvedFunctions = unresolvedFunctionsCurrentRun;
            } while (unresolvedFunctions > 0);

            return mlir::success();
        }
        */

        /*
        mlir::LogicalResult mlirGen(TypeScriptParserANTLR::DeclarationContext *declarationAST, const GenContext &genContext)
        {
            if (auto *functionDeclaration = declarationAST->functionDeclaration())
            {
                mlirGen(functionDeclaration, genContext);
            }
            else
            {
                llvm_unreachable("unknown statement");
            }

            return mlir::success();
        }

        std::vector<std::unique_ptr<FunctionParamDOM>> mlirGen(TypeScriptParserANTLR::FormalParametersContext *formalParametersContextAST,
                                                               const GenContext &genContext)
        {
            std::vector<std::unique_ptr<FunctionParamDOM>> params;
            if (!formalParametersContextAST)
            {
                return params;
            }

            auto formalParams = formalParametersContextAST->formalParameter();

            // add extra parameter to send number of parameters
            auto anyOptionalParam = std::find_if(formalParams.begin(), formalParams.end(), [](auto &param) {
                                        if (param->QUESTION_TOKEN())
                                        {
                                            return true;
                                        }

                                        auto initializer = param->initializer();
                                        if (!initializer)
                                        {
                                            return false;
                                        }

                                        return !!initializer->assignmentExpression();
                                    }) != formalParams.end();

            if (anyOptionalParam)
            {
                params.push_back(std::make_unique<FunctionParamDOM>(nullptr, "__count_params", builder.getI32Type(), false));
            }

            for (auto &arg : formalParams)
            {
                auto name = arg->IdentifierName()->getText();
                mlir::Type type;
                auto isOptional = !!arg->QUESTION_TOKEN();
                auto typeParameter = arg->typeParameter();
                if (typeParameter)
                {
                    auto type = getType(typeParameter);
                    if (!type)
                    {
                        return params;
                    }
                }

                // process init value
                tree::ParseTree *initValueTree = nullptr;
                auto initializer = arg->initializer();
                if (initializer)
                {
                    auto assignmentExpression = initializer->assignmentExpression();
                    if (assignmentExpression)
                    {
                        initValueTree = assignmentExpression;

                        // we need to add temporary block
                        auto tempFuncType = builder.getFunctionType(llvm::None, llvm::None);
                        auto tempFuncOp = mlir::FuncOp::create(loc(initializer), StringRef(name), tempFuncType);
                        auto &entryBlock = *tempFuncOp.addEntryBlock();

                        auto insertPoint = builder.saveInsertionPoint();
                        builder.setInsertionPointToStart(&entryBlock);

                        auto initValue = mlirGen(assignmentExpression, genContext);
                        if (initValue)
                        {
                            // TODO: set type if not provided
                            isOptional = true;
                            if (!type)
                            {
                                auto baseType = initValue.getType();
                                //type = OptionalType::get(baseType);
                                type = baseType;
                            }

                            // remove generated node as we need to detect type only
                            initValue.getDefiningOp()->erase();
                        }

                        // remove temp block
                        builder.restoreInsertionPoint(insertPoint);
                        entryBlock.erase();
                    }
                }

                params.push_back(std::make_unique<FunctionParamDOM>(arg, name, type, isOptional, initValueTree));
            }

            return params;
        }

        std::tuple<mlir::FuncOp, FunctionPrototypeDOM::TypePtr, bool> mlirGenFunctionPrototype(TypeScriptParserANTLR::FunctionDeclarationContext *functionDeclarationAST,
                                                                                               const GenContext &genContext)
        {
            auto location = loc(functionDeclarationAST);

            std::vector<FunctionParamDOM::TypePtr> params = mlirGen(functionDeclarationAST->formalParameters(), genContext);
            SmallVector<mlir::Type> argTypes;
            auto argNumber = 0;
            auto argOptionalFrom = -1;

            for (const auto &param : params)
            {
                auto paramType = param->getType();
                if (!paramType)
                {
                    return std::make_tuple(mlir::FuncOp(), FunctionPrototypeDOM::TypePtr(nullptr), false);
                }

                argTypes.push_back(paramType);
                if (param->getIsOptional() && argOptionalFrom < 0)
                {
                    argOptionalFrom = argNumber;
                }

                argNumber++;
            }

            std::string name;
            auto *identifier = functionDeclarationAST->IdentifierName();
            if (identifier)
            {
                name = identifier->getText();
            }
            else
            {
                // auto calculate name
                // __func+location
            }

            auto funcProto = std::make_unique<FunctionPrototypeDOM>(functionDeclarationAST, name, std::move(params));

            mlir::FunctionType funcType;
            if (auto *typeParameter = functionDeclarationAST->typeParameter())
            {
                auto returnType = getType(typeParameter);
                funcType = builder.getFunctionType(argTypes, returnType);
            }
            else if (auto returnType = getReturnType(functionDeclarationAST, name, argTypes, funcProto, genContext))
            {
                funcType = builder.getFunctionType(argTypes, returnType);
            }
            else
            {
                // no return type
                funcType = builder.getFunctionType(argTypes, llvm::None);
            }

            SmallVector<mlir::NamedAttribute> attrs;
            // save info about optional parameters
            if (argOptionalFrom >= 0)
            {
                attrs.push_back(builder.getNamedAttr("OptionalFrom", builder.getI8IntegerAttr(argOptionalFrom)));
            }

            auto funcOp = mlir::FuncOp::create(location, StringRef(name), funcType, ArrayRef<mlir::NamedAttribute>(attrs));

            return std::make_tuple(funcOp, std::move(funcProto), true);
        }

        mlir::Type getReturnType(TypeScriptParserANTLR::FunctionDeclarationContext *functionDeclarationAST, std::string name,
                                 const SmallVector<mlir::Type> &argTypes, const FunctionPrototypeDOM::TypePtr &funcProto, const GenContext &genContext)
        {
            mlir::Type returnType;

            // check if we have any return with expration
            auto hasReturnStatementWithExpr = false;
            FilterVisitorAST<TypeScriptParserANTLR::ReturnStatementContext> visitorAST1(
                [&](auto *retStatement) {
                    if (auto *expression = retStatement->expression())
                    {
                        hasReturnStatementWithExpr = true;
                    }
                });
            visitorAST1.visit(functionDeclarationAST);

            if (!hasReturnStatementWithExpr)
            {
                return returnType;
            }

            auto partialDeclFuncType = builder.getFunctionType(argTypes, llvm::None);
            auto dummyFuncOp = mlir::FuncOp::create(loc(functionDeclarationAST), StringRef(name), partialDeclFuncType);

            // simulate scope
            SymbolTableScopeT varScope(symbolTable);

            returnType = mlirGenFunctionBody(functionDeclarationAST, dummyFuncOp, funcProto, genContext, true);
            return returnType;
        }

        mlir::LogicalResult mlirGen(TypeScriptParserANTLR::FunctionDeclarationContext *functionDeclarationAST, const GenContext &genContext)
        {
            SymbolTableScopeT varScope(symbolTable);
            auto funcOpWithFuncProto = mlirGenFunctionPrototype(functionDeclarationAST, genContext);

            auto &funcOp = std::get<0>(funcOpWithFuncProto);
            auto &funcProto = std::get<1>(funcOpWithFuncProto);
            auto result = std::get<2>(funcOpWithFuncProto);
            if (!result || !funcOp)
            {
                return mlir::failure();
            }

            auto returnType = mlirGenFunctionBody(functionDeclarationAST, funcOp, funcProto, genContext);

            // set visibility index
            if (functionDeclarationAST->IdentifierName()->getText() != "main")
            {
                funcOp.setPrivate();
            }

            theModule.push_back(funcOp);
            functionMap.insert({funcOp.getName(), funcOp});
            theModuleDOM.getFunctionProtos().push_back(std::move(funcProto));

            return mlir::success();
        }

        mlir::Type mlirGenFunctionBody(TypeScriptParserANTLR::FunctionDeclarationContext *functionDeclarationAST,
                                       mlir::FuncOp funcOp, const FunctionPrototypeDOM::TypePtr &funcProto, const GenContext &genContext, bool dummyRun = false)
        {
            mlir::Type returnType;

            auto &entryBlock = *funcOp.addEntryBlock();

            // process function params
            for (const auto paramPairs : llvm::zip(funcProto->getArgs(), entryBlock.getArguments()))
            {
                if (failed(declare(*std::get<0>(paramPairs), std::get<1>(paramPairs))))
                {
                    return returnType;
                }
            }

            // allocate all params

            builder.setInsertionPointToStart(&entryBlock);

            auto arguments = entryBlock.getArguments();

            auto index = -1;
            for (const auto &param : funcProto->getArgs())
            {
                index++;

                // skip __const_params, it is not real param
                if (!param->getParseTree())
                {
                    continue;
                }

                mlir::Value paramValue;

                // alloc all args
                // process optional parameters
                if (param->getIsOptional() || param->getInitVal())
                {
                    // process init expression
                    auto location = loc(param->getInitVal());

                    auto countArgsValue = arguments[0];

                    auto paramOptionalOp = builder.create<ParamOptionalOp>(
                        location, 
                        mlir::MemRefType::get(ArrayRef<int64_t>(), param->getType()), 
                        arguments[index], 
                        countArgsValue, 
                        builder.getI32IntegerAttr(index));

                    paramValue = paramOptionalOp;

                    auto *defValueBlock = new mlir::Block();
                    paramOptionalOp.defaultValueRegion().push_back(defValueBlock);

                    auto sp = builder.saveInsertionPoint();
                    builder.setInsertionPointToStart(defValueBlock);

                    mlir::Value defaultValue;
                    auto assignmentExpression2 = dynamic_cast<TypeScriptParserANTLR::AssignmentExpressionContext *>(param->getInitVal());
                    if (assignmentExpression2)
                    {
                        defaultValue = mlirGen(assignmentExpression2, genContext);
                    }
                    else
                    {
                        llvm_unreachable("unknown statement");
                    }                    

                    builder.create<ParamDefaultValueOp>(location, defaultValue);

                    builder.restoreInsertionPoint(sp);
                }
                else
                {
                    paramValue = builder.create<ParamOp>(
                        loc(param->getParseTree()), 
                        mlir::MemRefType::get(ArrayRef<int64_t>(), param->getType()), 
                        arguments[index]);
                }

                if (paramValue)
                {
                    // redefine variable
                    param->SetReadWriteAccess();
                    declare(*param, paramValue, true);
                }
            }

            for (auto *statementListItem : functionDeclarationAST->functionBody()->functionBodyItem())
            {
                if (auto *statement = statementListItem->statement())
                {
                    mlirGen(statement, genContext);
                }
                else if (auto *declaration = statementListItem->declaration())
                {
                    mlirGen(declaration, genContext);
                }
                else
                {
                    llvm_unreachable("unknown statement");
                }
            }

            // add return
            mlir::ReturnOp returnOp;
            if (!entryBlock.empty())
            {
                returnOp = dyn_cast<mlir::ReturnOp>(entryBlock.back());
            }

            if (!returnOp)
            {
                returnOp = builder.create<mlir::ReturnOp>(loc(functionDeclarationAST->functionBody()));
            }
            else if (!returnOp.operands().empty())
            {
                // Otherwise, if this return operation has an operand then add a result to the function.
                funcOp.setType(builder.getFunctionType(funcOp.getType().getInputs(), *returnOp.operand_type_begin()));
            }

            if (returnOp.getNumOperands() > 0)
            {
                returnType = returnOp.getOperand(0).getType();
            }

            if (dummyRun)
            {
                entryBlock.erase();
            }

            return returnType;
        }

        mlir::LogicalResult mlirGen(TypeScriptParserANTLR::StatementContext *statementItemAST, const GenContext &genContext)
        {
            if (auto *expressionStatement = statementItemAST->expressionStatement())
            {
                mlirGen(expressionStatement->expression(), genContext);
                // ignore result in statement
                return mlir::success();
            }
            else if (auto *returnStatement = statementItemAST->returnStatement())
            {
                return mlirGen(returnStatement, genContext);
            }
            else
            {
                llvm_unreachable("unknown statement");
            }
        }

        mlir::LogicalResult mlirGen(TypeScriptParserANTLR::ReturnStatementContext *returnStatementAST, const GenContext &genContext)
        {
            if (auto *expression = returnStatementAST->expression())
            {
                auto expressionValue = mlirGen(expression, genContext);
                builder.create<mlir::ReturnOp>(loc(returnStatementAST), expressionValue);
            }
            else
            {
                builder.create<mlir::ReturnOp>(loc(returnStatementAST));
            }

            return mlir::success();
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::ExpressionContext *expressionAST, const GenContext &genContext)
        {
            if (auto *primaryExpression = expressionAST->primaryExpression())
            {
                return mlirGen(primaryExpression, genContext);
            }
            else if (auto *leftHandSideExpression = expressionAST->leftHandSideExpression())
            {
                return mlirGen(leftHandSideExpression, genContext);
            }
            else
            {
                llvm_unreachable("unknown statement");
            }
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::PrimaryExpressionContext *primaryExpression, const GenContext &genContext)
        {
            if (auto *literal = primaryExpression->literal())
            {
                return mlirGen(literal, genContext);
            }
            else if (auto *identifierReference = primaryExpression->identifierReference())
            {
                return mlirGen(identifierReference, genContext);
            }
            else
            {
                llvm_unreachable("unknown statement");
            }
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::LeftHandSideExpressionContext *leftHandSideExpression, const GenContext &genContext)
        {
            if (auto *callExpression = leftHandSideExpression->callExpression())
            {
                return mlirGen(callExpression, genContext);
            }
            else if (auto *memberExpression = leftHandSideExpression->memberExpression())
            {
                return mlirGen(memberExpression, genContext);
            }
            else
            {
                llvm_unreachable("unknown statement");
            }
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::AssignmentExpressionContext *assignmentExpressionContext, const GenContext &genContext)
        {
            if (auto *leftHandSideExpression = assignmentExpressionContext->leftHandSideExpression())
            {
                return mlirGen(leftHandSideExpression, genContext);
            }
            else
            {
                llvm_unreachable("unknown statement");
            }
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::CallExpressionContext *callExpression, const GenContext &genContext)
        {
            auto location = loc(callExpression);

            mlir::Value result;

            // get function ref.
            if (auto *memberExpression = callExpression->memberExpression())
            {
                result = mlirGen(memberExpression, genContext);
            }
            else if (auto *callExpressionRecursive = callExpression->callExpression())
            {
                result = mlirGen(callExpressionRecursive, genContext);
            }

            auto definingOp = result.getDefiningOp();
            if (definingOp)
            {
                auto opName = definingOp->getName().getStringRef();
                auto attrName = StringRef("identifier");
                if (definingOp->hasAttrOfType<mlir::FlatSymbolRefAttr>(attrName))
                {
                    auto calleeName = definingOp->getAttrOfType<mlir::FlatSymbolRefAttr>(attrName);
                    auto functionName = calleeName.getValue();

                    // resolve function
                    auto calledFuncIt = functionMap.find(functionName);
                    if (calledFuncIt == functionMap.end())
                    {
                        if (!genContext.allowPartialResolve)
                        {
                            emitError(location) << "no defined function found for '" << functionName << "'";
                        }

                        return nullptr;
                    }

                    auto calledFunc = calledFuncIt->second;

                    // process arguments
                    SmallVector<mlir::Value, 0> operands;

                    auto *argumentsContext = callExpression->arguments();
                    auto opArgsCount = argumentsContext ? argumentsContext->expression().size() : 0;
                    auto hasOptionalFrom = calledFunc.getOperation()->hasAttrOfType<mlir::IntegerAttr>("OptionalFrom");
                    if (hasOptionalFrom)
                    {
                        auto constNumOfParams = builder.create<mlir::ConstantOp>(location, builder.getI32Type(), builder.getI32IntegerAttr(opArgsCount));
                        operands.push_back(constNumOfParams);
                    }

                    mlirGen(argumentsContext, operands, genContext);

                    if (hasOptionalFrom)
                    {
                        auto funcArgsCount = calledFunc.getNumArguments();
                        auto optionalFrom = funcArgsCount - opArgsCount;
                        if (hasOptionalFrom && optionalFrom > 0)
                        {
                            // -1 to exclude count params
                            for (auto i = (size_t)opArgsCount; i < funcArgsCount - 1; i++)
                            {
                                operands.push_back(builder.create<UndefOp>(location, calledFunc.getType().getInput(i)));
                            }
                        }
                    }

                    // print - internal command;
                    if (functionName.compare(StringRef("print")) == 0 && mlir::succeeded(mlirGenPrint(location, operands)))
                    {
                        return nullptr;
                    }

                    // assert - internal command;
                    if (functionName.compare(StringRef("assert")) == 0 && operands.size() > 0 && mlir::succeeded(mlirGenAssert(location, operands)))
                    {
                        return nullptr;
                    }

                    // default call by name
                    auto callOp =
                        builder.create<CallOp>(
                            location,
                            calledFunc,
                            operands);

                    if (calledFunc.getType().getNumResults() > 0)
                    {
                        return callOp.getResult(0);
                    }

                    return nullptr;
                }
            }

            return nullptr;
        }

        mlir::LogicalResult mlirGenPrint(const mlir::Location &location, const SmallVector<mlir::Value, 0> &operands)
        {
            auto printOp =
                builder.create<PrintOp>(
                    location,
                    operands.front());

            return mlir::success();
        }

        mlir::LogicalResult mlirGenAssert(const mlir::Location &location, const SmallVector<mlir::Value, 0> &operands)
        {
            auto msg = StringRef("assert");
            if (operands.size() > 1)
            {
                auto param2 = operands[1];
                auto definingOpParam2 = param2.getDefiningOp();
                auto valueAttrName = StringRef("value");
                if (definingOpParam2)
                {
                    auto valueAttr = definingOpParam2->getAttrOfType<mlir::StringAttr>(valueAttrName);
                    if (valueAttr)
                    {
                        msg = valueAttr.getValue();
                        definingOpParam2->erase();
                    }
                }
            }

            auto assertOp =
                builder.create<AssertOp>(
                    location,
                    operands.front(),
                    mlir::StringAttr::get(msg, theModule.getContext()));

            return mlir::success();
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::MemberExpressionContext *memberExpression, const GenContext &genContext)
        {
            if (auto *primaryExpression = memberExpression->primaryExpression())
            {
                return mlirGen(primaryExpression, genContext);
            }
            else if (auto *memberExpressionRecursive = memberExpression->memberExpression())
            {
                return mlirGen(memberExpressionRecursive, genContext);
            }
            else
            {
                return mlirGenIdentifierName(memberExpression->IdentifierName());
            }
        }

        mlir::LogicalResult mlirGen(TypeScriptParserANTLR::ArgumentsContext *arguments, SmallVector<mlir::Value, 0> &operands, const GenContext &genContext)
        {
            for (auto &next : arguments->expression())
            {
                operands.push_back(mlirGen(next, genContext));
            }

            return mlir::success();
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::LiteralContext *literal, const GenContext &genContext)
        {
            if (auto *nullLiteral = literal->nullLiteral())
            {
                return mlirGen(nullLiteral, genContext);
            }
            else if (auto *booleanLiteral = literal->booleanLiteral())
            {
                return mlirGen(booleanLiteral, genContext);
            }
            else if (auto *numericLiteral = literal->numericLiteral())
            {
                return mlirGen(numericLiteral, genContext);
            }
            else
            {
                return mlirGenStringLiteral(literal->StringLiteral());
            }
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::NullLiteralContext *nullLiteral, const GenContext &genContext)
        {
            llvm_unreachable("not implemented");
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::BooleanLiteralContext *booleanLiteral, const GenContext &genContext)
        {
            bool result;
            if (booleanLiteral->TRUE_KEYWORD())
            {
                result = true;
            }
            else if (booleanLiteral->FALSE_KEYWORD())
            {
                result = false;
            }
            else
            {
                llvm_unreachable("not implemented");
            }

            return builder.create<mlir::ConstantOp>(
                loc(booleanLiteral),
                builder.getI1Type(),
                mlir::BoolAttr::get(result, theModule.getContext()));
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::NumericLiteralContext *numericLiteral, const GenContext &genContext)
        {
            if (auto *decimalLiteral = numericLiteral->DecimalLiteral())
            {
                return mlirGenDecimalLiteral(decimalLiteral);
            }
            else if (auto *decimalIntegerLiteral = numericLiteral->DecimalIntegerLiteral())
            {
                return mlirGenDecimalIntegerLiteral(decimalIntegerLiteral);
            }
            else if (auto *decimalBigIntegerLiteral = numericLiteral->DecimalBigIntegerLiteral())
            {
                return mlirGenDecimalBigIntegerLiteral(decimalBigIntegerLiteral);
            }
            else if (auto *binaryBigIntegerLiteral = numericLiteral->BinaryBigIntegerLiteral())
            {
                return mlirGenBinaryBigIntegerLiteral(binaryBigIntegerLiteral);
            }
            else if (auto *octalBigIntegerLiteral = numericLiteral->OctalBigIntegerLiteral())
            {
                return mlirGenOctalBigIntegerLiteral(octalBigIntegerLiteral);
            }
            else if (auto *hexBigIntegerLiteral = numericLiteral->HexBigIntegerLiteral())
            {
                return mlirGenHexBigIntegerLiteral(hexBigIntegerLiteral);
            }
            else
            {
                llvm_unreachable("unknown statement");
            }
        }

        mlir::Value mlirGen(TypeScriptParserANTLR::IdentifierReferenceContext *identifierReference, const GenContext &genContext)
        {
            return mlirGenIdentifierName(identifierReference->IdentifierName());
        }

        mlir::Value mlirGenIdentifierName(antlr4::tree::TerminalNode *identifierName)
        {
            // resolve name
            auto name = identifierName->getText();

            auto value = resolve(name);
            if (value.first)
            {
                // load value if memref
                if (value.second)
                {
                    return builder.create<mlir::LoadOp>(value.first.getLoc(), value.first);
                }

                return value.first;
            }

            // unresolved reference (for call for example)
            return IdentifierReference::create(loc(identifierName), name);
        }

        mlir::Value mlirGenStringLiteral(antlr4::tree::TerminalNode *stringLiteral)
        {
            auto text = stringLiteral->getText();
            auto innerText = text.substr(1, text.length() - 2);

            return builder.create<mlir::ConstantOp>(
                loc(stringLiteral),
                mlir::UnrankedTensorType::get(builder.getI1Type()),
                builder.getStringAttr(StringRef(innerText)));
        }

        mlir::Value mlirGenDecimalLiteral(antlr4::tree::TerminalNode *decimalLiteral)
        {
            // TODO:
            llvm_unreachable("not implemented");
        }

        mlir::Value mlirGenDecimalIntegerLiteral(antlr4::tree::TerminalNode *decimalIntegerLiteral)
        {
            return builder.create<mlir::ConstantOp>(
                loc(decimalIntegerLiteral),
                builder.getI32Type(),
                builder.getI32IntegerAttr(std::stoi(decimalIntegerLiteral->getText())));
        }

        mlir::Value mlirGenDecimalBigIntegerLiteral(antlr4::tree::TerminalNode *decimalBigIntegerLiteraligIntegerLiteral)
        {
            // TODO:
            llvm_unreachable("not implemented");
        }

        mlir::Value mlirGenBinaryBigIntegerLiteral(antlr4::tree::TerminalNode *binaryBigIntegerLiteral)
        {
            // TODO:
            llvm_unreachable("not implemented");
        }

        mlir::Value mlirGenOctalBigIntegerLiteral(antlr4::tree::TerminalNode *octalBigIntegerLiteral)
        {
            // TODO:
            llvm_unreachable("not implemented");
        }

        mlir::Value mlirGenHexBigIntegerLiteral(antlr4::tree::TerminalNode *hexBigIntegerLiteral)
        {
            // TODO:
            llvm_unreachable("not implemented");
        }

        mlir::Type getType(TypeScriptParserANTLR::TypeParameterContext *typeParameterAST)
        {
            if (auto *typeDeclaration = typeParameterAST->typeDeclaration())
            {
                return getType(typeDeclaration);
            }

            return getAnyType();
        }

        mlir::Type getType(TypeScriptParserANTLR::TypeDeclarationContext *typeDeclarationAST)
        {
            if (auto boolean = typeDeclarationAST->BOOLEAN_KEYWORD())
            {
                return builder.getI1Type();
            }
            else if (auto boolean = typeDeclarationAST->NUMBER_KEYWORD())
            {
                return builder.getF32Type();
            }
            else if (auto boolean = typeDeclarationAST->BIGINT_KEYWORD())
            {
                return builder.getI64Type();
            }
            else if (auto boolean = typeDeclarationAST->STRING_KEYWORD())
            {
                return getStringType();
            }

            return getAnyType();
        }
        */

        mlir::Type getStringType()
        {
            return mlir::UnrankedMemRefType::get(builder.getI1Type(), 0);
        }

        mlir::Type getAnyType()
        {
            return mlir::UnrankedMemRefType::get(builder.getI1Type(), 0);
        }

        mlir::LogicalResult declare(VariableDeclarationDOM &var, mlir::Value value, bool redeclare = false)
        {
            const auto &name = var.getName();
            if (!redeclare && symbolTable.count(name))
            {
                return mlir::failure();
            }

            symbolTable.insert(name, {value, &var});
            return mlir::success();
        }

        std::pair<mlir::Value, bool> resolve(StringRef name)
        {
            auto varIt = symbolTable.lookup(name);
            if (varIt.first)
            {
                return std::make_pair(varIt.first, varIt.second->getReadWriteAccess());
            }

            return std::make_pair(mlir::Value(), false);
        }

    private:
        /// Helper conversion for a TypeScript AST location to an MLIR location.
        mlir::Location loc(const typescript::TextRange &loc)
        {
            return builder.getFileLineColLoc(builder.getIdentifier(fileName), loc.pos, loc.end);
        }

        /// A "module" matches a TypeScript source file: containing a list of functions.
        mlir::ModuleOp theModule;
        ModuleDOM theModuleDOM;

        /// The builder is a helper class to create IR inside a function. The builder
        /// is stateful, in particular it keeps an "insertion point": this is where
        /// the next operations will be introduced.
        mlir::OpBuilder builder;

        mlir::StringRef fileName;

        llvm::ScopedHashTable<StringRef, VariablePairT> symbolTable;

        llvm::StringMap<mlir::FuncOp> functionMap;
    };

} // namespace

namespace typescript
{
    llvm::StringRef dumpFromSource(const llvm::StringRef &source)
    {
        antlr4::ANTLRInputStream input((std::string)source);
        typescript::TypeScriptLexerANTLR lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        typescript::TypeScriptParserANTLR parser(&tokens);
        auto *moduleAST = parser.main();
        return llvm::StringRef(moduleAST->toStringTree());
    }

    mlir::OwningModuleRef mlirGenFromSource(const mlir::MLIRContext &context, const llvm::StringRef &source, const llvm::StringRef &fileName)
    {
        antlr4::ANTLRInputStream input((std::string)source);
        typescript::TypeScriptLexerANTLR lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        typescript::TypeScriptParserANTLR parser(&tokens);
        return MLIRGenImpl(context, fileName).mlirGen(*parser.getModuleAST().get());
    }

} // namespace typescript
