#include <iostream>
#include <string>
#include <queue>
#include <stack>
#include <unordered_map>
#include <fstream>

using namespace std;

int sizeOfOriginalFile;

struct Node
{
    char ch;
    int frequency;
    Node *left, *right;
};  

Node* getNewNode(char ch, int frequency, Node* left, Node* right)
{
    Node* node = new Node();

    node->ch = ch;
    node->frequency = frequency;
    node->left = left;
    node->right = right;

    return node;
}

struct comp
{
    bool operator()(Node *l, Node *r)
    {
        // highest priority item has lowest frequency
        return l->frequency > r->frequency;
    }
};

void decompress(string fileName);
void compress(string fileName);
string extractInformation(string fileName);
unordered_map<char, int> getFrequencies(string information);
Node *buildHuffmanTree(string information, unordered_map<char, int> frequency);
void createEncoding(Node *huffmanTreeRoot, string code, unordered_map<char, string> &codeBook);
void encodeFileContents(string fileName, unordered_map<char, int> frequency, unordered_map<char, string> codeBook);
string convertToBinary(int a);
string encode(string information, unordered_map<char, string> codeBook, int &dummyBits);
void createTreeData(Node *huffmanTreeRoot, string &treeData);
void writeToFile(string fileName, string encoding, string treeData, int dummyBits);
void extractTreeDataAndEncoding(string fileName, string &treeData, string &encoding);
Node *decodeTree(string treeData, int &count);
string createDecoding(Node *root, string encoding);
string convertToBinary(int a);
void createDecompressedFile(string fileName, string encoding);

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        cout << "ERROR: Not enough parameters provided\n";
        return 1;
    }
    else
    {
        string option = argv[1];
        string fileName = argv[2];
        if(option == "-c")
        {
            compress(fileName);
        }
        else if(option == "-d")
        {
            //checking if file to be decompressed is of the valid format
            string extension = ".kzip";
            if(fileName.size() >= extension.size() && fileName.compare(fileName.size() - extension.size(), extension.size(), extension) == 0)
                decompress(fileName);
            else 
            {
                cout << "ERROR: Can only decompress files with \".kzip\" format\n";
                return 1;
            }
        }
        else 
        {
            cout << "ERROR: Invalid option provided\n";
            return 1;
        }
    }
    cout << endl;
    return 0;
}

void compress(string fileName)
{
    unordered_map<char, string> codeBook;
    string information = extractInformation(fileName);
    unordered_map<char, int> frequency = getFrequencies(information);
    Node *huffmanTreeRoot = buildHuffmanTree(information, frequency);
    createEncoding(huffmanTreeRoot, "", codeBook);
    int dummyBits = 0;
    string encoding = encode(information, codeBook, dummyBits);
    string treeData = "";
    createTreeData(huffmanTreeRoot, treeData);
    // for(unordered_map<char, string> ::iterator it = codeBook.begin(); it != codeBook.end(); ++it)
    //     cout << it->first << ":" << it->second << endl;
    //cout << encoding.size();
    writeToFile(fileName, encoding, treeData, dummyBits);
}

void extractTreeDataAndEncoding(string fileName, string &treeData, string &encoding)
{
    ifstream file(fileName);
    file >> noskipws;
    unsigned char preprocessing;
    int lengthOfTreeData = 0;

    //Reading the number of bits (actually char's) representing the treeData stored in char and converting it to an int
    while(file >> preprocessing && preprocessing != '.') 
    {
        lengthOfTreeData = lengthOfTreeData * 10 + preprocessing - '0';
    }

    //Reading treeData till lengthOfTreeData
    int count = 0;
    
    while(count < lengthOfTreeData)
    {
        file >> preprocessing;
        treeData += preprocessing;
        ++count;
    }

    while(file >> preprocessing)
        encoding += preprocessing;
}

void display(Node *root)
{
    queue <Node *> q;
    q.push(root);
    while(!q.empty())
    {
        Node *temp = q.front();
        q.pop();
        cout << temp->ch;
        if(temp->left) q.push(temp->left);
        if(temp->right) q.push(temp->right);
    }
}

