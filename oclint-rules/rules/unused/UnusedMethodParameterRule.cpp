#include <clang/AST/AST.h>

#include "oclint/Rule.h"
#include "oclint/RuleSet.h"
#include "oclint/util/ASTUtil.h"

class UnusedMethodParameterRule : public Rule<UnusedMethodParameterRule>
{
private:
    static RuleSet rules;

    bool isFunctionDeclaration(DeclContext *context)
    {
        FunctionDecl *decl = dyn_cast<FunctionDecl>(context);
        return decl && !decl->doesThisDeclarationHaveABody();
    }

    bool isBlockDeclaration(DeclContext *context)
    {
        return dyn_cast<BlockDecl>(context);
    }

    bool isObjCMethodDeclaration(DeclContext *context)
    {
        ObjCMethodDecl *decl = dyn_cast<ObjCMethodDecl>(context);
        return isObjCMethodDeclLocatedInInterfaceContainer(decl);
    }

    bool isObjCOverrideMethod(DeclContext *context)
    {
        ObjCMethodDecl *decl = dyn_cast<ObjCMethodDecl>(context);
        return isObjCMethodDeclaredInSuperClass(decl) ||
            isObjCMethodDeclaredInProtocol(decl);
    }

    bool isCppFunctionDeclaration(DeclContext *context)
    {
        FunctionDecl *decl = dyn_cast<FunctionDecl>(context);
        return isa<CXXRecordDecl>(context) || (decl && !decl->hasBody());
    }

    bool isCppOverrideFunction(DeclContext *context)
    {
        CXXMethodDecl *decl = dyn_cast<CXXMethodDecl>(context);
        return decl && decl->isVirtual();
    }

    bool isExistingByContract(DeclContext *context)
    {
        return isFunctionDeclaration(context) ||
            isBlockDeclaration(context) ||
            isObjCMethodDeclaration(context) || 
            isObjCOverrideMethod(context) || 
            isCppFunctionDeclaration(context) || 
            isCppOverrideFunction(context);
    }

    bool isExistingByContract(ParmVarDecl *decl)
    {
        DeclContext *lexicalContext = decl->getLexicalDeclContext();
        while (lexicalContext)
        {
            if (isExistingByContract(lexicalContext))
            {
                return true;
            }
            lexicalContext = lexicalContext->getLexicalParent();
        }
        return false;
    }

public:
    virtual const string name() const
    {
        return "unused method parameter";
    }

    virtual const int priority() const
    {
        return 2;
    }

    bool VisitParmVarDecl(ParmVarDecl *varDecl)
    {
        if (varDecl && !varDecl->isUsed() && !isExistingByContract(varDecl))
        {
            _violationSet->addViolation(varDecl, this);
        }

        return true;
    }
};

RuleSet UnusedMethodParameterRule::rules(new UnusedMethodParameterRule());
