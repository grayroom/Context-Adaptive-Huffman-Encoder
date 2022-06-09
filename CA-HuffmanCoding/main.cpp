#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <set>
#include <map>

#define OFFSET 128
#define LEN 256

using namespace std;

class Node{
public:
    set<char> represent; // 포함할 수 있는 char종류 -> leaf에서는 단 하나의 원소
    Node* leftChild{};
    Node* rightChild{};

    Node(int uChar)
            : leftChild{nullptr}, rightChild{nullptr} {
        represent.insert((char)(uChar-OFFSET));
    };

    Node(Node* lChild, Node* rChild) {
        leftChild = lChild;
        rightChild = rChild;
        for(auto item: lChild->represent)
            represent.insert(item);
        for(auto item: rChild->represent)
            represent.insert(item);
    };

    // 짧은걸 먼저 가져오겠음
    Node operator > (Node& operand) const {
        return this->represent.size() < operand.represent.size();
    }
};

void wordAlignOutput(ofstream& ofs, string binaryStr = "") {
    static char buffer{};
    static size_t size {};

    if (binaryStr == "") {
        while(size < 8) {
            buffer = buffer << 1;
            size++;
        }
        ofs.write(&buffer, 1);
        buffer = 0;
        size = 0;
    } else {
        while(binaryStr.size()) {
            while(size < 8) {
                if (*binaryStr.begin() == '1') // 1 case
                    buffer = buffer << 1 | 1;
                else
                    buffer = buffer << 1;
                binaryStr.erase(binaryStr.begin());
                size++;

                if (binaryStr.size() == 0)
                    break;
            }

            if (size == 8) {
                ofs.write(&buffer, 1);
                buffer = 0;
                size = 0;
            }
        }
    }
}

struct compare {
    bool operator() (pair<int, Node*> lhs, pair<int, Node*> rhs) {
        if (lhs.first == rhs.first) {
            return lhs.second < rhs.second;
        } else {
            return lhs.first > rhs.first;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    ifstream ifs{};
    ofstream ofs{};
    ifs.open("./test_input1.txt", ifstream::binary);
    ofs.open("./huffman_code.hbs");
    vector<int> freqASCII(256, 0);
    vector<uint8_t> vec_buf{};
    if(ifs.good()) {
        vector<uint8_t> v_buf((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
        vec_buf = v_buf;
        ifs.close();
    }
    for (auto b: vec_buf)
        freqASCII[b + OFFSET]++;

    // key: frequency, value: unsigned ASCII(OFFSET)
    priority_queue<pair<int, Node*>, vector<pair<int ,Node*>>, compare> PQ{};
    for (int i{}; i < LEN; ++i)
        if (freqASCII[i])
            PQ.push(make_pair(freqASCII[i], new Node{i}));

    while(PQ.size() != 1) {
        // get two minimum frequency item
        Node* firstN{PQ.top().second};
        int firstF{PQ.top().first};
        PQ.pop();
        Node* secondN{PQ.top().second};
        int secondF{PQ.top().first};
        PQ.pop();

        int totFreq{firstF + secondF};
        Node* parent {new Node{firstN, secondN}};
        PQ.push(make_pair(totFreq, parent));
    }

    map<uint8_t, string> M{};
    queue<pair<Node*, string>> Q{};
    Q.push(make_pair(PQ.top().second, ""));
    while(!Q.empty()) {
        Node* cur{ Q.front().first };
        string  partialStr{ Q.front().second };
        Q.pop();

        if (cur->leftChild == nullptr&& cur->rightChild == nullptr) // leaf node case
            M.insert(make_pair((uint8_t)(*(cur->represent.begin()) + OFFSET), partialStr));

        if (cur->leftChild != nullptr) {
            string newStr{partialStr};
            newStr.append("0");
            Q.push(make_pair(cur->leftChild, newStr));
        }
        if (cur->rightChild != nullptr) {
            string newStr{partialStr};
            newStr.append("1");
            Q.push(make_pair(cur->rightChild, newStr));
        }
    }

    // Encoded data
    for (uint8_t b: vec_buf) { // 각각의 unsigned char(uint8_t)에 대해서
        string code {M.at(b - OFFSET)};
        wordAlignOutput(ofs, code);
    }
    wordAlignOutput(ofs);

    multimap<int, string> mm{};
    for (auto item: mm)
        cout << item.second << '\n';

    // table data
    ofstream tableOfs{"./Huffman_table.hbs"};
    for (auto m: M) {
        string tableStr{};

        uint8_t symbol = (uint8_t)(m.first - OFFSET);
        for (int i{}; i < 8; ++i) {
            if ((int)(symbol & (128 >> i)) > 0)
                tableStr += "1";
            else
                tableStr += "0";
        }
        uint8_t length = (uint8_t)m.second.length();
        for (int i{}; i < 8; ++i) {
            if (length & (128 >> i))
                tableStr += "1";
            else
                tableStr += "0";
        }
        tableStr += m.second;

        wordAlignOutput(tableOfs, tableStr);
    }
    wordAlignOutput(tableOfs);

    return 0;
}
