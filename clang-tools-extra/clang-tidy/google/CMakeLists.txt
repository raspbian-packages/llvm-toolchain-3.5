set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyGoogleModule
  AvoidCStyleCastsCheck.cpp
  ExplicitConstructorCheck.cpp
  ExplicitMakePairCheck.cpp
  GoogleTidyModule.cpp
  MemsetZeroLengthCheck.cpp
  NamedParameterCheck.cpp
  OverloadedUnaryAndCheck.cpp
  StringReferenceMemberCheck.cpp
  UnnamedNamespaceInHeaderCheck.cpp
  UsingNamespaceDirectiveCheck.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTidy
  )
