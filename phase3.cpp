#include<iostream>
#include<cstdio>
#include<algorithm>
#include<map>
#include<vector>
#include "compressed_file_reader.h"
#include "compressed_file_writer.h"
#include "init_file_names.cpp"

using namespace std;
#define DAMP 0.85
#define STOP 0.000000000001
#define CHUNK (1<<20)
#define vui vector<ui>

vector< vui > backlinks;
vector<double> pageranks;
vui outlinks;

ull total_no_pages;

void init_backlinks()
{
	vui temp;
	backlinks.push_back(temp);
	CompressedFileReader backlinks_reader("backlinks", CHUNK);
	ui a,prev;
	for ( ui i = 1; i < total_no_pages; i++ ){
		prev = 0;
		backlinks.push_back(temp);
		while ( (a = backlinks_reader.read_uint()) ){
			backlinks[i].push_back(a + prev);
			prev = a + prev;
		}
	}
	printf("Total pages in corpus : %llu\n\n", total_no_pages );
}
void init_outlinks(){
	outlinks.push_back(0);
	CompressedFileReader outlinks_reader("outlinks", CHUNK);
	for ( ui i = 1; i < total_no_pages; i++ ){
		outlinks.push_back( outlinks_reader.read_uint());
	}
}
void init_pageranks(){
	double default_value = 1.0/total_no_pages;
	for (ui i=0; i < total_no_pages; i++)
		pageranks.push_back(default_value);

}

void init(){
	goto_arena();
	total_no_pages = get_number_pages();
	init_backlinks();
	init_outlinks();
	init_pageranks();
}

double iterate(bool includeDangling){
	double innerSum;
	double oldPageRank;
	double threshold;

	threshold = 0;
	for (ui i=1; i<total_no_pages; i++){

		innerSum = 0;
		if (!backlinks[i].empty()){
			if ((!includeDangling and outlinks[i])  or  (includeDangling and outlinks[i]==0)){
				for (ui j=0; j<backlinks[i].size(); j++){
					if ( outlinks[backlinks[i][j]] > 0 )
						innerSum += pageranks[ backlinks[i][j] ] / outlinks[ backlinks[i][j] ];
					if ( innerSum != innerSum ){
						cout << i << " L " << innerSum << " : " << pageranks[i] << " : " << oldPageRank << endl;
						cout << j << " L " << backlinks[i][j] << " : " << outlinks[backlinks[i][j]] << endl ;
						exit(1);
					}
				}
			}
		}

		if ((!includeDangling and outlinks[i])  or ( includeDangling and outlinks[i]==0)){

			oldPageRank = pageranks[i];
			pageranks[i] = DAMP * innerSum  + (1.0 - DAMP) / total_no_pages;
			threshold += (pageranks[i] - oldPageRank)*(pageranks[i] - oldPageRank);
		}
	}
	return threshold;
}
void write_pageranks (){
	CompressedFileWriter pagerank_writer("pageranks", CHUNK );
	for ( ui i = 1 ; i < total_no_pages ; i++ )
		pagerank_writer.write_double(pageranks[i]);
}
void calculate_pagerank(){
	double threshold;

	printf("Calculating PageRank excluding dangling links\n");
	int loop_count = 0;
	do{
		threshold = iterate(false);
		printf("Iteration : %2d \t -> Threshold : %.13f\n", loop_count,threshold);
		loop_count++;
	}while (threshold > STOP);

	printf("Including Dangling links\n");
	threshold = iterate(true);
	printf("Finished!");
}

int main(){
	init();
	calculate_pagerank();
	write_pageranks();
	
	return 0;
}
