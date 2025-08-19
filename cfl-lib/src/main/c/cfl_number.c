/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "cfl_number.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfl_mem.h"

static void numbits_init(CFL_NUM_BITS *b) {
  b->words = NULL;
  b->numBits = 0;
  b->wordCapacity = 0;
}

static void numbits_free(CFL_NUM_BITS *b) {
  free(b->words);
  b->words = NULL;
  b->numBits = 0;
  b->wordCapacity = 0;
}

static CFL_UINT16 numbits_words(const CFL_NUM_BITS *b) {
  return (CFL_UINT16) (b->numBits + 31u) / 32u;
}

static void numbits_ensure_bits(CFL_NUM_BITS *b, CFL_UINT16 bits) {
  CFL_UINT16 needw = (CFL_UINT16) (bits + 31u) / 32u;
  if (needw > b->wordCapacity) {
    CFL_UINT16 ncap = b->wordCapacity ? b->wordCapacity : 4;
    while (ncap < needw) ncap *= 2;
    b->words = (CFL_UINT32 *)CFL_MEM_REALLOC(b->words, ncap * sizeof(CFL_UINT32));
    // clean new area
    for (size_t i = b->wordCapacity; i < ncap; ++i) b->words[i] = 0;
    b->wordCapacity = ncap;
  }
}

static void numbits_trim(CFL_NUM_BITS *b) {
  CFL_UINT16 nw = numbits_words(b);
  while (nw > 0 && b->words[nw - 1] == 0) nw--;
  if (nw == 0) {
    b->numBits = 0;
    return;
  }
  // localiza MSB do último word
  CFL_UINT32 x = b->words[nw - 1];
  CFL_UINT16 msb = 31u;
  while (msb > 0 && ((x >> msb) & 1u) == 0) msb--;
  b->numBits = (nw - 1) * 32u + (msb + 1);
}

static int numbits_is_zero(const CFL_NUM_BITS *b) { 
  return b->numBits == 0; 
}

static CFL_UINT32 numbits_get_bit(const CFL_NUM_BITS *b, CFL_UINT16 i) {
  if (i >= b->numBits) return 0;
  CFL_UINT16 wi = (CFL_UINT16) i >> 5;
  unsigned sh = (unsigned)(i & 31u);
  return (b->words[wi] >> sh) & 1u;
}

static void numbits_set_bit(CFL_NUM_BITS *b, CFL_UINT16 i, CFL_UINT32 v) {
  if (v) {
    numbits_ensure_bits(b, i + 1);
    CFL_UINT16 wi = (CFL_UINT16) i >> 5;
    unsigned sh = (unsigned)(i & 31u);
    b->words[wi] |= (1u << sh);
    if (i + 1 > b->numBits) b->numBits = i + 1;
  } else {
    if (i >= b->numBits) return;
    CFL_UINT16 wi = (CFL_UINT16) i >> 5;
    unsigned sh = (unsigned)(i & 31u);
    b->words[wi] &= ~(1u << sh);
    if (i + 1 == b->numBits) numbits_trim(b);
  }
}

static CFL_NUM_BITS numbits_clone(const CFL_NUM_BITS *a) {
  CFL_NUM_BITS r;
  numbits_init(&r);
  CFL_UINT16 nw = numbits_words(a);
  if (nw) {
    r.words = (CFL_UINT32 *)CFL_MEM_ALLOC(nw * sizeof(CFL_UINT32));
    memcpy(r.words, a->words, nw * sizeof(CFL_UINT32));
  }
  r.wordCapacity = nw;
  r.numBits = a->numBits;
  return r;
}

static void numbits_from_uint64(CFL_NUM_BITS *b, CFL_UINT64 v) {
  numbits_init(b);
  if (v == 0) return;
  numbits_ensure_bits(b, 64);
  b->words[0] = (CFL_UINT32)(v & 0xFFFFFFFFu);
  b->words[1] = (CFL_UINT32)(v >> 32);
  b->numBits = (v >> 32) ? 64 : 32;
  numbits_trim(b);
}

