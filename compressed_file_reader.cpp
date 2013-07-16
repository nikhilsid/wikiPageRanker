#include "compressed_file_reader.h"
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<cstring>

using namespace std;

CompressedFileReader::CompressedFileReader(const char *file_name, ull buf_max ){
	buf_len_ = 0;
	buf_max_ = buf_max;
	strcpy(file_name_, file_name);
	double_size_ = sizeof(double);
	if ( double_size_ > buf_max ){
		cout << "Too small buffer size, allot more than " << double_size_ << " bytes" << endl;
		exit(3);
	}

	buf_ = (uc *) malloc ( (buf_max_ )* sizeof(uc));
	if ( !buf_){
		cout << "Could Not allocate reading buffer for file : " << file_name << " Exiting!" << endl;
		exit(2);
	}

	fp_ = fopen(file_name, "r");
	if ( !fp_ ){
		cout << "Could not open the file : " << file_name << " for reading! Exiting!" << endl;
		exit(1);
	}
}
CompressedFileReader::CompressedFileReader(){
}
CompressedFileReader::~CompressedFileReader(){
	if ( buf_max_ > 0 )
		free(buf_);
	fclose(fp_);
}
void CompressedFileReader::set_buf_max(ull buf_max){
	buf_len_ = 0;
	buf_max_ = buf_max;
	double_size_ = sizeof(double);
	if ( double_size_ > buf_max ){
		cout << "Too small buffer size, allot more than " << double_size_ << " bytes" << endl;
		exit(3);
	}

	buf_ = (uc *) malloc ( (buf_max_ )* sizeof(uc));
	if ( !buf_){
		cout << "Could Not allocate reading buffer for file  Exiting!" << endl;
		exit(2);
	}
}
void CompressedFileReader::set_file(const char* file_name){
	fp_ = fopen(file_name, "r");
	strcpy(file_name_, file_name);
	if ( !fp_ ){
		cout << "Could not open the file : " << file_name << " for reading! Exiting!" << endl;
		exit(1);
	}
}
void CompressedFileReader::set_offset(ull offset){
	fseek(fp_, offset, SEEK_SET);
	buf_pos_ = buf_len_ = 0;
}
void CompressedFileReader::read_from_file() {
	buf_pos_ = 0;
	buf_len_ = fread( buf_, 1, buf_max_, fp_);
	if ( buf_len_ == 0 && feof(fp_) ){
		cout << "Reader trying to read beyond EOF. Exiting!" << endl;
		exit(4);
	}
}
bool CompressedFileReader::eof(){
	if ( buf_pos_ >= buf_len_ ){
		buf_pos_ = 0;
		buf_len_ = fread( buf_, 1, buf_max_, fp_);
		if ( buf_len_ == 0 )
			return true;
	}
	return false;
}
uc CompressedFileReader::current_uchar (){
	if ( buf_pos_ < buf_len_ )
		return buf_[buf_pos_];
	read_from_file();
	return buf_[buf_pos_];
}
uc CompressedFileReader::read_uchar(){
	if ( buf_pos_ < buf_len_ )
		return buf_[buf_pos_++];
	return current_uchar();
}
us CompressedFileReader::read_ushort(){
	us ans = 0;
	ans = read_uchar();
	ans <<= 8;
	ans |= read_uchar();
	return ans;
}
double CompressedFileReader::read_double(){
	double a;
	if ( buf_pos_ + double_size_ <= buf_len_ ){
		memcpy(&a, &buf_[buf_pos_], double_size_);
		buf_pos_ += double_size_;
		return a;
	}
	read_from_file();
	return read_double();
}
string CompressedFileReader::read_string(){
	/* Three characters in 2 bytes
	 * and last bit is multibyte indicator.
	 */
	us val;
	int c = 0, cont = 0;
	char tempW[128];
	do{
		val = read_uchar();
		val <<= 8;
		val |= read_uchar();
		cont = val&1;
		val >>= 1;
		tempW[c] = tempW[c+1] = tempW[c+2] = 0;
		if ( val&31)
			tempW[c+2] = (val&31) + 96;
		val >>= 5;
		if ( val&31)
			tempW[c+1] = (val&31) + 96;
		val >>= 5;
		tempW[c] = val + 96;
		c += 3;
	} while ( cont ); 
	string word(tempW);
	return word;
}
ui CompressedFileReader::read_uint (){
	/* Normal multibyte encoding
	 * First 7 bits -> Value ( least significant 7 bits)
	 * 8th bit 	-> Next byte relevant ?
	 * if relevant : repeat 
	 */
	ui val = 0, temp, inc = 0;
	do {
		temp = current_uchar()>>1;
		temp <<= inc;
		val |= temp;
		inc += 7;
	}while ( read_uchar()&1 );
	return val;
}
ull CompressedFileReader::read_ulong(){
	/* Exactly same as read_uint()
	 */
	ull val = 0, temp;
	int inc = 0;
	do {
		temp = current_uchar()>>1;
		temp <<= inc;
		val |= temp;
		inc += 7;
	}while ( read_uchar()&1 );
	return val;
}
int CompressedFileReader::read_int (){
	/* Tricky : 
	 * First bit	 -> sign
	 * Next 6 bits	 -> value
	 * 8th bit   	 -> multiByte indicator
	 * Next normal multibyte as above
	 */
	bool first = true;
	bool negative = false;
	int val = 0, temp, inc = 0;
	do {
		if ( first ){
			temp = current_uchar();
			if ( temp & 128 ){
				negative = true;
				temp = temp - 128;
			}
			temp >>= 1;
			val = temp;
			inc += 6;
			first = false;
		}
		else{
			temp = current_uchar()>>1;
			temp <<= inc;
			val |= temp;
			inc += 7;
		}
	}while ( read_uchar()&1 );
	return (negative) ? -val : val ;
}
ll CompressedFileReader::read_long(){
	/* Similar to above 
	 */
	bool first = true;
	bool negative = false;
	ll val = 0, temp, inc = 0;
	do {
		if ( first ){
			temp = current_uchar();
			if ( temp & 128 ){
				negative = true;
				temp = temp - 128;
			}
			temp >>= 1;
			val = temp;
			inc += 6;
			first = false;
		}
		else{
			temp = current_uchar()>>1;
			temp <<= inc;
			val |= temp;
			inc += 7;
		}
	}while ( read_uchar()&1 );
	
	return (negative) ? -val : val ;
}
string CompressedFileReader::read_string_from_ulong (){
	ull a = read_ulong();
	int k = 15;
	char tempW[16];
	tempW[k--] = 0;
	while ( a ){
		tempW[k--] = 96 + (a%27);
		a /= 27;
	}
	string word(&tempW[k+1]);
	return word;
}
string CompressedFileReader::read_uncompressed_string(){
	string ans("");
	uc temp_char;
	while ( (temp_char = read_uchar()) ){
		ans += temp_char;
	}
	return ans;
}
