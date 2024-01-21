#include "headData.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "rwFile.h"

using namespace std;

string intToHex(unsigned int num) {
    std::stringstream stream;
    stream << std::hex << num;  // Convert decimal to hexadecimal
    return stream.str();        // Return the hexadecimal string
}

int hexToInt(const std::string &str) {
    int ans;
    sscanf(str.c_str(), "%x", &ans);
    return ans;
}

string showType(unsigned short tpye) {
    switch ((int)tpye) {
        case 0:
            return "streamInfo";
            break;
        case 1:
            return "padding";
            break;
        case 2:
            return "application";
            break;
        case 3:
            return "seektable";
            break;
        case 4:
            return "vorbis_comment";
            break;
        case 5:
            return "cuesheet";
            break;
        case 6:
            return "picture";
            break;
        case 7:
            return "reserved";
            break;
        default:
            return "default";
            break;
    }
}

// 读取文件头部信息
void HeadData::printMetaDataBlock(metaDataBlockData mD) {
    // cout << "metaDataBlock" << endl;
    for (int i = 0; i < mD.headers.size(); i++) {
        cout << "----metaDataBlock----" << endl;
        cout << "isLast: " << ((mD.headers[i].isLast) ? " is last" : "no last") << endl;
        cout << "type: " << showType(mD.headers[i].type) << endl;
        cout << "length: " << mD.headers[i].length << endl;
        switch (mD.headers[i].type) {
            case 0:
                cout << "streamInfo" << endl;
                cout << "minBlockSize: " << mD.streamInfo.minBlockSize << endl;
                cout << "maxBlockSize: " << mD.streamInfo.maxBlockSize << endl;
                cout << "minFrameSize: " << mD.streamInfo.minFrameSize << endl;
                cout << "maxFrameSize: " << mD.streamInfo.maxFrameSize << endl;
                cout << "sampleRate: " << mD.streamInfo.sampleRate << endl;
                cout << "numChannels: " << mD.streamInfo.numChannels << endl;
                cout << "sampleDepth: " << mD.streamInfo.sampleDepth << endl;
                cout << "totalSamples: " << mD.streamInfo.totalSamples << endl;
                cout << "md5: ";
                for (int i = 0; i < 4; i++) {
                    cout << mD.streamInfo.md5[i];
                }
                cout << endl;
                break;
            case 4:
                cout << "commentBlockSize: " << mD.vorbisComment.commentBlockSize << endl;
                cout << "vendorLength: " << mD.vorbisComment.vendorLength << endl;
                cout << "vendorString: " << mD.vorbisComment.vendorString << endl;
                cout << "commentListLength: " << mD.vorbisComment.commentListLength << endl;
                for (int i = 0; i < mD.vorbisComment.commentListLength; i++) {
                    cout << "comment: " << mD.vorbisComment.commentList[i] << endl;
                }
                break;
            default:
                cout << "default" << endl;
                break;
        }
    }
}

