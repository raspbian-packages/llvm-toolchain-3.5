// RUN: $(dirname %s)/check_clang_tidy_fix.sh %s google-readability-function %t
// REQUIRES: shell

void Method(char *) { /* */ }
// CHECK-MESSAGES: :[[@LINE-1]]:19: warning: all parameters should be named in a function
// CHECK-FIXES: void Method(char * /*unused*/) { /* */ }
void Method2(char *);
// CHECK-MESSAGES: :[[@LINE-1]]:20: warning: all parameters should be named in a function
// CHECK-FIXES: void Method2(char * /*unused*/);
void Method3(char *, void *);
// CHECK-MESSAGES: :[[@LINE-1]]:20: warning: all parameters should be named in a function
// CHECK-FIXES: void Method3(char * /*unused*/, void * /*unused*/);
void Method4(char *, int /*unused*/);
// CHECK-MESSAGES: :[[@LINE-1]]:20: warning: all parameters should be named in a function
// CHECK-FIXES: void Method4(char * /*unused*/, int /*unused*/);
void operator delete[](void *) throw();
// CHECK-MESSAGES: :[[@LINE-1]]:30: warning: all parameters should be named in a function
// CHECK-FIXES: void operator delete[](void * /*unused*/) throw();
int Method5(int);
// CHECK-MESSAGES: :[[@LINE-1]]:16: warning: all parameters should be named in a function
// CHECK-FIXES: int Method5(int /*unused*/);
void Method6(void (*)(void *)) {}
// CHECK-MESSAGES: :[[@LINE-1]]:21: warning: all parameters should be named in a function
// CHECK-FIXES: void Method6(void (* /*unused*/)(void *)) {}
template <typename T> void Method7(T);
// CHECK-MESSAGES: :[[@LINE-1]]:37: warning: all parameters should be named in a function
// CHECK-FIXES: template <typename T> void Method7(T /*unused*/);

// Don't warn in macros.
#define M void MethodM(int);
M

void operator delete(void *x) throw();
void Method7(char * /*x*/) {}
void Method8(char *x);
typedef void (*TypeM)(int x);
void operator delete[](void *x) throw();
void operator delete[](void * /*x*/) throw();

struct X {
  X operator++(int);
// CHECK-MESSAGES: :[[@LINE-1]]:19: warning: all parameters should be named in a function
// CHECK-FIXES: X operator++(int /*unused*/);
  X operator--(int /*unused*/);

  const int &i;
};

void (*Func1)(void *);
void Func2(void (*func)(void *)) {}
template <void Func(void *)> void Func3();

template <typename T>
struct Y {
  void foo(T);
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: all parameters should be named in a function
// CHECK-FIXES: void foo(T /*unused*/);
};

Y<int> y;
Y<float> z;

struct Base {
  virtual void foo(bool notThisOne);
  virtual void foo(int argname);
};

struct Derived : public Base {
  void foo(int);
// CHECK-MESSAGES: :[[@LINE-1]]:15: warning: all parameters should be named in a function
// CHECK-FIXES: void foo(int /*argname*/);
};
