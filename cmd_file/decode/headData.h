
#ifndef HEADDATA_H
#define HEADDATA_H

#include <string>
#include <vector>
#include<iostream>
#include "../IOFile/rwFile.h"
#include <fstream>

using namespace std;

class HeadData {
   public:
    struct metaDataBlockHeader {
        unsigned short isLast;  // 1 bit: 0 for not last, 1 for last
        unsigned short type;    // 7 bits: 0 for streaminfo, 1 for padding, 2 for application, 3 for seektable, 4 for vorbis_comment, 5 for cuesheet, 6 for picture, 7 for reserved // 只关注streaminfo 0, vorbis_comment： 4
        unsigned int length;    // 24 bits: length of metadata to follow, 单位是byte
    };
    struct metaDataBlockData {
        vector<metaDataBlockHeader> headers;
        // 特定于 streaminfo 元数据块的字段
        struct StreamInfo {
            unsigned short minBlockSize;      // 16bit 最小块大小, 单位是sample
            unsigned short maxBlockSize;      // 16bit 最大块大小, 单位是sample
            unsigned int minFrameSize;        // 24bit 最小帧大小, 单位是byte, 0表示未知
            unsigned int maxFrameSize;        // 24bit 最大帧大小， 单位是byte, 0表示未知
            unsigned int sampleRate;          // 20 bit 采样率hz
            unsigned short numChannels;       // 3 bit 通道数减一(通道数=numChannels+1)
            unsigned short sampleDepth;       // 5 bit 采样深度减一(采样深度=sampleDepth+1)
            unsigned long long totalSamples;  // 36bit 总样本数
            string md5[4];            // 128bit MD5校验码
            // 如果需要，可以添加更多特定于 streaminfo 的字段
        } streamInfo;

        // 特定于 padding 元数据块的字段
        struct Padding {
            // 如果需要，可以添加特定于 padding 的字段
            vector<string> comments;
        } padding;

        // 特定于 vorbis_comment 元数据块的字段
        struct VorbisComment {
            unsigned int commentBlockSize = 0; // 32bit 元数据块大小, 单位是byte
            //vendorString的长度
            unsigned int vendorLength = 0;
            //字符串
            string vendorString;
            //单个字符的集合
            vector<unsigned int> vendorStringOriginal;
            unsigned int commentListLength = 0;
            vector<string> commentList;
            vector<vector<unsigned int>> commentsOriginal;
        } vorbisComment;

        // 如果需要，可以添加更多用于其他元数据块类型的结构
    };
    struct metaDataEdit {
        int alterSize = 0;
        bool isEditVendorString = false;
        string newVendorString;
        bool modifyComment = false;
        int modifyCommentIndex = -1;
        string modifiedComment;
        vector<string> newComments;
        bool appendComment = false;
        bool removeComment = false;
        int removeCommentIndex = -1;
    };

    struct frameHeader {
        unsigned char sync[2];  // 2bit 同步码, 固定为0x66, 0x66
        unsigned char reserved1;  // 1bit 保留位, 固定为0
        unsigned char blockingStrategy;  // 1bit 块大小策略, 0表示固定大小, 1表示可变大小
        unsigned char blockSize[2];  // 4bit 块大小, 如果blockingStrategy为0, 则块大小为固定大小, 单位是byte, 否则为最大块大小, 单位是byte
        unsigned char sampleRate[3];  // 20bit 采样率, 单位是hz
        unsigned char channelAssignment;  // 4bit 通道数减一(通道数=numChannels+1)
        unsigned char sampleSize[3];  // 20bit 采样深度减一(采样深度=sampleDepth+1)
        unsigned char reserved2;  // 3bit 保留位, 固定为0
        unsigned char crc8;  // 8bit CRC-8校验码
    };
    struct frameData {
        vector<unsigned char> subframes;
        unsigned char padding[256];
        unsigned char crc16[2];
    };
    struct frame {
        frameHeader header;
        frameData data;
    };
    struct frameInfo {
        vector<frame> frames;
    };
    struct frameEdit {
        int alterSize = 0;
        bool isEditComment = false;
        string newVendorString;
        bool modifyComment = false;
        int modifyCommentIndex = -1;
        string modifiedComment;
        vector<string> newComments;
        bool appendComment = false;
        bool removeComment = false;
        int removeCommentIndex = -1;
    };
    // Function to read metadata block header
    metaDataBlockData readMetaDataBlock(fileReader& file);

    static void printMetaDataBlock(metaDataBlockData mD);

    // Function to read metadata block data
    void editMetaDataBlock(fileReader& infile, fileWriter &outfile, metaDataEdit edit);
};

#endif  // HEADDATA_H