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
  b->w = NULL;
  b->nbits = 0;
  b->capw = 0;
}

static void numbits_free(CFL_NUM_BITS *b) {
  free(b->w);
  b->w = NULL;
  b->nbits = 0;
  b->capw = 0;
}

static size_t numbits_words(const CFL_NUM_BITS *b) {
  return (b->nbits + 31u) / 32u;
}

static void numbits_ensure_bits(CFL_NUM_BITS *b, size_t bits) {
  size_t needw = (bits + 31u) / 32u;
  if (needw > b->capw) {
    size_t ncap = b->capw ? b->capw : 4;
    while (ncap < needw) ncap *= 2;
    b->w = (uint32_t *)CFL_MEM_REALLOC(b->w, ncap * sizeof(uint32_t));
    // zera área nova
    for (size_t i = b->capw; i < ncap; ++i) b->w[i] = 0;
    b->capw = ncap;
  }
}

static void numbits_trim(CFL_NUM_BITS *b) {
  size_t nw = numbits_words(b);
  while (nw > 0 && b->w[nw - 1] == 0) nw--;
  if (nw == 0) {
    b->nbits = 0;
    return;
  }
  // localiza MSB do último word
  uint32_t x = b->w[nw - 1];
  size_t msb = 31u;
  while (msb > 0 && ((x >> msb) & 1u) == 0) msb--;
  b->nbits = (nw - 1) * 32u + (msb + 1);
}

static int numbits_is_zero(const CFL_NUM_BITS *b) { return b->nbits == 0; }

static uint32_t numbits_get_bit(const CFL_NUM_BITS *b, size_t i) {
  if (i >= b->nbits) return 0;
  size_t wi = i >> 5;
  unsigned sh = (unsigned)(i & 31u);
  return (b->w[wi] >> sh) & 1u;
}

static void numbits_set_bit(CFL_NUM_BITS *b, size_t i, uint32_t v) {
  if (v) {
    numbits_ensure_bits(b, i + 1);
    size_t wi = i >> 5;
    unsigned sh = (unsigned)(i & 31u);
    b->w[wi] |= (1u << sh);
    if (i + 1 > b->nbits) b->nbits = i + 1;
  } else {
    if (i >= b->nbits) return;
    size_t wi = i >> 5;
    unsigned sh = (unsigned)(i & 31u);
    b->w[wi] &= ~(1u << sh);
    if (i + 1 == b->nbits) numbits_trim(b);
  }
}

static CFL_NUM_BITS numbits_clone(const CFL_NUM_BITS *a) {
  CFL_NUM_BITS r;
  numbits_init(&r);
  size_t nw = numbits_words(a);
  if (nw) {
    r.w = (uint32_t *)CFL_MEM_ALLOC(nw * sizeof(uint32_t));
    memcpy(r.w, a->w, nw * sizeof(uint32_t));
  }
  r.capw = nw;
  r.nbits = a->nbits;
  return r;
}

static void numbits_from_uint64(CFL_NUM_BITS *b, uint64_t v) {
  numbits_init(b);
  if (v == 0) return;
  numbits_ensure_bits(b, 64);
  b->w[0] = (uint32_t)(v & 0xFFFFFFFFu);
  b->w[1] = (uint32_t)(v >> 32);
  b->nbits = (v >> 32) ? 64 : 32;
  numbits_trim(b);
}

static int numbits_cmp(const CFL_NUM_BITS *a, const CFL_NUM_BITS *b) {
  if (a->nbits != b->nbits) return (a->nbits < b->nbits) ? -1 : +1;
  size_t wa = numbits_words(a);
  for (size_t i = wa; i-- > 0;) {
    if (a->w[i] != b->w[i]) return (a->w[i] < b->w[i]) ? -1 : +1;
  }
  return 0;
}

