#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <stdbool.h>

void hex_dump(void *data, size_t len) {
    unsigned char *chr = data;
    for ( size_t pos = 0; pos < len; pos++, chr++ ) { printf("%02x", *chr & 0xff); }
  printf("\n");
}
#define _byteswap_uint64 __builtin_bswap64

#define BISIZE 256
#define NB64BLOCK 5
#define NB32BLOCK 10
typedef struct {
  uint32_t bits[NB32BLOCK];
  uint64_t bits64[NB64BLOCK];
} Int;

void leCLEAR(Int *b) {
b->bits64[4] = 0;
  b->bits64[3] = 0;
  b->bits64[2] = 0;
  b->bits64[1] = 0;
  b->bits64[0] = 0;

}
void AddOne(Int *b) {

  unsigned char c = 0;
  c = _addcarry_u64(c, b->bits64[0],1, b->bits64 +0);
  c = _addcarry_u64(c, b->bits64[1],0, b->bits64 +1);
  c = _addcarry_u64(c, b->bits64[2],0, b->bits64 +2);
  c = _addcarry_u64(c, b->bits64[3],0, b->bits64 +3);
  c = _addcarry_u64(c, b->bits64[4],0, b->bits64 +4);

}
void Int_Add(Int *a,Int *b) {

  unsigned char c = 0;
  c = _addcarry_u64(c, b->bits64[0], a->bits64[0], a->bits64 +0);
  c = _addcarry_u64(c, b->bits64[1], a->bits64[1], a->bits64 +1);
  c = _addcarry_u64(c, b->bits64[2], a->bits64[2], a->bits64 +2);
  c = _addcarry_u64(c, b->bits64[3], a->bits64[3], a->bits64 +3);
  c = _addcarry_u64(c, b->bits64[4], a->bits64[4], a->bits64 +4);

}
void Sub_n(Int *a,Int *b) {

  unsigned char c = 0;
  c = _subborrow_u64(c, a->bits64[0], b->bits64[0], a->bits64 + 0);
  c = _subborrow_u64(c, a->bits64[1], b->bits64[1], a->bits64 + 1);
  c = _subborrow_u64(c, a->bits64[2], b->bits64[2], a->bits64 + 2);
  c = _subborrow_u64(c, a->bits64[3], b->bits64[3], a->bits64 + 3);
  c = _subborrow_u64(c, a->bits64[4], b->bits64[4], a->bits64 + 4);

}
void Sub_int64(Int *b,uint64_t a) {
  unsigned char c = 0;
  c = _subborrow_u64(c, b->bits64[0], a, b->bits64 + 0);
  c = _subborrow_u64(c, b->bits64[1], 0, b->bits64 + 1);
  c = _subborrow_u64(c, b->bits64[2], 0, b->bits64 + 2);
  c = _subborrow_u64(c, b->bits64[3], 0, b->bits64 + 3);
  c = _subborrow_u64(c, b->bits64[4], 0, b->bits64 + 4);
}
void Set32Bytesb(unsigned char *bytes,Int *b) {
  leCLEAR(b);
  uint64_t *ptr = (uint64_t *)bytes;
  b->bits64[3] = _byteswap_uint64(ptr[0]);
  b->bits64[2] = _byteswap_uint64(ptr[1]);
  b->bits64[1] = _byteswap_uint64(ptr[2]);
  b->bits64[0] = _byteswap_uint64(ptr[3]);

}
void Get32Bytesb(unsigned char *buff,Int *b) {

  uint64_t *ptr = (uint64_t *)buff;
  ptr[3] = _byteswap_uint64(b->bits64[0]);
  ptr[2] = _byteswap_uint64(b->bits64[1]);
  ptr[1] = _byteswap_uint64(b->bits64[2]);
  ptr[0] = _byteswap_uint64(b->bits64[3]);

}

void SetInt_n(Int *b,uint64_t value) {
  leCLEAR(b);
  b->bits64[0]=value;

}
bool IsGreater_n(Int *mn,Int *a) {

  int i;

  for(i=NB64BLOCK-1;i>=0;) {
    if( a->bits64[i]!= mn->bits64[i] )
		break;
    i--;
  }

  if(i>=0) {
    return mn->bits64[i]>a->bits64[i];
  } else {
    return false;
  }

}

void set_n(Int *b,Int *a) {
  leCLEAR(b);
  for (int i = 0; i<NB64BLOCK; i++)
  	b->bits64[i] = a->bits64[i];

}

bool IsEqual_n(Int *b,Int *a) {

return (b->bits64[4] == a->bits64[4]) && (b->bits64[3] == a->bits64[3]) && (b->bits64[2] == a->bits64[2]) && (b->bits64[1] == a->bits64[1]) && (b->bits64[0] == a->bits64[0]);

}