HeadData::metaDataBlockData HeadData::readMetaDataBlock(fileReader &input) {
    if (input.readBigUInt(32) != 0x664c6143) {
        throw runtime_error("Not a valid FLAC file");
    }
    // cout << "metaDataBlock" << endl;
    // metaData a;
    metaDataBlockData metaData;

    metaDataBlockHeader header;
    // 一定有一个streaminfo元数据块，所以可以直接进行第一次读取
    bool isLast = false;
    // string vendorString;
    do {
        isLast = input.readBigUInt(1);
        header.isLast = isLast;
        header.type = input.readBigUInt(7);
        header.length = input.readBigUInt(24);
        // cout << "isLast: " << header.isLast << endl;
        // cout << "type: " << header.type << endl;
        // cout << "length: " << header.length << endl;
        metaData.headers.push_back(header);
        if (header.type == 0) {
            //         unsigned short minBlockSize;      // 16bit 最小块大小, 单位是sample
            // unsigned short maxBlockSize;      // 16bit 最大块大小, 单位是sample
            // unsigned int minFrameSize;        // 24bit 最小帧大小, 单位是byte, 0表示未知
            // unsigned int maxFrameSize;        // 24bit 最大帧大小， 单位是byte, 0表示未知
            // unsigned int sampleRate;          // 20 bit 采样率hz
            // unsigned short numChannels;       // 3 bit 通道数减一(通道数=numChannels+1)
            // unsigned short sampleDepth;       // 5 bit 采样深度减一(采样深度=sampleDepth+1)
            // unsigned long long totalSamples;  // 36bit 总样本数
            // unsigned char md5[16];            // 128bit MD5校验码
            metaData.streamInfo.minBlockSize = input.readBigUInt(16);
            metaData.streamInfo.maxBlockSize = input.readBigUInt(16);
            metaData.streamInfo.minFrameSize = input.readBigUInt(24);
            metaData.streamInfo.maxFrameSize = input.readBigUInt(24);
            metaData.streamInfo.sampleRate = input.readBigUInt(20);
            metaData.streamInfo.numChannels = input.readBigUInt(3) + 1;
            metaData.streamInfo.sampleDepth = input.readBigUInt(5) + 1;
            metaData.streamInfo.totalSamples = (input.readBigUInt(32) << 4) | input.readBigUInt(4);
            for (int i = 0; i < 4; i++) {
                metaData.streamInfo.md5[i] = intToHex(input.readBigUInt(32));
            }
        } else if (header.type == 4) {
            // 实际编码时，因为向量之间没有分隔符，必须编码每个向量的长度。每个向量编码时，先将其长度作为一个有符号int整数写入，再写入向量字符串。有符号int叫做向量长度，按照Little Endian顺序。因为供应商字符串必须存在，单独解码；向量可以放在循环里解码，所以在供应商字符串后有一个有符号int向量总数。向量总数不包括供应商字符串。
            metaData.vorbisComment.commentBlockSize = header.length;
            metaData.vorbisComment.vendorLength = input.readLittleUInt(32);
            // cout << "vendorLength: " <<  vendorLength << endl;
            metaData.vorbisComment.vendorStringOriginal.reserve(metaData.vorbisComment.vendorLength);

            for (int i = 0; i < metaData.vorbisComment.vendorLength; i++) {
                auto temp = input.readLittleUInt(8);
                // cout << "temp: " << temp << endl;
                metaData.vorbisComment.vendorStringOriginal.push_back(temp);
            }
            string vendorString(metaData.vorbisComment.vendorStringOriginal.begin(), metaData.vorbisComment.vendorStringOriginal.end());
            // cout << "vendorString: " << vendorString << endl;
            metaData.vorbisComment.vendorString = vendorString;

            metaData.vorbisComment.commentListLength = input.readLittleUInt(32);
            // cout << "commentListLength: " << metaData.vorbisComment.commentListLength << endl;

            for (int i = 0; i < metaData.vorbisComment.commentListLength; i++) {
                unsigned int commentLength = input.readLittleUInt(32);
                vector<unsigned int> commentOriginal;
                commentOriginal.reserve(commentLength);
                for (int j = 0; j < commentLength; j++) {
                    commentOriginal.push_back(input.readLittleUInt(8));
                }
                string comment(commentOriginal.begin(), commentOriginal.end());
                // cout << "comment: " << i << ": " << comment << endl;
                metaData.vorbisComment.commentList.push_back(comment);
                metaData.vorbisComment.commentsOriginal.push_back(commentOriginal);
            }
        } else {
            for (int i = 0; i < header.length; ++i) {
                input.readBigUInt(8);
            }
            // cout << "default" << endl;
        }

    } while (!isLast);
    // printMetaDataBlock(metaData);
    return metaData;
}

