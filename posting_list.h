#ifndef POSTING_LIST_H
#define POSTING_LIST_H

#define vs vector<string>
#define vui vector<ui>
#define uii pair<ui,ui>
#define vuii vector< uii >
#define FREQ 8

#include<vector>
#include<iostream>
#include "compressed_file_reader.h"
using namespace std;

class PostingList {
	public :
		PostingList(){
			valid = false;
		}
		bool valid;
		ui flags;
		ull word_val;
		ull pos;
		ui chunk_num;
		vui low_list[FREQ];
		vuii title_list;
		vuii category_list;
		vuii high_list;
		void add_list(PostingList list){
			for ( ui i = 1; i < FREQ; i++ )
				low_list[i].insert(low_list[i].end(), list.low_list[i].begin(), list.low_list[i].end());
			high_list.insert(high_list.end(), list.high_list.begin(), list.high_list.end());
			category_list.insert(category_list.end(), list.category_list.begin(), list.category_list.end());
			title_list.insert(title_list.end(), list.title_list.begin(), list.title_list.end());
		}
		void print_list(vuii list){
			ui size = list.size();
			cout << "Size : " << size << endl;
			for ( ui i = 0 ; i < size; i++)
				cout << list[i].first << " : " << list[i].second << " , " ;
			cout << endl;
		}
		void print(){
			cout << "Printing Titles" << endl;
			print_list(title_list);
			cout << "Printing Categories" << endl;
			print_list(category_list);
			cout << "Printing High List" << endl;
			print_list(high_list);
		}
};
#endif
