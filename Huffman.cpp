#include <iostream>
#include <string>
#include <queue>
#include <stack>
#include <unordered_map>
#include <fstream>
#include <sstream>

using namespace std;

struct Node
{
    char ch;
    int frequency;
    Node *left, *right;
};  

Node *getNewNode(char ch, int frequency, Node* left, Node* right)
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

//Compression Functions
void compress(string);
string extractInformation(string);
unordered_map<char, int> getFrequencies(string);
Node *buildHuffmanTree(string, unordered_map<char, int>);
void createEncoding(Node *, string, unordered_map<char, string> &);
string encode(string, unordered_map<char, string>, int &);
void createTreeData(Node *, string &);
void writeToFile(string, string, string, int);

//Decompression Functions
void decompress(string);
void extractTreeDataAndEncoding(string, string &, string &);
Node *decodeTree(string, int &);
string createDecoding(Node *, string);
void createDecompressedFile(string, string);

//Helper Function
string convertToBinary(int a);

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
            cout << "Compression Successful";
        }
        else if(option == "-d")
        {
            //checking if file to be decompressed is of the valid format
            string extension = ".kzip";
            if(fileName.size() >= extension.size() && fileName.compare(fileName.size() - extension.size(), extension.size(), extension) == 0)
            {
                decompress(fileName);
                cout << "Decompression Successful";
            }
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
    writeToFile(fileName, encoding, treeData, dummyBits);
}

string extractInformation(string fileName)
{
    ifstream t(fileName);
    stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
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

void createTreeData(Node *huffmanTreeRoot, string &treeData)
{
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

void decompress(string fileName)
{
    string treeData = "", encoding = "";
    extractTreeDataAndEncoding(fileName, treeData, encoding);
    int count = 0;
    Node *huffmanTreeRoot = decodeTree(treeData, count);
    string decoding = createDecoding(huffmanTreeRoot, encoding);
    createDecompressedFile(fileName, decoding);
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

Node *decodeTree(string treeData, int &count)
{    
    if(count < treeData.size())
    {
        if(treeData[count] == '1')
        {
            count++;
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

void createDecompressedFile(string fileName, string encoding)
{
    string decompressedFileName = "DECOMPRESSED_" + fileName.erase(fileName.find(".kzip"));
    ofstream file(decompressedFileName, ios :: trunc);
    file << encoding;
}

string convertToBinary(int a)
{
    string res = "";
    while(a)
    {
    	if(a & 1) 
    		res.push_back('1');
    	else
    		res.push_back('0');
    	a >>= 1;
    }
    while(res.length() < 8)
    	res.push_back('0');
    reverse(res.begin(), res.end());
    return res;
}