int leGetSize(Int *a) {
  int i=NB32BLOCK-1;
  while(i>0 && a->bits[i]==0) i--;
  return i+1;

}
static uint64_t inline __shiftright128(uint64_t a, uint64_t b,unsigned char n) {
  uint64_t c;
  __asm__ ("movq %1,%0;shrdq %3,%2,%0;" : "=D"(c) : "r"(a),"r"(b),"c"(n));
  return  c;
}
static uint64_t inline __shiftleft128(uint64_t a, uint64_t b,unsigned char n) {
  uint64_t c;
  __asm__ ("movq %1,%0;shldq %3,%2,%0;" : "=D"(c) : "r"(b),"r"(a),"c"(n));
  return  c;
}
void ShiftL64Bit(Int *a) {

	for (int i = NB64BLOCK-1 ; i>0; i--) {
		a->bits64[i] = a->bits64[i - 1];
	}
	a->bits64[0] = 0;

}

void ShiftR64Bit(Int *a) {

	for (int i = 0; i<NB64BLOCK - 1; i++) {
		a->bits64[i] = a->bits64[i + 1];
	}
	if (((int64_t)a->bits64[NB64BLOCK - 2])<0)
		a->bits64[NB64BLOCK - 1] = 0xFFFFFFFFFFFFFFFF;
	else
		a->bits64[NB64BLOCK - 1] = 0;

}

static void inline shiftR(unsigned char n, uint64_t *d) {

  d[0] = __shiftright128(d[0], d[1], n);
  d[1] = __shiftright128(d[1], d[2], n);
  d[2] = __shiftright128(d[2], d[3], n);
  d[3] = __shiftright128(d[3], d[4], n);
  d[NB64BLOCK-1] = ((int64_t)d[NB64BLOCK-1]) >> n;

}
static void inline shiftL(unsigned char n, uint64_t *d) {

  d[4] = __shiftleft128(d[3], d[4], n);
  d[3] = __shiftleft128(d[2], d[3], n);
  d[2] = __shiftleft128(d[1], d[2], n);
  d[1] = __shiftleft128(d[0], d[1], n);
  d[0] = d[0] << n;
}

void Int_ShiftR(Int *a,uint32_t n) {
    
  if( n<64 ) {
    shiftR((unsigned char)n, a->bits64);
  } else {
    uint32_t nb64 = n/64;
    uint32_t nb   = n%64;
    for(uint32_t i=0;i<nb64;i++) ShiftR64Bit(a);
	  shiftR((unsigned char)nb, a->bits64);
  }
  
}

void ShiftL_n(Int *a,uint32_t n) {
    
  if( n<64 ) {
	shiftL((unsigned char)n, a->bits64);
  } else {
    uint32_t nb64 = n/64;
    uint32_t nb   = n%64;
    for(uint32_t i=0;i<nb64;i++) ShiftL64Bit(a);
	  shiftL((unsigned char)nb, a->bits64);
  }
  
}

static uint64_t inline _umul128(uint64_t a, uint64_t b, uint64_t *h) {
  uint64_t rhi;
  uint64_t rlo;
  __asm__( "mulq  %[b];" :"=d"(rhi),"=a"(rlo) :"1"(a),[b]"rm"(b));
    *h = rhi;
    return rlo;
}
void Int_ShiftL32BitAndSub(Int *mn,Int *a,int n) {

  Int b;
  int i=NB32BLOCK-1;

  for(;i>=n;i--)
    b.bits[i] = ~a->bits[i-n];
  for(;i>=0;i--)
    b.bits[i] = 0xFFFFFFFF;

  Int_Add(mn,&b);
  AddOne(mn);

}
static void inline imm_mul(uint64_t *x, uint64_t y, uint64_t *dst) {

  unsigned char c = 0;
  uint64_t h, carry;
  dst[0] = _umul128(x[0], y, &h); carry = h;
  c = _addcarry_u64(c, _umul128(x[1], y, &h), carry, dst + 1); carry = h;
  c = _addcarry_u64(c, _umul128(x[2], y, &h), carry, dst + 2); carry = h;
  c = _addcarry_u64(c, _umul128(x[3], y, &h), carry, dst + 3); carry = h;
  c = _addcarry_u64(c, _umul128(x[4], y, &h), carry, dst + 4); carry = h;

}

void Mult_n(Int *mn,Int *a, uint64_t b) {

  imm_mul(a->bits64, b, mn->bits64);

}
static uint32_t bitLength(uint32_t dw) {
  
  uint32_t mask = 0x80000000;
  uint32_t b=0;
  while(b<32 && (mask & dw)==0) {
    b++;
    mask >>= 1;
  }
  return b;

}

bool Int_IsNegative(Int *mn) {
  return (int64_t)(mn->bits64[NB64BLOCK - 1])<0;
}

