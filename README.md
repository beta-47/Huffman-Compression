# Text-Compressor
Lossless data compression and decompression utility for ASCII text files, written in C++. Employs Huffman encoding to achieve about 50% compression on text files.


Running the compression tool on an ASCII text file creates a '.kzip' file containing five fields:
1. The first field stores the length of the Huffman frequency tree for the ASCII characters of the file
2. The second field '.' is used to separate the first and third fields
3. The third field contains the Huffman frequency tree
4. The fourth field contains a single byte indicating the number of dummy bits appended to the original text
5. The fifth field contains the enocded text
