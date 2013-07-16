g++-4.7 -O3 -std=c++11 -o phase1n2 phase1n2.cpp compressed_file_writer.cpp compressed_file_reader.cpp
echo "phase1n2 generated"
g++ -o phase3 phase3.cpp compressed_file_reader.cpp compressed_file_writer.cpp -O3
echo "phase3 generated"
g++-4.7 -std=c++11 chunk_handler.cpp phase4a.cpp compressed_file_reader.cpp compressed_file_writer.cpp -w -o phase4a -O3
echo "phase4a generated"
g++ -O3 -o phase4b phase4b.cpp compressed_file_reader.cpp compressed_file_writer.cpp
echo "phase4b generated"
g++-4.7 -std=c++11 -o phase5 phase5.cpp compressed_file_reader.cpp -w -O3
echo "phase5 generated"