static int numbits_cmp(const CFL_NUM_BITS *a, const CFL_NUM_BITS *b) {
  if (a->numBits != b->numBits) return (a->numBits < b->numBits) ? -1 : +1;
  CFL_UINT16 wa = numbits_words(a);
  for (CFL_UINT16 i = wa; i-- > 0;) {
    if (a->words[i] != b->words[i]) return (a->words[i] < b->words[i]) ? -1 : +1;
  }
  return 0;
}

static CFL_NUM_BITS numbits_add(const CFL_NUM_BITS *a, const CFL_NUM_BITS *b) {
  CFL_NUM_BITS r;
  numbits_init(&r);
  CFL_UINT16 wa = numbits_words(a), wb = numbits_words(b);
  CFL_UINT16 wn = (wa > wb ? wa : wb) + 1;
  if (wn == 0) return r;
  r.words = (CFL_UINT32 *)CFL_MEM_CALLOC(wn, sizeof(CFL_UINT32));
  r.wordCapacity = wn;
  CFL_UINT64 carry = 0;
  for (CFL_UINT16 i = 0; i < wn - 1; ++i) {
    CFL_UINT64 av = (i < wa) ? a->words[i] : 0u;
    CFL_UINT64 bv = (i < wb) ? b->words[i] : 0u;
    CFL_UINT64 s = av + bv + carry;
    r.words[i] = (CFL_UINT32)s;
    carry = s >> 32;
  }
  r.words[wn - 1] = (CFL_UINT32)carry;
  r.numBits = wn * 32u;
  numbits_trim(&r);
  return r;
}

static CFL_NUM_BITS numbits_sub(const CFL_NUM_BITS *a, const CFL_NUM_BITS *b) {
  // assume a >= b
  CFL_NUM_BITS r;
  numbits_init(&r);
  CFL_UINT16 wa = numbits_words(a), wb = numbits_words(b);
  CFL_UINT16 wn = wa ? wa : 1;
  r.words = (CFL_UINT32 *)CFL_MEM_CALLOC(wn, sizeof(CFL_UINT32));
  r.wordCapacity = wn;
  CFL_INT64 borrow = 0;
  for (CFL_UINT16 i = 0; i < wn; ++i) {
    CFL_INT64 av = (i < wa) ? (CFL_INT64)(CFL_UINT64)a->words[i] : 0;
    CFL_INT64 bv = (i < wb) ? (CFL_INT64)(CFL_UINT64)b->words[i] : 0;
    CFL_INT64 s = av - bv - borrow;
    if (s < 0) {
      s += ((CFL_INT64)1 << 32);
      borrow = 1;
    } else
      borrow = 0;
    r.words[i] = (CFL_UINT32)s;
  }
  r.numBits = wn * 32u;
  numbits_trim(&r);
  return r;
}

static void numbits_shl_inplace(CFL_NUM_BITS *a, CFL_UINT16 k) {
  if (numbits_is_zero(a) || k == 0) return;
  CFL_UINT16 word_shift = (CFL_UINT16) k / 32u;
  unsigned bit_shift = (unsigned)(k % 32u);
  CFL_UINT16 wa = numbits_words(a);
  CFL_UINT16 new_w = wa + word_shift + 1;
  numbits_ensure_bits(a, (wa * 32u) + k + 1);
  // move de cima para baixo
  for (CFL_UINT16 i = wa; i-- > 0;) {
    CFL_UINT32 cur = a->words[i];
    CFL_UINT16 dst = i + word_shift;
    a->words[dst] |= (bit_shift ? (cur << bit_shift) : cur);
    if (bit_shift && dst + 1 < a->wordCapacity) {
      a->words[dst + 1] |= (cur >> (32 - bit_shift));
    }
    a->words[i] = 0;  // limpar origem
  }
  a->numBits += k;
  numbits_trim(a);
}