void HeadData::editMetaDataBlock(fileReader &infile, fileWriter &outfile, metaDataEdit edit) {
    cout << "--------befor modify---------" << endl;
    metaDataBlockData metaData;
    metaData = readMetaDataBlock(infile);
    printMetaDataBlock(metaData);
    cout << "-----------------" << endl;
    cout << "Flac metadata edit into: " << endl;

    if (edit.isEditVendorString) {
        cout << "edit vendor string" << endl;
        cout << "befor edit vendor string: " << metaData.vorbisComment.vendorString << endl;
        vector<unsigned int> vendorStringOriginal(metaData.vorbisComment.vendorString.begin(), metaData.vorbisComment.vendorString.end());
        metaData.vorbisComment.vendorString = edit.newVendorString;
        vector<unsigned int> myVector(metaData.vorbisComment.vendorString.begin(), metaData.vorbisComment.vendorString.end());
        metaData.vorbisComment.vendorStringOriginal = myVector;
        edit.alterSize = metaData.vorbisComment.vendorStringOriginal.size() - vendorStringOriginal.size();
        cout << "after edit vendor string: " << metaData.vorbisComment.vendorString << endl;
    }
    if (edit.modifyComment) {
        if (edit.modifyCommentIndex == -1 || edit.modifyCommentIndex >= metaData.vorbisComment.commentListLength) {
            throw runtime_error("modify comment index error");
        }
        cout << "modify comment" << endl;
        cout << "befor modify comment " << edit.modifyCommentIndex << ": " << metaData.vorbisComment.commentList[edit.modifyCommentIndex] << endl;

        metaData.vorbisComment.commentList[edit.modifyCommentIndex] = edit.modifiedComment;
        vector<unsigned int> myVector(edit.modifiedComment.begin(), edit.modifiedComment.end());
        edit.alterSize += myVector.size() - metaData.vorbisComment.commentsOriginal[edit.modifyCommentIndex].size();
        metaData.vorbisComment.commentsOriginal[edit.modifyCommentIndex] = myVector;

        cout << "after modify comment " << edit.modifyCommentIndex << ": " << metaData.vorbisComment.commentList[edit.modifyCommentIndex] << endl;
    }
    if (edit.appendComment) {
        cout << "append comment" << endl;
        cout << "append comment: " << edit.newComments.size() << " line(s)" << endl;
        for (int i = 0; i < edit.newComments.size(); i++) {
            cout << "append comment: " << i << " line: " << edit.newComments[i] << endl;
            metaData.vorbisComment.commentListLength++;
            metaData.vorbisComment.commentList.push_back(edit.newComments[i]);
            vector<unsigned int> myVector(edit.newComments[i].begin(), edit.newComments[i].end());
            metaData.vorbisComment.commentsOriginal.push_back(myVector);
            edit.alterSize += myVector.size();
        }
    }
    if (edit.removeComment) {
        if (edit.removeCommentIndex == -1 || edit.removeCommentIndex >= metaData.vorbisComment.commentListLength) {
            throw runtime_error("remove comment index error");
        }
        cout << "remove comment" << endl;
        cout << "remove comment " << edit.removeCommentIndex << " : " << metaData.vorbisComment.commentList[edit.removeCommentIndex] << endl;
        edit.alterSize -= metaData.vorbisComment.commentsOriginal[edit.removeCommentIndex].size();
        metaData.vorbisComment.commentList.erase(metaData.vorbisComment.commentList.begin() + edit.removeCommentIndex);
        metaData.vorbisComment.commentsOriginal.erase(metaData.vorbisComment.commentsOriginal.begin() + edit.removeCommentIndex);
        metaData.vorbisComment.commentListLength--;
    }

    // copy
    outfile.writeBigInt(0x664c6143, 32);
    int lastAlterSize = 0;
    for (int i = 0; i < metaData.headers.size(); i++) {
        outfile.writeBigInt(metaData.headers[i].isLast, 1);
        outfile.writeBigInt(metaData.headers[i].type, 7);

        if (metaData.headers[i].type == 0) {
            cout << "writing streamInfo" << endl;
            outfile.writeBigInt(metaData.headers[i].length, 24);
            outfile.writeBigInt(metaData.streamInfo.minBlockSize, 16);
            outfile.writeBigInt(metaData.streamInfo.maxBlockSize, 16);
            outfile.writeBigInt(metaData.streamInfo.minFrameSize, 24);
            outfile.writeBigInt(metaData.streamInfo.maxFrameSize, 24);
            outfile.writeBigInt(metaData.streamInfo.sampleRate, 20);
            outfile.writeBigInt(metaData.streamInfo.numChannels - 1, 3);
            outfile.writeBigInt(metaData.streamInfo.sampleDepth - 1, 5);
            outfile.writeBigInt(metaData.streamInfo.totalSamples >> 18, 18);
            outfile.writeBigInt(metaData.streamInfo.totalSamples & 0x3FFFF, 18);
            for (int j = 0; j < 4; j++) {
                outfile.writeBigInt(hexToInt(metaData.streamInfo.md5[j]), 32);
            }
            lastAlterSize = 0;
        } else if (metaData.headers[i].type == 4) {
            outfile.writeBigInt(metaData.headers[i].length + edit.alterSize, 24);
            cout << "writing vorbisComment" << endl;
            cout << "alterSize: " << edit.alterSize << endl;
            outfile.writeLittleInt((int)(metaData.vorbisComment.vendorString.length()), 32);
            for (char j : metaData.vorbisComment.vendorStringOriginal) {
                outfile.writeLittleInt((int)j, 8);
            }
            outfile.writeLittleInt(metaData.vorbisComment.commentListLength, 32);
            for (int j = 0; j < metaData.vorbisComment.commentListLength; j++) {
                outfile.writeLittleInt(metaData.vorbisComment.commentList[j].length(), 32);
                for (unsigned int j : metaData.vorbisComment.commentsOriginal[j]) {
                    outfile.writeLittleInt(j, 8);
                }
            }
        } else {
            outfile.writeBigInt(metaData.headers[i].length, 24);
            for (int j = 0; j < metaData.headers[i].length; j++) {
                outfile.writeBigInt(0, 8);
            }
        }
    }
    int temp;
    while (1) {
        try {
            temp = infile.readBigUInt(8);
            outfile.writeBigInt(temp, 8);
        } catch (runtime_error &e) {
            cout << "finish" << endl;
            break;
        }
    }
}

