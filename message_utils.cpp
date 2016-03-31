//
// Created by Austin Nodland on 1/12/16.
//

#include <string.h>
#include "message_utils.h"
#include "byte_utils.h"

// Write

void writeMessage(CanMessage &message, uint32_t message_id, uint8_t data[8], uint8_t length) {
    message.mtype = MTYPE_EXTENDED_DATA;
    message.sid = (uint16_t) (message_id & 0x7FF);
    message.eid = message_id >> 11;
    message.length = length;

    memcpy(message.data, (const void *) data, length);
}

void writeAuthMessage(CanMessage &toMessage, const AuthMessage &fromMessage) {
    uint8_t _node_id[4];
    expandValue(fromMessage.node_id, _node_id);

    uint8_t data[5] = {_node_id[0], _node_id[1], _node_id[2], _node_id[3], (uint8_t) fromMessage.authorized};
    writeMessage(toMessage, 1, data, 5);
}

void writeStatusMessage(CanMessage &toMessage, const StatusMessage &fromMessage) {
    uint8_t _user_id[4];
    expandValue(fromMessage.user_id, _user_id);

    uint8_t data[5] = {(uint8_t) fromMessage.authorized, _user_id[0], _user_id[1], _user_id[2], _user_id[3]};
    writeMessage(toMessage, 0x10000000 | fromMessage.node_id, data, 5);
}

void writeUserMessage(CanMessage &toMessage, const UserMessage &fromMessage) {
    uint8_t _user_id[4];
    expandValue(fromMessage.user_id, _user_id);

    uint8_t _user_pass[4];
    expandValue(fromMessage.user_pass, _user_pass);

    uint8_t data[8] = {_user_id[0], _user_id[1], _user_id[2], _user_id[3],
                       _user_pass[0], _user_pass[1], _user_pass[2], _user_pass[3]};
    writeMessage(toMessage, fromMessage.node_id, data, 8);
}

// Read

uint32_t combineMessageId(const CanMessage &message) {
    return (message.eid << 11) | ((uint32_t) message.sid);
}

MessageType determineMessageType(const CanMessage &message) {
    uint32_t message_id = combineMessageId(message);

    if (message_id == 0) {
        return mUndefined;
    }

    if (message_id == 1) {
        return mAuth;
    }

    if ((message_id & 0x10000000) == 0) {
        return mUser;
    }

    return mStatus;
}

void readAuthMessage(AuthMessage &toMessage, const CanMessage &fromMessage) {
    toMessage.node_id = appendBytes(fromMessage.data);
    toMessage.authorized = (bool) fromMessage.data[4];
}

void readStatusMessage(StatusMessage &toMessage, const CanMessage &fromMessage) {
    uint32_t message_id = combineMessageId(fromMessage);

    toMessage.node_id = message_id & 0xFFFFFFF;
    toMessage.authorized = (bool) fromMessage.data[0];
    toMessage.user_id = appendBytes(&fromMessage.data[1]);
}

void readUserMessage(UserMessage &toMessage, const CanMessage &fromMessage) {
    uint32_t message_id = combineMessageId(fromMessage);

    toMessage.node_id = message_id;
    toMessage.user_id = appendBytes(fromMessage.data);
    toMessage.user_pass = appendBytes(&fromMessage.data[4]);
}
