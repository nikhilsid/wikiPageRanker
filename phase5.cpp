#include<iostream>
#include "porterStemmer.cpp"
#include "compressed_file_reader.h"
#include<cstdio>
#include<fstream>
#include <vector>
#include<unordered_set>
#include<algorithm>
#include<string>
#include<unistd.h>
#include<queue>
#include<dirent.h>
#include<map>
#include<time.h>
#include<sys/time.h>
#include "posting_list.h"

using namespace std;

#define TITLE_VAL 100
#define CATEGORY_VAL 30
#define vs vector<string>
#define uii pair<ui,ui>
#define vuii vector< uii >
#define pull pair< ull , ull >
#define vull vector< pull >
#define LEN 1048576
#define CHUNK 106954752
#define TITLE_BUF 310*LEN
#define WORD_LEN 64
#define pl PostingList
#define vpl vector<pl>

//#define USE_RAM_TITLES

char input[LEN];
struct timeval start_time, end_time;
char tempW[LEN];
vector<double> pageranks;
vull words;

int max_idf;

vector<ui> titles_index_links;
ull total_no_pages;
unordered_set<ull> stop_words;
vpl word_pl ;
pl empty_pl;
CompressedFileReader *pl_reader;

FILE *title_fp ;

#ifdef USE_RAM_TITLES 
char title_buf_[TITLE_BUF];
void init_titles(){
	FILE *fp = fopen("titles", "r");
	fread(title_buf_, 1, TITLE_BUF, fp);
	fclose(fp);
}
#endif

string longToString ( ull a ){
	ui k = WORD_LEN - 1;
	tempW[k--] = 0;
	while ( a ){
		tempW[k--] = 96 + (a%27);
		a /= 27;
	}
	string word(&tempW[k+1]);
	return word;
}

void init_total_no_pages() {
	CompressedFileReader page_stats_reader("page_id_stats", LEN);
	total_no_pages = page_stats_reader.read_ulong();
}
void init_title_index () {
	titles_index_links.push_back(0);
	titles_index_links.push_back(0);
	CompressedFileReader titles_index_links_reader("titles_index", LEN);
	ui prev = 0;
	for( ui i = 1; i < total_no_pages; i++ ) {
		ui cur_len = titles_index_links_reader.read_uint();
		titles_index_links.push_back( cur_len + prev);
		prev += cur_len;
	}
}
string fetch_title ( ui page_id ){
	ui offset = titles_index_links[page_id];
	ui length = titles_index_links[page_id + 1 ] - offset - 1;
#ifdef USE_RAM_TITLES
	string temp_string("");
	for (ui i = 0 ; i < length; i++){
		temp_string += title_buf_[offset+i];
	}
#else
	fseek( title_fp, offset, SEEK_SET);
	tempW[fread( tempW, 1, length, title_fp )] = 0;
	string temp_string(tempW);
#endif
	return temp_string;
}
void init_pageranks(){
	CompressedFileReader page_rank_reader("pageranks" , LEN );
	pageranks.push_back(0.0);
	for ( ui i = 1 ; i < total_no_pages; i++){
		pageranks.push_back(page_rank_reader.read_double());
	}
}

void init_secondary_index (){
	CompressedFileReader sec_index_reader("secondary_index", LEN );
	ull word_val = 0, offset = 0, temp;
	while( sec_index_reader.current_uchar() != 0 ){
		temp = sec_index_reader.read_ulong();
		word_val += temp;
		temp = sec_index_reader.read_ulong();
		offset += temp;
		words.push_back(make_pair(word_val, offset));
	}
}
void init_stop_words(){
	FILE *fp ;
	fp = fopen ("stop_words", "r");
	if ( !fp ){
		cout << "No stop_words file in the directory" << endl;
		exit(4);
	}
	stop_words.insert(0);
	struct stemmer *z = create_stemmer();
	while ( fscanf(fp,"%[^\n]",tempW) > EOF ){
		fscanf(fp,"%*c");
		ull wv = 0;
		ui wl = strlen(tempW);
		wl =  stem(z, tempW , wl - 1 ) + 1;
		for ( ui i = 0 ; i < wl; i++ )
			wv = wv * 27 + (tempW[i] - 96);
		stop_words.insert(wv);
	}
	fclose(fp);
}
void init (){
	init_stop_words();
	chdir("arena");
#ifdef USE_RAM_TITLES
	init_titles();
#endif
	init_secondary_index();
	init_total_no_pages();
	init_title_index();
	title_fp = fopen("titles", "r");
	init_pageranks();
}
/* Now parsing , reading etc 
 */