vector<string> strSplit(const string &text, char sep) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(text);

    while (getline(tokenStream, token, sep)) {
        tokens.push_back(token);
    }

    return tokens;
}

HeadData::metaDataEdit getEditData(const string &modifyVendor, const string &modifyComment, const string &modifyCommentIndex,
                                   const string &appendComment, const string &removeCommentIndex) {
    HeadData::metaDataEdit metaEditInfo;
    if (!modifyVendor.empty()) {
        metaEditInfo.newVendorString = modifyVendor;
        metaEditInfo.isEditVendorString = true;
        // cout << "modifyVendor: " << metaEditInfo.modifyVendorString << endl;
    }
    if (!modifyComment.empty()) {
        metaEditInfo.modifiedComment = modifyComment;
        metaEditInfo.modifyComment = true;
        if (!modifyCommentIndex.empty()) {
            metaEditInfo.modifyCommentIndex = stoi(modifyCommentIndex);
        } else {
            throw runtime_error("Error acquiring comment index for modifying (main::main)");
        }
    }
    if (!appendComment.empty()) {
        metaEditInfo.newComments = strSplit(appendComment, ';');
        metaEditInfo.appendComment = true;
    }
    if (!removeCommentIndex.empty()) {
        metaEditInfo.removeComment = true;
        if (!removeCommentIndex.empty()) {
            metaEditInfo.removeCommentIndex = stoi(removeCommentIndex);
        } else {
            throw runtime_error("Error acquiring comment index for removing (main::main)");
        }
    }
    return metaEditInfo;
}