static CFL_NUM_BITS numbits_shl(const CFL_NUM_BITS *a, CFL_UINT16 k) {
  CFL_NUM_BITS r = numbits_clone(a);
  numbits_shl_inplace(&r, k);
  return r;
}

static CFL_NUM_BITS numbits_mul_small(const CFL_NUM_BITS *a, CFL_UINT32 m) {
  CFL_NUM_BITS r;
  numbits_init(&r);
  if (m == 0 || numbits_is_zero(a)) return r;
  if (m == 1) return numbits_clone(a);
  CFL_UINT16 wa = numbits_words(a);
  r.wordCapacity = wa + 2;
  r.words = (CFL_UINT32 *)CFL_MEM_CALLOC(r.wordCapacity, sizeof(CFL_UINT32));
  CFL_UINT64 carry = 0;
  for (CFL_UINT16 i = 0; i < wa; ++i) {
    CFL_UINT64 p = (CFL_UINT64)a->words[i] * m + carry;
    r.words[i] = (CFL_UINT32)p;
    carry = p >> 32;
  }
  r.words[wa] = (CFL_UINT32)carry;
  r.numBits = (wa + 1) * 32u;
  numbits_trim(&r);
  return r;
}

static void numbits_mul_small_inplace(CFL_NUM_BITS *a, CFL_UINT32 m) {
  if (m == 0 || numbits_is_zero(a)) {
    numbits_free(a);
    numbits_init(a);
    return;
  }
  if (m == 1) return;
  CFL_UINT16 wa = numbits_words(a);
  numbits_ensure_bits(a, (wa + 1) * 32u);
  CFL_UINT64 carry = 0;
  for (CFL_UINT16 i = 0; i < wa; ++i) {
    CFL_UINT64 p = (CFL_UINT64)a->words[i] * m + carry;
    a->words[i] = (CFL_UINT32)p;
    carry = p >> 32;
  }
  a->words[wa] = (CFL_UINT32)carry;
  a->numBits = (wa + 1) * 32u;
  numbits_trim(a);
}

static void numbits_add_small_inplace(CFL_NUM_BITS *a, CFL_UINT32 s) {
  CFL_UINT16 wa = numbits_words(a);
  if (wa == 0) {
    numbits_from_uint64(a, s);
    return;
  }
  CFL_UINT64 carry = s;
  for (CFL_UINT16 i = 0; i < wa; ++i) {
    CFL_UINT64 v = (CFL_UINT64)a->words[i] + carry;
    a->words[i] = (CFL_UINT32)v;
    carry = v >> 32;
    if (!carry) break;
  }
  if (carry) {
    numbits_ensure_bits(a, (wa + 1) * 32u);
    a->words[wa] = (CFL_UINT32)carry;
  }
  a->numBits = (wa + (carry ? 1 : 0)) * 32u;
  numbits_trim(a);
}

static CFL_NUM_BITS numbits_mul(const CFL_NUM_BITS *a, const CFL_NUM_BITS *b) {
  CFL_NUM_BITS r;
  numbits_init(&r);
  if (numbits_is_zero(a) || numbits_is_zero(b)) return r;
  CFL_UINT16 wa = numbits_words(a), wb = numbits_words(b);
  CFL_UINT16 wn = wa + wb + 1;
  r.words = (CFL_UINT32 *)CFL_MEM_CALLOC(wn, sizeof(CFL_UINT32));
  r.wordCapacity = wn;
  for (CFL_UINT16 i = 0; i < wa; ++i) {
    CFL_UINT64 carry = 0;
    for (CFL_UINT16 j = 0; j < wb; ++j) {
      CFL_UINT64 cur = (CFL_UINT64)a->words[i] * b->words[j] + r.words[i + j] + carry;
      r.words[i + j] = (CFL_UINT32)cur;
      carry = cur >> 32;
    }
    r.words[i + wb] += (CFL_UINT32)carry;
  }
  r.numBits = wn * 32u;
  numbits_trim(&r);
  return r;
}

