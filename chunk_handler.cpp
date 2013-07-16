#include "chunk_handler.h"
#include "compressed_file_writer.h"
#include "porterStemmer.cpp"
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

/***************************************************************
 *Getting the list of stop words from the file stopWords
 **************************************************************/
void ChunkHandler::init_stop_words(){
	FILE *fp ;
	fp = fopen ("stop_words", "r");
	if ( !fp ){
		cout << "No stop_words file in the directory" << endl;
		exit(4);
	}
	stop_words.insert(0);
	char tempW[WORD_LEN];
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
void ChunkHandler::read_file ( string file_name ){
	FILE *fp = fopen(file_name.c_str(), "r");
	if ( !fp ){
		cout << "Could not open " << file_name << " chunk for reading" << endl;
		exit(5);
	}
	ui bytes_read = fread( buf_, 1, CHUNK, fp );
	buf_[bytes_read] = 0;
	fclose(fp);
}
void ChunkHandler::write_file(ui file_num ){
	cout << file_num << " : " << index_map.size() << endl ;

	char out_name[16];
	sprintf(out_name, "index%d", file_num);
	CompressedFileWriter index_writer(out_name, SMALL_CHUNK);

//	index_writer.write_uint(0);
	map<ull, vuii>::iterator it;
	for ( it = index_map.begin(); it != index_map.end(); it++ ){
		index_writer.write_ulong(it->first);

		ui len = (it->second).size();
		ui prev = (it->second)[0].first;
		index_writer.write_uint(prev);
		index_writer.write_uint((it->second)[0].second);

		for ( int i = 1 ; i < len ; i++ ){
			index_writer.write_uint((it->second)[i].first - prev);
			prev = (it->second)[i].first;
			index_writer.write_uint((it->second)[i].second);
		}
		index_writer.write_uint(0);
	}
}

void ChunkHandler::parser(string file_name, ui file_num, ull page_id){

	read_file(file_name);
	index_map.clear();
	page_id--;
	char *ptr = buf_;
	char tempW[WORD_LEN];
	struct stemmer *z = create_stemmer();

	while (1){
		while ( *ptr && ( *ptr++ != '<' || *ptr++ != 'p' || *ptr++ != 'a' || *ptr++ != 'g' || *ptr++ != 'e') );
		if ( *ptr ){
			page_id++;
			unordered_map<ull, ui> word_map;
			word_map.reserve(1300);

			while ( *ptr++ != '<' || *ptr++ != 't' || *ptr++ != 'i' || *ptr++ != 't' || *ptr++ != 'l' || *ptr++ != 'e' );
			while ( *ptr++ != '>' );

			ui wl = 0;
			ull wv = 0;
			/* Handling titles, masking there bit as 1
			 */
			while ( 1 ){
				if (*ptr >=65 && *ptr <= 90 )
					*ptr += 32;
				if ( *ptr >= 97 && *ptr <= 122 ){
					if ( wl < WORD_LEN - 1 ){
						tempW[wl] = *ptr;
						wl++;
					}
				}
				else if ( wl > 0 ){
					wl =  stem(z, tempW, wl - 1 ) + 1;
					wv = 0;
					for ( ui l = 0 ; l < wl; l++ )
						wv = wv*27 + (tempW[l] - 96 );
					if ( stop_words.find(wv) == stop_words.end()){
						word_map[wv] += ADDSHIFT;
						word_map[wv] |= TITLE;
					}
					wl = 0;
				}
				if ( *ptr == '<' )
					break;
				ptr++;
			}

			while ( *ptr++ != '<' || *ptr++ != 't' || *ptr++ != 'e' || *ptr++ != 'x' || *ptr++ != 't');

			/*Handling REDIRECT 
			 */
			if ( *ptr == '#' )	
				continue;

			/* Now handling the text part 
			 */
			if ( *ptr >= 65 && *ptr <= 90 )
				*ptr += 32;
			char lastChar = *ptr;
			bool foundCategory = false;
			while ( *ptr != '<' ){
				while ( *ptr != '<' && ( *ptr < 97 || *ptr > 122 )){
					lastChar = *ptr;
					ptr++;
					if ( *ptr >= 65 && *ptr <= 90 )
						*ptr += 32;
				}
				wl = 0;
				wv = 0;
				if ( *ptr != '<' ){
					while( *ptr != '<' && ( *ptr >= 97 && *ptr <= 122 )){
						if ( wl < WORD_LEN ){
							tempW[wl] = *ptr;
							wl++;
						}
						ptr++;
						if ( *ptr >= 65 && *ptr <= 90 )
							*ptr += 32;
					}
					tempW[wl] = 0;
					wl =  stem(z, tempW , wl - 1 ) + 1;
					for ( ui l = 0 ; l < wl; l++ )
						wv = wv*27 + (tempW[l] - 96 );
					if ( wv != CATEGORY_VALUE ){
						if ( ! foundCategory ){
							if ( stop_words.find(wv) == stop_words.end())
								word_map[wv] += ADDSHIFT;
						}
						else 
							break;
					}
					else if ( lastChar == '[' ){
						foundCategory = true;
						wl = 0;
						while ( *ptr != ']' ){
							if (*ptr >=65 && *ptr <= 90 )
								*ptr += 32;
							if ( *ptr >= 97 && *ptr <= 122 ){
								if ( wl < WORD_LEN ){
									tempW[wl] = *ptr;
									wl++;
								}
							}
							else if ( wl > 0 ){
								wl =  stem(z, tempW , wl - 1 ) + 1;
								wv = 0;
								for ( ui l = 0 ; l < wl; l++ )
									wv = wv*27 + (tempW[l] - 96 );
								if ( stop_words.find(wv) == stop_words.end()){
									word_map[wv] += ADDSHIFT;
									word_map[wv] |= CATEGORY;
								}
								wl = 0;
							}
							ptr++;
						}
					}
				}
			} 
			for(auto word_it = word_map.begin(); word_it != word_map.end(); word_it++){
				(index_map[word_it->first]).push_back( make_pair(page_id, word_it->second));
			}
		}
		else 
			break;
	}
	write_file(file_num);
}
