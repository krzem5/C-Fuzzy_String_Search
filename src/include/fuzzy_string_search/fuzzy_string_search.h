#ifndef _FUZZY_STRING_SEARCH_H_
#define _FUZZY_STRING_SEARCH_H_ 1
#include <stddef.h>



typedef struct _FSS_WORD{
	char* data;
	unsigned int length;
	unsigned int _offset;
	unsigned int _next_word_index;
} fss_word_t;



typedef	struct _FSS_CONTEXT{
	fss_word_t* words;
	unsigned int word_count;
	unsigned int _first_word_index;
} fss_context_t;



static inline fss_word_t* fss_context_first_word(fss_context_t* ctx){
	return (ctx->_first_word_index==0xffffffff?NULL:ctx->words+ctx->_first_word_index);
}



static inline fss_word_t* fss_context_next_word(fss_context_t* ctx,fss_word_t* word){
	return (word->_next_word_index==0xffffffff?NULL:ctx->words+word->_next_word_index);
}



void fss_context_init(fss_context_t* out);



void fss_context_deinit(fss_context_t* ctx);



void fss_context_add_word(fss_context_t* ctx,const char* word);



void fss_context_add_letter(fss_context_t* ctx,char letter);



#endif