static CFL_NUM_BITS numbits_sub_inplace_clone(CFL_NUM_BITS *A,
                                              const CFL_NUM_BITS *B) {
  // retorna A - B (não altera B), assume A>=B, altera A in-place e retorna
  // clone de A
  CFL_NUM_BITS r;
  numbits_init(&r);
  CFL_UINT16 wa = numbits_words(A), wb = numbits_words(B);
  CFL_INT64 borrow = 0;
  for (CFL_UINT16 i = 0; i < wa; ++i) {
    CFL_INT64 av = (CFL_INT64)(CFL_UINT64)A->words[i];
    CFL_INT64 bv = (i < wb) ? (CFL_INT64)(CFL_UINT64)B->words[i] : 0;
    CFL_INT64 s = av - bv - borrow;
    if (s < 0) {
      s += ((CFL_INT64)1 << 32);
      borrow = 1;
    } else
      borrow = 0;
    A->words[i] = (CFL_UINT32)s;
  }
  A->numBits = wa * 32u;
  numbits_trim(A);
  return numbits_clone(A);
}

static CFL_NUM_BITS numbits_divmod(const CFL_NUM_BITS *A, const CFL_NUM_BITS *B,
                                   CFL_NUM_BITS *Q, CFL_NUM_BITS *R) {
  // Retorna Q e R tais que A = Q*B + R, B != 0
  if (numbits_is_zero(B)) {  // não deve acontecer; proteção
    numbits_init(Q);
    numbits_init(R);
    return *Q;
  }
  CFL_NUM_BITS Rloc = numbits_clone(A);
  CFL_NUM_BITS Qloc;
  numbits_init(&Qloc);
  if (numbits_cmp(A, B) < 0) {
    // Q=0, R=A
    *Q = Qloc;
    *R = Rloc;
    CFL_NUM_BITS dummy;
    numbits_init(&dummy);
    return dummy;
  }
  CFL_UINT16 msbA = A->numBits - 1;
  CFL_UINT16 msbB = B->numBits - 1;
  CFL_UINT16 shift = msbA - msbB;

  numbits_ensure_bits(&Qloc, shift + 1);

  for (CFL_UINT16 k = shift + 1; k-- > 0;) {
    CFL_NUM_BITS T = numbits_shl(B, k);
    if (numbits_cmp(&Rloc, &T) >= 0) {
      CFL_NUM_BITS tmp = numbits_sub_inplace_clone(&Rloc, &T);
      numbits_free(&tmp);
      numbits_set_bit(&Qloc, k, 1);
    }
    numbits_free(&T);
    if (k == 0) break;
  }
  numbits_trim(&Qloc);
  numbits_trim(&Rloc);
  *Q = Qloc;
  *R = Rloc;
  CFL_NUM_BITS dummy;
  numbits_init(&dummy);
  return dummy;
}

static void numbits_div_small(const CFL_NUM_BITS *A, CFL_UINT32 d,
                              CFL_NUM_BITS *Q, CFL_UINT32 *R) {
  // d != 0, divide por pequeno inteiro
  CFL_UINT16 wa = numbits_words(A);
  if (wa == 0) {
    numbits_init(Q);
    *R = 0;
    return;
  }
  CFL_NUM_BITS q;
  numbits_init(&q);
  q.wordCapacity = wa;
  q.words = (CFL_UINT32 *)CFL_MEM_CALLOC(wa, sizeof(CFL_UINT32));
  CFL_UINT64 rem = 0;
  for (CFL_UINT16 i = wa; i-- > 0;) {
    CFL_UINT64 cur = (rem << 32) | A->words[i];
    CFL_UINT32 qw = (CFL_UINT32)(cur / d);
    rem = cur % d;
    q.words[i] = qw;
  }
  q.numBits = wa * 32u;
  numbits_trim(&q);
  *Q = q;
  *R = (CFL_UINT32)rem;
}

