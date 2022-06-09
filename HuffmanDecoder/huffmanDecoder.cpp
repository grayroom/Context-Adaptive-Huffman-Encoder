#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#define OFFSET 128

using namespace std;

class tableRow {
public:
    int precedingLevel{};
    string  precedingSymbol{};
    char symbol{}; // OFFSET
    uint8_t length{};
    string codeword{};

    tableRow() {}
};

void bitAlignInput(vector<uint8_t>& vec_buf, tableRow& row) {
    static uint8_t buffer{};
    static int length{};

    int count_symLength{},
        count_precedingLength{},
        count_symbol{},
        count_length{},
        count_codeword{};

    if (vec_buf.empty()) {
        buffer = 0;
        length = 0;
        return;
    }

    // read preceding symbol level(length) data
    while (count_symLength < 3) {
        if (length == 0) { // load 8bit on buffer
            buffer = *(vec_buf.begin());
            vec_buf.erase(vec_buf.begin());
            length = 8;
        }

        row.precedingLevel = (buffer & (1 << (length - 1))) ?
            (row.precedingLevel << 1 | 1) : (row.precedingLevel << 1);
        length--;
        count_symLength++;
    }

    // read preceding symbol
    while (count_precedingLength < row.precedingLevel) {
        int count_byte{};
        char particleSymbol{};
        while (count_byte < 8) {
            if (length == 0) { // load 8bit on buffer
                buffer = *(vec_buf.begin());
                vec_buf.erase(vec_buf.begin());
                length = 8;
            }

            particleSymbol = (buffer & (1 << (length - 1))) ?
                (particleSymbol << 1 | 1) : (particleSymbol << 1);
            length--;
            count_byte++;
        }
        row.precedingSymbol += particleSymbol;
        count_precedingLength++;
    }

    // read symbol data
    while (count_symbol < 8) {
        if (length == 0) { // load 8bit on buffer
            buffer = *(vec_buf.begin());
            vec_buf.erase(vec_buf.begin());
            length = 8;
        }

        row.symbol = (buffer & (1 << (length - 1))) ? (row.symbol << 1 | 1) : (row.symbol << 1);
        length--;
        count_symbol++;
    }

    // read length data
    while (count_length < 8) {
        if (length == 0) { // load 8bit on buffer
            buffer = *(vec_buf.begin());
            vec_buf.erase(vec_buf.begin());
            length = 8;
        }

        row.length = (buffer & (1 << (length - 1))) ? (row.length << 1 | 1) : (row.length << 1);

        length--;
        count_length++;
    }

    while (count_codeword < row.length) {
        if (length == 0) { // load 8bit on buffer
            buffer = *(vec_buf.begin());
            vec_buf.erase(vec_buf.begin());
            length = 8;
        }

        if ((buffer & (1 << (length - 1))))
            row.codeword.append("1");
        else
            row.codeword.append("0");

        length--;
        count_codeword++;
    }
}

char decode(vector<map<string, map<string, char>>>& mappings, vector<uint8_t>& vec_buf) {
    static uint8_t buffer{};
    static int length{};
    static string previousSymbol{};

    int fin{};
    string codeword{};
    char symbol{};

    if (vec_buf.empty()) {
        buffer = 0;
        length = 0;
        previousSymbol = "";

        return 'T';
    }

    while (true) {
        if (length == 0) { // load 8bit on buffer
            buffer = *(vec_buf.begin());
            vec_buf.erase(vec_buf.begin());
            length = 8;
        }

        if ((buffer & (1 << (length - 1))))
            codeword.append("1");
        else
            codeword.append("0");
        length--;

        for (int i{ 7 }; i >= 0; --i) {
            if (previousSymbol.size() < i)
                continue;
            string target = previousSymbol.substr(previousSymbol.size() - i, i);
            if (mappings[i].find(target) != mappings[i].end()) { // 각 level에 encoding가능한 테이블 row가 존재하는지 찾는다
                if (mappings[i].at(target).find(codeword) != mappings[i].at(target).end()) {
                    symbol = mappings[i].at(target).at(codeword);
                    previousSymbol += symbol;
                    if (previousSymbol.size() > 10)
                        previousSymbol.erase(previousSymbol.begin());
                }
                // 이번 level에서 해석할 수 있다면 codeword를 늘려가며 맞는걸 찾음
                break;
            }
        }

        if (symbol)
            break;
    }

    return symbol;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    // table data 읽어오기
    ifstream ifs{};
    ifs.open("./huffman_table.hbs", ifstream::binary);
    vector<uint8_t> vec_buf{};
    if (ifs.good()) {
        vector<uint8_t> v_buf((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
        vec_buf = v_buf;
        ifs.close();
    }
    vector<vector<tableRow>> tableSet(8);
    while (!vec_buf.empty()) {
        tableRow row{};
        bitAlignInput(vec_buf, row);
        tableSet[row.precedingLevel].push_back(row);
    }
    tableRow row2{};
    bitAlignInput(vec_buf, row2);

    ifstream ifs2{};
    ifs2.open("./context_adaptive_huffman_table.hbs", ifstream::binary);
    vector<uint8_t> vec_buf2{};
    if (ifs2.good()) {
        vector<uint8_t> v_buf2((istreambuf_iterator<char>(ifs2)), (istreambuf_iterator<char>()));
        vec_buf2 = v_buf2;
        ifs2.close();
    }
    while (!vec_buf2.empty()) {
        tableRow row{};
        bitAlignInput(vec_buf2, row);
        tableSet[row.precedingLevel].push_back(row);
    }
    bitAlignInput(vec_buf2, row2);

    // table data -> map mapping
    vector<map<string, map<string, char>>> mappings{};
    for (vector<tableRow> table : tableSet) {
        map<string, map<string, char>> mapping{};
        for (tableRow row : table) {
            mapping.insert(make_pair(row.precedingSymbol, map<string, char>{}));
            mapping.at(row.precedingSymbol).insert(make_pair(row.codeword, row.symbol));
        }
        mappings.emplace_back(mapping);
    }

    // 테스트파일 각각 decoding
    for (int fileNo{}; fileNo <= 3; ++fileNo) {
        char inPath[50] = "\0";
        char outPath[50] = "\0";
        if (fileNo != 3) {
            sprintf(inPath, "./test_input%d_code.hbs", fileNo + 1);
            sprintf(outPath, "./output%d.txt", fileNo + 1);
        }
        else {
            sprintf(inPath, "./training_input_code.hbs");
            sprintf(outPath, "./training_output.txt");
        }
        ifstream codeIfs{};
        ofstream ofs{};
        codeIfs.open(inPath, ifstream::binary);
        ofs.open(outPath, ofstream::binary);

        vector<uint8_t> code{};
        if (codeIfs.good()) {
            vector<uint8_t> v_buf((istreambuf_iterator<char>(codeIfs)), (istreambuf_iterator<char>()));
            code = v_buf;
            codeIfs.close();
        }
        while (!code.empty())
            ofs << decode(mappings, code);
        decode(mappings, code);
        ofs.close();
    }

    return 0;
}
