#ifndef READ_FILE_H
#define READ_FILE_H

#include<string>
using namespace std;

#define ull unsigned long long int
#define ll long long int
#define us unsigned short
#define uc unsigned char 
#define ui unsigned int

class CompressedFileReader {
	public : 
		CompressedFileReader(const char *, ull);
		CompressedFileReader();
		~CompressedFileReader();
		bool eof();
		void set_buf_max(ull);
		void set_file(const char*);
		void set_offset(ull);
		int read_int();
		ui read_uint();
		ll read_long();
		ull read_ulong();
		us read_ushort();
		uc read_uchar();
		uc current_uchar();
		double read_double();
		string read_string();
		string read_uncompressed_string();
		string read_string_from_ulong();
		char file_name_[128];
	private : 
		FILE *fp_;
		uc *buf_;
		ull buf_len_, buf_pos_, buf_max_;
		void read_from_file();
		ui double_size_;
};
#endif