bool comparator_fn ( pull a, pull b ){
	return ( a.first < b.first );
}
pull hash_word ( char *ptr, int wl ){
	struct stemmer *z = create_stemmer();
	wl =  stem(z, ptr , wl - 1 ) + 1;
	ull word_val = 0;
	ull pos_word = 0;

	for ( ui l = 0 ; l < wl; l++ )
		word_val = word_val*27 + (*(ptr + l) - 96 );

	if ( stop_words.find(word_val) == stop_words.end()){
		pos_word = lower_bound(words.begin(), words.end(), make_pair(word_val,0), comparator_fn ) - words.begin();
		if ( pos_word == 0 )
			word_val = 0;
	}
	else 
		word_val = 0;
	return make_pair(word_val, pos_word);
}
void input_query(){
	cout << "Enter Query : " ;
	fgets( input , LEN, stdin);
}
void parse_query(){
	word_pl.clear();
	char temp_word[WORD_LEN];

	int len = strlen(input);
	input[len-1] = ' ';
	ui wl = 0;
	pl temp_pl;
	ui flag = 7;
	for ( ui i = 0 ; i < len ; i++ ){
		if ( input[i] >= 'A' && input[i] <= 'Z' ){
			input[i] += 32;
		}
		if ( input[i] >= 'a' && input[i] <= 'z' ){
			if ( wl < WORD_LEN - 1 ){
				temp_word[wl] = input[i];
				wl++;
			}
		}
		if ( input[i] == ':' ){
			switch(temp_word[0]){
				case 't' : 
					flag = 1;
					break;
				case 'c' :
					flag = 3;
					break;
				default :
					flag = 7;
					break;
			}
			wl = 0;
		}
		if ( input[i] == ' ' ){
			temp_pl = empty_pl;
			temp_pl.flags = flag;
			temp_word[wl] = 0;
			pull ret_val = hash_word ( temp_word, wl);
			if ( ret_val.first != 0 ){
				temp_pl.word_val = ret_val.first;
				temp_pl.pos = ret_val.second;
				word_pl.push_back(temp_pl);
			}
			wl = 0;
			flag = 7;
		}
	}
}
void fill_title(ui n){
	ui page_id = 0, freq;
	while( pl_reader[n].current_uchar() != 0 ){
		page_id += pl_reader[n].read_uint();
		freq = pl_reader[n].read_uint();
		word_pl[n].title_list.push_back(make_pair(page_id, freq));
	}
	pl_reader[n].read_uint();
}
void fill_category(ui n){
	ui page_id = 0, freq;
	while( pl_reader[n].current_uchar() != 0 ){
		page_id += pl_reader[n].read_uint();
		freq = pl_reader[n].read_uint();
		word_pl[n].category_list.push_back(make_pair(page_id, freq));
	}
	pl_reader[n].read_uint();
}
void fill_high_list (ui n){
	ui page_id = 0, freq;
	while( pl_reader[n].current_uchar() != 0 ){
		page_id += pl_reader[n].read_uint();
		freq = pl_reader[n].read_uint();
		word_pl[n].high_list.push_back(make_pair(page_id, freq));
	}
	pl_reader[n].read_uint();
}

