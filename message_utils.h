//
// Created by Austin Nodland on 1/12/16.
//

#ifndef DEVICE_LOGGER_C_MESSAGE_UTILS_H
#define DEVICE_LOGGER_C_MESSAGE_UTILS_H

extern "C" {
  #include <mcp2515.h>
}

typedef struct {
    uint32_t node_id;
    bool authorized;
} AuthMessage;

typedef struct {
    uint32_t node_id;
    bool authorized;
    uint32_t user_id;
} StatusMessage;

typedef struct {
    uint32_t node_id;
    uint32_t user_id;
    uint32_t user_pass;
} UserMessage;

typedef enum {
    mAuth, mStatus, mUser, mUndefined
} MessageType;

// Write

void writeAuthMessage(CanMessage &toMessage, const AuthMessage &fromMessage);

void writeStatusMessage(CanMessage &toMessage, const StatusMessage &fromMessage);

void writeUserMessage(CanMessage &toMessage, const UserMessage &fromMessage);

// Read

MessageType determineMessageType(const CanMessage &message);

void readAuthMessage(AuthMessage &toMessage, const CanMessage &fromMessage);

void readStatusMessage(StatusMessage &toMessage, const CanMessage &fromMessage);

void readUserMessage(UserMessage &toMessage, const CanMessage &fromMessage);

#endif //DEVICE_LOGGER_C_MESSAGE_UTILS_H
