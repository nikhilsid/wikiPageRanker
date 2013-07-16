#include "compressed_file_writer.h"
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<iostream>
using namespace std;

CompressedFileWriter::CompressedFileWriter( const char *file_name, ull buf_max ){
	buf_max_ = buf_max;
	buf_pos_ = 0;

	double_size_ = sizeof(double);
	if ( buf_max < double_size_ ){
		cout << "Insufficient Buffer Length, at least " << double_size_ << " bytes" << endl;
		exit(3);
	}

	buf_ = (uc *) malloc ( buf_max_ * sizeof(uc));
	if ( !buf_ ){
		cout << "Could not allocate writer buffer for file : " << file_name << " Exiting!" << endl;
		exit(2);
	}

	fp_ = fopen( file_name, "w");
	if ( !fp_ ){
		cout << "Could Not Open File : " << file_name << " Exiting!" << endl;
		exit(1);
	}
}
CompressedFileWriter::CompressedFileWriter(ull buf_max){
	buf_max_ = buf_max;
	buf_pos_ = 0;
	double_size_ = sizeof(double);
	if ( buf_max < double_size_ ){
		cout << "Insufficient Buffer Length, at least " << double_size_ << " bytes" << endl;
		exit(3);
	}

	buf_ = (uc *) malloc ( buf_max_ * sizeof(uc));
	if ( !buf_ ){
		cout << "Could not allocate writer buffer! Exiting!" << endl;
		exit(2);
	}
}
void CompressedFileWriter::set_file(const char *file_name){
	buf_pos_ = 0;
	fp_ = fopen( file_name, "w");
	if ( !fp_ ){
		cout << "Could Not Open File : " << file_name << " Exiting!" << endl;
		exit(1);
	}
}
CompressedFileWriter::~CompressedFileWriter(){
	write_to_file();
	free ( buf_ );
	fclose(fp_);
}

void CompressedFileWriter::write_to_file () {
	fwrite(buf_, 1, buf_pos_, fp_);
	buf_pos_ = 0;
	chunks_done_++;
}
ull CompressedFileWriter::get_byte_offset(){
	ull byte_offset = chunks_done_;
	byte_offset *= buf_max_;
	byte_offset += buf_pos_;
	return byte_offset;
}
void CompressedFileWriter::write_uchar (uc a){
	if ( buf_pos_ < buf_max_ )
		buf_[buf_pos_++] = a;
	else {
		write_to_file();
		buf_[buf_pos_++] = a;
	}
}

void CompressedFileWriter::write_double(double a){
	if ( buf_pos_ + double_size_ < buf_max_ ){
		memcpy(&buf_[buf_pos_], &a, double_size_ ); 
		buf_pos_ += double_size_;
	}
	else {
		write_to_file();
		write_double(a);
	}
}

void CompressedFileWriter::write_ushort (us a){
	write_uchar(a>>8);
	write_uchar(a&255);
}

void CompressedFileWriter::write_uint (ui a){
	/* writing integer as multiple bytes
	 * use the last bit of a byte 
	 * as multi byte indicator 
	 */
	if ( a == 0 )
		write_uchar(0);
	while(a){
		uc b = (a & 127)<<1;
		a >>= 7;
		if ( a ) 
			b |= 1;
		write_uchar(b);
	}
}

void CompressedFileWriter::write_ulong (ull a){
	/* Same as unsigned int 
	 * MultiByte encoding 
	 */
	if ( a == 0 )
		write_uchar(0);
	while(a){
		uc b = (a & 127)<<1;
		a >>= 7;
		if ( a ) 
			b |= 1;
		write_uchar(b);
	}
}

void CompressedFileWriter::write_int (int a){
	/* Here Sign of int becomes relevant 
	 * So we set the first bit of the compressed
	 * data as sign, 1 for negative, else 0
	 * rest similar to write_uint();
	 */
	bool first = true;
	uc temp = 0;
	if ( a < 0 ){
		temp = (uc)(1<<7);		// Set sign bit
		a = -a;				// find absolute value
	}
	if ( a == 0 )
		write_uchar(0);
	while(a){
		if ( first ){
			uc b = (a & 63)<<1;
			b |= temp;		// Merge Sign bit
			a >>= 6;
			if ( a ) 
				b |= 1;
			write_uchar(b);
			first = false;
		}
		else {
			uc b = (a & 127)<<1;
			a >>= 7;
			if ( a ) 
				b |= 1;
			write_uchar(b);
		}
	}
}

void CompressedFileWriter::write_long (ll a){
	/* Same as above write_int(int a); 
	 */
	bool first = true;
	uc temp = 0;
	if ( a < 0 ){
		temp = (uc)(1<<7);		// Set sign bit
		a = -a;				// find absolute value
	}
	if ( a == 0 )
		write_uchar(0);
	while(a){
		if ( first ){
			uc b = (a & 63)<<1;
			b |= temp;		// Merge Sign bit
			a >>= 6;
			if ( a ) 
				b |= 1;
			write_uchar(b);
			first = false;
		}
		else {
			uc b = (a & 127)<<1;
			a >>= 7;
			if ( a ) 
				b |= 1;
			write_uchar(b);
		}
	}
}

void CompressedFileWriter::write_string (const char *a){
	/* writing string as collection of shorts 
	 * 3 characters in 16 bits : last bit 
	 * used a multi byte indicator 
	 */
	int len = strlen(a);
	for ( int i = 0; i <= len/3; i++ ){
		us val = a[3*i] - 96;
		val <<= 5;
		if ( 3*i + 1 < len )
			val |= a[3*i+1] - 96;
		val <<= 5;
		if ( 3*i + 2 < len )
			val |= a[3*i+2] - 96;
		val <<= 1;
		if ( 3 * i + 2 < len ) 
			val |= 1;
		write_ushort(val);
	}
}

void CompressedFileWriter::write_string (string a){
	write_string(a.c_str());
}

void CompressedFileWriter::write_uncompressed_string(const char *a){
	while (*a != 0 ){
		write_uchar(*a);
		a++;
	}
	write_uchar(0);
}

void CompressedFileWriter::write_uncompressed_string(string a){
	write_uncompressed_string(a.c_str());
}