void cfl_number_init(CFL_NUMBER *x) {
  x->sign = 0;
  x->scale = 0;
  numbits_init(&x->magnitude);
}

void cfl_number_free(CFL_NUMBER *x) {
  numbits_free(&x->magnitude);
  x->sign = 0;
  x->scale = 0;
}

static void cfl_number_normalize_zero(CFL_NUMBER *x) {
  numbits_trim(&x->magnitude);
  if (numbits_is_zero(&x->magnitude)) {
    x->sign = 0;
    x->scale = 0;
  }
}

static void cfl_number_trim_trailing_zeros(CFL_NUMBER *x) {
  // divide por 10 enquanto possível e houver escala
  while (x->scale > 0 && !numbits_is_zero(&x->magnitude)) {
    CFL_NUM_BITS q;
    CFL_UINT32 r;
    numbits_div_small(&x->magnitude, 10u, &q, &r);
    if (r != 0) {
      numbits_free(&q);
      break;
    }
    numbits_free(&x->magnitude);
    x->magnitude = q;
    x->scale--;
  }
  cfl_number_normalize_zero(x);
}

CFL_NUMBER cfl_number_from_int64(CFL_INT64 v) {
  CFL_NUMBER x;
  cfl_number_init(&x);
  if (v == 0) return x;
  if (v < 0) {
    x.sign = -1;
    CFL_UINT64 uv = (CFL_UINT64)(-v);
    numbits_from_uint64(&x.magnitude, uv);
  } else {
    x.sign = +1;
    numbits_from_uint64(&x.magnitude, (CFL_UINT64)v);
  }
  x.scale = 0;
  return x;
}

CFL_NUMBER cfl_number_from_string(const char *s) {
  CFL_NUMBER x;
  cfl_number_init(&x);
  while (isspace((unsigned char)*s)) s++;
  int sign = +1;
  if (*s == '+') {
    s++;
  } else if (*s == '-') {
    sign = -1;
    s++;
  }

  int saw_digit = 0, saw_dot = 0;
  CFL_UINT16 frac = 0;
  CFL_NUM_BITS mag;
  numbits_init(&mag);

  for (const char *p = s; *p; ++p) {
    if (isdigit((unsigned char)*p)) {
      saw_digit = 1;
      CFL_UINT32 d = (CFL_UINT32)(*p - '0');
      numbits_mul_small_inplace(&mag, 10u);
      if (d) numbits_add_small_inplace(&mag, d);
      if (saw_dot) frac++;
    } else if (*p == '.' && !saw_dot) {
      saw_dot = 1;
    } else if (isspace((unsigned char)*p)) {
      break;
    } else {
      break;  // para simples
    }
  }

  if (!saw_digit || numbits_is_zero(&mag)) {
    numbits_free(&mag);
    return x;  // zero
  }

  x.sign = sign;
  x.scale = frac;
  x.magnitude = mag;

  // Opcional: remover zeros decimais à direita
  cfl_number_trim_trailing_zeros(&x);
  return x;
}

