#include<iostream>
#include<cstdio>
#include<fstream>
#include <vector>
#include<algorithm>
#include<string>
#include<queue>
#include<map>
#include "init_file_names.cpp"
#include "compressed_file_reader.h"
#include "compressed_file_writer.h"
#include "posting_list.h"

using namespace std;

#define vs vector<string>
#define vui vector<ui>
#define uii pair<ui,ui>
#define vuii vector< uii >
#define WORD_LEN 64
#define SMALL_CHUNK 1048576
#define CHUNK (1<<20)
#define TITLE 1
#define CATEGORY 2
#define ADDSHIFT 4
#define SHIFT 2

char tempW[WORD_LEN];

ui number_of_files;
vuii empty_rec;
vuii cur_rec;
vs file_names;
ull offset,prev_word;
ui number_words;

CompressedFileReader *chunk_reader;
CompressedFileWriter index_writer(CHUNK);
CompressedFileWriter sec_index_writer(SMALL_CHUNK);

PostingList current_list;

string long_to_string ( ull a ){
	int k = WORD_LEN - 1;
	tempW[k--] = 0;
	while ( a ){
		tempW[k--] = 96 + (a%27);
		a /= 27;
	}
	string word(&tempW[k+1]);
	return word;
}

PostingList parse_record( ui n ){
	PostingList list;
	if ( !chunk_reader[n].eof() ){
		list.valid = true;
		list.chunk_num = n;
		list.word_val = chunk_reader[n].read_ulong();
		ui page_id = 0;
		ui freq;
		ui t_freq;
		ui t_page_id;
		ui count = 0;
		while ( chunk_reader[n].current_uchar() != 0 ){
			page_id += chunk_reader[n].read_uint();
			freq = chunk_reader[n].read_uint();
			t_freq = freq>>SHIFT;
			count++;
			if ( freq & TITLE )
				list.title_list.push_back( make_pair(page_id, t_freq) );
			else if ( freq & CATEGORY )
				list.category_list.push_back(make_pair(page_id,t_freq));
			else if ( t_freq < FREQ )
				list.low_list[t_freq].push_back(page_id);
			else 
				list.high_list.push_back( make_pair(page_id,t_freq));
		}
		chunk_reader[n].read_uchar();
	//	cout << chunk_reader[n].file_name_ << " : " << list.word_val << " : " << long_to_string(list.word_val) << " : "  << count << endl;
	}
	return list;
}

void write_record( PostingList list ){
	if ( list.word_val == current_list.word_val ){
		current_list.add_list(list);
	}
	else {
		if ( current_list.word_val ){
			sec_index_writer.write_ulong(current_list.word_val - prev_word);
			sec_index_writer.write_ulong(index_writer.get_byte_offset() - offset);	
			offset = index_writer.get_byte_offset();
			//cout << current_list.word_val << " : " << long_to_string(current_list.word_val) << " : " << offset << endl;
			number_words++;

			ui prev = 0;
			for ( ui i = 0 ; i < current_list.title_list.size(); i++){
				index_writer.write_uint(current_list.title_list[i].first - prev );
				index_writer.write_uint(current_list.title_list[i].second);
				prev = current_list.title_list[i].first;
			}
			index_writer.write_uint(0);
			prev = 0;
			for ( ui i = 0 ; i < current_list.category_list.size(); i++){
				index_writer.write_uint(current_list.category_list[i].first - prev );
				index_writer.write_uint(current_list.category_list[i].second);
				prev = current_list.category_list[i].first;
			}
			index_writer.write_uint(0);
			prev = 0;
			for (ui i = 0 ; i < current_list.high_list.size(); i++){
				index_writer.write_uint(current_list.high_list[i].first - prev );
				index_writer.write_uint(current_list.high_list[i].second);
				prev = current_list.high_list[i].first;
			}
			index_writer.write_uint(0);
			prev = 0;
			for ( ui i = 0 ; i < FREQ; i++ ){
				for ( ui j = 0 ; j < current_list.low_list[i].size(); j++ ){
					index_writer.write_uint(current_list.low_list[i][j] - prev);
					prev = current_list.low_list[i][j];
				}
				index_writer.write_uint(0);
			}
		}
		prev_word = current_list.word_val;
		current_list = list;
	}
}

/**********************NOW HEAP PART ********************/
class HeapHandler {
	public : 
		void make_heap (){
			for ( int i = 0 ; i < number_of_files; i++ )
				insert(i);
		}
		bool pop (){
			if ( heap.empty())
				return false;
			PostingList list;
			list = heap.top();
			heap.pop();
			write_record(list);
			insert(list.chunk_num);
			return true;
		}

	private : 
		struct Comparator{
			bool operator () ( PostingList &a0, PostingList &a1 ){
				if ( a0.word_val < a1.word_val )
					return false;
				else if ( a0.word_val > a1.word_val )
					return true;
				else 
					return ( a0.chunk_num > a1.chunk_num );
			}
		};
		void insert ( ui n ){
			PostingList ans = parse_record(n);
			if ( ans.valid )
				heap.push ( ans );
		}

		priority_queue< PostingList , vector< PostingList >, Comparator > heap;

};
int main(int argc, char *argv[]) {
	
	pair<ui, vs > ret_val = init_file_names('i');
	goto_arena();
	number_of_files = ret_val.first;
	file_names = ret_val.second;

	chunk_reader = ( CompressedFileReader *) malloc ( number_of_files * sizeof(CompressedFileReader));
	for ( ui i = 0 ; i < number_of_files; i++ ){
		chunk_reader[i].set_buf_max(SMALL_CHUNK);
		chunk_reader[i].set_file(file_names[i].c_str());
	}

	index_writer.set_file("primary_index");
	sec_index_writer.set_file("secondary_index");

	cout << "Merging " << number_of_files << endl ;
	
	prev_word = 0;
	offset = 0;
	HeapHandler heapHandler;
	heapHandler.make_heap();

	while ( heapHandler.pop() );

	ull temp = 1;
	temp <<= 63;
	PostingList last;
	last.word_val = temp;
	write_record(last);

	sec_index_writer.write_uint(0);
	cout << "Number Of Words : " << number_words << endl;
	return 0;
}
