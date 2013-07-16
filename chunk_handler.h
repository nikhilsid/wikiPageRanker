#ifndef CHUNK_HANDLER_H
#define CHUNK_HANDLER_H

#include "compressed_file_writer.h"
#include<unordered_set>
#include<string>
#include<map>
#include<vector>
using namespace std;

#define CHUNK 100*(1<<20)
#define uii pair<ui,ui>
#define vuii vector< uii >
#define WORD_LEN 64
#define TITLE 1
#define CATEGORY 2
#define SHIFT 2
#define ADDSHIFT 4
#define SMALL_CHUNK (1<<20)
#define CATEGORY_VALUE 32058264654L


class ChunkHandler {
	public : 
		void init_stop_words();
		void parser(string,ui,ull);
	private : 
		char buf_[CHUNK];
		void read_file(string);
		void write_file(ui);
		unordered_set<ull> stop_words;
		map<ull, vuii > index_map;
};
#endif