void createDecompressedFile(string fileName, string encoding)
{
    string decompressedFileName = "DECOMPRESSED_" + fileName.erase(fileName.find(".kzip"));
    ofstream file(decompressedFileName, ios :: trunc);
    file << encoding;
}

void decompress(string fileName)
{
    string treeData = "", encoding = "";
    extractTreeDataAndEncoding(fileName, treeData, encoding);
    int count = 0;
    //cout << "treeData: " << treeData << "encoding size: " << encoding.size();
    // cout << "encoding[0]: " << encoding[0];
    Node *huffmanTreeRoot = decodeTree(treeData, count);
    //display(huffmanTreeRoot);
    string decoding = createDecoding(huffmanTreeRoot, encoding);
    createDecompressedFile(fileName, decoding);
}

string createDecoding(Node *huffmanTreeRoot, string encoding)
{
    string decoding = "";
    int dummyBits = encoding[0] - '0';
    Node *temp = huffmanTreeRoot;

    int x = 0;

    for(int i = 1; i < encoding.size(); ++i)
    {
        unsigned char ch = encoding[i]; //Important! Cannot skip this, otherwise buffer will have an invalid value
                                        //since string normally consists of signed char's which only range upto 127
        string buffer = convertToBinary(ch);
        if(i == encoding.size() - 1) //check for dummy bits in the last byte
        { 
            x = dummyBits;
        }
        for(int i = 0; i < buffer.size() - x; ++i)
        {
            char ch = buffer[i];
            if(ch == '0')
                temp = temp->left;
            else if(ch == '1')
                temp = temp->right;
            if(!temp->left && !temp->right)
            {
                decoding += temp->ch;
                temp = huffmanTreeRoot;
            }
        }
    }

    return decoding;
}

Node *decodeTree(string treeData, int &count)
{    
    if(count < treeData.size())
    {
        if(treeData[count] == '1')
        {
            count++;
            //cout << treeData[count];
            return getNewNode(treeData[count++], 0, nullptr, nullptr);
        }
        else 
        {
            count++;
            Node *left = decodeTree(treeData, count);
            Node *right = decodeTree(treeData, count);
            return getNewNode('0', 0, left, right);
        }
    } 

    else return nullptr;
}

void writeToFile(string fileName, string encoding, string treeData, int dummyBits)
{   
    string newFile = fileName + ".kzip";
    //Clear the contents of the compressed file, if it exists
    ofstream file(newFile, ios :: trunc | ios :: binary);
    file << treeData.size() << ".";
    file << treeData;
    file << char('0' + dummyBits);
    file << encoding;
    file.close();
}

void createTreeData(Node *huffmanTreeRoot, string &treeData)
{

    // for(unordered_map<char, string> :: iterator it = codeBook.begin(); it != codeBook.end(); ++it)
    // {
    //     treeData += it->first + it->second;
    // }

    //-------TO DO: STORING ONLY THE LENGTHS OF CODES-------------------


    //---------STORING THE TREE AS INTERNAL AND LEAF NODES--------------

    if(!huffmanTreeRoot->left && !huffmanTreeRoot->right) // leaf node
    {
        treeData += "1";
        treeData += huffmanTreeRoot->ch;
    }
    else
    {
        treeData += "0";
        createTreeData(huffmanTreeRoot->left, treeData);
        createTreeData(huffmanTreeRoot->right, treeData);
    }

}

string encode(string information, unordered_map<char, string> codeBook, int &dummyBits)
{
    string encoding = "";
    unsigned char buffer = 0;
    int count = 0;
    for(int i = 0; i < information.size(); ++i)
    {
        char ch = information[i];
        for(int i = 0; i < codeBook[ch].length(); ++i)
        {
            buffer <<= 1;
            if(codeBook[ch][i] == '1') buffer |= 1;
            ++count;
            if(count == 8)
            {
                encoding += buffer;
                buffer = 0;
                count = 0;
            }
        }
    }

    if(count != 0)
    {
        while(count < 8)
        {
            buffer <<= 1;
            ++dummyBits;
            count++;
        }
        encoding += buffer;
    }

    return encoding;
}

