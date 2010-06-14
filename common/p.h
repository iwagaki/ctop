/*
  Copyright (c) 2009,2010 iwagaki@users.sourceforge.net

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef P_H_
#define P_H_

#include <cstdio>

void debug_printf(char* str, int val) { printf("%s = %d\n", str, val); }
void debug_printf(char* str, long val) { printf("%s = %ld\n", str, val); }
void debug_printf(char* str, long long val) { printf("%s = %lld\n", str, val); }
void debug_printf(char* str, unsigned int val) { printf("%s = %u\n", str, val); }
void debug_printf(char* str, unsigned long val) { printf("%s = %lu\n", str, val); }
void debug_printf(char* str, unsigned long long val) { printf("%s = %llu\n", str, val); }
void debug_printf(char* str, char* val) { printf("%s = %s\n", str, val); }
void debug_printf(char* str, bool val) { printf("%s = %s\n", str, val ? "true" : "false"); }
void debug_printf(char* str, void* val) { printf("%s = %p\n", str, val); }

#ifdef DEBUG
#define p(x) debug_printf(#x, x)
#else
#define p(x)
#endif

#endif // P_H_
