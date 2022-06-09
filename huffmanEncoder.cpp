#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <set>
#include <map>
#include <chrono>


using namespace std;

class Node {
public:
    set<string> represent;
    Node* leftChild{};
    Node* rightChild{};

    explicit Node(const string& uChar)
            : leftChild{ nullptr }, rightChild{ nullptr } {
        represent.insert(uChar);
    };

    Node(Node* lChild, Node* rChild) {
        leftChild = lChild;
        rightChild = rChild;
        for (const string& item : lChild->represent)
            represent.insert(item);
        for (const string& item : rChild->represent)
            represent.insert(item);
    };

    bool operator > (Node& operand) const {
        return this->represent.size() > operand.represent.size();
    }
};

struct compare { // first: freq, second: symbol
    bool operator() (pair<int, Node*> lhs, pair<int, Node*> rhs) {
        if (lhs.first == rhs.first) // string 길이에 대해서 내림차순
            return lhs.second < rhs.second;
        else // 빈도수에 대해서 오름차순
            return lhs.first > rhs.first;
    }
};

class Table {
public:
    uint8_t precedingLength{};
    map<string, string> table{};

    explicit Table(uint8_t precedingLength)
            :precedingLength{ precedingLength } {};

    explicit Table(map<string, int>& freqs, const string& precedingSymbol = "") {
        precedingLength = (uint8_t)precedingSymbol.size();

        // FIXME: 127 -> 128? 뭐가 됐든 전처리 매크로로 변경하자
        // NOTE: 모든 등장횟수를 1씩 증가시켜서 존재하지 않는 symbol에 대한 huffman또한 만든다
        for (char i{-1}; i < 127; ++i) {
            string symbol{ i };
            if (freqs.find(symbol) == freqs.end())
                freqs.insert(make_pair(symbol, 1));
            else
                freqs.find(symbol)->second *= 10;
        }

        priority_queue<pair<int, Node*>, vector<pair<int, Node*>>, compare> PQ{};
        for (pair<string, int> item : freqs)
            PQ.push(make_pair(item.second, new Node(item.first)));

        // Tree merge를 수행하여 Huffman tree를 생성
        while (PQ.size() != 1) {
            Node* firstN{ PQ.top().second };
            int firstF{ PQ.top().first };
            PQ.pop();
            Node* secondN{ PQ.top().second };
            int secondF{ PQ.top().first };
            PQ.pop();

            int totFreq{ firstF + secondF };
            Node* parent{ new Node{firstN, secondN} };
            PQ.push(make_pair(totFreq, parent));
        }

        // Huffman table encoding
        // NOTE: BFS를 수행하여 각 node들에 symbol을 부여하고 leaf node의 symbol을 table에 추가한다
        queue<pair<Node*, string>> Q{};
        Q.push(make_pair(PQ.top().second, ""));
        while (!Q.empty()) {
            Node* cur{ Q.front().first };
            string  partialStr{ Q.front().second };
            Q.pop();

            if (cur->leftChild == nullptr && cur->rightChild == nullptr) { // leaf node case
                string symbol = precedingSymbol + *(cur->represent.begin()); //NOTE: leaf node는 size가 1임에 주목
                table.insert(make_pair(symbol, partialStr));
            }

            if (cur->leftChild != nullptr)
                Q.push(make_pair(cur->leftChild, partialStr + "0"));
            if (cur->rightChild != nullptr)
                Q.push(make_pair(cur->rightChild, partialStr + "1"));
        }
    }
};

void wordAlignOutput(ofstream& ofs, string binaryStr = "") {
    static uint8_t buffer{};
    static size_t size{};

    if (binaryStr.empty()) {
        if (size == 0)
            return;

        while (size < 8) {
            buffer = buffer << 1;
            size++;
        }
        char outBuf{ (char)buffer };
        ofs.write(&outBuf, 1);
        buffer = 0;
        size = 0;
    }
    else {
        while (!binaryStr.empty()) {
            while (size < 8) {
                if (*binaryStr.begin() == '1') // 1 case
                    buffer = buffer << 1 | 1;
                else
                    buffer = buffer << 1;
                binaryStr.erase(binaryStr.begin());
                size++;

                if (binaryStr.empty())
                    break;
            }

            if (size == 8) {
                char outBuf{ (char)buffer };
                ofs.write(&outBuf, 1);
                buffer = 0;
                size = 0;
            }
        }
    }
}

vector<int> getPi(string p) {
    int m{ (int)p.size() }, j{};
    vector<int> pi(m, 0);

    for (int i{ 1 }; i < m; i++) {
        while (j > 0 && p[i] != p[j])
            j = pi[j - 1];
        if (p[i] == p[j])
            pi[i] = ++j;
    }
    return pi;
}

vector<int> kmp(string s, string p) {
    int N{ (int)s.size() }, M{ (int)p.size() }, j{};
    vector<int> ans{}, pi{ getPi(p) };

    for (int i{}; i < N; i++) {
        while (j > 0 && s[i] != p[j])
            j = pi[j - 1];
        if (s[i] == p[j]) {
            if (j == M - 1) {
                ans.push_back(i - M + 1);
                j = pi[j];
            }
            else
                j++;
        }
    }
    return ans;
}