static CFL_NUM_BITS numbits_add(const CFL_NUM_BITS *a, const CFL_NUM_BITS *b) {
  CFL_NUM_BITS r;
  numbits_init(&r);
  size_t wa = numbits_words(a), wb = numbits_words(b);
  size_t wn = (wa > wb ? wa : wb) + 1;
  if (wn == 0) return r;
  r.w = (uint32_t *)CFL_MEM_CALLOC(wn, sizeof(uint32_t));
  r.capw = wn;
  uint64_t carry = 0;
  for (size_t i = 0; i < wn - 1; ++i) {
    uint64_t av = (i < wa) ? a->w[i] : 0u;
    uint64_t bv = (i < wb) ? b->w[i] : 0u;
    uint64_t s = av + bv + carry;
    r.w[i] = (uint32_t)s;
    carry = s >> 32;
  }
  r.w[wn - 1] = (uint32_t)carry;
  r.nbits = wn * 32u;
  numbits_trim(&r);
  return r;
}

static CFL_NUM_BITS numbits_sub(const CFL_NUM_BITS *a, const CFL_NUM_BITS *b) {
  // assume a >= b
  CFL_NUM_BITS r;
  numbits_init(&r);
  size_t wa = numbits_words(a), wb = numbits_words(b);
  size_t wn = wa ? wa : 1;
  r.w = (uint32_t *)CFL_MEM_CALLOC(wn, sizeof(uint32_t));
  r.capw = wn;
  int64_t borrow = 0;
  for (size_t i = 0; i < wn; ++i) {
    int64_t av = (i < wa) ? (int64_t)(uint64_t)a->w[i] : 0;
    int64_t bv = (i < wb) ? (int64_t)(uint64_t)b->w[i] : 0;
    int64_t s = av - bv - borrow;
    if (s < 0) {
      s += ((int64_t)1 << 32);
      borrow = 1;
    } else
      borrow = 0;
    r.w[i] = (uint32_t)s;
  }
  r.nbits = wn * 32u;
  numbits_trim(&r);
  return r;
}

static void numbits_shl_inplace(CFL_NUM_BITS *a, size_t k) {
  if (numbits_is_zero(a) || k == 0) return;
  size_t word_shift = k / 32u;
  unsigned bit_shift = (unsigned)(k % 32u);
  size_t wa = numbits_words(a);
  size_t new_w = wa + word_shift + 1;
  numbits_ensure_bits(a, (wa * 32u) + k + 1);
  // move de cima para baixo
  for (size_t i = wa; i-- > 0;) {
    uint32_t cur = a->w[i];
    size_t dst = i + word_shift;
    a->w[dst] |= (bit_shift ? (cur << bit_shift) : cur);
    if (bit_shift && dst + 1 < a->capw) {
      a->w[dst + 1] |= (cur >> (32 - bit_shift));
    }
    a->w[i] = 0;  // limpar origem
  }
  a->nbits += k;
  numbits_trim(a);
}

static CFL_NUM_BITS numbits_shl(const CFL_NUM_BITS *a, size_t k) {
  CFL_NUM_BITS r = numbits_clone(a);
  numbits_shl_inplace(&r, k);
  return r;
}

static CFL_NUM_BITS numbits_mul_small(const CFL_NUM_BITS *a, uint32_t m) {
  CFL_NUM_BITS r;
  numbits_init(&r);
  if (m == 0 || numbits_is_zero(a)) return r;
  if (m == 1) return numbits_clone(a);
  size_t wa = numbits_words(a);
  r.capw = wa + 2;
  r.w = (uint32_t *)CFL_MEM_CALLOC(r.capw, sizeof(uint32_t));
  uint64_t carry = 0;
  for (size_t i = 0; i < wa; ++i) {
    uint64_t p = (uint64_t)a->w[i] * m + carry;
    r.w[i] = (uint32_t)p;
    carry = p >> 32;
  }
  r.w[wa] = (uint32_t)carry;
  r.nbits = (wa + 1) * 32u;
  numbits_trim(&r);
  return r;
}

static void numbits_mul_small_inplace(CFL_NUM_BITS *a, uint32_t m) {
  if (m == 0 || numbits_is_zero(a)) {
    numbits_free(a);
    numbits_init(a);
    return;
  }
  if (m == 1) return;
  size_t wa = numbits_words(a);
  numbits_ensure_bits(a, (wa + 1) * 32u);
  uint64_t carry = 0;
  for (size_t i = 0; i < wa; ++i) {
    uint64_t p = (uint64_t)a->w[i] * m + carry;
    a->w[i] = (uint32_t)p;
    carry = p >> 32;
  }
  a->w[wa] = (uint32_t)carry;
  a->nbits = (wa + 1) * 32u;
  numbits_trim(a);
}

