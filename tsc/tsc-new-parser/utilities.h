#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <regex>
#include <functional>

#include "core.h"
#include "enums.h"
#include "types.h"
#include "node_test.h"
#include "scanner.h"
#include "parser.h"

namespace ts
{
    namespace Extension
    {
        static const string Ts = S(".ts");
        static const string Tsx = S(".tsx");
        static const string Dts = S(".d.ts");
        static const string Js = S(".js");
        static const string Jsx = S(".jsx");
        static const string Json = S(".json");
        static const string TsBuildInfo = S(".tsbuildinfo");
    };

    inline auto positionIsSynthesized(number pos) -> boolean
    {
        // This is a fast way of testing the following conditions:
        //  pos === undefined || pos === null || isNaN(pos) || pos < 0;
        return !(pos >= 0);
    }

    inline auto getScriptKindFromFileName(string fileName) -> ScriptKind
    {
        auto pos = fileName.find(S('.'));
        if (pos == string::npos)
        {
            return ScriptKind::Unknown;
        }

        auto ext = fileName.substr(pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](char_t c) { return std::tolower(c); });
        if (ext == S("js"))
            return ScriptKind::JS;
        if (ext == S("jsx"))
            return ScriptKind::JSX;
        if (ext == S("ts"))
            return ScriptKind::TS;
        if (ext == S("tsx"))
            return ScriptKind::TSX;
        if (ext == S("json"))
            return ScriptKind::JSON;
        return ScriptKind::Unknown;
    }

    inline auto ensureScriptKind(string fileName, ScriptKind scriptKind = ScriptKind::Unknown) -> ScriptKind
    {
        // Using scriptKind as a condition handles both:
        // - 'scriptKind' is unspecified and thus it is `undefined`
        // - 'scriptKind' is set and it is `Unknown` (0)
        // If the 'scriptKind' is 'undefined' or 'Unknown' then we attempt
        // to get the ScriptKind from the file name. If it cannot be resolved
        // from the file name then the default 'TS' script kind is returned.
        return scriptKind != ScriptKind::Unknown ? scriptKind : scriptKind = getScriptKindFromFileName(fileName), scriptKind != ScriptKind::Unknown ? scriptKind : ScriptKind::TS;
    }

    inline auto isDiagnosticWithDetachedLocation(DiagnosticRelatedInformation diagnostic) -> boolean
    {
        // TODO: sort out
        //return diagnostic->start != -1 && diagnostic->length != -1 && diagnostic->fileName != S("");
        return false;
    }

    static auto attachFileToDiagnostic(Diagnostic diagnostic, SourceFile file) -> DiagnosticWithLocation
    {
        auto fileName = file->fileName;
        auto length = file->text.length();
        // TODO: review it
        //Debug::assertEqual(diagnostic->fileName, fileName);
        Debug::assertLessThanOrEqual(diagnostic->start, length);
        Debug::assertLessThanOrEqual(diagnostic->start + diagnostic->length, length);
        DiagnosticWithLocation diagnosticWithLocation;
        // TODO: review it
        //diagnosticWithLocation->file = file;
        diagnosticWithLocation->start = diagnostic->start;
        diagnosticWithLocation->length = diagnostic->length;
        diagnosticWithLocation->messageText = diagnostic->messageText;
        diagnosticWithLocation->category = diagnostic->category;
        diagnosticWithLocation->code = diagnostic->code;
        // TODO: review it
        /*
    diagnosticWithLocation->reportsUnnecessary = diagnostic->reportsUnnecessary;

    if (!diagnostic->relatedInformation.empty())
    {
        for (auto &related : diagnostic->relatedInformation)
        {
            if (isDiagnosticWithDetachedLocation(related) && related.fileName == fileName)
            {
                Debug::assertLessThanOrEqual(related.start, length);
                Debug::assertLessThanOrEqual(related.start + related.length, length);
                diagnosticWithLocation->relatedInformation.push_back(attachFileToDiagnostic(related, file));
            }
            else
            {
                diagnosticWithLocation->relatedInformation.push_back(related);
            }
        }
    }
    */

        return diagnosticWithLocation;
    }

    static auto attachFileToDiagnostics(std::vector<DiagnosticWithDetachedLocation> diagnostics, SourceFile file) -> std::vector<DiagnosticWithLocation>
    {
        std::vector<DiagnosticWithLocation> diagnosticsWithLocation;
        for (auto &diagnostic : diagnostics)
        {
            diagnosticsWithLocation.push_back(attachFileToDiagnostic(diagnostic, file));
        }
        return diagnosticsWithLocation;
    }

    static auto assertDiagnosticLocation(SourceFile file, number start, number length)
    {
        Debug::assertGreaterThanOrEqual(start, 0);
        Debug::assertGreaterThanOrEqual(length, 0);

        if (!!file)
        {
            Debug::assertLessThanOrEqual(start, file->text.length());
            Debug::assertLessThanOrEqual(start + length, file->text.length());
        }
    }

    static auto getLocaleSpecificMessage(DiagnosticMessage message) -> string
    {
        return string(message->message);
    }

    static auto createDetachedDiagnostic(string fileName, number start, number length, DiagnosticMessage message) -> DiagnosticWithDetachedLocation
    {
        assertDiagnosticLocation(/*file*/ SourceFile(), start, length);
        auto text = getLocaleSpecificMessage(message);

        /*
    if (arguments.length > 4) {
        text = formatStringFromArgs(text, arguments, 4);
    }
    */

        DiagnosticWithDetachedLocation d;
        d->start = start;
        d->length = length;

        d->messageText = text;
        d->category = message->category;
        d->code = message->code;
        //d.reportsUnnecessary = message.reportsUnnecessary;
        d->fileName = fileName;

        return d;
    }

    static auto createDetachedDiagnostic(string fileName, number start, number length, DiagnosticMessage message, string arg0, ...) -> DiagnosticWithDetachedLocation
    {
        assertDiagnosticLocation(/*file*/ SourceFile(), start, length);
        auto text = getLocaleSpecificMessage(message);

        // TODO:
        /*
    if (arguments.length > 4) {
        text = formatStringFromArgs(text, arguments, 4);
    }
    */

        DiagnosticWithDetachedLocation d;
        d->start = start;
        d->length = length;

        d->messageText = text;
        d->category = message->category;
        d->code = message->code;
        //diagnosticWithDetachedLocation.reportsUnnecessary = message.reportsUnnecessary;
        d->fileName = fileName;

        return d;
    }

    inline auto normalizePath(string path) -> string
    {
        // TODO: finish it
        return path;
    }

    inline auto getLanguageVariant(ScriptKind scriptKind) -> LanguageVariant
    {
        // .tsx and .jsx files are treated as jsx language variant.
        return scriptKind == ScriptKind::TSX || scriptKind == ScriptKind::JSX || scriptKind == ScriptKind::JS || scriptKind == ScriptKind::JSON ? LanguageVariant::JSX : LanguageVariant::Standard;
    }

    inline auto startsWith(string str, string prefix) -> boolean
    {
        auto expectedPos = 0;
        return str.find(prefix, expectedPos) == expectedPos;
    }

    inline auto endsWith(string str, string suffix) -> boolean
    {
        auto expectedPos = str.length() - suffix.length();
        return expectedPos >= 0 && str.find(suffix, expectedPos) == expectedPos;
    }

    inline auto fileExtensionIs(string path, string extension) -> boolean
    {
        return path.length() > extension.length() && endsWith(path, extension);
    }

    template <typename T>
    inline auto setTextRangePos(T range, number pos)
    {
        ((TextRange &)range)->pos = pos;
        return range;
    }

    template <typename T>
    inline auto setTextRangeEnd(T range, number end) -> T
    {
        ((TextRange &)range)->end = end;
        return range;
    }

    template <typename T>
    inline auto setTextRangePosEnd(T range, number pos, number end)
    {
        return setTextRangeEnd(setTextRangePos(range, pos), end);
    }

    template <typename T>
    inline auto setTextRangePosWidth(T range, number pos, number width)
    {
        return setTextRangePosEnd(range, pos, pos + width);
    }

    template <typename T>
    inline auto setTextRange(T range, TextRange location) -> T
    {
        return !!location ? setTextRangePosEnd(range, location->pos, location->end) : range;
    }

    inline static auto hasJSDocNodes(Node node) -> boolean
    {
        auto jsDoc = node.as<JSDocContainer>()->jsDoc;
        return !!jsDoc && jsDoc.size() > 0;
    }

    // JSDoc

    /** True if node is of some JSDoc syntax kind. */
    /* @internal */
    inline static auto isJSDocNode(Node node) -> boolean
    {
        return node->kind >= SyntaxKind::FirstJSDocNode && node->kind <= SyntaxKind::LastJSDocNode;
    }

    template <typename T>
    static auto visitNode(NodeFuncT<T> cbNode, Node node) -> T
    {
        return node ? cbNode(node) : T();
    }

    template <typename T>
    static auto visitNodes(NodeFuncT<T> cbNode, NodeArrayFuncT<T> cbNodes, NodeArray<T> nodes) -> T
    {
        if (!!nodes)
        {
            if (cbNodes)
            {
                return cbNodes(nodes);
            }
            for (auto node : nodes)
            {
                auto result = cbNode(node);
                if (result)
                {
                    return result;
                }
            }
        }

        return undefined;
    }

    template <typename T, typename U>
    static auto visitNodes(NodeFuncT<T> cbNode, NodeArrayFuncT<T> cbNodes, NodeArray<U> nodes) -> T
    {
        if (!!nodes)
        {
            if (cbNodes)
            {
                return cbNodes(NodeArray<T>(nodes.begin(), nodes.end()));
            }
            for (auto node : nodes)
            {
                auto result = cbNode(node.as<T>());
                if (result)
                {
                    return result;
                }
            }
        }

        return undefined;
    }

    /**
 * Invokes a callback for each child of the given node-> The 'cbNode' callback is invoked for all child nodes
 * stored in properties. If a 'cbNodes' callback is specified, it is invoked for embedded arrays; otherwise,
 * embedded arrays are flattened and the 'cbNode' callback is invoked for each element. If a callback returns
 * a truthy value, iteration stops and that value is returned. Otherwise, undefined is returned.
 *
 * @param node a given node to visit its children
 * @param cbNode a callback to be invoked for all child nodes
 * @param cbNodes a callback to be invoked for embedded array
 *
 * @remarks `forEachChild` must visit the children of a node in the order
 * that they appear in the source code. The language service depends on this property to locate nodes by position.
 */
    template <typename T>
    static auto forEachChild(Node node, NodeFuncT<T> cbNode, NodeArrayFuncT<T> cbNodes = nullptr) -> T
    {
        if (!node || node->kind <= SyntaxKind::LastToken)
        {
            return undefined;
        }
        switch (node->kind)
        {
        case SyntaxKind::QualifiedName:
            return visitNode(cbNode, node.as<QualifiedName>()->left) ||
                   visitNode(cbNode, node.as<QualifiedName>()->right);
        case SyntaxKind::TypeParameter:
            return visitNode(cbNode, node.as<TypeParameterDeclaration>()->name) ||
                   visitNode(cbNode, node.as<TypeParameterDeclaration>()->constraint) ||
                   visitNode(cbNode, node.as<TypeParameterDeclaration>()->_default) ||
                   visitNode(cbNode, node.as<TypeParameterDeclaration>()->expression);
        case SyntaxKind::ShorthandPropertyAssignment:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<ShorthandPropertyAssignment>()->name) ||
                   visitNode(cbNode, node.as<ShorthandPropertyAssignment>()->questionToken) ||
                   visitNode(cbNode, node.as<ShorthandPropertyAssignment>()->exclamationToken) ||
                   visitNode(cbNode, node.as<ShorthandPropertyAssignment>()->equalsToken) ||
                   visitNode(cbNode, node.as<ShorthandPropertyAssignment>()->objectAssignmentInitializer);
        case SyntaxKind::SpreadAssignment:
            return visitNode(cbNode, node.as<SpreadAssignment>()->expression);
        case SyntaxKind::Parameter:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<ParameterDeclaration>()->dotDotDotToken) ||
                   visitNode(cbNode, node.as<ParameterDeclaration>()->name) ||
                   visitNode(cbNode, node.as<ParameterDeclaration>()->questionToken) ||
                   visitNode(cbNode, node.as<ParameterDeclaration>()->type) ||
                   visitNode(cbNode, node.as<ParameterDeclaration>()->initializer);
        case SyntaxKind::PropertyDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<PropertyDeclaration>()->name) ||
                   visitNode(cbNode, node.as<PropertyDeclaration>()->questionToken) ||
                   visitNode(cbNode, node.as<PropertyDeclaration>()->exclamationToken) ||
                   visitNode(cbNode, node.as<PropertyDeclaration>()->type) ||
                   visitNode(cbNode, node.as<PropertyDeclaration>()->initializer);
        case SyntaxKind::PropertySignature:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<PropertySignature>()->name) ||
                   visitNode(cbNode, node.as<PropertySignature>()->questionToken) ||
                   visitNode(cbNode, node.as<PropertySignature>()->type) ||
                   visitNode(cbNode, node.as<PropertySignature>()->initializer);
        case SyntaxKind::PropertyAssignment:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<PropertyAssignment>()->name) ||
                   visitNode(cbNode, node.as<PropertyAssignment>()->questionToken) ||
                   visitNode(cbNode, node.as<PropertyAssignment>()->initializer);
        case SyntaxKind::VariableDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<VariableDeclaration>()->name) ||
                   visitNode(cbNode, node.as<VariableDeclaration>()->exclamationToken) ||
                   visitNode(cbNode, node.as<VariableDeclaration>()->type) ||
                   visitNode(cbNode, node.as<VariableDeclaration>()->initializer);
        case SyntaxKind::BindingElement:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<BindingElement>()->dotDotDotToken) ||
                   visitNode(cbNode, node.as<BindingElement>()->propertyName) ||
                   visitNode(cbNode, node.as<BindingElement>()->name) ||
                   visitNode(cbNode, node.as<BindingElement>()->initializer);
        case SyntaxKind::FunctionType:
        case SyntaxKind::ConstructorType:
        case SyntaxKind::CallSignature:
        case SyntaxKind::ConstructSignature:
        case SyntaxKind::IndexSignature:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNodes(cbNode, cbNodes, node.as<SignatureDeclarationBase>()->typeParameters) ||
                   visitNodes(cbNode, cbNodes, node.as<SignatureDeclarationBase>()->parameters) ||
                   visitNode(cbNode, node.as<SignatureDeclarationBase>()->type);
        case SyntaxKind::MethodDeclaration:
        case SyntaxKind::MethodSignature:
        case SyntaxKind::Constructor:
        case SyntaxKind::GetAccessor:
        case SyntaxKind::SetAccessor:
        case SyntaxKind::FunctionExpression:
        case SyntaxKind::FunctionDeclaration:
        case SyntaxKind::ArrowFunction:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<FunctionLikeDeclarationBase>()->asteriskToken) ||
                   visitNode(cbNode, node.as<FunctionLikeDeclarationBase>()->name) ||
                   visitNode(cbNode, node.as<FunctionLikeDeclarationBase>()->questionToken) ||
                   visitNode(cbNode, node.as<FunctionLikeDeclarationBase>()->exclamationToken) ||
                   visitNodes(cbNode, cbNodes, node.as<FunctionLikeDeclarationBase>()->typeParameters) ||
                   visitNodes(cbNode, cbNodes, node.as<FunctionLikeDeclarationBase>()->parameters) ||
                   visitNode(cbNode, node.as<FunctionLikeDeclarationBase>()->type) ||
                   visitNode(cbNode, node.as<ArrowFunction>()->equalsGreaterThanToken) ||
                   visitNode(cbNode, node.as<FunctionLikeDeclarationBase>()->body);
        case SyntaxKind::TypeReference:
            return visitNode(cbNode, node.as<TypeReferenceNode>()->typeName) ||
                   visitNodes(cbNode, cbNodes, node.as<TypeReferenceNode>()->typeArguments);
        case SyntaxKind::TypePredicate:
            return visitNode(cbNode, node.as<TypePredicateNode>()->assertsModifier) ||
                   visitNode(cbNode, node.as<TypePredicateNode>()->parameterName) ||
                   visitNode(cbNode, node.as<TypePredicateNode>()->type);
        case SyntaxKind::TypeQuery:
            return visitNode(cbNode, node.as<TypeQueryNode>()->exprName);
        case SyntaxKind::TypeLiteral:
            return visitNodes(cbNode, cbNodes, node.as<TypeLiteralNode>()->members);
        case SyntaxKind::ArrayType:
            return visitNode(cbNode, node.as<ArrayTypeNode>()->elementType);
        case SyntaxKind::TupleType:
            return visitNodes(cbNode, cbNodes, node.as<TupleTypeNode>()->elements);
        case SyntaxKind::UnionType:
            return visitNodes(cbNode, cbNodes, node.as<UnionTypeNode>()->types);
        case SyntaxKind::IntersectionType:
            return visitNodes(cbNode, cbNodes, node.as<IntersectionTypeNode>()->types);
        case SyntaxKind::ConditionalType:
            return visitNode(cbNode, node.as<ConditionalTypeNode>()->checkType) ||
                   visitNode(cbNode, node.as<ConditionalTypeNode>()->extendsType) ||
                   visitNode(cbNode, node.as<ConditionalTypeNode>()->trueType) ||
                   visitNode(cbNode, node.as<ConditionalTypeNode>()->falseType);
        case SyntaxKind::InferType:
            return visitNode(cbNode, node.as<InferTypeNode>()->typeParameter);
        case SyntaxKind::ImportType:
            return visitNode(cbNode, node.as<ImportTypeNode>()->argument) ||
                   visitNode(cbNode, node.as<ImportTypeNode>()->qualifier) ||
                   visitNodes(cbNode, cbNodes, node.as<ImportTypeNode>()->typeArguments);
        case SyntaxKind::ParenthesizedType:
            return visitNode(cbNode, node.as<ParenthesizedTypeNode>()->type);
        case SyntaxKind::TypeOperator:
            return visitNode(cbNode, node.as<TypeOperatorNode>()->type);
        case SyntaxKind::IndexedAccessType:
            return visitNode(cbNode, node.as<IndexedAccessTypeNode>()->objectType) ||
                   visitNode(cbNode, node.as<IndexedAccessTypeNode>()->indexType);
        case SyntaxKind::MappedType:
            return visitNode(cbNode, node.as<MappedTypeNode>()->readonlyToken) ||
                   visitNode(cbNode, node.as<MappedTypeNode>()->typeParameter) ||
                   visitNode(cbNode, node.as<MappedTypeNode>()->nameType) ||
                   visitNode(cbNode, node.as<MappedTypeNode>()->questionToken) ||
                   visitNode(cbNode, node.as<MappedTypeNode>()->type);
        case SyntaxKind::LiteralType:
            return visitNode(cbNode, node.as<LiteralTypeNode>()->literal);
        case SyntaxKind::NamedTupleMember:
            return visitNode(cbNode, node.as<NamedTupleMember>()->dotDotDotToken) ||
                   visitNode(cbNode, node.as<NamedTupleMember>()->name) ||
                   visitNode(cbNode, node.as<NamedTupleMember>()->questionToken) ||
                   visitNode(cbNode, node.as<NamedTupleMember>()->type);
        case SyntaxKind::ObjectBindingPattern:
            return visitNodes(cbNode, cbNodes, node.as<ObjectBindingPattern>()->elements);
        case SyntaxKind::ArrayBindingPattern:
            return visitNodes(cbNode, cbNodes, node.as<ArrayBindingPattern>()->elements);
        case SyntaxKind::ArrayLiteralExpression:
            return visitNodes(cbNode, cbNodes, node.as<ArrayLiteralExpression>()->elements);
        case SyntaxKind::ObjectLiteralExpression:
            return visitNodes(cbNode, cbNodes, node.as<ObjectLiteralExpression>()->properties);
        case SyntaxKind::PropertyAccessExpression:
            return visitNode(cbNode, node.as<PropertyAccessExpression>()->expression) ||
                   visitNode(cbNode, node.as<PropertyAccessExpression>()->questionDotToken) ||
                   visitNode(cbNode, node.as<PropertyAccessExpression>()->name);
        case SyntaxKind::ElementAccessExpression:
            return visitNode(cbNode, node.as<ElementAccessExpression>()->expression) ||
                   visitNode(cbNode, node.as<ElementAccessExpression>()->questionDotToken) ||
                   visitNode(cbNode, node.as<ElementAccessExpression>()->argumentExpression);
        case SyntaxKind::CallExpression:
        case SyntaxKind::NewExpression:
            return visitNode(cbNode, node.as<CallExpression>()->expression) ||
                   visitNode(cbNode, node.as<CallExpression>()->questionDotToken) ||
                   visitNodes(cbNode, cbNodes, node.as<CallExpression>()->typeArguments) ||
                   visitNodes(cbNode, cbNodes, node.as<CallExpression>()->arguments);
        case SyntaxKind::TaggedTemplateExpression:
            return visitNode(cbNode, node.as<TaggedTemplateExpression>()->tag) ||
                   visitNode(cbNode, node.as<TaggedTemplateExpression>()->questionDotToken) ||
                   visitNodes(cbNode, cbNodes, node.as<TaggedTemplateExpression>()->typeArguments) ||
                   visitNode(cbNode, node.as<TaggedTemplateExpression>()->_template);
        case SyntaxKind::TypeAssertionExpression:
            return visitNode(cbNode, node.as<TypeAssertion>()->type) ||
                   visitNode(cbNode, node.as<TypeAssertion>()->expression);
        case SyntaxKind::ParenthesizedExpression:
            return visitNode(cbNode, node.as<ParenthesizedExpression>()->expression);
        case SyntaxKind::DeleteExpression:
            return visitNode(cbNode, node.as<DeleteExpression>()->expression);
        case SyntaxKind::TypeOfExpression:
            return visitNode(cbNode, node.as<TypeOfExpression>()->expression);
        case SyntaxKind::VoidExpression:
            return visitNode(cbNode, node.as<VoidExpression>()->expression);
        case SyntaxKind::PrefixUnaryExpression:
            return visitNode(cbNode, node.as<PrefixUnaryExpression>()->operand);
        case SyntaxKind::YieldExpression:
            return visitNode(cbNode, node.as<YieldExpression>()->asteriskToken) ||
                   visitNode(cbNode, node.as<YieldExpression>()->expression);
        case SyntaxKind::AwaitExpression:
            return visitNode(cbNode, node.as<AwaitExpression>()->expression);
        case SyntaxKind::PostfixUnaryExpression:
            return visitNode(cbNode, node.as<PostfixUnaryExpression>()->operand);
        case SyntaxKind::BinaryExpression:
            return visitNode(cbNode, node.as<BinaryExpression>()->left) ||
                   visitNode(cbNode, node.as<BinaryExpression>()->operatorToken) ||
                   visitNode(cbNode, node.as<BinaryExpression>()->right);
        case SyntaxKind::AsExpression:
            return visitNode(cbNode, node.as<AsExpression>()->expression) ||
                   visitNode(cbNode, node.as<AsExpression>()->type);
        case SyntaxKind::NonNullExpression:
            return visitNode(cbNode, node.as<NonNullExpression>()->expression);
        case SyntaxKind::MetaProperty:
            return visitNode(cbNode, node.as<MetaProperty>()->name);
        case SyntaxKind::ConditionalExpression:
            return visitNode(cbNode, node.as<ConditionalExpression>()->condition) ||
                   visitNode(cbNode, node.as<ConditionalExpression>()->questionToken) ||
                   visitNode(cbNode, node.as<ConditionalExpression>()->whenTrue) ||
                   visitNode(cbNode, node.as<ConditionalExpression>()->colonToken) ||
                   visitNode(cbNode, node.as<ConditionalExpression>()->whenFalse);
        case SyntaxKind::SpreadElement:
            return visitNode(cbNode, node.as<SpreadElement>()->expression);
        case SyntaxKind::Block:
        case SyntaxKind::ModuleBlock:
            return visitNodes(cbNode, cbNodes, node.as<Block>()->statements);
        case SyntaxKind::SourceFile:
            return visitNodes(cbNode, cbNodes, node.as<SourceFile>()->statements) ||
                   visitNode(cbNode, node.as<SourceFile>()->endOfFileToken);
        case SyntaxKind::VariableStatement:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<VariableStatement>()->declarationList);
        case SyntaxKind::VariableDeclarationList:
            return visitNodes(cbNode, cbNodes, node.as<VariableDeclarationList>()->declarations);
        case SyntaxKind::ExpressionStatement:
            return visitNode(cbNode, node.as<ExpressionStatement>()->expression);
        case SyntaxKind::IfStatement:
            return visitNode(cbNode, node.as<IfStatement>()->expression) ||
                   visitNode(cbNode, node.as<IfStatement>()->thenStatement) ||
                   visitNode(cbNode, node.as<IfStatement>()->elseStatement);
        case SyntaxKind::DoStatement:
            return visitNode(cbNode, node.as<DoStatement>()->statement) ||
                   visitNode(cbNode, node.as<DoStatement>()->expression);
        case SyntaxKind::WhileStatement:
            return visitNode(cbNode, node.as<WhileStatement>()->expression) ||
                   visitNode(cbNode, node.as<WhileStatement>()->statement);
        case SyntaxKind::ForStatement:
            return visitNode(cbNode, node.as<ForStatement>()->initializer) ||
                   visitNode(cbNode, node.as<ForStatement>()->condition) ||
                   visitNode(cbNode, node.as<ForStatement>()->incrementor) ||
                   visitNode(cbNode, node.as<ForStatement>()->statement);
        case SyntaxKind::ForInStatement:
            return visitNode(cbNode, node.as<ForInStatement>()->initializer) ||
                   visitNode(cbNode, node.as<ForInStatement>()->expression) ||
                   visitNode(cbNode, node.as<ForInStatement>()->statement);
        case SyntaxKind::ForOfStatement:
            return visitNode(cbNode, node.as<ForOfStatement>()->awaitModifier) ||
                   visitNode(cbNode, node.as<ForOfStatement>()->initializer) ||
                   visitNode(cbNode, node.as<ForOfStatement>()->expression) ||
                   visitNode(cbNode, node.as<ForOfStatement>()->statement);
        case SyntaxKind::ContinueStatement:
            return visitNode(cbNode, node.as<ContinueStatement>()->label);
        case SyntaxKind::BreakStatement:
            return visitNode(cbNode, node.as<BreakStatement>()->label);
        case SyntaxKind::ReturnStatement:
            return visitNode(cbNode, node.as<ReturnStatement>()->expression);
        case SyntaxKind::WithStatement:
            return visitNode(cbNode, node.as<WithStatement>()->expression) ||
                   visitNode(cbNode, node.as<WithStatement>()->statement);
        case SyntaxKind::SwitchStatement:
            return visitNode(cbNode, node.as<SwitchStatement>()->expression) ||
                   visitNode(cbNode, node.as<SwitchStatement>()->caseBlock);
        case SyntaxKind::CaseBlock:
            return visitNodes(cbNode, cbNodes, node.as<CaseBlock>()->clauses);
        case SyntaxKind::CaseClause:
            return visitNode(cbNode, node.as<CaseClause>()->expression) ||
                   visitNodes(cbNode, cbNodes, node.as<CaseClause>()->statements);
        case SyntaxKind::DefaultClause:
            return visitNodes(cbNode, cbNodes, node.as<DefaultClause>()->statements);
        case SyntaxKind::LabeledStatement:
            return visitNode(cbNode, node.as<LabeledStatement>()->label) ||
                   visitNode(cbNode, node.as<LabeledStatement>()->statement);
        case SyntaxKind::ThrowStatement:
            return visitNode(cbNode, node.as<ThrowStatement>()->expression);
        case SyntaxKind::TryStatement:
            return visitNode(cbNode, node.as<TryStatement>()->tryBlock) ||
                   visitNode(cbNode, node.as<TryStatement>()->catchClause) ||
                   visitNode(cbNode, node.as<TryStatement>()->finallyBlock);
        case SyntaxKind::CatchClause:
            return visitNode(cbNode, node.as<CatchClause>()->variableDeclaration) ||
                   visitNode(cbNode, node.as<CatchClause>()->block);
        case SyntaxKind::Decorator:
            return visitNode(cbNode, node.as<Decorator>()->expression);
        case SyntaxKind::ClassDeclaration:
        case SyntaxKind::ClassExpression:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<ClassLikeDeclarationBase>()->name) ||
                   visitNodes(cbNode, cbNodes, node.as<ClassLikeDeclarationBase>()->typeParameters) ||
                   visitNodes(cbNode, cbNodes, node.as<ClassLikeDeclarationBase>()->heritageClauses) ||
                   visitNodes(cbNode, cbNodes, node.as<ClassLikeDeclarationBase>()->members);
        case SyntaxKind::InterfaceDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<InterfaceDeclaration>()->name) ||
                   visitNodes(cbNode, cbNodes, node.as<InterfaceDeclaration>()->typeParameters) ||
                   visitNodes(cbNode, cbNodes, node.as<ClassDeclaration>()->heritageClauses) ||
                   visitNodes(cbNode, cbNodes, node.as<InterfaceDeclaration>()->members);
        case SyntaxKind::TypeAliasDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<TypeAliasDeclaration>()->name) ||
                   visitNodes(cbNode, cbNodes, node.as<TypeAliasDeclaration>()->typeParameters) ||
                   visitNode(cbNode, node.as<TypeAliasDeclaration>()->type);
        case SyntaxKind::EnumDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<EnumDeclaration>()->name) ||
                   visitNodes(cbNode, cbNodes, node.as<EnumDeclaration>()->members);
        case SyntaxKind::EnumMember:
            return visitNode(cbNode, node.as<EnumMember>()->name) ||
                   visitNode(cbNode, node.as<EnumMember>()->initializer);
        case SyntaxKind::ModuleDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<ModuleDeclaration>()->name) ||
                   visitNode(cbNode, node.as<ModuleDeclaration>()->body);
        case SyntaxKind::ImportEqualsDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<ImportEqualsDeclaration>()->name) ||
                   visitNode(cbNode, node.as<ImportEqualsDeclaration>()->moduleReference);
        case SyntaxKind::ImportDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<ImportDeclaration>()->importClause) ||
                   visitNode(cbNode, node.as<ImportDeclaration>()->moduleSpecifier);
        case SyntaxKind::ImportClause:
            return visitNode(cbNode, node.as<ImportClause>()->name) ||
                   visitNode(cbNode, node.as<ImportClause>()->namedBindings);
        case SyntaxKind::NamespaceExportDeclaration:
            return visitNode(cbNode, node.as<NamespaceExportDeclaration>()->name);

        case SyntaxKind::NamespaceImport:
            return visitNode(cbNode, node.as<NamespaceImport>()->name);
        case SyntaxKind::NamespaceExport:
            return visitNode(cbNode, node.as<NamespaceExport>()->name);
        case SyntaxKind::NamedImports:
            return visitNodes(cbNode, cbNodes, node.as<NamedImports>()->elements);
        case SyntaxKind::NamedExports:
            return visitNodes(cbNode, cbNodes, node.as<NamedExports>()->elements);
        case SyntaxKind::ExportDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<ExportDeclaration>()->exportClause) ||
                   visitNode(cbNode, node.as<ExportDeclaration>()->moduleSpecifier);
        case SyntaxKind::ImportSpecifier:
            return visitNode(cbNode, node.as<ImportSpecifier>()->propertyName) ||
                   visitNode(cbNode, node.as<ImportSpecifier>()->name);
        case SyntaxKind::ExportSpecifier:
            return visitNode(cbNode, node.as<ExportSpecifier>()->propertyName) ||
                   visitNode(cbNode, node.as<ExportSpecifier>()->name);
        case SyntaxKind::ExportAssignment:
            return visitNodes(cbNode, cbNodes, node->decorators) ||
                   visitNodes(cbNode, cbNodes, node->modifiers) ||
                   visitNode(cbNode, node.as<ExportAssignment>()->expression);
        case SyntaxKind::TemplateExpression:
            return visitNode(cbNode, node.as<TemplateExpression>()->head) || visitNodes(cbNode, cbNodes, node.as<TemplateExpression>()->templateSpans);
        case SyntaxKind::TemplateSpan:
            return visitNode(cbNode, node.as<TemplateSpan>()->expression) || visitNode(cbNode, node.as<TemplateSpan>()->literal);
        case SyntaxKind::TemplateLiteralType:
            return visitNode(cbNode, node.as<TemplateLiteralTypeNode>()->head) || visitNodes(cbNode, cbNodes, node.as<TemplateLiteralTypeNode>()->templateSpans);
        case SyntaxKind::TemplateLiteralTypeSpan:
            return visitNode(cbNode, node.as<TemplateLiteralTypeSpan>()->type) || visitNode(cbNode, node.as<TemplateLiteralTypeSpan>()->literal);
        case SyntaxKind::ComputedPropertyName:
            return visitNode(cbNode, node.as<ComputedPropertyName>()->expression);
        case SyntaxKind::HeritageClause:
            return visitNodes(cbNode, cbNodes, node.as<HeritageClause>()->types);
        case SyntaxKind::ExpressionWithTypeArguments:
            return visitNode(cbNode, node.as<ExpressionWithTypeArguments>()->expression) ||
                   visitNodes(cbNode, cbNodes, node.as<ExpressionWithTypeArguments>()->typeArguments);
        case SyntaxKind::ExternalModuleReference:
            return visitNode(cbNode, node.as<ExternalModuleReference>()->expression);
        case SyntaxKind::MissingDeclaration:
            return visitNodes(cbNode, cbNodes, node->decorators);
        case SyntaxKind::CommaListExpression:
            return visitNodes(cbNode, cbNodes, node.as<CommaListExpression>()->elements);

        case SyntaxKind::JsxElement:
            return visitNode(cbNode, node.as<JsxElement>()->openingElement) ||
                   visitNodes(cbNode, cbNodes, node.as<JsxElement>()->children) ||
                   visitNode(cbNode, node.as<JsxElement>()->closingElement);
        case SyntaxKind::JsxFragment:
            return visitNode(cbNode, node.as<JsxFragment>()->openingFragment) ||
                   visitNodes(cbNode, cbNodes, node.as<JsxFragment>()->children) ||
                   visitNode(cbNode, node.as<JsxFragment>()->closingFragment);
        case SyntaxKind::JsxSelfClosingElement:
            return visitNode(cbNode, node.as<JsxSelfClosingElement>()->tagName) ||
                   visitNodes(cbNode, cbNodes, node.as<JsxSelfClosingElement>()->typeArguments) ||
                   visitNode(cbNode, node.as<JsxSelfClosingElement>()->attributes);
        case SyntaxKind::JsxOpeningElement:
            return visitNode(cbNode, node.as<JsxOpeningElement>()->tagName) ||
                   visitNodes(cbNode, cbNodes, node.as<JsxOpeningElement>()->typeArguments) ||
                   visitNode(cbNode, node.as<JsxOpeningElement>()->attributes);
        case SyntaxKind::JsxAttributes:
            return visitNodes(cbNode, cbNodes, node.as<JsxAttributes>()->properties);
        case SyntaxKind::JsxAttribute:
            return visitNode(cbNode, node.as<JsxAttribute>()->name) ||
                   visitNode(cbNode, node.as<JsxAttribute>()->initializer);
        case SyntaxKind::JsxSpreadAttribute:
            return visitNode(cbNode, node.as<JsxSpreadAttribute>()->expression);
        case SyntaxKind::JsxExpression:
            return visitNode(cbNode, node.as<JsxExpression>()->dotDotDotToken) ||
                   visitNode(cbNode, node.as<JsxExpression>()->expression);
        case SyntaxKind::JsxClosingElement:
            return visitNode(cbNode, node.as<JsxClosingElement>()->tagName);

        case SyntaxKind::OptionalType:
            return visitNode(cbNode, node.as<OptionalTypeNode>()->type);
        case SyntaxKind::RestType:
            return visitNode(cbNode, node.as<RestTypeNode>()->type);
        case SyntaxKind::JSDocTypeExpression:
            return visitNode(cbNode, node.as<JSDocTypeExpression>()->type);
        case SyntaxKind::JSDocNonNullableType:
            return visitNode(cbNode, node.as<JSDocNonNullableType>()->type);
        case SyntaxKind::JSDocNullableType:
            return visitNode(cbNode, node.as<JSDocNullableType>()->type);
        case SyntaxKind::JSDocOptionalType:
            return visitNode(cbNode, node.as<JSDocOptionalType>()->type);
        case SyntaxKind::JSDocVariadicType:
            return visitNode(cbNode, node.as<JSDocVariadicType>()->type);
        case SyntaxKind::JSDocFunctionType:
            return visitNodes(cbNode, cbNodes, node.as<JSDocFunctionType>()->parameters) ||
                   visitNode(cbNode, node.as<JSDocFunctionType>()->type);
        case SyntaxKind::JSDocComment:
            return visitNodes(cbNode, cbNodes, node.as<JSDoc>()->tags);
        case SyntaxKind::JSDocSeeTag:
            return visitNode(cbNode, node.as<JSDocSeeTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocSeeTag>()->name);
        case SyntaxKind::JSDocNameReference:
            return visitNode(cbNode, node.as<JSDocNameReference>()->name);
        case SyntaxKind::JSDocParameterTag:
        case SyntaxKind::JSDocPropertyTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   (node.as<JSDocPropertyLikeTag>()->isNameFirst
                        ? visitNode(cbNode, node.as<JSDocPropertyLikeTag>()->name) ||
                              visitNode(cbNode, node.as<JSDocPropertyLikeTag>()->typeExpression)
                        : visitNode(cbNode, node.as<JSDocPropertyLikeTag>()->typeExpression) ||
                              visitNode(cbNode, node.as<JSDocPropertyLikeTag>()->name));
        case SyntaxKind::JSDocAuthorTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName);
        case SyntaxKind::JSDocImplementsTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocImplementsTag>()->_class);
        case SyntaxKind::JSDocAugmentsTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocAugmentsTag>()->_class);
        case SyntaxKind::JSDocTemplateTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocTemplateTag>()->constraint) ||
                   visitNodes(cbNode, cbNodes, node.as<JSDocTemplateTag>()->typeParameters);
        case SyntaxKind::JSDocTypedefTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   (node.as<JSDocTypedefTag>()->typeExpression &&
                            node.as<JSDocTypedefTag>()->typeExpression->kind == SyntaxKind::JSDocTypeExpression
                        ? visitNode(cbNode, node.as<JSDocTypedefTag>()->typeExpression) ||
                              visitNode(cbNode, node.as<JSDocTypedefTag>()->fullName)
                        : visitNode(cbNode, node.as<JSDocTypedefTag>()->fullName) ||
                              visitNode(cbNode, node.as<JSDocTypedefTag>()->typeExpression));
        case SyntaxKind::JSDocCallbackTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocCallbackTag>()->fullName) ||
                   visitNode(cbNode, node.as<JSDocCallbackTag>()->typeExpression);
        case SyntaxKind::JSDocReturnTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocReturnTag>()->typeExpression);
        case SyntaxKind::JSDocTypeTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocTypeTag>()->typeExpression);
        case SyntaxKind::JSDocThisTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocThisTag>()->typeExpression);
        case SyntaxKind::JSDocEnumTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName) ||
                   visitNode(cbNode, node.as<JSDocEnumTag>()->typeExpression);
        case SyntaxKind::JSDocSignature:
            return forEach<decltype(node.as<JSDocSignature>()->typeParameters), T>(node.as<JSDocSignature>()->typeParameters, cbNode) ||
                   forEach<decltype(node.as<JSDocSignature>()->parameters), T>(node.as<JSDocSignature>()->parameters, cbNode) ||
                   visitNode(cbNode, node.as<JSDocSignature>()->type);
        case SyntaxKind::JSDocTypeLiteral:
            return forEach<decltype(node.as<JSDocTypeLiteral>()->jsDocPropertyTags), T>(node.as<JSDocTypeLiteral>()->jsDocPropertyTags, cbNode);
        case SyntaxKind::JSDocTag:
        case SyntaxKind::JSDocClassTag:
        case SyntaxKind::JSDocPublicTag:
        case SyntaxKind::JSDocPrivateTag:
        case SyntaxKind::JSDocProtectedTag:
        case SyntaxKind::JSDocReadonlyTag:
            return visitNode(cbNode, node.as<JSDocTag>()->tagName);
        case SyntaxKind::PartiallyEmittedExpression:
            return visitNode(cbNode, node.as<PartiallyEmittedExpression>()->expression);
        }

        return undefined;
    }

    static auto gatherPossibleChildren(Node node) -> NodeArray<Node>
    {
        NodeArray<Node> children;

        auto addWorkItem = [&](auto n) -> Node {
            children.emplace(children.begin(), n);
            return Node();
        };

        // TODO:
        //forEachChild<Node>(node, addWorkItem, addWorkItem); // By using a stack above and `unshift` here, we emulate a depth-first preorder traversal
        return children;
    }

    /** @internal */
    /**
    * Invokes a callback for each child of the given node-> The 'cbNode' callback is invoked for all child nodes
    * stored in properties. If a 'cbNodes' callback is specified, it is invoked for embedded arrays; additionally,
    * unlike `forEachChild`, embedded arrays are flattened and the 'cbNode' callback is invoked for each element.
    *  If a callback returns a truthy value, iteration stops and that value is returned. Otherwise, undefined is returned.
    *
    * @param node a given node to visit its children
    * @param cbNode a callback to be invoked for all child nodes
    * @param cbNodes a callback to be invoked for embedded array
    *
    * @remarks Unlike `forEachChild`, `forEachChildRecursively` handles recursively invoking the traversal on each child node found,
    * and while doing so, handles traversing the structure without relying on the callstack to encode the tree structure.
    */
    template <typename T>
    static auto forEachChildRecursively(Node rootNode, NodeWithParentFuncT<T> cbNode, NodeWithParentArrayFuncT<T> cbNodes = nullptr) -> T
    {
        auto queue = gatherPossibleChildren(rootNode);
        NodeArray<Node> parents; // tracks parent references for elements in queue
        while (parents.size() < queue.size())
        {
            parents.push_back(rootNode);
        }
        while (queue.size() != 0)
        {
            auto current = queue.pop();
            auto parent = parents.pop();
            // TODO: review it
            /*
        if (isArray(current)) {
            if (cbNodes) {
                auto res = cbNodes(current.asArray<boolean>(), parent);
                if (res) {
                    //TODO: review it
                    //if (res == "skip") continue;
                    return res;
                }
            }
            for (int i = current.size() - 1; i >= 0; --i) {
                queue.push_back(current[i]);
                parents.push_back(parent);
            }
        }
        else {
        */
            auto res = cbNode(current, parent);
            if (res)
            {
                //TODO: review it
                //if (res == "skip") continue;
                return res;
            }
            if (current->kind >= SyntaxKind::FirstNode)
            {
                // add children in reverse order to the queue, so popping gives the first child
                for (auto child : gatherPossibleChildren(current))
                {
                    queue.push_back(child);
                    parents.push_back(current);
                }
            }
            //}
        }

        return false;
    }

    template <typename T>
    static auto setParentRecursive(T rootNode, boolean incremental) -> T
    {
        auto bindParentToChildIgnoringJSDoc = [&](auto child, auto parent) /*true is skip*/ {
            if (incremental && child->parent == parent)
            {
                return true;
            }
            setParent(child, parent);
            return false;
        };

        auto bindJSDoc = [&](auto child) {
            if (hasJSDocNodes(child))
            {
                for (auto &doc : child.as<JSDocContainer>()->jsDoc)
                {
                    bindParentToChildIgnoringJSDoc(doc, child);
                    forEachChildRecursively<boolean>(doc, bindParentToChildIgnoringJSDoc);
                }
            }

            return false;
        };

        auto bindParentToChild = [&](Node child, Node parent) {
            return bindParentToChildIgnoringJSDoc(child, parent) || bindJSDoc(child);
        };

        if (!rootNode)
        {
            return rootNode;
        }

        if (isJSDocNode(rootNode))
            forEachChildRecursively<boolean>(rootNode, bindParentToChildIgnoringJSDoc);
        else
            forEachChildRecursively<boolean>(rootNode, bindParentToChild);
        return rootNode;
    }

    inline auto isKeyword(SyntaxKind token) -> boolean
    {
        return SyntaxKind::FirstKeyword <= token && token <= SyntaxKind::LastKeyword;
    }

    inline auto isTemplateLiteralKind(SyntaxKind kind) -> boolean
    {
        return SyntaxKind::FirstTemplateToken <= kind && kind <= SyntaxKind::LastTemplateToken;
    }

    inline auto isModifierKind(SyntaxKind token) -> boolean
    {
        switch (token)
        {
        case SyntaxKind::AbstractKeyword:
        case SyntaxKind::AsyncKeyword:
        case SyntaxKind::ConstKeyword:
        case SyntaxKind::DeclareKeyword:
        case SyntaxKind::DefaultKeyword:
        case SyntaxKind::ExportKeyword:
        case SyntaxKind::PublicKeyword:
        case SyntaxKind::PrivateKeyword:
        case SyntaxKind::ProtectedKeyword:
        case SyntaxKind::ReadonlyKeyword:
        case SyntaxKind::StaticKeyword:
            return true;
        }
        return false;
    }

    inline auto nodeIsMissing(Node node) -> boolean
    {
        if (node == undefined)
        {
            return true;
        }

        return node->pos == node->end && node->pos >= 0 && node->kind != SyntaxKind::EndOfFileToken;
    }

    inline auto nodeIsPresent(Node node) -> boolean
    {
        return !nodeIsMissing(node);
    }

    inline auto containsParseError(Node node) -> boolean
    {
        return (node->flags & NodeFlags::ThisNodeOrAnySubNodesHasError) != NodeFlags::None;
    }

    inline auto isLiteralKind(SyntaxKind kind) -> boolean
    {
        return SyntaxKind::FirstLiteralToken <= kind && kind <= SyntaxKind::LastLiteralToken;
    }

    inline auto getFullWidth(Node node) -> number
    {
        return node->end - node->pos;
    }

    inline auto isOuterExpression(Node node, OuterExpressionKinds kinds = OuterExpressionKinds::All) -> boolean
    {
        switch (node->kind)
        {
        case SyntaxKind::ParenthesizedExpression:
            return (kinds & OuterExpressionKinds::Parentheses) != OuterExpressionKinds::None;
        case SyntaxKind::TypeAssertionExpression:
        case SyntaxKind::AsExpression:
            return (kinds & OuterExpressionKinds::TypeAssertions) != OuterExpressionKinds::None;
        case SyntaxKind::NonNullExpression:
            return (kinds & OuterExpressionKinds::NonNullAssertions) != OuterExpressionKinds::None;
        case SyntaxKind::PartiallyEmittedExpression:
            return (kinds & OuterExpressionKinds::PartiallyEmittedExpressions) != OuterExpressionKinds::None;
        }
        return false;
    }

    inline auto skipOuterExpressions(Node node, OuterExpressionKinds kinds = OuterExpressionKinds::All)
    {
        while (isOuterExpression(node, kinds))
        {
            switch (node->kind)
            {
            case SyntaxKind::ParenthesizedExpression:
                node = node.as<ParenthesizedExpression>()->expression;
                break;
            case SyntaxKind::TypeAssertionExpression:
                node = node.as<TypeAssertion>()->expression;
                break;
            case SyntaxKind::AsExpression:
                node = node.as<AsExpression>()->expression;
                break;
            case SyntaxKind::NonNullExpression:
                node = node.as<NonNullExpression>()->expression;
                break;
            case SyntaxKind::PartiallyEmittedExpression:
                node = node.as<PartiallyEmittedExpression>()->expression;
                break;
            }
        }
        return node;
    }

    inline auto skipPartiallyEmittedExpressions(Node node)
    {
        return skipOuterExpressions(node, OuterExpressionKinds::PartiallyEmittedExpressions);
    }

    inline auto isLeftHandSideExpressionKind(SyntaxKind kind) -> boolean
    {
        switch (kind)
        {
        case SyntaxKind::PropertyAccessExpression:
        case SyntaxKind::ElementAccessExpression:
        case SyntaxKind::NewExpression:
        case SyntaxKind::CallExpression:
        case SyntaxKind::JsxElement:
        case SyntaxKind::JsxSelfClosingElement:
        case SyntaxKind::JsxFragment:
        case SyntaxKind::TaggedTemplateExpression:
        case SyntaxKind::ArrayLiteralExpression:
        case SyntaxKind::ParenthesizedExpression:
        case SyntaxKind::ObjectLiteralExpression:
        case SyntaxKind::ClassExpression:
        case SyntaxKind::FunctionExpression:
        case SyntaxKind::Identifier:
        case SyntaxKind::RegularExpressionLiteral:
        case SyntaxKind::NumericLiteral:
        case SyntaxKind::BigIntLiteral:
        case SyntaxKind::StringLiteral:
        case SyntaxKind::NoSubstitutionTemplateLiteral:
        case SyntaxKind::TemplateExpression:
        case SyntaxKind::FalseKeyword:
        case SyntaxKind::NullKeyword:
        case SyntaxKind::ThisKeyword:
        case SyntaxKind::TrueKeyword:
        case SyntaxKind::SuperKeyword:
        case SyntaxKind::NonNullExpression:
        case SyntaxKind::MetaProperty:
        case SyntaxKind::ImportKeyword: // technically this is only an Expression if it's in a CallExpression
            return true;
        default:
            return false;
        }
    }

    inline auto isLeftHandSideExpression(Node node) -> boolean
    {
        return isLeftHandSideExpressionKind(skipPartiallyEmittedExpressions(node)->kind);
    }

    inline auto isAssignmentOperator(SyntaxKind token) -> boolean
    {
        return token >= SyntaxKind::FirstAssignment && token <= SyntaxKind::LastAssignment;
    }

    inline auto getBinaryOperatorPrecedence(SyntaxKind kind) -> OperatorPrecedence
    {
        switch (kind)
        {
        case SyntaxKind::QuestionQuestionToken:
            return OperatorPrecedence::Coalesce;
        case SyntaxKind::BarBarToken:
            return OperatorPrecedence::LogicalOR;
        case SyntaxKind::AmpersandAmpersandToken:
            return OperatorPrecedence::LogicalAND;
        case SyntaxKind::BarToken:
            return OperatorPrecedence::BitwiseOR;
        case SyntaxKind::CaretToken:
            return OperatorPrecedence::BitwiseXOR;
        case SyntaxKind::AmpersandToken:
            return OperatorPrecedence::BitwiseAND;
        case SyntaxKind::EqualsEqualsToken:
        case SyntaxKind::ExclamationEqualsToken:
        case SyntaxKind::EqualsEqualsEqualsToken:
        case SyntaxKind::ExclamationEqualsEqualsToken:
            return OperatorPrecedence::Equality;
        case SyntaxKind::LessThanToken:
        case SyntaxKind::GreaterThanToken:
        case SyntaxKind::LessThanEqualsToken:
        case SyntaxKind::GreaterThanEqualsToken:
        case SyntaxKind::InstanceOfKeyword:
        case SyntaxKind::InKeyword:
        case SyntaxKind::AsKeyword:
            return OperatorPrecedence::Relational;
        case SyntaxKind::LessThanLessThanToken:
        case SyntaxKind::GreaterThanGreaterThanToken:
        case SyntaxKind::GreaterThanGreaterThanGreaterThanToken:
            return OperatorPrecedence::Shift;
        case SyntaxKind::PlusToken:
        case SyntaxKind::MinusToken:
            return OperatorPrecedence::Additive;
        case SyntaxKind::AsteriskToken:
        case SyntaxKind::SlashToken:
        case SyntaxKind::PercentToken:
            return OperatorPrecedence::Multiplicative;
        case SyntaxKind::AsteriskAsteriskToken:
            return OperatorPrecedence::Exponentiation;
        }

        // -1 is lower than all other precedences.  Returning it will cause binary expression
        // parsing to stop.
        return OperatorPrecedence::Invalid;
    }

    static auto findAncestor(Node node, std::function<boolean(Node)> callback) -> Node
    {
        while (node)
        {
            auto result = callback(node);
            if (result)
            {
                return node;
            }
            node = node->parent;
        }
        return undefined;
    }

    static auto isJSDocTypeExpressionOrChild(Node node) -> boolean
    {
        return !!findAncestor(node, [](Node n) { return ts::isJSDocTypeExpression(n); });
    }

    static auto getTextOfNodeFromSourceText(safe_string sourceText, Node node, boolean includeTrivia = false, ts::Scanner *scanner = nullptr) -> string
    {
        if (nodeIsMissing(node))
        {
            return string();
        }

        auto text = sourceText.substring(includeTrivia ? node->pos : scanner->skipTrivia(sourceText, node->pos), node->end);

        if (isJSDocTypeExpressionOrChild(node))
        {
            // strip space + asterisk at line start
            auto reg = regex(S("(^|\\r?\\n|\\r)\\s*\\*\\s*"), std::regex_constants::extended);
            text = regex_replace(text, reg, S("$1"));
        }

        return text;
    }

    static auto isStringLiteralLike(Node node) -> boolean
    {
        return node->kind == SyntaxKind::StringLiteral || node->kind == SyntaxKind::NoSubstitutionTemplateLiteral;
    }

    static auto isStringOrNumericLiteralLike(Node node) -> boolean
    {
        return isStringLiteralLike(node) || ts::isNumericLiteral(node);
    }

    template <typename T>
    static auto addRelatedInfo(T diagnostic, std::vector<DiagnosticRelatedInformation> relatedInformation) -> T
    {
        if (!relatedInformation.size())
        {
            return diagnostic;
        }
        if (diagnostic.relatedInformation.size() > 0)
        {
            diagnostic.relatedInformation.clear();
        }
        Debug::_assert(diagnostic.relatedInformation.size() != 0, S("Diagnostic had empty array singleton for related info, but is still being constructed!"));
        for (auto &item : relatedInformation)
        {
            diagnostic.relatedInformation.push_back(item);
        }

        return diagnostic;
    }

    template <typename T>
    auto addRelatedInfo(T diagnostic, DiagnosticRelatedInformation relatedInformation) -> T
    {
        if (!relatedInformation)
        {
            return diagnostic;
        }
        if (diagnostic->relatedInformation.size() > 0)
        {
            diagnostic->relatedInformation.clear();
        }
        Debug::_assert(diagnostic->relatedInformation.size() != 0, S("Diagnostic had empty array singleton for related info, but is still being constructed!"));
        diagnostic->relatedInformation.push_back(relatedInformation);

        return diagnostic;
    }

    static auto modifierToFlag(SyntaxKind token) -> ModifierFlags
    {
        switch (token)
        {
        case SyntaxKind::StaticKeyword:
            return ModifierFlags::Static;
        case SyntaxKind::PublicKeyword:
            return ModifierFlags::Public;
        case SyntaxKind::ProtectedKeyword:
            return ModifierFlags::Protected;
        case SyntaxKind::PrivateKeyword:
            return ModifierFlags::Private;
        case SyntaxKind::AbstractKeyword:
            return ModifierFlags::Abstract;
        case SyntaxKind::ExportKeyword:
            return ModifierFlags::Export;
        case SyntaxKind::DeclareKeyword:
            return ModifierFlags::Ambient;
        case SyntaxKind::ConstKeyword:
            return ModifierFlags::Const;
        case SyntaxKind::DefaultKeyword:
            return ModifierFlags::Default;
        case SyntaxKind::AsyncKeyword:
            return ModifierFlags::Async;
        case SyntaxKind::ReadonlyKeyword:
            return ModifierFlags::Readonly;
        }
        return ModifierFlags::None;
    }

    static auto modifiersToFlags(ModifiersArray modifiers) -> ModifierFlags
    {
        auto flags = ModifierFlags::None;
        if (!!modifiers)
        {
            for (auto &modifier : modifiers)
            {
                flags |= modifierToFlag(modifier->kind);
            }
        }
        return flags;
    }

    inline auto isParameterPropertyModifier(SyntaxKind kind) -> boolean
    {
        return !!(modifierToFlag(kind) & ModifierFlags::ParameterPropertyModifier);
    }

    inline auto isClassMemberModifier(SyntaxKind idToken) -> boolean
    {
        return isParameterPropertyModifier(idToken) || idToken == SyntaxKind::StaticKeyword;
    }

    inline static auto setParent(Node child, Node parent) -> Node
    {
        if (!!child && !!parent)
        {
            child->parent = parent;
        }

        return child;
    }

    inline static auto isNamedDeclaration(Node node) -> boolean
    {
        return !!node.is<NamedDeclaration>(); // A 'name' property should always be a DeclarationName.
    }

    inline static auto isPropertyName(Node node) -> boolean
    {
        auto kind = node->kind;
        return kind == SyntaxKind::Identifier || kind == SyntaxKind::PrivateIdentifier || kind == SyntaxKind::StringLiteral || kind == SyntaxKind::NumericLiteral || kind == SyntaxKind::ComputedPropertyName;
    }

    inline static auto escapeLeadingUnderscores(string identifier) -> string
    {
        return (identifier.size() >= 2 && (CharacterCodes)identifier[0] == CharacterCodes::_ && (CharacterCodes)identifier[1] == CharacterCodes::_ ? S("_") + identifier : identifier);
    }

    inline static auto isCommaSequence(Expression node) -> boolean
    {
        return node->kind == SyntaxKind::BinaryExpression && node.as<BinaryExpression>()->operatorToken->kind == SyntaxKind::CommaToken ||
               node->kind == SyntaxKind::CommaListExpression;
    }

    inline static auto isIdentifierTypePredicate(TypePredicateNode predicate) -> boolean
    {
        return predicate && predicate->kind == SyntaxKind::Identifier;
    }

    inline static auto identifierIsThisKeyword(Identifier id) -> boolean
    {
        return id->originalKeywordKind == SyntaxKind::ThisKeyword;
    }

    inline static auto isThisIdentifier(Node node) -> boolean
    {
        return !!node && node->kind == SyntaxKind::Identifier && identifierIsThisKeyword(node.as<Identifier>());
    }

    inline static auto isInJSFile(Node node) -> boolean
    {
        return !!node && !!(node->flags & NodeFlags::JavaScriptFile);
    }

    inline static auto getSyntacticModifierFlagsNoCache(Node node) -> ModifierFlags
    {
        auto flags = modifiersToFlags(node->modifiers);
        if (!!(node->flags & NodeFlags::NestedNamespace) || (node->kind == SyntaxKind::Identifier && node.as<Identifier>()->isInJSDocNamespace))
        {
            flags |= ModifierFlags::Export;
        }
        return flags;
    }

    inline static auto  getJSDocTagsWorker(Node node, boolean noCache = false) -> NodeArray<JSDocTag> {
        auto tags = node.as<JSDocContainer>()->jsDocCache;
        // If cache is 'null', that means we did the work of searching for JSDoc tags and came up with nothing.
        if (tags == undefined || noCache) {
            // TODO: finish it
            /*
            auto comments = getJSDocCommentsAndTags(node, noCache);
            Debug::_assert(comments.length < 2 || comments[0] != comments[1]);
            tags = flatMap(comments, j => isJSDoc(j) ? j.tags : j);
            */
            if (!noCache) {
                node.as<JSDocContainer>()->jsDocCache = tags;
            }
        }
        return tags;
    }

    inline static auto getFirstJSDocTag(Node node, std::function<boolean(Node)> predicate, boolean noCache = false) -> Node {
        return find(getJSDocTagsWorker(node, noCache), predicate);
    }

    inline static auto getJSDocPublicTagNoCache(Node node) -> JSDocPublicTag {
        return getFirstJSDocTag(node, [](Node node) { return isJSDocPublicTag(node); }, /*noCache*/ true);
    }

    inline static auto getJSDocPrivateTagNoCache(Node node) -> JSDocPublicTag {
        return getFirstJSDocTag(node, [](Node node) { return isJSDocPrivateTag(node); }, /*noCache*/ true);
    }

    inline static auto getJSDocProtectedTagNoCache(Node node) -> JSDocProtectedTag {
        return getFirstJSDocTag(node, [](Node node) { return isJSDocProtectedTag(node); }, /*noCache*/ true);
    }

    inline static auto getJSDocReadonlyTagNoCache(Node node) -> JSDocReadonlyTag {
        return getFirstJSDocTag(node, [](Node node) { return isJSDocReadonlyTag(node); }, /*noCache*/ true);
    }

    inline static auto getJSDocDeprecatedTagNoCache(Node node) -> JSDocDeprecatedTag {
        return getFirstJSDocTag(node, [](Node node) { return isJSDocDeprecatedTag(node); }, /*noCache*/ true);
    }

    inline static auto getJSDocModifierFlagsNoCache(Node node) -> ModifierFlags
    {
        auto flags = ModifierFlags::None;
        if (!!node->parent && !isParameter(node))
        {
            if (isInJSFile(node))
            {
                if (getJSDocPublicTagNoCache(node))
                    flags |= ModifierFlags::Public;
                if (getJSDocPrivateTagNoCache(node))
                    flags |= ModifierFlags::Private;
                if (getJSDocProtectedTagNoCache(node))
                    flags |= ModifierFlags::Protected;
                if (getJSDocReadonlyTagNoCache(node))
                    flags |= ModifierFlags::Readonly;
            }
            if (getJSDocDeprecatedTagNoCache(node))
                flags |= ModifierFlags::Deprecated;
        }

        return flags;
    }

    inline static auto getModifierFlagsWorker(Node node, boolean includeJSDoc, boolean alwaysIncludeJSDoc = false) -> ModifierFlags
    {
        if (node->kind >= SyntaxKind::FirstToken && node->kind <= SyntaxKind::LastToken)
        {
            return ModifierFlags::None;
        }

        if (!(node->modifierFlagsCache & ModifierFlags::HasComputedFlags))
        {
            node->modifierFlagsCache = getSyntacticModifierFlagsNoCache(node) | ModifierFlags::HasComputedFlags;
        }

        if (includeJSDoc && !(node->modifierFlagsCache & ModifierFlags::HasComputedJSDocModifiers) && (alwaysIncludeJSDoc || isInJSFile(node)) && node->parent)
        {
            node->modifierFlagsCache |= getJSDocModifierFlagsNoCache(node) | ModifierFlags::HasComputedJSDocModifiers;
        }

        return node->modifierFlagsCache & ~(ModifierFlags::HasComputedFlags | ModifierFlags::HasComputedJSDocModifiers);
    }

    inline static auto getSyntacticModifierFlags(Node node) -> ModifierFlags
    {
        return getModifierFlagsWorker(node, /*includeJSDoc*/ false);
    }

    inline static auto hasSyntacticModifiers(Node node) -> boolean
    {
        return getSyntacticModifierFlags(node) != ModifierFlags::None;
    }

    inline static auto getSelectedSyntacticModifierFlags(Node node, ModifierFlags flags) -> ModifierFlags {
        return getSyntacticModifierFlags(node) & flags;
    }

    inline static auto hasSyntacticModifier(Node node, ModifierFlags flags) -> boolean {
        return !!getSelectedSyntacticModifierFlags(node, flags);
    }

    inline static auto hasStaticModifier(Node node) -> boolean
    {
        return hasSyntacticModifier(node, ModifierFlags::Static);
    }

    inline static auto isSuperProperty(Node node) -> boolean {
        auto kind = node->kind;
        if (kind == SyntaxKind::PropertyAccessExpression) return node.as<PropertyAccessExpression>()->expression->kind == SyntaxKind::SuperKeyword;        
        if (kind == SyntaxKind::ElementAccessExpression) return node.as<ElementAccessExpression>()->expression->kind == SyntaxKind::SuperKeyword;        
        return false;
    }

    inline static auto hasInvalidEscape(TemplateLiteral _template) -> boolean {
        return _template && !!(isNoSubstitutionTemplateLiteral(_template)
            ? !!_template->templateFlags
            : (!!_template.as<TemplateLiteralTypeNode>()->head->templateFlags || some(_template.as<TemplateLiteralTypeNode>()->templateSpans, 
                [](TemplateSpan span) 
                { 
                    return !!span->literal.as<TemplateMiddle>()->templateFlags; 
                })));
    }

    inline static auto isAssignmentPattern(Node node) -> boolean {
        auto kind = node->kind;
        return kind == SyntaxKind::ArrayLiteralExpression
            || kind == SyntaxKind::ObjectLiteralExpression;
    }

    inline static auto isDeclarationBindingElement(Node bindingElement) -> boolean {
        switch (bindingElement->kind) {
            case SyntaxKind::VariableDeclaration:
            case SyntaxKind::Parameter:
            case SyntaxKind::BindingElement:
                return true;
        }

        return false;
    }

    inline static auto isObjectLiteralElementLike(Node node) -> boolean {
        auto kind = node->kind;
        return kind == SyntaxKind::PropertyAssignment
            || kind == SyntaxKind::ShorthandPropertyAssignment
            || kind == SyntaxKind::SpreadAssignment
            || kind == SyntaxKind::MethodDeclaration
            || kind == SyntaxKind::GetAccessor
            || kind == SyntaxKind::SetAccessor;
    }

    inline static auto getElementsOfBindingOrAssignmentPattern(Node name) -> NodeArray<BindingElement> {
        switch (name->kind) {
            case SyntaxKind::ObjectBindingPattern:
                // `a` in `{a}`
                // `a` in `[a]`
                return name.as<ObjectBindingPattern>()->elements;
            case SyntaxKind::ArrayBindingPattern:
                // `a` in `{a}`
                // `a` in `[a]`
                return NodeArray<BindingElement>(name.as<ArrayBindingPattern>()->elements);
            case SyntaxKind::ArrayLiteralExpression:
                // `a` in `{a}`
                // `a` in `[a]`
                return NodeArray<BindingElement>(name.as<ArrayLiteralExpression>()->elements);

            case SyntaxKind::ObjectLiteralExpression:
                // `a` in `{a}`
                return NodeArray<BindingElement>(name.as<ObjectLiteralExpression>()->properties);
        }

        return undefined;
    }

    inline static auto isAssignmentExpression(Node node, boolean excludeCompoundAssignment = false) -> boolean {
        return isBinaryExpression(node)
            && (excludeCompoundAssignment
                ? node.as<BinaryExpression>()->operatorToken->kind == SyntaxKind::EqualsToken
                : isAssignmentOperator(node.as<BinaryExpression>()->operatorToken->kind))
            && isLeftHandSideExpression(node.as<BinaryExpression>()->left);
    }

    inline static auto isLogicalOrCoalescingAssignmentOperator(SyntaxKind token) -> boolean {
        return token == SyntaxKind::BarBarEqualsToken
            || token == SyntaxKind::AmpersandAmpersandEqualsToken
            || token == SyntaxKind::QuestionQuestionEqualsToken;
    }

    inline static auto getTargetOfBindingOrAssignmentElement(Node bindingElement) -> Node {
        if (isDeclarationBindingElement(bindingElement)) {
            // `a` in `let { a } = ...`
            // `a` in `let { a = 1 } = ...`
            // `b` in `let { a: b } = ...`
            // `b` in `let { a: b = 1 } = ...`
            // `a` in `let { ...a } = ...`
            // `{b}` in `let { a: {b} } = ...`
            // `{b}` in `let { a: {b} = 1 } = ...`
            // `[b]` in `let { a: [b] } = ...`
            // `[b]` in `let { a: [b] = 1 } = ...`
            // `a` in `let [a] = ...`
            // `a` in `let [a = 1] = ...`
            // `a` in `let [...a] = ...`
            // `{a}` in `let [{a}] = ...`
            // `{a}` in `let [{a} = 1] = ...`
            // `[a]` in `let [[a]] = ...`
            // `[a]` in `let [[a] = 1] = ...`
            return bindingElement.as<NamedDeclaration>()->name;
        }

        if (isObjectLiteralElementLike(bindingElement)) {
            switch (bindingElement->kind) {
                case SyntaxKind::PropertyAssignment:
                    // `b` in `({ a: b } = ...)`
                    // `b` in `({ a: b = 1 } = ...)`
                    // `{b}` in `({ a: {b} } = ...)`
                    // `{b}` in `({ a: {b} = 1 } = ...)`
                    // `[b]` in `({ a: [b] } = ...)`
                    // `[b]` in `({ a: [b] = 1 } = ...)`
                    // `b.c` in `({ a: b.c } = ...)`
                    // `b.c` in `({ a: b.c = 1 } = ...)`
                    // `b[0]` in `({ a: b[0] } = ...)`
                    // `b[0]` in `({ a: b[0] = 1 } = ...)`
                    return getTargetOfBindingOrAssignmentElement(bindingElement.as<PropertyAssignment>()->initializer);

                case SyntaxKind::ShorthandPropertyAssignment:
                    // `a` in `({ a } = ...)`
                    // `a` in `({ a = 1 } = ...)`
                    return bindingElement.as<ShorthandPropertyAssignment>()->name;

                case SyntaxKind::SpreadAssignment:
                    // `a` in `({ ...a } = ...)`
                    return getTargetOfBindingOrAssignmentElement(bindingElement.as<SpreadAssignment>()->expression);
            }

            // no target
            return undefined;
        }

        if (isAssignmentExpression(bindingElement, /*excludeCompoundAssignment*/ true)) {
            // `a` in `[a = 1] = ...`
            // `{a}` in `[{a} = 1] = ...`
            // `[a]` in `[[a] = 1] = ...`
            // `a.b` in `[a.b = 1] = ...`
            // `a[0]` in `[a[0] = 1] = ...`
            return getTargetOfBindingOrAssignmentElement(bindingElement.as<BinaryExpression>()->left);
        }

        if (isSpreadElement(bindingElement)) {
            // `a` in `[...a] = ...`
            return getTargetOfBindingOrAssignmentElement(bindingElement.as<SpreadElement>()->expression);
        }

        // `a` in `[a] = ...`
        // `{a}` in `[{a}] = ...`
        // `[a]` in `[[a]] = ...`
        // `a.b` in `[a.b] = ...`
        // `a[0]` in `[a[0]] = ...`
        return bindingElement;
    }

    inline static auto getOperator(Expression expression) -> SyntaxKind {
        if (expression->kind == SyntaxKind::BinaryExpression) {
            return expression.as<BinaryExpression>()->operatorToken->kind;
        }
        else if (expression->kind == SyntaxKind::PrefixUnaryExpression) {
            return expression.as<PrefixUnaryExpression>()->_operator;
        }
        else if (expression->kind == SyntaxKind::PostfixUnaryExpression) {
            return expression.as<PostfixUnaryExpression>()->_operator;
        }
        else {
            return expression->kind;
        }

        return SyntaxKind::Unknown;
    }

    inline static auto getOperatorPrecedence(SyntaxKind nodeKind, SyntaxKind operatorKind, boolean hasArguments = false) {
        switch (nodeKind) {
            case SyntaxKind::CommaListExpression:
                return OperatorPrecedence::Comma;

            case SyntaxKind::SpreadElement:
                return OperatorPrecedence::Spread;

            case SyntaxKind::YieldExpression:
                return OperatorPrecedence::Yield;

            case SyntaxKind::ConditionalExpression:
                return OperatorPrecedence::Conditional;

            case SyntaxKind::BinaryExpression:
                switch (operatorKind) {
                    case SyntaxKind::CommaToken:
                        return OperatorPrecedence::Comma;

                    case SyntaxKind::EqualsToken:
                    case SyntaxKind::PlusEqualsToken:
                    case SyntaxKind::MinusEqualsToken:
                    case SyntaxKind::AsteriskAsteriskEqualsToken:
                    case SyntaxKind::AsteriskEqualsToken:
                    case SyntaxKind::SlashEqualsToken:
                    case SyntaxKind::PercentEqualsToken:
                    case SyntaxKind::LessThanLessThanEqualsToken:
                    case SyntaxKind::GreaterThanGreaterThanEqualsToken:
                    case SyntaxKind::GreaterThanGreaterThanGreaterThanEqualsToken:
                    case SyntaxKind::AmpersandEqualsToken:
                    case SyntaxKind::CaretEqualsToken:
                    case SyntaxKind::BarEqualsToken:
                    case SyntaxKind::BarBarEqualsToken:
                    case SyntaxKind::AmpersandAmpersandEqualsToken:
                    case SyntaxKind::QuestionQuestionEqualsToken:
                        return OperatorPrecedence::Assignment;

                    default:
                        return getBinaryOperatorPrecedence(operatorKind);
                }

            // TODO: Should prefix `++` and `--` be moved to the `Update` precedence?
            case SyntaxKind::TypeAssertionExpression:
            case SyntaxKind::NonNullExpression:
            case SyntaxKind::PrefixUnaryExpression:
            case SyntaxKind::TypeOfExpression:
            case SyntaxKind::VoidExpression:
            case SyntaxKind::DeleteExpression:
            case SyntaxKind::AwaitExpression:
                return OperatorPrecedence::Unary;

            case SyntaxKind::PostfixUnaryExpression:
                return OperatorPrecedence::Update;

            case SyntaxKind::CallExpression:
                return OperatorPrecedence::LeftHandSide;

            case SyntaxKind::NewExpression:
                return hasArguments ? OperatorPrecedence::Member : OperatorPrecedence::LeftHandSide;

            case SyntaxKind::TaggedTemplateExpression:
            case SyntaxKind::PropertyAccessExpression:
            case SyntaxKind::ElementAccessExpression:
            case SyntaxKind::MetaProperty:
                return OperatorPrecedence::Member;

            case SyntaxKind::AsExpression:
                return OperatorPrecedence::Relational;

            case SyntaxKind::ThisKeyword:
            case SyntaxKind::SuperKeyword:
            case SyntaxKind::Identifier:
            case SyntaxKind::NullKeyword:
            case SyntaxKind::TrueKeyword:
            case SyntaxKind::FalseKeyword:
            case SyntaxKind::NumericLiteral:
            case SyntaxKind::BigIntLiteral:
            case SyntaxKind::StringLiteral:
            case SyntaxKind::ArrayLiteralExpression:
            case SyntaxKind::ObjectLiteralExpression:
            case SyntaxKind::FunctionExpression:
            case SyntaxKind::ArrowFunction:
            case SyntaxKind::ClassExpression:
            case SyntaxKind::RegularExpressionLiteral:
            case SyntaxKind::NoSubstitutionTemplateLiteral:
            case SyntaxKind::TemplateExpression:
            case SyntaxKind::ParenthesizedExpression:
            case SyntaxKind::OmittedExpression:
            case SyntaxKind::JsxElement:
            case SyntaxKind::JsxSelfClosingElement:
            case SyntaxKind::JsxFragment:
                return OperatorPrecedence::Primary;

            default:
                return OperatorPrecedence::Invalid;
        }

        return OperatorPrecedence::Invalid;
    }

    inline static auto getExpressionPrecedence(Expression expression) {
        auto _operator = getOperator(expression);
        auto hasArguments = expression->kind == SyntaxKind::NewExpression && !!expression.as<NewExpression>()->arguments;
        return getOperatorPrecedence(expression->kind, _operator, hasArguments);
    }

    inline static auto getLeftmostExpression(Expression node, boolean stopAtCallExpressions) -> Node {
        while (true) {
            switch (node->kind) {
                case SyntaxKind::PostfixUnaryExpression:
                    node = node.as<PostfixUnaryExpression>()->operand;
                    continue;

                case SyntaxKind::BinaryExpression:
                    node = node.as<BinaryExpression>()->left;
                    continue;

                case SyntaxKind::ConditionalExpression:
                    node = node.as<ConditionalExpression>()->condition;
                    continue;

                case SyntaxKind::TaggedTemplateExpression:
                    node = node.as<TaggedTemplateExpression>()->tag;
                    continue;

                case SyntaxKind::CallExpression:
                    if (stopAtCallExpressions) {
                        return node;
                    }
                    node = node.as<CallExpression>()->expression;
                    continue;
                    // falls through
                case SyntaxKind::AsExpression:
                    node = node.as<AsExpression>()->expression;
                    continue;
                case SyntaxKind::ElementAccessExpression:
                    node = node.as<ElementAccessExpression>()->expression;
                    continue;
                case SyntaxKind::PropertyAccessExpression:
                    node = node.as<PropertyAccessExpression>()->expression;
                    continue;
                case SyntaxKind::NonNullExpression:
                    node = node.as<NonNullExpression>()->expression;
                    continue;
                case SyntaxKind::PartiallyEmittedExpression:
                    node = node.as<PartiallyEmittedExpression>()->expression;
                    continue;
            }

            return node;
        }
    }

    inline static auto isUnaryExpressionKind(SyntaxKind kind) -> boolean {
        switch (kind) {
            case SyntaxKind::PrefixUnaryExpression:
            case SyntaxKind::PostfixUnaryExpression:
            case SyntaxKind::DeleteExpression:
            case SyntaxKind::TypeOfExpression:
            case SyntaxKind::VoidExpression:
            case SyntaxKind::AwaitExpression:
            case SyntaxKind::TypeAssertionExpression:
                return true;
            default:
                return isLeftHandSideExpressionKind(kind);
        }
    }

    inline static auto isUnaryExpression(Node node) -> boolean {
        return isUnaryExpressionKind(skipPartiallyEmittedExpressions(node)->kind);
    }

    inline static auto getOperatorAssociativity(SyntaxKind kind, SyntaxKind _operator, boolean hasArguments = false) -> Associativity {
        switch (kind) {
            case SyntaxKind::NewExpression:
                return hasArguments ? Associativity::Left : Associativity::Right;

            case SyntaxKind::PrefixUnaryExpression:
            case SyntaxKind::TypeOfExpression:
            case SyntaxKind::VoidExpression:
            case SyntaxKind::DeleteExpression:
            case SyntaxKind::AwaitExpression:
            case SyntaxKind::ConditionalExpression:
            case SyntaxKind::YieldExpression:
                return Associativity::Right;

            case SyntaxKind::BinaryExpression:
                switch (_operator) {
                    case SyntaxKind::AsteriskAsteriskToken:
                    case SyntaxKind::EqualsToken:
                    case SyntaxKind::PlusEqualsToken:
                    case SyntaxKind::MinusEqualsToken:
                    case SyntaxKind::AsteriskAsteriskEqualsToken:
                    case SyntaxKind::AsteriskEqualsToken:
                    case SyntaxKind::SlashEqualsToken:
                    case SyntaxKind::PercentEqualsToken:
                    case SyntaxKind::LessThanLessThanEqualsToken:
                    case SyntaxKind::GreaterThanGreaterThanEqualsToken:
                    case SyntaxKind::GreaterThanGreaterThanGreaterThanEqualsToken:
                    case SyntaxKind::AmpersandEqualsToken:
                    case SyntaxKind::CaretEqualsToken:
                    case SyntaxKind::BarEqualsToken:
                    case SyntaxKind::BarBarEqualsToken:
                    case SyntaxKind::AmpersandAmpersandEqualsToken:
                    case SyntaxKind::QuestionQuestionEqualsToken:
                        return Associativity::Right;
                }
        }
        return Associativity::Left;
    }

    inline static auto getExpressionAssociativity(Expression expression) {
        auto _operator = getOperator(expression);
        auto hasArguments = expression->kind == SyntaxKind::NewExpression && !!expression.as<NewExpression>()->arguments;
        return getOperatorAssociativity(expression->kind, _operator, hasArguments);
    }

    inline static auto isFunctionOrConstructorTypeNode(Node node) -> boolean {
        switch (node->kind) {
            case SyntaxKind::FunctionType:
            case SyntaxKind::ConstructorType:
                return true;
        }

        return false;
    }

    static auto getJSDocTypeAliasName(JSDocNamespaceBody fullName) -> Identifier {
        // TODO: finish it: !rightNode.as<ModuleDeclaration>()->body
        if (fullName) {
            auto rightNode = fullName;
            while (true) {
                if (isIdentifier(rightNode) || !rightNode.as<ModuleDeclaration>()->body) {
                    return isIdentifier(rightNode) ? rightNode : rightNode.as<ModuleDeclaration>()->name;
                }
                rightNode = rightNode.as<ModuleDeclaration>()->body;
            }
        }

        return undefined;
    }

    inline static auto regex_exec(string &text, regex regEx) -> boolean
    {
        auto words_begin = sregex_iterator(text.begin(), text.end(), regEx);
        auto words_end = sregex_iterator();
        return words_begin != words_end;
    }
}

#endif // UTILITIES_H