static char *numbits_to_number_string(const CFL_NUM_BITS *m, CFL_UINT16 scale, int sign) {
  if (numbits_is_zero(m)) {
    // se scale>0, retorna "0" (sem casas), o chamador formata fração se quiser.
    char *z = (char *)CFL_MEM_ALLOC(2);
    strcpy(z, "0");
    return z;
  }
  // Extrai dígitos decimais por divisões sucessivas por 10
  CFL_NUM_BITS tmp = numbits_clone(m);
  char *digits = NULL;
  CFL_UINT16 len = 0, cap = 0;

  while (!numbits_is_zero(&tmp)) {
    CFL_NUM_BITS q;
    CFL_UINT32 r;
    numbits_div_small(&tmp, 10u, &q, &r);
    if (len + 1 > cap) {
      CFL_UINT16 nc = cap ? cap * 2 : 32;
      digits = (char *)CFL_MEM_REALLOC(digits, nc);
      cap = nc;
    }
    digits[len++] = (char)('0' + (int)r);  // LSD-first
    numbits_free(&tmp);
    tmp = q;
  }
  numbits_free(&tmp);

  // Agora digits contém o número não escalonado, invertido (LSD-first)
  // Vamos montar a string com sinal, ponto e zeros de padding
  CFL_UINT16 total_digits = len > 0 ? len : 1;
  CFL_UINT16 int_digits = (total_digits > scale) ? (total_digits - scale) : 0;
  CFL_UINT16 frac_digits = scale;
  CFL_UINT16 need_leading_zero = (int_digits == 0) ? 1 : 0;

  CFL_UINT16 out_len = (sign < 0 ? 1 : 0) + (int_digits ? int_digits : 1) +
                   (scale ? 1 + scale : 0) + 1;  // +1 para '\0'
  char *out = (char *)CFL_MEM_ALLOC(out_len);
  CFL_UINT16 pos = 0;

  if (sign < 0) out[pos++] = '-';

  // parte inteira
  if (int_digits == 0) {
    out[pos++] = '0';
  } else {
    for (CFL_UINT16 i = 0; i < int_digits; ++i) {
      CFL_UINT16 idx = total_digits - 1 - i;  // MSD-first
      char ch = (idx < len) ? digits[idx] : '0';
      out[pos++] = ch;
    }
  }

  // parte fracionária
  if (scale) {
    out[pos++] = '.';
    if (total_digits < scale) {
      // zeros à esquerda na fração
      CFL_UINT16 zeros = scale - total_digits;
      for (CFL_UINT16 z = 0; z < zeros; ++z) out[pos++] = '0';
      for (CFL_UINT16 i = 0; i < total_digits; ++i) {
        out[pos++] = digits[total_digits - 1 - i];
      }
    } else {
      for (CFL_UINT16 i = 0; i < scale; ++i) {
        CFL_UINT16 idx = scale - 1 - i;  // mais significativo da fração
        CFL_UINT16 src = idx;
        char ch = (src < len) ? digits[src] : '0';
        out[pos++] = ch;
      }
    }
  }

  out[pos] = '\0';
  free(digits);
  return out;
}

char *cfl_number_to_string(const CFL_NUMBER *x) {
  if (x->sign == 0 || numbits_is_zero(&x->magnitude)) {
    char *z = (char *)CFL_MEM_ALLOC(2);
    strcpy(z, "0");
    return z;
  }
  return numbits_to_number_string(&x->magnitude, x->scale, x->sign);
}

static void cfl_number_scale_up(CFL_NUMBER *a, CFL_UINT16 k) {
  // multiplica mag por 10^k
  for (CFL_UINT16 i = 0; i < k; ++i) numbits_mul_small_inplace(&a->magnitude, 10u);
  a->scale += k;
}

static void cfl_number_align_scales(CFL_NUMBER *a, CFL_NUMBER *b) {
  if (a->sign == 0) a->scale = b->scale;
  if (b->sign == 0) b->scale = a->scale;
  if (a->scale == b->scale) return;
  if (a->scale < b->scale) {
    CFL_UINT16 k = b->scale - a->scale;
    for (CFL_UINT16 i = 0; i < k; ++i) numbits_mul_small_inplace(&a->magnitude, 10u);
    a->scale = b->scale;
  } else {
    CFL_UINT16 k = a->scale - b->scale;
    for (CFL_UINT16 i = 0; i < k; ++i) numbits_mul_small_inplace(&b->magnitude, 10u);
    b->scale = a->scale;
  }
}