void preOut(ofstream& ofs, vector<Table>& tableSet, string preSymbols, int preLength) {
    if (preLength > 0)
        preOut(ofs, tableSet, preSymbols.substr(0, preSymbols.size() - 1), preLength - 1);

    string code{};
    for (int i{ preLength }; i >= 0; --i) {
        if (tableSet[i].table.find(preSymbols) != tableSet[i].table.end()) {
            code = tableSet[i].table.at(preSymbols);
            break;
        }
        else
            preSymbols.erase(preSymbols.begin());
    }
    wordAlignOutput(ofs, code);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    //DEBUG
    chrono::system_clock::time_point StartTime = std::chrono::system_clock::now();
    cout << "file reading...";
    //DEBUG
    //NOTE: 파일을 읽어온다
    ifstream ifs{};
    ifs.open("./training_input.txt", ifstream::binary);
    //DEBUG
    if (ifs.is_open())
        cout << "open!";
    else
        cout << "fail to open";
    //DEBUG
    string vec_buf{};
    time_t  begin{};
    if (ifs.good()) {
        string v_buf((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
        vec_buf = v_buf;
        ifs.close();
        cout << "done!" << "\n\n";//DEBUG
    }
    vec_buf += '\0';

    map<string, int> freqsASCII{};
    vector<string> vec_ascii{};
    for (char c{-1}; c < 127; ++c) {
        string pattern{ c };
        vector<int> posits{ kmp(vec_buf, pattern) };
        if (!posits.empty()) {
            string symbol{ c };
            freqsASCII.insert(make_pair(symbol, (int)posits.size()));
            if (find(vec_ascii.begin(), vec_ascii.end(), symbol) == vec_ascii.end()) {
                vec_ascii.emplace_back(symbol);
            }
        }
    }

    vector<Table> tableSet{};

    //NOTE: Normal Huffman Coding
    map<string, int> freqsOrigin{};
    for (const string& symbol : vec_ascii)
        freqsOrigin.insert(make_pair(symbol, freqsASCII[symbol]));
    tableSet.emplace_back(freqsOrigin);

    //NOTE: Context Adaptive Huffman Coding
    for (int precedingLevel{ 1 }; precedingLevel < 8; ++precedingLevel) {
        map<string, string> precedingTable = tableSet[precedingLevel - 1].table;

        tableSet.emplace_back(precedingLevel);
        //DEBUG
        chrono::system_clock::time_point levelStart = std::chrono::system_clock::now();
        cout << precedingLevel << " level in process...\nprogress: ";
        //DEBUG
        int loopCount{};
        int before{};
        int loss{}, win{};
        int totalReduce{};
        for (pair<string, string> preceding : precedingTable) {
            // DEBUG
            loopCount++;
            if ((loopCount * 100 / precedingTable.size()) > before) {
                cout << '@';
                cout.flush();
                before = loopCount * 100 / precedingTable.size();
            }
            // DEBUG

            map<string, int> freqs{}; //  preceding + symbol에 해당하는 패턴을 KMP알고리즘으로 찾아낸다
            for (const string& symbol : vec_ascii) {
                string pattern{ preceding.first + symbol };
                vector<int> posits{ kmp(vec_buf, pattern) };
                if (!posits.empty())
                    freqs.insert(make_pair(symbol, (int)posits.size()));
            }
            if (freqs.empty()) // preceding + symbol에 해당하는 패턴이 단한번도 등장하지 않는다면?
                continue;

            Table localHuffman{ freqs };

            // 이 table을 추가하는 경우에 Performance가 과연 개선되는가?
            double performance{};
            int improvement{};
            for (const string& symbol : vec_ascii) {
                // TODO: training_input에 존재하지 않는 symbol을 cost계산에서 제외 (? 그래야 할 이유가 없어보임)

                int originalLen{};
                // 이전까지의 Preceding level에서 symbol을 resolve하기 위해서 필요한 codeword의 길이를 계산
                for (int i{ precedingLevel - 1 }; i >= 0; --i) {
                    string target{ preceding.first.substr(preceding.first.size() - i, i) + symbol };
                    if (tableSet[i].table.find(target) != tableSet[i].table.end()) {
                        originalLen = freqs.at(symbol) * (int)tableSet[i].table.at(target).size();
                        break;
                    }
                }

                int improvedLen{ freqs.at(symbol) * (int)localHuffman.table.at(symbol).size() };
                int reduceSize{ originalLen - improvedLen };
                // TODO: table size = (codeword: 3, symbol: 8, codewordLen: 8) + preceding symbol: 8 * (preceding level) + codeword length
                int tableSize{ (19 + precedingLevel * 8 + (int)localHuffman.table.at(symbol).size()) };

                performance += -reduceSize / 10 + 0.1 * tableSize;
                improvement = reduceSize;
            }

            if (performance <= 0) { // performance가 개선되는 경우 해당 Huffmantalbe을 tableset에 추가
                win++; //DEBUG
                totalReduce += improvement; //DEBUG
                for (pair<string, string> row : localHuffman.table) {
                    string symbol{ preceding.first + row.first };
                    if (!row.second.empty()) //
                        tableSet[precedingLevel].table.insert(make_pair(symbol, row.second));
                }
            }
            else loss++; //DEBUG
        }

        //DEBUG
        chrono::system_clock::time_point levelEnd = chrono::system_clock::now();
        chrono::milliseconds levelMill = chrono::duration_cast<chrono::milliseconds>(levelEnd - levelStart);
        cout << " done!" << '\n';
        cout << "total reduce: " << totalReduce << '\n';
        cout << "win : " << win << ", loss : " << loss << '\n';
        cout << "elapse time : " << levelMill.count() << " milliseconds" << "\n\n";
        //DEBUG
    }

    for (int fileNo{}; fileNo <= 3; ++fileNo) {
        char inPath[50] = "\0";
        char outPath[50] = "\0";
        if (fileNo != 3) {
            sprintf(inPath, "./test_input%d.txt", fileNo + 1);
            sprintf(outPath, "./test_input%d_code.hbs", fileNo + 1);
        }
        else {
            sprintf(inPath, "./training_input.txt");
            sprintf(outPath, "./training_input_code.hbs");
        }
        ifstream sample{};
        ofstream ofs{};
        sample.open(inPath, ifstream::binary);
        ofs.open(outPath, ofstream::binary);

        string samples{};
        if (sample.good()) {
            string v_buf((istreambuf_iterator<char>(sample)), (istreambuf_iterator<char>()));
            samples = v_buf;
            sample.close();
        }

        // Encoded data second ~
        preOut(ofs, tableSet, samples.substr(0, 7), 6);

        string code{};
        for (int readingPosit{ 7 }; readingPosit < samples.size(); ++readingPosit) {
            string symbol{ samples.substr(readingPosit - 7, 8) };

            if (readingPosit == samples.size() - 1)
                cout << symbol;
            for (int i{ 7 }; i >= 0; --i) {
                if (tableSet[i].table.find(symbol) != tableSet[i].table.end()) {
                    code = tableSet[i].table.at(symbol);
                    break;
                }
                else
                    symbol.erase(symbol.begin());
            }
            wordAlignOutput(ofs, code);
        }
        wordAlignOutput(ofs, tableSet[0].table.at("/0"));
        wordAlignOutput(ofs); // buffer clear
        ofs.close();
    }

    // table data
    ofstream tableOfs{};
    tableOfs.open("./huffman_table.hbs", ofstream::binary);
    for (pair<string, string> row : tableSet[0].table) {
        string tableStr{};

        // preceding length code(3bit)
        for (int i{ 3 }; i > 0; --i)
            tableStr += (tableSet[0].precedingLength & (uint8_t)1 << (i - 1)) ? "1" : "0";

        // symbol preceding symbol + symbol
        for (char sym : row.first)
            for (int i{}; i < 8; ++i)
                tableStr += ((int)(sym & (128 >> i)) > 0) ? "1" : "0";

        // codeword length ä   ֱ
        auto length = (uint8_t)row.second.length();
        for (int i{}; i < 8; ++i)
            tableStr += (length & (128 >> i)) ? "1" : "0";

        tableStr += row.second;

        wordAlignOutput(tableOfs, tableStr);
    }
    wordAlignOutput(tableOfs); // buffer clear
    tableOfs.close();


    ofstream adaptiveTableOfs{};
    adaptiveTableOfs.open("./context_adaptive_huffman_table.hbs", ofstream::binary);
    for (int level{1}; level < 8; ++level) {
        for (pair<string, string> row : tableSet[level].table) {
            string tableStr{};

            // preceding length code(3bit)
            for (int i{ 3 }; i > 0; --i)
                tableStr += (tableSet[level].precedingLength & (uint8_t)1 << (i - 1)) ? "1" : "0";

            // symbol preceding symbol + symbol
            for (char sym : row.first)
                for (int i{}; i < 8; ++i)
                    tableStr += ((int)(sym & (128 >> i)) > 0) ? "1" : "0";

            // codeword length ä   ֱ
            auto length = (uint8_t)row.second.length();
            for (int i{}; i < 8; ++i)
                tableStr += (length & (128 >> i)) ? "1" : "0";

            tableStr += row.second;

            wordAlignOutput(adaptiveTableOfs, tableStr);
        }
    }
    wordAlignOutput(adaptiveTableOfs); // buffer clear
    adaptiveTableOfs.close();

    //DEBUG
    chrono::system_clock::time_point EndTime = chrono::system_clock::now();
    chrono::milliseconds mill = chrono::duration_cast<chrono::milliseconds>(EndTime - StartTime);
    cout << "elapse time (total) : " << mill.count() << " milliseconds";
    //DEBUG
    return 0;
}
