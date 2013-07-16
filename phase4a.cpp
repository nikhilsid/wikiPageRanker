#include<iostream>
#include<cstdio>
#include<fstream>
#include <vector>
#include<algorithm>
#include<string>
#include<map>
#include<unordered_map>
#include<unordered_set>
#include<pthread.h>
#include "compressed_file_reader.h"
#include "compressed_file_writer.h"
#include "init_file_names.cpp"
#include "chunk_handler.h"

using namespace std;
#define NTHREADS 4
#define vs vector<string>
#define vui vector<ui>

ui number_of_files;
vs file_names;
vui page_id_offsets;
ChunkHandler chunksHandlers[NTHREADS];

void init_stop_words(){
	for ( ui i = 0; i < NTHREADS ; i++ ){
		chunksHandlers[i].init_stop_words();
	}
}
void init_page_id_offsets(){
	goto_arena();
	CompressedFileReader offset_reader("page_id_stats", SMALL_CHUNK);
	offset_reader.read_ulong();
	for ( ui i = 0 ; i < number_of_files; i++ ){
		page_id_offsets.push_back(offset_reader.read_ulong());
	}
}
void* runner ( void *arg ){
	ui file_no = *((int *)(arg));

	for ( ; file_no < number_of_files; file_no += NTHREADS){
		int n = file_no % NTHREADS;
		chunksHandlers[n].parser(file_names[file_no], file_no, page_id_offsets[file_no]);	
	}
}
int main(int argc, char *argv[]) {
	init_stop_words();

	pair<ui, vs > ret_val = init_file_names('x');
	number_of_files = ret_val.first;
	file_names = ret_val.second;

	init_page_id_offsets();

	cout << "Processing : " << number_of_files << endl;
	
	pthread_t thread[NTHREADS];
	int val[NTHREADS];
	for ( int i = 0 ; i < NTHREADS; i++ )
		val[i] = i;

	for ( int i = 0 ; (i < NTHREADS) && (i < number_of_files); i++ ){
		if ( pthread_create( &thread[i], NULL, runner , (void *)&val[i]) != 0 )
			cout << "pthread error";
	}
	for ( int i = 0 ; i < NTHREADS && (i < number_of_files ); i++ )
		pthread_join(thread[i], NULL );
	return 0;
}