void print_list(vuii list){
	ui size = list.size();
	cout << "Size : " << size << endl;
	for ( ui i = 0 ; i < 10 && i < size; i++)
		cout << list[i].first << " : " << fetch_title(list[i].first) << " : " <<  list[i].second << " : " << pageranks[list[i].first] << endl ;
	cout << endl;
}
void print_complete(pl a){
	cout << "Printing Titles" << endl;
	print_list(a.title_list);
	cout << "Printing Categories" << endl;
	print_list(a.category_list);
	cout << "Printing High List" << endl;
	print_list(a.high_list);
}
void set_file_pl (){
	int n_words = word_pl.size();
	pl_reader = ( CompressedFileReader *) malloc ( n_words * sizeof(CompressedFileReader));
	for ( ui i = 0 ; i < n_words; i++ ){
		ll length = (ll)words[word_pl[i].pos + 1 ].second - (ll)words[word_pl[i].pos].second + 1 ;
		pl_reader[i].set_buf_max(length + 2);
		pl_reader[i].set_file("primary_index");
		pl_reader[i].set_offset(words[word_pl[i].pos].second);
	}
}
vui rank_list ( vuii list ){
	vector< pair<double, ui > > final_list;
	for ( ui i = 0 ; i < list.size(); i++ )
		final_list.push_back( make_pair ( pageranks[list[i].first]*list[i].second, list[i].first ) );
	sort ( final_list.rbegin(), final_list.rend());

	vui ans;
	for ( ui i = 0 ; i < 10 && i < final_list.size() ; i++ ){
		ans.push_back(final_list[i].second);
	}
	return ans;
}
void get_titles(){
	for ( ui i = 0 ; i < word_pl.size() ; i++){
		fill_title(i);
	}
}
void get_categories(){
	for ( ui i = 0 ; i < word_pl.size() ; i++){
		fill_category(i);
	}
}
void get_high_list(){
	for ( ui i = 0 ; i < word_pl.size() ; i++){
		fill_high_list(i);
	}
}
vuii merge_high_list(int n){
	vuii merge_list;
	for ( ui j = 0 ; j < word_pl[n].title_list.size(); j++)
		merge_list.push_back(make_pair(word_pl[n].title_list[j].first, word_pl[n].title_list[j].second*TITLE_VAL));
	for ( ui j = 0 ; j < word_pl[n].category_list.size(); j++)
		merge_list.push_back(make_pair(word_pl[n].category_list[j].first, word_pl[n].category_list[j].second*CATEGORY_VAL));
	merge_list.insert(merge_list.end(), word_pl[n].high_list.begin(), word_pl[n].high_list.end());

	sort(merge_list.begin(), merge_list.end());	
	if ( merge_list.size() > max_idf )
		max_idf = merge_list.size();
	return merge_list;
}

vuii intersect_list (vuii a, vuii b ){
	ui j, k = 0;
	ui len1, len2;
	len1 = a.size();
	len2 = b.size();
	vuii ans;
	for ( j = 0 ; j < len1 && k < len2 ; j++ ){
		while ( k < len2 && ( a[j].first > b[k].first))
			k++;
		if ( (k < len2) && (a[j].first == b[k].first )){
			int freq_val2 = max_idf/b.size();
			if ( freq_val2 > 100 )
				freq_val2 =100;
			ans.push_back( make_pair( a[j].first,  a[j].second + freq_val2*b[j].second));
		}
	}
	return ans;

}
void get_higher_ranks(){
	vector< vuii > high_merge_list;

	for ( ui i = 0 ; i < word_pl.size();i++){
		high_merge_list.push_back(merge_high_list(i));
	}
	if ( high_merge_list[0].size() > 0 ){
		int freq_val = max_idf/high_merge_list[0].size();
		if ( freq_val > 100 )
			freq_val = 100;
		for ( ui i = 0 ; i < high_merge_list[0].size(); i++ ){
			high_merge_list[0][i].second *= freq_val;
		}
		vuii intersection_list = high_merge_list[0];
		for ( ui i = 1 ; i < word_pl.size(); i++ ){
			intersection_list = intersect_list(intersection_list, high_merge_list[i]);
		}
		vui ans = rank_list ( intersection_list );
		for ( ui i = 0 ; i < 10 && i < ans.size() ; i++ ){
			cout << i << " : " << fetch_title (ans[i]) << endl;
		}
	}
}

int main(int argc, char *argv[]) {
	init();

	while ( 1 ){
		max_idf = 0;
		input_query();
		gettimeofday(&start_time, NULL);
		parse_query();
		set_file_pl();
		get_titles();
		get_categories();
		get_high_list();
		get_higher_ranks();
		gettimeofday(&end_time, NULL);
		cout << "Time " << (end_time.tv_sec - start_time.tv_sec)*1000 + 
			(end_time.tv_usec - start_time.tv_usec)/1000.0 << "ms" << endl<<endl ;
	}

	return 0;
}
