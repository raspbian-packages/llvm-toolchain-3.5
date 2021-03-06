//===--- SwappedArgumentsCheck.cpp - clang-tidy ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SwappedArgumentsCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/SmallPtrSet.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void SwappedArgumentsCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(callExpr().bind("call"), this);
}

/// \brief Look through lvalue to rvalue and nop casts. This filters out
/// implicit conversions that have no effect on the input but block our view for
/// other implicit casts.
static const Expr *ignoreNoOpCasts(const Expr *E) {
  if (auto *Cast = dyn_cast<CastExpr>(E))
    if (Cast->getCastKind() == CK_LValueToRValue ||
        Cast->getCastKind() == CK_NoOp)
      return ignoreNoOpCasts(Cast->getSubExpr());
  return E;
}

/// \brief Restrict the warning to implicit casts that are most likely
/// accidental. User defined or integral conversions fit in this category,
/// lvalue to rvalue or derived to base does not.
static bool isImplicitCastCandidate(const CastExpr *Cast) {
  return Cast->getCastKind() == CK_UserDefinedConversion ||
         Cast->getCastKind() == CK_FloatingToBoolean ||
         Cast->getCastKind() == CK_FloatingToIntegral ||
         Cast->getCastKind() == CK_IntegralToBoolean ||
         Cast->getCastKind() == CK_IntegralToFloating ||
         Cast->getCastKind() == CK_MemberPointerToBoolean ||
         Cast->getCastKind() == CK_PointerToBoolean;
}

/// \brief Get a StringRef representing a SourceRange.
static StringRef getAsString(const MatchFinder::MatchResult &Result,
                             SourceRange R) {
  const SourceManager &SM = *Result.SourceManager;
  // Don't even try to resolve macro or include contraptions. Not worth emitting
  // a fixit for.
  if (R.getBegin().isMacroID() ||
      !SM.isWrittenInSameFile(R.getBegin(), R.getEnd()))
    return StringRef();

  const char *Begin = SM.getCharacterData(R.getBegin());
  const char *End = SM.getCharacterData(Lexer::getLocForEndOfToken(
      R.getEnd(), 0, SM, Result.Context->getLangOpts()));

  return StringRef(Begin, End - Begin);
}

void SwappedArgumentsCheck::check(const MatchFinder::MatchResult &Result) {
  auto *Call = Result.Nodes.getStmtAs<CallExpr>("call");

  llvm::SmallPtrSet<const Expr *, 4> UsedArgs;
  for (unsigned I = 1, E = Call->getNumArgs(); I < E; ++I) {
    const Expr *LHS = Call->getArg(I - 1);
    const Expr *RHS = Call->getArg(I);

    // Only need to check RHS, as LHS has already been covered. We don't want to
    // emit two warnings for a single argument.
    if (UsedArgs.count(RHS))
      continue;

    const auto *LHSCast = dyn_cast<ImplicitCastExpr>(ignoreNoOpCasts(LHS));
    const auto *RHSCast = dyn_cast<ImplicitCastExpr>(ignoreNoOpCasts(RHS));

    // Look if this is a potentially swapped argument pair. First look for
    // implicit casts.
    if (!LHSCast || !RHSCast || !isImplicitCastCandidate(LHSCast) ||
        !isImplicitCastCandidate(RHSCast))
      continue;

    // If the types that go into the implicit casts match the types of the other
    // argument in the declaration there is a high probability that the
    // arguments were swapped.
    // TODO: We could make use of the edit distance between the argument name
    // and the name of the passed variable in addition to this type based
    // heuristic.
    const Expr *LHSFrom = ignoreNoOpCasts(LHSCast->getSubExpr());
    const Expr *RHSFrom = ignoreNoOpCasts(RHSCast->getSubExpr());
    if (LHS->getType() == RHS->getType() ||
        LHS->getType() != RHSFrom->getType() ||
        RHS->getType() != LHSFrom->getType())
      continue;

    // Emit a warning and fix-its that swap the arguments.
    SourceRange LHSRange = LHS->getSourceRange(),
                RHSRange = RHS->getSourceRange();
    auto D =
        diag(Call->getLocStart(), "argument with implicit conversion from %0 "
                                  "to %1 followed by argument converted from "
                                  "%2 to %3, potentially swapped arguments.")
        << LHS->getType() << LHSFrom->getType() << RHS->getType()
        << RHSFrom->getType() << LHSRange << RHSRange;

    StringRef RHSString = getAsString(Result, RHSRange);
    StringRef LHSString = getAsString(Result, LHSRange);
    if (!LHSString.empty() && !RHSString.empty()) {
      D << FixItHint::CreateReplacement(
               CharSourceRange::getTokenRange(LHSRange), RHSString)
        << FixItHint::CreateReplacement(
               CharSourceRange::getTokenRange(RHSRange), LHSString);
    }

    // Remember that we emitted a warning for this argument.
    UsedArgs.insert(RHSCast);
  }
}

} // namespace tidy
} // namespace clang
