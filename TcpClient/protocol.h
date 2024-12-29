#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <QString>

typedef unsigned int uint;

enum ENUM_MSG_TYPE {
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REQUEST,  // 注册请求，自动增长，为1
    ENUM_MSG_TYPE_RESPOND,  // 注册回复
    ENUM_MSG_TYPE_MAX = 0x00ffffff,
};

struct PDU{
    uint uiPDULen;  // 总的协议数据单元大小
    uint uiMsgType;  // 消息类型
    char caData[64];  // 用户信息
    uint uiMsgLen;  // 实际消息长度
    int caMsg[];  // 实际消息
};

PDU *mkPDU(uint uiMsgLen);


#endif // PROTOCOL_H
