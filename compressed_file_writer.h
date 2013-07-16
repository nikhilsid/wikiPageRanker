#ifndef WRITE_FILE_H
#define WRITE_FILE_H

#include<string>
using namespace std;

#define ull unsigned long long int
#define ll long long int
#define us unsigned short
#define uc unsigned char 
#define ui unsigned int

class CompressedFileWriter {
	public : 
		CompressedFileWriter(const char *, ull);
		~CompressedFileWriter();
		CompressedFileWriter(ull);
		void set_file(const char*);
		void write_string (string);
		void write_uncompressed_string(string);
		void write_uncompressed_string(const char *);
		void write_string (const char *);
		void write_int (int);
		void write_uint (ui);
		void write_long (ll);
		void write_ulong (ull);
		void write_ushort (us);
		void write_uchar (uc);
		void write_double(double);
		ull get_byte_offset();
	private : 
		FILE *fp_;
		uc *buf_;
		ull buf_max_, buf_pos_;
		void write_to_file();
		ui double_size_;
		ull chunks_done_;
};
#endif
