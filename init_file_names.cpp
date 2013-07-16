#include<dirent.h>
#include<unistd.h>
#include<string>
#include<vector>
#include "compressed_file_reader.h"

using namespace std;
void goto_arena(){
	chdir("arena");
}
pair< ui, vector<string> > init_file_names( char target){
	vector<string> filenames;
	DIR *d;
	struct dirent *dir;
	d = opendir("arena");
	ui number_of_files = 0;
	if ( d ){
		while ( ( dir = readdir(d)) != NULL ){
			if ( dir->d_name[0] != target )
				continue;
			string temp(dir->d_name);
			filenames.push_back( temp );
			number_of_files++;
		}
		closedir(d);
	}
	else {
		cout << "ERROR : arena folder does not exist, keep your split input files in arena folder" << endl ;
		exit(1);
	}
	return make_pair(number_of_files, filenames);
}
ull get_number_pages (){
	CompressedFileReader page_reader("page_id_stats", 1<<16);
	return page_reader.read_ulong(); 
}
