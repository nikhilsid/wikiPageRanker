#include "compressed_file_writer.h"
#include "compressed_file_reader.h"
#include<iostream>

using namespace std;

int write (){
	CompressedFileWriter writer("out", 1<<20);
	int a = 10;
	ui b = 11;
	ll c = 12;
	ull d = 13;
	double e = 3.14;
	us f = 15;
	uc g = 56;
	writer.write_int(-1);
	writer.write_int(0);
	writer.write_uint(b);
	writer.write_long(-10000);
	writer.write_ulong(d);
	writer.write_double(e);
	writer.write_ushort(f);
	writer.write_uchar(g);
	writer.write_string("ashutosh");
	writer.write_uncompressed_string("ashutosh kumar");
	return 0;
}
int read (){
	CompressedFileReader reader("out", 1<<20);

	cout << reader.read_int() << endl;
	cout << reader.read_int() << endl;
	cout << reader.read_uint() << endl;
	cout << reader.read_long() << endl;
	cout << reader.read_ulong() << endl;
	cout << reader.read_double() << endl;
	cout << reader.read_ushort() << endl;
	cout << reader.read_uchar() << endl;
	cout << reader.read_string() << endl;
	cout << reader.read_uncompressed_string() << endl;
	return 0;
}
int main (){
	write();
	read();
	return 0;
}
