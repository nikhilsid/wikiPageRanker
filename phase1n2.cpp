#include<iostream>
#include<cstdio>
#include<cstring>
#include<algorithm>
#include<string>
#include<map>
#include<unordered_map>
#include<vector>
#include "compressed_file_writer.h"
#include "init_file_names.cpp"

using namespace std;

#define vs vector<string>
#define ii pair<int,int>
#define vii vector<ii>
#define CHUNK (101<<20)
#define SMALL_CHUNK (1<<20)

vector<string> filenames;
char input_buf[CHUNK];
unordered_map<ull,int> title_map;
unordered_map<ull,int> blank_map;

int actual_col;
ull links;
ull found;
ull number_of_files, total_no_pages;

ull hash_title ( char **str ){
	char *ptr = *str;
	ull wv = 0;
	while ( *ptr != '<' ){
		if (*ptr >=65 && *ptr <= 90 )
			*ptr += 32;
		if (*ptr >= 97 && *ptr <= 122 )
			wv = wv*38 + (*ptr - 96); 
		else if ( *ptr >= '0' && *ptr <'9' )
			wv = wv*38 + (*ptr - '0' + 27 );
		else
			wv = wv*38 + ((*ptr == '.' ) ? 37 : (*ptr % 36) );
		ptr++;
	}
	*ptr = 0;
	ptr++;
	*str = ptr;
	return wv;
}
void phase1(){
	CompressedFileWriter title_writer("titles", SMALL_CHUNK);
	CompressedFileWriter title_index_writer("titles_index", SMALL_CHUNK);
	
	total_no_pages = 1;
	for ( ui file_id = 0; file_id < number_of_files; file_id++){
	
		FILE *input_fp = fopen( filenames[file_id].c_str(), "r" );
		int bytes_read = fread( input_buf, 1, CHUNK, input_fp );
		fclose(input_fp);
		input_buf[bytes_read] = 0;
		char *ptr = input_buf;

		while (*ptr ){
			while ( *ptr && ( *ptr++ != '<' || *ptr++ != 'p' || *ptr++ != 'a' || *ptr++ != 'g' || *ptr++ != 'e') );
			if ( *ptr ){	
				while ( *ptr++ != '<' || *ptr++ != 't' || *ptr++ != 'i' || *ptr++ != 't' || *ptr++ != 'l' || *ptr++ != 'e' );
				while ( *ptr++ != '>' );
				
				char *page_title = ptr;
				ull wv = hash_title(&ptr);
				title_index_writer.write_uint (ptr - page_title);
				title_writer.write_uncompressed_string(page_title);
				title_map[wv] = total_no_pages;
				total_no_pages++;
			}
		}
	}
}
void phase2(){
	
	vector< vector<ui> > backlinks(total_no_pages+1);
	ull page_id = 0;
	
	CompressedFileWriter outlinks_writer ("outlinks", SMALL_CHUNK);
	CompressedFileWriter page_offset_writer("page_id_stats", SMALL_CHUNK);
	
	page_offset_writer.write_ulong(total_no_pages);
	
	for ( ui file_id = 0; file_id < number_of_files; file_id++){
		page_offset_writer.write_ulong(page_id + 1);
		
		FILE *input_fp = fopen( filenames[file_id].c_str(), "r" );
		ull bytes_read = fread( input_buf, 1, CHUNK, input_fp );
		fclose(input_fp);
		input_buf[bytes_read] = 0;
		ui current_out_links = 0;
		char *ptr = input_buf;

		while (*ptr ){
			while ( *ptr && ( *ptr++ != '<' || *ptr++ != 'p' || *ptr++ != 'a' || *ptr++ != 'g' || *ptr++ != 'e') );
			if ( *ptr ){	
				while ( *ptr++ != '<' || *ptr++ != 't' || *ptr++ != 'i' || *ptr++ != 't' || *ptr++ != 'l' || *ptr++ != 'e' );
				while ( *ptr++ != '>' );

				while ( *ptr++ != '<' || *ptr++ != 't' || *ptr++ != 'e' || *ptr++ != 'x' || *ptr++ != 't');
				
				page_id++;
				current_out_links = 0;
				while ( *ptr != '<' ){
					char last;
					last = *ptr++;
					while ( *ptr != '<' && !( *ptr == '[' && last == '[' )){
						last = *ptr;
						ptr++;
					}
					if ( *ptr != '<' )
					{
						ptr++;
						if ( strncmp ( ptr, "Category:", 9 ) == 0 )
							break;
						if (( strncmp ( ptr, "Image:", 6) == 0 ) || (strncmp ( ptr, "File:", 5) == 0 ))
							continue;
						char *link_begin = ptr;
						last = *ptr;
						while ( *ptr != '|' && !( *ptr == ']' && last == ']' )){
							last = *ptr;
							ptr++;
						}
						if ( *ptr == '|' )
							*ptr = '<';
						else 
							*(ptr-1) = '<';
						ptr = link_begin;
						ull hash_value = hash_title(&ptr);
						if ( title_map.find(hash_value ) != title_map.end() ){
							ui dest_page_id = title_map[hash_value];
							int len = backlinks[dest_page_id].size();
							if ( len > 0 && backlinks[dest_page_id][len-1] == page_id )
								continue;
							backlinks[dest_page_id].push_back(page_id);
							current_out_links++;
							found++;
						}
						links++;
					}
				}
				outlinks_writer.write_uint(current_out_links);
			}
		}
	}
	cout << "After phase 2 found " << page_id << "pages\n";
	
	CompressedFileWriter backlink_writer("backlinks", SMALL_CHUNK );
	for ( ui i = 1; i < total_no_pages; i++ ){
		for ( ui j = 0 ; j < backlinks[i].size(); j++){
			if ( j == 0 )
				backlink_writer.write_uint(backlinks[i][j]);
			else 
				backlink_writer.write_uint(backlinks[i][j] - backlinks[i][j-1]);
		}
		backlink_writer.write_uint(0);
	}
}

int main(int argc, char *argv[]) {
	pair<int, vector<string> > return_val = init_file_names('x');
	number_of_files = return_val.first;
	filenames = return_val.second;
	goto_arena();
	phase1();
	phase2();
	cout << "no of pages : " << total_no_pages << " " << title_map.size()<< endl;
	cout << "links " << links << "\t found " << found << endl;
	return 0;
}
