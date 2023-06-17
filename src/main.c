#include <fuzzy_string_search/fuzzy_string_search.h>
#include <stdio.h>



int main(void){
	fss_context_t ctx;
	fss_context_init(&ctx);
	fss_context_add_word(&ctx,"The quick brown fox jumped over the lazy dog.");
	fss_context_add_word(&ctx,"brown dog");
	fss_context_add_word(&ctx,"brown rabbit");
	fss_context_add_word(&ctx,"jump");
	fss_context_add_letter(&ctx,'j');
	fss_context_add_letter(&ctx,'p');
	for (fss_word_t* word=fss_context_first_word(&ctx);word;word=fss_context_next_word(&ctx,word)){
		printf("%s\n",word->data);
	}
	fss_context_deinit(&ctx);
	return 0;
}