CFL_NUMBER cfl_number_add(const CFL_NUMBER *A, const CFL_NUMBER *B) {
  CFL_NUMBER a = *A;
  a.magnitude = numbits_clone(&A->magnitude);
  CFL_NUMBER b = *B;
  b.magnitude = numbits_clone(&B->magnitude);
  cfl_number_align_scales(&a, &b);

  CFL_NUMBER r;
  cfl_number_init(&r);
  r.scale = a.scale;

  if (a.sign == 0) {
    r = b;
    return r;
  }
  if (b.sign == 0) {
    r = a;
    return r;
  }

  if (a.sign == b.sign) {
    r.magnitude = numbits_add(&a.magnitude, &b.magnitude);
    r.sign = a.sign;
  } else {
    int cmp = numbits_cmp(&a.magnitude, &b.magnitude);
    if (cmp == 0) {
      // zero
    } else if (cmp > 0) {
      r.magnitude = numbits_sub(&a.magnitude, &b.magnitude);
      r.sign = a.sign;
    } else {
      r.magnitude = numbits_sub(&b.magnitude, &a.magnitude);
      r.sign = b.sign;
    }
  }
  cfl_number_trim_trailing_zeros(&r);
  numbits_free(&a.magnitude);
  numbits_free(&b.magnitude);
  return r;
}

CFL_NUMBER cfl_number_sub(const CFL_NUMBER *A, const CFL_NUMBER *B) {
  CFL_NUMBER nb = *B;
  nb.magnitude = numbits_clone(&B->magnitude);
  nb.sign = -nb.sign;
  CFL_NUMBER r = cfl_number_add(A, &nb);
  numbits_free(&nb.magnitude);
  return r;
}

CFL_NUMBER cfl_number_mul(const CFL_NUMBER *A, const CFL_NUMBER *B) {
  CFL_NUMBER r;
  cfl_number_init(&r);
  if (A->sign == 0 || B->sign == 0) return r;
  r.magnitude = numbits_mul(&A->magnitude, &B->magnitude);
  r.scale = A->scale + B->scale;
  r.sign = (A->sign == B->sign) ? +1 : -1;
  cfl_number_trim_trailing_zeros(&r);
  return r;
}

// Divisão com escala de destino e arredondamento half-up.
// Retorna 0 em sucesso, !=0 em erro (ex.: divisor zero).
int cfl_number_div(const CFL_NUMBER *A, const CFL_NUMBER *B, CFL_UINT16 out_scale, CFL_NUMBER *Out) {
  if (B->sign == 0 || numbits_is_zero(&B->magnitude)) return -1;

  CFL_NUMBER r;
  cfl_number_init(&r);
  if (A->sign == 0) {
    *Out = r;
    return 0;
  }

  // num = magA * 10^(B.scale + out_scale)
  // den = magB * 10^(A.scale)
  CFL_NUM_BITS num = numbits_clone(&A->magnitude);
  for (CFL_UINT16 i = 0; i < B->scale + out_scale; ++i)
    numbits_mul_small_inplace(&num, 10u);
  CFL_NUM_BITS den = numbits_clone(&B->magnitude);
  for (CFL_UINT16 i = 0; i < A->scale; ++i) numbits_mul_small_inplace(&den, 10u);

  CFL_NUM_BITS q, rem;
  numbits_init(&q);
  numbits_init(&rem);
  numbits_divmod(&num, &den, &q, &rem);

  // arredondamento half-up: se 2*rem >= den => q++
  if (!numbits_is_zero(&rem)) {
    CFL_NUM_BITS twice = numbits_shl(&rem, 1);
    int cmp = numbits_cmp(&twice, &den);
    numbits_free(&twice);
    if (cmp >= 0) {
      CFL_NUM_BITS one;
      numbits_from_uint64(&one, 1);
      CFL_NUM_BITS q1 = numbits_add(&q, &one);
      numbits_free(&one);
      numbits_free(&q);
      q = q1;
    }
  }

  r.magnitude = q;
  r.scale = out_scale;
  r.sign = (A->sign == B->sign) ? +1 : -1;

  // Não removemos zeros para preservar out_scale (faça manualmente se desejar).
  cfl_number_normalize_zero(&r);

  numbits_free(&num);
  numbits_free(&den);
  numbits_free(&rem);
  *Out = r;
  return 0;
}
