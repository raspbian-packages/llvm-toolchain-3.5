// RUN: $(dirname %s)/check_clang_tidy_fix.sh %s google-readability-casting %t
// REQUIRES: shell

bool g() { return false; }

struct X {};
struct Y : public X {};

void f(int a, double b, const char *cpc, const void *cpv, X *pX) {
  const char *cpc2 = (const char*)cpc;
  // CHECK-MESSAGES: :[[@LINE-1]]:22: warning: Redundant cast to the same type. [google-readability-casting]
  // CHECK-FIXES: const char *cpc2 = cpc;

  char *pc = (char*)cpc;
  // CHECK-MESSAGES: :[[@LINE-1]]:14: warning: C-style casts are discouraged. Use const_cast. {{.*}}
  // CHECK-FIXES: char *pc = const_cast<char *>(cpc);

  char *pc2 = (char*)(cpc + 33);
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: C-style casts are discouraged. Use const_cast. {{.*}}
  // CHECK-FIXES: char *pc2 = const_cast<char *>(cpc + 33);

  const char &crc = *cpc;
  char &rc = (char&)crc;
  // CHECK-MESSAGES: :[[@LINE-1]]:14: warning: C-style casts are discouraged. Use const_cast. {{.*}}
  // CHECK-FIXES: char &rc = const_cast<char &>(crc);

  char &rc2 = (char&)*cpc;
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: C-style casts are discouraged. Use const_cast. {{.*}}
  // CHECK-FIXES: char &rc2 = const_cast<char &>(*cpc);

  char ** const* const* ppcpcpc;
  char ****ppppc = (char****)ppcpcpc;
  // CHECK-MESSAGES: :[[@LINE-1]]:20: warning: C-style casts are discouraged. Use const_cast. {{.*}}
  // CHECK-FIXES: char ****ppppc = const_cast<char ****>(ppcpcpc);

  char ***pppc = (char***)*(ppcpcpc);
  // CHECK-MESSAGES: :[[@LINE-1]]:18: warning: C-style casts are discouraged. Use const_cast. {{.*}}
  // CHECK-FIXES: char ***pppc = const_cast<char ***>(*(ppcpcpc));

  char ***pppc2 = (char***)(*ppcpcpc);
  // CHECK-MESSAGES: :[[@LINE-1]]:19: warning: C-style casts are discouraged. Use const_cast. {{.*}}
  // CHECK-FIXES: char ***pppc2 = const_cast<char ***>(*ppcpcpc);

  char *pc5 = (char*)(const char*)(cpv);
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: C-style casts are discouraged. Use const_cast. {{.*}}
  // CHECK-MESSAGES: :[[@LINE-2]]:22: warning: C-style casts are discouraged. Use reinterpret_cast. {{.*}}
  // CHECK-FIXES: char *pc5 = const_cast<char *>(reinterpret_cast<const char *>(cpv));


  int b1 = (int)b;
  // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: C-style casts are discouraged. Use static_cast. [google-readability-casting]
  // CHECK-FIXES: int b1 = static_cast<int>(b);

  Y *pB = (Y*)pX;
  // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: C-style casts are discouraged. Use static_cast/const_cast/reinterpret_cast. [google-readability-casting]
  Y &rB = (Y&)*pX;
  // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: C-style casts are discouraged. Use static_cast/const_cast/reinterpret_cast. [google-readability-casting]

  const char *pc3 = (const char*)cpv;
  // CHECK-MESSAGES: :[[@LINE-1]]:21: warning: C-style casts are discouraged. Use reinterpret_cast. [google-readability-casting]
  // CHECK-FIXES: const char *pc3 = reinterpret_cast<const char *>(cpv);

  char *pc4 = (char*)cpv;
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: C-style casts are discouraged. Use static_cast/const_cast/reinterpret_cast. [google-readability-casting]
  // CHECK-FIXES: char *pc4 = (char*)cpv;

  // CHECK-MESSAGES-NOT: warning:
  int b2 = int(b);
  int b3 = static_cast<double>(b);
  int b4 = b;
  double aa = a;
  (void)b2;
  return (void)g();
}

template <typename T>
void template_function(T t, int n) {
  int i = (int)t;
  // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: C-style casts are discouraged. Use static_cast/const_cast/reinterpret_cast.
  // CHECK-FIXES: int i = (int)t;
  int j = (int)n;
  // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: Redundant cast to the same type.
  // CHECK-FIXES: int j = n;
}

template <typename T>
struct TemplateStruct {
  void f(T t, int n) {
    int k = (int)t;
    // CHECK-MESSAGES: :[[@LINE-1]]:13: warning: C-style casts are discouraged. Use static_cast/const_cast/reinterpret_cast.
    // CHECK-FIXES: int k = (int)t;
    int l = (int)n;
    // CHECK-MESSAGES: :[[@LINE-1]]:13: warning: Redundant cast to the same type.
    // CHECK-FIXES: int l = n;
  }
};

void test_templates() {
  template_function(1, 42);
  template_function(1.0, 42);
  TemplateStruct<int>().f(1, 42);
  TemplateStruct<double>().f(1.0, 42);
}

#define CAST(type, value) (type)(value)
void macros(double d) {
  int i = CAST(int, d);
  // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: C-style casts are discouraged. Use static_cast.
  // CHECK-FIXES: #define CAST(type, value) (type)(value)
  // CHECK-FIXES: int i = CAST(int, d);
}

enum E { E1 = 1 };
template <E e>
struct A {
  // Usage of template argument e = E1 is represented as (E)1 in the AST for
  // some reason. We have a special treatment of this case to avoid warnings
  // here.
  static const E ee = e;
};
struct B : public A<E1> {};
