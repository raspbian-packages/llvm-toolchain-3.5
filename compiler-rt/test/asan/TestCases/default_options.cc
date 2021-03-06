// RUN: %clangxx_asan -O2 %s -o %t
// RUN: %run %t 2>&1 | FileCheck %s

// __asan_default_options() are not supported on Windows.
// XFAIL: win32

const char *kAsanDefaultOptions="verbosity=1 foo=bar";

extern "C"
__attribute__((no_sanitize_address))
const char *__asan_default_options() {
  // CHECK: Using the defaults from __asan_default_options: {{.*}} foo=bar
  return kAsanDefaultOptions;
}

int main() {
  return 0;
}
