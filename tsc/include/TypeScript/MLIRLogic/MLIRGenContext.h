#ifndef MLIR_TYPESCRIPT_MLIRGENCONTEXT_H_
#define MLIR_TYPESCRIPT_MLIRGENCONTEXT_H_

#include "TypeScript/TypeScriptDialect.h"
#include "TypeScript/TypeScriptOps.h"

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Types.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"

#include "TypeScript/DOM.h"
#include "TypeScript/MLIRLogic/MLIRGenStore.h"
#include "TypeScript/MLIRLogic/MLIRTypeHelper.h"

#include "parser_types.h"

#include <numeric>

using namespace ::typescript;
using namespace ts;
namespace mlir_ts = mlir::typescript;

using llvm::ArrayRef;
using llvm::cast;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::isa;
using llvm::makeArrayRef;
using llvm::SmallVector;
using llvm::StringRef;
using llvm::Twine;

namespace
{

struct PassResult
{
    PassResult() : functionReturnTypeShouldBeProvided(false)
    {
    }

    mlir::Type functionReturnType;
    bool functionReturnTypeShouldBeProvided;
    llvm::StringMap<ts::VariableDeclarationDOM::TypePtr> outerVariables;
    SmallVector<mlir_ts::FieldInfo> extraFieldsInThisContext;
};

struct GenContext
{
    GenContext() = default;

    void clearScopeVars()
    {
        passResult = nullptr;
        capturedVars = nullptr;

        currentOperation = nullptr;
    }

    // TODO: you are using "theModule.getBody()->clear();", do you need this hack anymore?
    void clean()
    {
        if (cleanUps)
        {
            for (auto op : *cleanUps)
            {
                op->dropAllDefinedValueUses();
                op->dropAllUses();
                op->dropAllReferences();
                op->erase();
            }

            delete cleanUps;
            cleanUps = nullptr;
        }

        if (passResult)
        {
            delete passResult;
            passResult = nullptr;
        }

        cleanState();

        cleanFuncOp();
    }

    void cleanState()
    {
        if (state)
        {
            delete state;
            state = nullptr;
        }
    }

    void cleanFuncOp()
    {
        if (funcOp)
        {
            funcOp->dropAllDefinedValueUses();
            funcOp->dropAllUses();
            funcOp->dropAllReferences();
            funcOp->erase();
        }
    }

    bool allowPartialResolve;
    bool dummyRun;
    bool allowConstEval;
    bool allocateVarsInContextThis;
    bool allocateVarsOutsideOfOperation;
    bool skipProcessed;
    bool rediscover;
    bool discoverParamsOnly;
    bool insertIntoParentScope;
    mlir::Operation *currentOperation;
    mlir_ts::FuncOp funcOp;
    llvm::StringMap<ts::VariableDeclarationDOM::TypePtr> *capturedVars;
    mlir::Type thisType;
    mlir::Type receiverFuncType;
    mlir::Type receiverType;
    PassResult *passResult;
    mlir::SmallVector<mlir::Block *> *cleanUps;
    NodeArray<Statement> generatedStatements;
    llvm::StringMap<mlir::Type> typeAliasMap;
    llvm::StringMap<std::pair<TypeParameterDOM::TypePtr, mlir::Type>> typeParamsWithArgs;
    ArrayRef<mlir::Value> callOperands;
    int *state;
};

struct ValueOrLogicalResult 
{
    ValueOrLogicalResult() = default;
    ValueOrLogicalResult(mlir::LogicalResult result) : result(result) {};
    ValueOrLogicalResult(mlir::Value value) : result(mlir::success()), value(value) {};

    mlir::LogicalResult result;
    mlir::Value value;

    operator bool()
    {
        return mlir::succeeded(result);
    }

    bool failed()
    {
        return mlir::failed(result);
    }

    bool failed_or_no_value()
    {
        return failed() || !value;
    }

    operator mlir::LogicalResult()
    {
        return failed_or_no_value() ? mlir::failure() : mlir::success();
    } 

    operator mlir::Value()
    {
        return value;
    }    
};

#define V(x) static_cast<mlir::Value>(x)

} // namespace

#endif // MLIR_TYPESCRIPT_MLIRGENCONTEXT_H_
