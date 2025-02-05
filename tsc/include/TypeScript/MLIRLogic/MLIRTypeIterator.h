#ifndef MLIR_TYPESCRIPT_COMMONGENLOGIC_MLIRTYPEITERATOR_H_
#define MLIR_TYPESCRIPT_COMMONGENLOGIC_MLIRTYPEITERATOR_H_

#include "TypeScript/TypeScriptOps.h"

#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"

#include <functional>

namespace mlir_ts = mlir::typescript;

namespace typescript
{

class MLIRTypeIterator
{
  public:
    std::function<bool(mlir::Type)> f;

    MLIRTypeIterator(std::function<bool(mlir::Type)> f_) : f(f_)
    {
    }

    // TODO: add logic to resolve class type, interafce type, etc
    bool iterate(mlir::Type def)
    {
        if (!f(def))
            return false;

        auto result = llvm::TypeSwitch<mlir::Type, bool>(def)
                          .Case<mlir_ts::ArrayType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              return true;
                          })
                          .Case<mlir_ts::BoundFunctionType>([&](auto t) {
                              for (auto subType : t.getInputs())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              for (auto subType : t.getResults())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              return true;
                          })
                          .Case<mlir_ts::BoundRefType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              return true;
                          })
                          .Case<mlir_ts::ClassType>([&](auto t) {
                              // TODO:
                              return true;
                          })
                          .Case<mlir_ts::ClassStorageType>([&](auto t) {
                              // TODO:
                              return true;
                          })
                          .Case<mlir_ts::ConstArrayType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              return true;
                          })
                          .Case<mlir_ts::ConstArrayValueType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              return true;
                          })
                          .Case<mlir_ts::ConstTupleType>([&](auto t) {
                              for (auto subType : t.getFields())
                              {
                                  if (!iterate(subType.type))
                                      return false;
                              }

                              return true;
                          })
                          .Case<mlir_ts::EnumType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;

                              return true;
                          })
                          .Case<mlir_ts::FunctionType>([&](auto t) {
                              for (auto subType : t.getInputs())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              for (auto subType : t.getResults())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              return true;
                          })
                          .Case<mlir_ts::HybridFunctionType>([&](auto t) {
                              for (auto subType : t.getInputs())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              for (auto subType : t.getResults())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              return true;
                          })
                          .Case<mlir_ts::InferType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              return true;
                          })
                          .Case<mlir_ts::InterfaceType>([&](auto t) {
                              // TODO:
                              return true;
                          })
                          .Case<mlir_ts::LiteralType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              return true;
                          })
                          .Case<mlir_ts::OptionalType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              return true;
                          })
                          .Case<mlir_ts::RefType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              return true;
                          })
                          .Case<mlir_ts::TupleType>([&](auto t) {
                              for (auto subType : t.getFields())
                              {
                                  if (!iterate(subType.type))
                                      return false;
                              }

                              return true;
                          })
                          .Case<mlir_ts::UnionType>([&](auto t) {
                              for (auto subType : t.getTypes())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              return true;
                          })
                          .Case<mlir_ts::IntersectionType>([&](auto t) {
                              for (auto subType : t.getTypes())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              return true;
                          })
                          .Case<mlir_ts::ValueRefType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;

                              return true;
                          })
                          .Case<mlir_ts::ConditionalType>([&](auto t) {
                              if (!iterate(t.getCheckType()))
                                  return false;
                              if (!iterate(t.getExtendsType()))
                                  return false;
                              if (!iterate(t.getTrueType()))
                                  return false;
                              if (!iterate(t.getFalseType()))
                                  return false;

                              return true;
                          })
                          .Case<mlir_ts::IndexAccessType>([&](auto t) {
                              if (!iterate(t.getType()))
                                  return false;
                              if (!iterate(t.getIndexType()))
                                  return false;

                              return true;
                          })                          
                          .Case<mlir_ts::KeyOfType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;

                              return true;
                          })                          
                          .Case<mlir_ts::MappedType>([&](auto t) {
                              if (!iterate(t.getElementType()))
                                  return false;
                              if (!iterate(t.getNameType()))
                                  return false;
                              if (!iterate(t.getConstrainType()))
                                  return false;

                              return true;
                          })                          
                          .Case<mlir_ts::TypeReferenceType>([&](auto t) {
                              for (auto subType : t.getTypes())
                              {
                                  if (!iterate(subType))
                                      return false;
                              }

                              return true;
                          })                          
                          .Case<mlir_ts::NeverType>([&](auto) {
                              return true;
                          })                          
                          .Case<mlir_ts::UnknownType>([&](auto) {
                              return true;
                          })                          
                          .Case<mlir_ts::AnyType>([&](auto) {
                              return true;
                          })                          
                          .Case<mlir_ts::NumberType>([&](auto) {
                              return true;
                          })                          
                          .Case<mlir_ts::StringType>([&](auto) {
                              return true;
                          })                          
                          .Case<mlir::IntegerType>([&](auto) {
                              return true;
                          })                          
                          .Case<mlir::FloatType>([&](auto) {
                              return true;
                          })                          
                          .Case<mlir::IndexType>([&](auto) {
                              return true;
                          })                          
                          .Default([](mlir::Type t) { 
                            LLVM_DEBUG(llvm::dbgs() << "\n!! Type Iteration is not implemented for : " << t << "\n";);
                            llvm_unreachable("not implemented");
                            return true; 
                          });

        return result;
    }
};

class MLIRTypeIteratorLogic
{
  public:
    MLIRTypeIteratorLogic() = default;

    void forEach(mlir::Type type, std::function<bool(mlir::Type)> f)
    {
        MLIRTypeIterator iter(f);
        iter.iterate(type);
    }

    bool some(mlir::Type type, std::function<bool(mlir::Type)> f)
    {
        auto result = false;
        forEach(type, [&](mlir::Type type) {
            result |= f(type);
            return !result;
        });

        return result;
    }

    bool every(mlir::Type type, std::function<bool(mlir::Type)> f)
    {
        auto result = true;
        forEach(type, [&](mlir::Type type) {
            result &= f(type);
            return result;
        });

        return result;
    }
};

} // namespace typescript

#endif // MLIR_TYPESCRIPT_COMMONGENLOGIC_MLIRTYPEITERATOR_H_
