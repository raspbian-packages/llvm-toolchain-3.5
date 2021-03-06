//===--- NamedParameterCheck.cpp - clang-tidy -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NamedParameterCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/ASTContext.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace readability {

void NamedParameterCheck::registerMatchers(ast_matchers::MatchFinder *Finder) {
  Finder->addMatcher(
      functionDecl(
          unless(hasAncestor(decl(
              anyOf(recordDecl(ast_matchers::isTemplateInstantiation()),
                    functionDecl(ast_matchers::isTemplateInstantiation()))))))
          .bind("decl"),
      this);
}

void NamedParameterCheck::check(const MatchFinder::MatchResult &Result) {
  const SourceManager &SM = *Result.SourceManager;
  const auto *Function = Result.Nodes.getNodeAs<FunctionDecl>("decl");
  SmallVector<std::pair<const FunctionDecl *, unsigned>, 4> UnnamedParams;

  // Ignore implicitly generated members.
  if (Function->isImplicit())
    return;

  // TODO: Handle overloads.
  // TODO: We could check that all redeclarations use the same name for
  //       arguments in the same position.
  for (unsigned I = 0, E = Function->getNumParams(); I != E; ++I) {
    const ParmVarDecl *Parm = Function->getParamDecl(I);
    // Look for unnamed parameters.
    if (!Parm->getName().empty())
      continue;

    // Sanity check the source locations.
    if (!Parm->getLocation().isValid() || Parm->getLocation().isMacroID() ||
        !SM.isWrittenInSameFile(Parm->getLocStart(), Parm->getLocation()))
      continue;

    // Look for comments. We explicitly want to allow idioms like
    // void foo(int /*unused*/)
    const char *Begin = SM.getCharacterData(Parm->getLocStart());
    const char *End = SM.getCharacterData(Parm->getLocation());
    StringRef Data(Begin, End - Begin);
    if (Data.find("/*") != StringRef::npos)
      continue;

    UnnamedParams.push_back(std::make_pair(Function, I));
  }

  // Emit only one warning per function but fixits for all unnamed parameters.
  if (!UnnamedParams.empty()) {
    const ParmVarDecl *FirstParm =
        UnnamedParams.front().first->getParamDecl(UnnamedParams.front().second);
    auto D = diag(FirstParm->getLocation(),
                  "all parameters should be named in a function");

    for (auto P : UnnamedParams) {
      // If the method is overridden, try to copy the name from the base method
      // into the overrider.
      const ParmVarDecl *Parm = P.first->getParamDecl(P.second);
      const auto *M = dyn_cast<CXXMethodDecl>(P.first);
      if (M && M->size_overridden_methods() > 0) {
        const ParmVarDecl *OtherParm =
            (*M->begin_overridden_methods())->getParamDecl(P.second);
        std::string Name = OtherParm->getNameAsString();
        if (!Name.empty()) {
          D << FixItHint::CreateInsertion(Parm->getLocation(),
                                          " /*" + Name + "*/");
          continue;
        }
      }

      // Otherwise just insert an unused marker. Note that getLocation() points
      // to the place where the name would be, this allows us to also get
      // complex cases like function pointers right.
      D << FixItHint::CreateInsertion(Parm->getLocation(), " /*unused*/");
    }
  }
}

} // namespace readability
} // namespace tidy
} // namespace clang