void MMod_n(Int *a,Int *mod) {

  if(IsGreater_n(mod,a)) {
    set_n(a,mod);
	printf("high");
    return;
  }

  if(IsEqual_n(a,mod)) {
    leCLEAR(a);
	printf("same");
    return;
  }

  //Division algorithm D (Knuth section 4.3.1)

  Int rem;
  leCLEAR(&rem);
  set_n(&rem,a);
  Int d;
  leCLEAR(&d);
  set_n(&d,mod);
  Int dq;
  leCLEAR(&dq);

  // Size
  uint32_t dSize = leGetSize(&d);
  printf("d+:%d\n",dSize);
  uint32_t tSize = leGetSize(&rem);
  printf("t+:%d\n",tSize);
  uint32_t qSize = tSize - dSize + 1;

  // D1 normalize the divisor
  uint32_t shift = bitLength(d.bits[dSize-1]);
  if (shift > 0) {
    ShiftL_n(&d,shift);
    ShiftL_n(&rem,shift);
  }

  uint32_t  _dh    = d.bits[dSize-1];
  uint64_t  dhLong = _dh;
  uint32_t  _dl    = (dSize>1)?d.bits[dSize-2]:0;
  int sb = tSize-1;
        
  // D2 Initialize j
  for(int j=0; j<(int)qSize; j++) {

    // D3 Estimate qhat
    uint32_t qhat = 0;
    uint32_t qrem = 0;
    int skipCorrection = false;
    uint32_t nh = rem.bits[sb-j+1];
    uint32_t nm = rem.bits[sb-j];

    if (nh == _dh) {
      qhat = ~0;
      qrem = nh + nm;
      skipCorrection = qrem < nh;
    } else {
      uint64_t nChunk = ((uint64_t)nh << 32) | (uint64_t)nm;
      qhat = (uint32_t) (nChunk / dhLong);
      qrem = (uint32_t) (nChunk % dhLong);
    }

    if (qhat == 0)
      continue;

    if (!skipCorrection) { 

      // Correct qhat
      uint64_t nl = (uint64_t)rem.bits[sb-j-1];
      uint64_t rs = ((uint64_t)qrem << 32) | nl;
      uint64_t estProduct = (uint64_t)_dl * (uint64_t)(qhat);

      if (estProduct>rs) {
        qhat--;
        qrem = (uint32_t)(qrem + (uint32_t)dhLong);
        if ((uint64_t)qrem >= dhLong) {
          estProduct = (uint64_t)_dl * (uint64_t)(qhat);
          rs = ((uint64_t)qrem << 32) | nl;
          if(estProduct>rs)
            qhat--;
        }
      }

    }

    // D4 Multiply and subtract    
    Mult_n(&dq,&d,(uint64_t)qhat);
    Int_ShiftL32BitAndSub(&rem,&dq,qSize-j-1);
    if(Int_IsNegative(&rem)) {
	  Int_Add(&rem,&d);
      qhat--;
    }

    //a->bits[qSize-j-1] = qhat;

 }

   Int_ShiftR(&rem,shift) ;
   set_n(a,&rem);

}

int main(int argc, char** argv) {
  unsigned char baseline_privkey[32] = {
    // generated using srand(31415926), first 256 calls of rand() & 0xFF
    0xb9, 0x43, 0x14, 0xa3, 0x7d, 0x33, 0x46, 0x16, 0xd8, 0x0d, 0x62, 0x1b, 0x11, 0xa5, 0x9f, 0xdd,
    0x13, 0x56, 0xf6, 0xec, 0xbb, 0x9e, 0xb1, 0x9e, 0xfd, 0xe6, 0xe0, 0x55, 0x43, 0xb4, 0x1f, 0x30
};
  unsigned char baseline_privkeyy[32] = {
    // generated using srand(31415926), first 256 calls of rand() & 0xFF
    0x01, 0x43, 0x14, 0xa3, 0x7d, 0x33, 0x46, 0x16, 0xd8, 0x0d, 0x62, 0x1b, 0x11, 0xa5, 0x9f, 0xdd,
    0x13, 0x56, 0xf6, 0xec, 0xbb, 0x9e, 0xb1, 0x9e, 0xfd, 0xe6, 0xe0, 0x55, 0x43, 0xb4, 0x1f, 0x30
};
  unsigned char sha_bin[32];
  Int s1;
  Int s2;
  Int s3;
  leCLEAR(&s2);
  leCLEAR(&s1);
  leCLEAR(&s3);
/*   SetInt_n(&s2,53);
  SetInt_n(&s1,5); */
  Set32Bytesb(baseline_privkey,&s1);
  Set32Bytesb(baseline_privkeyy,&s2);
/*   AddOne(&s1);
  AddOne(&s1); */
  //Sub_n(&s1,&s2);
  //Sub_int64(&s1,55);
  MMod_n(&s1,&s2);
  //Mult_n(&s1,&s3,60);
  Get32Bytesb(sha_bin,&s1);
  hex_dump(baseline_privkey, 32);
  hex_dump(sha_bin, 32);
	
}