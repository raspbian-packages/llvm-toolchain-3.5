// RUN: $(dirname %s)/check_clang_tidy_fix.sh %s misc-bool-pointer-implicit-conversion %t
// REQUIRES: shell

bool *SomeFunction();
void SomeOtherFunction(bool*);
bool F();
void G(bool);


template <typename T>
void t(T b) {
  if (b) {
  }
}

void foo() {
  bool *b = SomeFunction();
  if (b) {
// CHECK-MESSAGES: :[[@LINE-1]]:7: warning: dubious check of 'bool *' against 'nullptr'
// CHECK-FIXES: if (*b) {
  }

  if (F() && b) {
// CHECK-MESSAGES: :[[@LINE-1]]:14: warning: dubious check of 'bool *' against 'nullptr'
// CHECK-FIXES: if (F() && *b) {
  }

  // TODO: warn here.
  if (b) {
    G(b);
  }

#define TESTMACRO if (b || F())

  TESTMACRO {
// CHECK-MESSAGES: :[[@LINE-1]]:3: warning: dubious check of 'bool *' against 'nullptr'
// Can't fix this.
// CHECK-FIXES: #define TESTMACRO if (b || F())
// CHECK-FIXES: TESTMACRO {
  }

  t(b);

  if (!b) {
    // no-warning
  }

  if (SomeFunction()) {
    // no-warning
  }

  bool *c = SomeFunction();
  if (c) {
    (void)c;
    (void)*c; // no-warning
  }

  if (c) {
    *c = true; // no-warning
  }

  if (c) {
    c[0] = false; // no-warning
  }

  if (c) {
    SomeOtherFunction(c); // no-warning
  }

  if (c) {
    delete[] c; // no-warning
  }

  if (c) {
    *(c) = false; // no-warning
  }

  struct {
    bool *b;
  } d = { SomeFunction() };

  if (d.b)
    (void)*d.b; // no-warning
}