string extractInformation(string fileName)
{
    string information = "";
    unsigned char ch;
    ifstream file(fileName);

    file >> noskipws;
    while(file >> ch)    
    {
        information += ch;
    }
    
    return information;
}

unordered_map<char, int> getFrequencies(string information)
{
    unordered_map<char, int> frequency;

    for(int i = 0; i < information.length(); ++i) 
        frequency[information[i]]++;

    return frequency;
}

Node *buildHuffmanTree(string information, unordered_map<char, int> frequency)
{
    priority_queue<Node*, vector<Node*>, comp> pq;
    for(unordered_map<char, int> :: iterator it = frequency.begin(); it != frequency.end(); ++it) 
        pq.push(getNewNode(it->first, it->second, nullptr, nullptr));

    while (pq.size() != 1)
    {
        Node *left = pq.top(); 
        pq.pop();
        Node *right = pq.top();    
        pq.pop();

        int sum = left->frequency + right->frequency;
        pq.push(getNewNode('\0', sum, left, right));
    }

    return pq.top();
}

void createEncoding(Node *root, string str, unordered_map<char, string> &codeBook)
{
    if (root == nullptr)
        return;

    if (!root->left && !root->right) 
        codeBook[root->ch] = str;

    createEncoding(root->left, str + "0", codeBook);
    createEncoding(root->right, str + "1", codeBook);
}
/*
void createDecompressedFile(string decompressedFileName, Node *root, string encoding)
{
    cout << "\nDecoding:\n";

    string decoding = "";
    string buffer;
    Node *temp = root;

    int count = 0;
    int size = 12;

    for(int j = 0; j < encoding.size(); ++j)
    {
        buffer = convertToBinary(encoding[j]);
        cout << encoding[j] << endl;
        for(int i = 0; i < buffer.size(); ++i)
        {
            //if(count == size) break;
            char ch = buffer[i];
            //cout << ch;
            if(ch == '0')
                temp = temp->left;
            else if(ch == '1')
                temp = temp->right;
            if(!temp->left && !temp->right)
            {
                decoding += temp->ch;
                temp = root;
                count++;
            }
        }
    }

    ofstream file(decompressedFileName);
    file << decoding;
}*/

string convertToBinary(int a)
{
    string res = "";
    stack<int> s;
    while(a != 0)
    {
        s.push(a % 2);
        a = a / 2;
    }
    int count = s.size();
    while(count < 8) {count++; res += '0';}
    while(!s.empty())
    {
        res += to_string(s.top());
        s.pop();
    }
    return res;
}

int sizeOfFile(string fileName)
{
    int count = 0;
    int sizeOfChar = sizeof(char);
    ifstream file(fileName);
    char ch;
    file >> noskipws; //Spaces are not skipped
    while(file >> ch) count++;
    return count * sizeOfChar; 
}
    
/*

    // cout << "Huffman Codes are :\n" << '\n';
    // for (auto pair: huffmanCode) {
    //     cout << pair.first << " " << pair.second << '\n';
    // }

    decompress("Compressed.txt", root);

    // cout << "\nOriginal string was :\n" << text << '\n';

    // // print encoded string
    // string str = "";
    // for (char ch: text) {
    //     str += huffmanCode[ch];
    // }

    // cout << "\nEncoded string is :\n" << str << '\n';

    // // traverse the Huffman Tree again and this time
    // // decode the encoded string
    // int index = -1;
    // cout << "\nDecoded string is: \n";
    // while (index < (int)str.size() - 2) {
    //     decode(root, index, str);
    // }



// traverse the Huffman Tree and decode the encoded string
void decode(Node* root, int &index, string str)
{
    if (root == nullptr) {
        return;
    }

    // found a leaf node
    if (!root->left && !root->right)
    {
        cout << root->ch;
        return;
    }

    index++;

    if (str[index] =='0')
        decode(root->left, index, str);
    else
        decode(root->right, index, str);
}







int sizeOfFile(string fileName)
{
    int count = 0;
    int sizeOfChar = sizeof(char);
    ifstream file(fileName);
    char ch;
    file >> noskipws; //Spaces are not skipped
    while(file >> ch) count++;
    return count * sizeOfChar; 
}



// Builds Huffman Tree and decode given input text

*/

