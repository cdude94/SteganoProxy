/* 
 * File:   PacketRTP.cpp
 * Author: Michal Lisicki
 * 
 * Created on April 8, 2012, 4:22 AM
 */

#include <utility>
#include <cmath>
#include <cstring>
#include <iostream>
#include "../ApplicationManager.h"

#include "PacketRTP.h"

using namespace PacketHandler;

PacketRTP::PacketRTP(char* msg, int packetSize) {
    msgSize_ = packetSize;
    memcpy(msg_, msg, msgSize_);

    headers_["version"] = new std::pair<int, int>(0,1);
    headers_["padding"] = new std::pair<int, int>(2,2);
    headers_["extension"] = new std::pair<int, int>(3,3);
    headers_["csrc_count"] = new std::pair<int, int>(4,7);
    headers_["marker"] = new std::pair<int, int>(8,8);
    headers_["payload_type"] = new std::pair<int, int>(9,15);
    headers_["seq_num"] = new std::pair<int, int>(16,31);
    headers_["timestamp"] = new std::pair<int, int>(32,63);
    headers_["ssrc"] = new std::pair<int, int>(64,95);
    headers_["payload"] = new std::pair<int, int>(96,8*msgSize_);
}

PacketRTP::PacketRTP(const PacketRTP& orig) {
}

PacketRTP::~PacketRTP() {

}

unsigned char PacketRTP::getField(std::string header) {
    int fnum = floor(headers_[header]->first/8);
    unsigned char field = msg_[fnum];
    unsigned char mask = 0;
    for(int i=headers_[header]->first%8; i <= headers_[header]->second%8; i++) {
        mask += pow(2,8-i);
    }
    field = field & mask;
    field >> (7-headers_[header]->second%8);
    
    return field;
}

int PacketRTP::getSequenceNumber() {
    int fnum = floor(headers_["seq_num"]->first/8);
    unsigned char* seq = &msg_[fnum];

    int seqNum = (int)seq[0];
    seqNum = (seqNum << 8);
//    std::cout << "seq[0]=" << (int)seq[0] << "seq[1]=" << (int)seq[1] << "seqNum=" << seqNum << std::endl;
    seqNum += (int)seq[1];
        
    return seqNum;
}

void PacketRTP::setSequenceNumber(int seqNum) {
    int fnum = floor(headers_["seq_num"]->first/8);

    msg_[fnum+1] = (unsigned char) (seqNum & 255);
    seqNum = (seqNum >> 8);
    msg_[fnum] = (unsigned char) seqNum;
}

std::string PacketRTP::getPayload() {
    std::string msg = "";
    for(int i=floor(headers_["payload"]->first/8); i < floor(headers_["payload"]->second/8); i++) {
        msg += msg_[i];
//        printf("%c",msg_[i]);
    }
    return msg;
}

int PacketRTP::getPayloadSize() {
    return (floor(headers_["payload"]->second/8) - floor(headers_["payload"]->first/8));
}

void PacketRTP::setPayload(std::string payload) {
     for(int i=floor(headers_["payload"]->first/8); i < floor(headers_["payload"]->second/8); i++) {
         if((i-floor(headers_["payload"]->first/8))<payload.size()) {
             msg_[i] = payload[i-floor(headers_["payload"]->first/8)];
         }
         else {
             msg_[i] = '\0';
         }
     }
}

void PacketRTP::setPayload(std::stringstream& payload) {
    for(int i=floor(headers_["payload"]->first/8); i < floor(headers_["payload"]->second/8); i++)
        msg_[i] = '\0';
    // int size = payload.str().size();
    payload.read((msg_+(int)(floor(headers_["payload"]->first/8))),getPayloadSize());
    // for(int i=floor(headers_["payload"]->first/8)+size; i<floor(headers_["payload"]->second/8); i++)
    //    msg_[i] = '\0';
}

std::string PacketRTP::getMsg() {
    std::string msg = "";
    for(int i=0; i< msgSize_; i++)
        msg.push_back(msg_[i]);
    
    return msg;
}