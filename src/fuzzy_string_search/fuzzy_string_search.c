#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#endif
#include <fuzzy_string_search/fuzzy_string_search.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



#ifdef _MSC_VER
#pragma intrinsic(_BitScanForward64)
static inline __forceinline unsigned int FIND_FIRST_SET_BIT(unsigned long long int m){
	unsigned long o;
	_BitScanForward64(&o,m);
	return o;
}
#else
#define FIND_FIRST_SET_BIT(m) (__builtin_ffsll(m)-1)
#endif



static inline unsigned int _string_length(const char* data){
	const char* base=data;
	while (((uintptr_t)data)&7){
		if (!(*data)){
			return data-base;
		}
		data++;
	}
	const uint64_t* data64=(const uint64_t*)data;
	while (1){
		uint64_t v=((*data64)-0x101010101010101ull)&0x8080808080808080ull&(~(*data64));
		if (v){
			return ((uintptr_t)data64)+(FIND_FIRST_SET_BIT(v)>>3)-((uintptr_t)base);
		}
		data64++;
	}
}



static inline void _string_copy(const char* src,char* dst,unsigned int length){
	if (length<16){
		while (length){
			*dst=*src;
			src++;
			dst++;
			length--;
		}
		return;
	}
	if (((uintptr_t)dst)&7){
		unsigned int i=8-(((uintptr_t)dst)&7);
		*((uint64_t*)dst)=*((const uint64_t*)src);
		src+=i;
		dst+=i;
		length-=i;
	}
	while (length>31){
		_mm256_storeu_si256((__m256i*)dst,_mm256_lddqu_si256((const __m256i*)src));
		length-=32;
		src+=32;
		dst+=32;
	}
	if (length>15){
		_mm_storeu_si128((__m128i*)dst,_mm_lddqu_si128((const __m128i*)src));
		length-=16;
		src+=16;
		dst+=16;
	}
	const uint64_t* src64=(const uint64_t*)src;
	uint64_t* dst64=(uint64_t*)dst;
	while (length>7){
		*dst64=*src64;
		src64++;
		dst64++;
		length-=8;
	}
	src=(const char*)src64;
	dst=(char*)dst64;
	while (length){
		*dst=*src;
		src++;
		dst++;
		length--;
	}
}



static inline unsigned int _string_next_char_index(const char* str,unsigned int length,char char_,unsigned int start_index){
	const uint64_t* str64=(const uint64_t*)str;
	uint64_t m=0x101010101010101ull*char_;
	uint64_t n=0x8080808080808080ull<<((start_index&7)<<3);
	unsigned int i=start_index>>3;
	str64+=i;
	for (;i<((length+7)>>3);i++){
		uint64_t v=(*str64)^m;
		v=(v-0x101010101010101ull)&(~v)&n;
		if (v){
			return (i<<3)+(FIND_FIRST_SET_BIT(v)>>3);
		}
		str64++;
		n=0x8080808080808080ull;
	}
	return length;
}



void fss_context_init(fss_context_t* out){
	out->words=NULL;
	out->word_count=0;
	out->_first_word_index=0xffffffff;
}



void fss_context_deinit(fss_context_t* ctx){
	ctx->_first_word_index=0xffffffff;
	while (ctx->word_count){
		ctx->word_count--;
		free((ctx->words+ctx->word_count)->data);
	}
	free(ctx->words);
	ctx->words=NULL;
}



void fss_context_add_word(fss_context_t* ctx,const char* word){
	unsigned int length=_string_length(word);
	ctx->word_count++;
	ctx->words=realloc(ctx->words,ctx->word_count*sizeof(fss_word_t));
	(ctx->words+ctx->word_count-1)->data=malloc(length+1); // align on 256-bit boundary
	_string_copy(word,(ctx->words+ctx->word_count-1)->data,length+1);
	(ctx->words+ctx->word_count-1)->length=length;
	(ctx->words+ctx->word_count-1)->_offset=0;
	(ctx->words+ctx->word_count-1)->_next_word_index=ctx->_first_word_index;
	ctx->_first_word_index=ctx->word_count-1;
}



void fss_context_add_letter(fss_context_t* ctx,char letter){
	unsigned int prev_idx=0xffffffff;
	unsigned int idx=ctx->_first_word_index;
	while (idx!=0xffffffff){
		fss_word_t* word=ctx->words+idx;
		word->_offset=_string_next_char_index(word->data,word->length,letter,word->_offset);
		if (word->_offset!=word->length){
			prev_idx=idx;
			idx=word->_next_word_index;
			continue;
		}
		idx=word->_next_word_index;
		if (prev_idx==0xffffffff){
			ctx->_first_word_index=idx;
		}
		else{
			(ctx->words+prev_idx)->_next_word_index=idx;
		}
	}
}