static void numbits_add_small_inplace(CFL_NUM_BITS *a, uint32_t s) {
  size_t wa = numbits_words(a);
  if (wa == 0) {
    numbits_from_uint64(a, s);
    return;
  }
  uint64_t carry = s;
  for (size_t i = 0; i < wa; ++i) {
    uint64_t v = (uint64_t)a->w[i] + carry;
    a->w[i] = (uint32_t)v;
    carry = v >> 32;
    if (!carry) break;
  }
  if (carry) {
    numbits_ensure_bits(a, (wa + 1) * 32u);
    a->w[wa] = (uint32_t)carry;
  }
  a->nbits = (wa + (carry ? 1 : 0)) * 32u;
  numbits_trim(a);
}

static CFL_NUM_BITS numbits_mul(const CFL_NUM_BITS *a, const CFL_NUM_BITS *b) {
  CFL_NUM_BITS r;
  numbits_init(&r);
  if (numbits_is_zero(a) || numbits_is_zero(b)) return r;
  size_t wa = numbits_words(a), wb = numbits_words(b);
  size_t wn = wa + wb + 1;
  r.w = (uint32_t *)CFL_MEM_CALLOC(wn, sizeof(uint32_t));
  r.capw = wn;
  for (size_t i = 0; i < wa; ++i) {
    uint64_t carry = 0;
    for (size_t j = 0; j < wb; ++j) {
      uint64_t cur = (uint64_t)a->w[i] * b->w[j] + r.w[i + j] + carry;
      r.w[i + j] = (uint32_t)cur;
      carry = cur >> 32;
    }
    r.w[i + wb] += (uint32_t)carry;
  }
  r.nbits = wn * 32u;
  numbits_trim(&r);
  return r;
}