int main(int argc, char *argv[]) {
    string input_path, output_path;
    string modifyVendor,
        modifyComment, modifyCommentIndex,
        appendComment,
        removeCommentIndex;

    if (argc == 2) {
        cout << "Usage: ./main <inputfile> <output file>" << endl;
        cout << "using default output file: output.flac" << endl;
        input_path = argv[1];
        output_path = "output.flac";
    } else if (argc == 1) {
        cout << "Usage: ./main <inputfile> <output file>" << endl;
        cout << "using default file: test.flac output.flac" << endl;
        input_path = "test.flac";
        output_path = "output.flac";
    } else if (argc == 3) {
        input_path = argv[1];
        output_path = argv[2];
    }
    cout << "Do you want to modify vendor string? (y/n)" << endl;
    cin >> modifyVendor;
    if (modifyVendor == "y") {
        cout << "Please input new vendor string: " << endl;
        cin >> modifyVendor;
    } else {
        modifyVendor = "";
    }
    cout << "Do you want to append comment? (y/n)" << endl;
    cin >> appendComment;
    if (appendComment == "y") {
        cout << "Please input new comment(s) separated by ';': " << endl;
        cin >> appendComment;
    } else {
        appendComment = "";
    }
    cout << "Do you want to modify comment? (y/n)" << endl;
    cin >> modifyComment;
    if (modifyComment == "y") {
        cout << "Please input comment index: " << endl;
        cin >> modifyCommentIndex;
        cout << "Please input new comment: " << endl;
        cin >> modifyComment;
    } else {
        modifyComment = "";
        modifyCommentIndex = "";
    }

    cout << "Do you want to remove comment? (y/n)" << endl;
    cin >> removeCommentIndex;
    if (removeCommentIndex == "y") {
        cout << "Please input comment index: " << endl;
        cin >> removeCommentIndex;
    } else {
        removeCommentIndex = "";
    }

    // bool noModify = true;
    // if (noModify) {
    //     cout << "Do you want to modify vendor string? (y/n)" << endl;
    //     cin >> modifyVendor;
    //     if (modifyVendor == "y") {
    //         cout << "Please input new vendor string: " << endl;
    //         cin >> modifyVendor;
    //         noModify = false;
    //     } else {
    //         modifyVendor = "";
    //     }
    // }
    // if (noModify) {
    //     cout << "Do you want to modify comment? (y/n)" << endl;
    //     cin >> modifyComment;
    //     if (modifyComment == "y") {
    //         cout << "Please input comment index: " << endl;
    //         cin >> modifyCommentIndex;
    //         cout << "Please input new comment: " << endl;
    //         cin >> modifyComment;
    //         noModify = false;
    //     } else {
    //         modifyComment = "";
    //         modifyCommentIndex = "";
    //     }
    // }
    // if (noModify) {
    //     cout << "Do you want to append comment? (y/n)" << endl;
    //     cin >> appendComment;
    //     if (appendComment == "y") {
    //         cout << "Please input new comment(s) separated by ';': " << endl;
    //         cin >> appendComment;
    //         noModify = false;
    //     } else {
    //         appendComment = "";
    //     }
    // }
    // if (noModify) {
    //     cout << "Do you want to remove comment? (y/n)" << endl;
    //     cin >> removeCommentIndex;
    //     if (removeCommentIndex == "y") {
    //         cout << "Please input comment index: " << endl;
    //         cin >> removeCommentIndex;
    //         noModify = false;
    //     } else {
    //         removeCommentIndex = "";
    //     }
    // }

    std::ifstream inputFile1(input_path, ios::in | ios::binary);
    ofstream outputFile(output_path, ios::out | ios::trunc | ios::binary);
    if (!inputFile1.is_open()) {
        cout << "Cannot open file " << input_path << endl;
        return 0;
    }
    if (!outputFile.is_open()) {
        throw runtime_error("Error opening output file (main::main)");
    }
    HeadData::metaDataEdit metaEditInfo = getEditData(modifyVendor, modifyComment, modifyCommentIndex, appendComment, removeCommentIndex);

    fileReader reader(inputFile1);
    fileWriter writer(outputFile);
    try {
        HeadData headData;
        cout << "--------modifying---------" << endl;
        headData.editMetaDataBlock(reader, writer, metaEditInfo);
        reader.closeReader();
        inputFile1.close();
        outputFile.close();
        writer.closeWriter();
        cout << "--------after modify---------" << endl;
        std::ifstream inputFile2(output_path, ios::in | ios::binary);
        if (!inputFile2.is_open()) {
            throw runtime_error("Error opening output file (main::main)");
        }
        fileReader reader2(inputFile2);
        HeadData headData2;
        HeadData::metaDataBlockData metaData2 = headData2.readMetaDataBlock(reader2);
        headData2.printMetaDataBlock(metaData2);
        reader2.closeReader();
        inputFile2.close();
    } catch (exception &e) {
        cout << e.what() << endl;
    }
    return 0;
}
