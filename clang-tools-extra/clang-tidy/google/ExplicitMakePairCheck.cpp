//===--- ExplicitMakePairCheck.cpp - clang-tidy -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ExplicitMakePairCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/ASTContext.h"

using namespace clang::ast_matchers;

namespace clang {

namespace ast_matchers {
AST_MATCHER(DeclRefExpr, hasExplicitTemplateArgs) {
  return Node.hasExplicitTemplateArgs();
}

// FIXME: This should just be callee(ignoringImpCasts()) but it's not overloaded
// for Expr.
AST_MATCHER_P(CallExpr, calleeIgnoringParenImpCasts, internal::Matcher<Stmt>,
              InnerMatcher) {
  const Expr *ExprNode = Node.getCallee();
  return (ExprNode != nullptr &&
          InnerMatcher.matches(*ExprNode->IgnoreParenImpCasts(), Finder, Builder));
}
} // namespace ast_matchers

namespace tidy {
namespace build {

void
ExplicitMakePairCheck::registerMatchers(ast_matchers::MatchFinder *Finder) {
  // Look for std::make_pair with explicit template args. Ignore calls in
  // templates.
  Finder->addMatcher(
      callExpr(unless(hasAncestor(decl(anyOf(
                   recordDecl(ast_matchers::isTemplateInstantiation()),
                   functionDecl(ast_matchers::isTemplateInstantiation()))))),
               calleeIgnoringParenImpCasts(
                   declRefExpr(hasExplicitTemplateArgs(),
                               to(functionDecl(hasName("::std::make_pair"))))
                       .bind("declref"))).bind("call"),
      this);
}

void ExplicitMakePairCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("call");
  const auto *DeclRef = Result.Nodes.getNodeAs<DeclRefExpr>("declref");

  // Sanity check: The use might have overriden ::std::make_pair.
  if (Call->getNumArgs() != 2)
    return;

  const Expr *Arg0 = Call->getArg(0)->IgnoreParenImpCasts();
  const Expr *Arg1 = Call->getArg(1)->IgnoreParenImpCasts();

  // If types don't match, we suggest replacing with std::pair and explicit
  // template arguments. Otherwise just remove the template arguments from
  // make_pair.
  if (Arg0->getType() != Call->getArg(0)->getType() ||
      Arg1->getType() != Call->getArg(1)->getType()) {
    diag(Call->getLocStart(), "for C++11-compatibility, use pair directly")
        << FixItHint::CreateReplacement(
            SourceRange(DeclRef->getLocStart(), DeclRef->getLAngleLoc()),
            "std::pair<");
  } else {
    diag(Call->getLocStart(),
         "for C++11-compatibility, omit template arguments from make_pair")
        << FixItHint::CreateRemoval(
            SourceRange(DeclRef->getLAngleLoc(), DeclRef->getRAngleLoc()));
  }
}

} // namespace build
} // namespace tidy
} // namespace clang