static CFL_NUM_BITS numbits_sub_inplace_clone(CFL_NUM_BITS *A,
                                              const CFL_NUM_BITS *B) {
  // retorna A - B (não altera B), assume A>=B, altera A in-place e retorna
  // clone de A
  CFL_NUM_BITS r;
  numbits_init(&r);
  size_t wa = numbits_words(A), wb = numbits_words(B);
  int64_t borrow = 0;
  for (size_t i = 0; i < wa; ++i) {
    int64_t av = (int64_t)(uint64_t)A->w[i];
    int64_t bv = (i < wb) ? (int64_t)(uint64_t)B->w[i] : 0;
    int64_t s = av - bv - borrow;
    if (s < 0) {
      s += ((int64_t)1 << 32);
      borrow = 1;
    } else
      borrow = 0;
    A->w[i] = (uint32_t)s;
  }
  A->nbits = wa * 32u;
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
  size_t msbA = A->nbits - 1;
  size_t msbB = B->nbits - 1;
  size_t shift = msbA - msbB;

  numbits_ensure_bits(&Qloc, shift + 1);

  for (size_t k = shift + 1; k-- > 0;) {
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

static void numbits_div_small(const CFL_NUM_BITS *A, uint32_t d,
                              CFL_NUM_BITS *Q, uint32_t *R) {
  // d != 0, divide por pequeno inteiro
  size_t wa = numbits_words(A);
  if (wa == 0) {
    numbits_init(Q);
    *R = 0;
    return;
  }
  CFL_NUM_BITS q;
  numbits_init(&q);
  q.capw = wa;
  q.w = (uint32_t *)CFL_MEM_CALLOC(wa, sizeof(uint32_t));
  uint64_t rem = 0;
  for (size_t i = wa; i-- > 0;) {
    uint64_t cur = (rem << 32) | A->w[i];
    uint32_t qw = (uint32_t)(cur / d);
    rem = cur % d;
    q.w[i] = qw;
  }
  q.nbits = wa * 32u;
  numbits_trim(&q);
  *Q = q;
  *R = (uint32_t)rem;
}

void cfl_number_init(CFL_NUMBER *x) {
  x->sign = 0;
  x->scale = 0;
  numbits_init(&x->mag);
}

void cfl_number_free(CFL_NUMBER *x) {
  numbits_free(&x->mag);
  x->sign = 0;
  x->scale = 0;
}

static void cfl_number_normalize_zero(CFL_NUMBER *x) {
  numbits_trim(&x->mag);
  if (numbits_is_zero(&x->mag)) {
    x->sign = 0;
    x->scale = 0;
  }
}

static void cfl_number_trim_trailing_zeros(CFL_NUMBER *x) {
  // divide por 10 enquanto possível e houver escala
  while (x->scale > 0 && !numbits_is_zero(&x->mag)) {
    CFL_NUM_BITS q;
    uint32_t r;
    numbits_div_small(&x->mag, 10u, &q, &r);
    if (r != 0) {
      numbits_free(&q);
      break;
    }
    numbits_free(&x->mag);
    x->mag = q;
    x->scale--;
  }
  cfl_number_normalize_zero(x);
}

static CFL_NUMBER cfl_number_from_int64(long long v) {
  CFL_NUMBER x;
  cfl_number_init(&x);
  if (v == 0) return x;
  if (v < 0) {
    x.sign = -1;
    uint64_t uv = (uint64_t)(-v);
    numbits_from_uint64(&x.mag, uv);
  } else {
    x.sign = +1;
    numbits_from_uint64(&x.mag, (uint64_t)v);
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
  size_t frac = 0;
  CFL_NUM_BITS mag;
  numbits_init(&mag);

  for (const char *p = s; *p; ++p) {
    if (isdigit((unsigned char)*p)) {
      saw_digit = 1;
      uint32_t d = (uint32_t)(*p - '0');
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
  x.mag = mag;

  // Opcional: remover zeros decimais à direita
  cfl_number_trim_trailing_zeros(&x);
  return x;
}

static char *numbits_to_number_string(const CFL_NUM_BITS *m, size_t scale,
                                   int sign) {
  if (numbits_is_zero(m)) {
    // se scale>0, retorna "0" (sem casas), o chamador formata fração se quiser.
    char *z = (char *)CFL_MEM_ALLOC(2);
    strcpy(z, "0");
    return z;
  }
  // Extrai dígitos decimais por divisões sucessivas por 10
  CFL_NUM_BITS tmp = numbits_clone(m);
  char *digits = NULL;
  size_t len = 0, cap = 0;

  while (!numbits_is_zero(&tmp)) {
    CFL_NUM_BITS q;
    uint32_t r;
    numbits_div_small(&tmp, 10u, &q, &r);
    if (len + 1 > cap) {
      size_t nc = cap ? cap * 2 : 32;
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
  size_t total_digits = len > 0 ? len : 1;
  size_t int_digits = (total_digits > scale) ? (total_digits - scale) : 0;
  size_t frac_digits = scale;
  size_t need_leading_zero = (int_digits == 0) ? 1 : 0;

  size_t out_len = (sign < 0 ? 1 : 0) + (int_digits ? int_digits : 1) +
                   (scale ? 1 + scale : 0) + 1;  // +1 para '\0'
  char *out = (char *)CFL_MEM_ALLOC(out_len);
  size_t pos = 0;

  if (sign < 0) out[pos++] = '-';

  // parte inteira
  if (int_digits == 0) {
    out[pos++] = '0';
  } else {
    for (size_t i = 0; i < int_digits; ++i) {
      size_t idx = total_digits - 1 - i;  // MSD-first
      char ch = (idx < len) ? digits[idx] : '0';
      out[pos++] = ch;
    }
  }

  // parte fracionária
  if (scale) {
    out[pos++] = '.';
    if (total_digits < scale) {
      // zeros à esquerda na fração
      size_t zeros = scale - total_digits;
      for (size_t z = 0; z < zeros; ++z) out[pos++] = '0';
      for (size_t i = 0; i < total_digits; ++i) {
        out[pos++] = digits[total_digits - 1 - i];
      }
    } else {
      for (size_t i = 0; i < scale; ++i) {
        size_t idx = scale - 1 - i;  // mais significativo da fração
        size_t src = idx;
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
  if (x->sign == 0 || numbits_is_zero(&x->mag)) {
    char *z = (char *)CFL_MEM_ALLOC(2);
    strcpy(z, "0");
    return z;
  }
  return numbits_to_number_string(&x->mag, x->scale, x->sign);
}

static void cfl_number_scale_up(CFL_NUMBER *a, size_t k) {
  // multiplica mag por 10^k
  for (size_t i = 0; i < k; ++i) numbits_mul_small_inplace(&a->mag, 10u);
  a->scale += k;
}

static void cfl_number_align_scales(CFL_NUMBER *a, CFL_NUMBER *b) {
  if (a->sign == 0) a->scale = b->scale;
  if (b->sign == 0) b->scale = a->scale;
  if (a->scale == b->scale) return;
  if (a->scale < b->scale) {
    size_t k = b->scale - a->scale;
    for (size_t i = 0; i < k; ++i) numbits_mul_small_inplace(&a->mag, 10u);
    a->scale = b->scale;
  } else {
    size_t k = a->scale - b->scale;
    for (size_t i = 0; i < k; ++i) numbits_mul_small_inplace(&b->mag, 10u);
    b->scale = a->scale;
  }
}

CFL_NUMBER cfl_number_add(const CFL_NUMBER *A, const CFL_NUMBER *B) {
  CFL_NUMBER a = *A;
  a.mag = numbits_clone(&A->mag);
  CFL_NUMBER b = *B;
  b.mag = numbits_clone(&B->mag);
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
    r.mag = numbits_add(&a.mag, &b.mag);
    r.sign = a.sign;
  } else {
    int cmp = numbits_cmp(&a.mag, &b.mag);
    if (cmp == 0) {
      // zero
    } else if (cmp > 0) {
      r.mag = numbits_sub(&a.mag, &b.mag);
      r.sign = a.sign;
    } else {
      r.mag = numbits_sub(&b.mag, &a.mag);
      r.sign = b.sign;
    }
  }
  cfl_number_trim_trailing_zeros(&r);
  numbits_free(&a.mag);
  numbits_free(&b.mag);
  return r;
}

CFL_NUMBER cfl_number_sub(const CFL_NUMBER *A, const CFL_NUMBER *B) {
  CFL_NUMBER nb = *B;
  nb.mag = numbits_clone(&B->mag);
  nb.sign = -nb.sign;
  CFL_NUMBER r = cfl_number_add(A, &nb);
  numbits_free(&nb.mag);
  return r;
}

CFL_NUMBER cfl_number_mul(const CFL_NUMBER *A, const CFL_NUMBER *B) {
  CFL_NUMBER r;
  cfl_number_init(&r);
  if (A->sign == 0 || B->sign == 0) return r;
  r.mag = numbits_mul(&A->mag, &B->mag);
  r.scale = A->scale + B->scale;
  r.sign = (A->sign == B->sign) ? +1 : -1;
  cfl_number_trim_trailing_zeros(&r);
  return r;
}

// Divisão com escala de destino e arredondamento half-up.
// Retorna 0 em sucesso, !=0 em erro (ex.: divisor zero).
int cfl_number_div(const CFL_NUMBER *A, const CFL_NUMBER *B, size_t out_scale, CFL_NUMBER *Out) {
  if (B->sign == 0 || numbits_is_zero(&B->mag)) return -1;

  CFL_NUMBER r;
  cfl_number_init(&r);
  if (A->sign == 0) {
    *Out = r;
    return 0;
  }

  // num = magA * 10^(B.scale + out_scale)
  // den = magB * 10^(A.scale)
  CFL_NUM_BITS num = numbits_clone(&A->mag);
  for (size_t i = 0; i < B->scale + out_scale; ++i)
    numbits_mul_small_inplace(&num, 10u);
  CFL_NUM_BITS den = numbits_clone(&B->mag);
  for (size_t i = 0; i < A->scale; ++i) numbits_mul_small_inplace(&den, 10u);

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

  r.mag = q;
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
