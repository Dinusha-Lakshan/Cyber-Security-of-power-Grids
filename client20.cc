/*
 * client20.cc
 *
 *  Created on: Jul 25, 2023
 *      Author: DELL
 */
#include <omnetpp.h>
#include "CTSGenerator20.h"  // Include the header file

using namespace omnetpp;

class client20 : public cSimpleModule {
private:
    int messageCount;
    int maxMessageCount;
    simtime_t nextMessageTime;  // Track the scheduled time for the next message
    cMessage *droppedMessageRequest = nullptr; // Store the dropped message request
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void sendNextMessage();
    void handleDroppedMessage(cMessage *msg);
public:
    client20();
    virtual ~client20();
};

Define_Module(client20);

client20::client20() {
    messageCount = 0;
    maxMessageCount = 100;
}

client20::~client20() {
}

void client20::initialize() {
    EV << "Client initialize" << endl;

    // Schedule the first message after a 2-second interval
    simtime_t interval = 0.0; // Interval of 2 seconds
    nextMessageTime = simTime() + interval;
    scheduleAt(nextMessageTime, new cMessage("RTS"));

    // Start sending messages
    sendNextMessage();
}

void client20::handleMessage(cMessage *msg) {
    EV << "Client handle message" << endl;

    if (strcmp(msg->getName(), "Request") == 0) { // Check if it's a dropped message request
            handleDroppedMessage(msg);
        } else if (strcmp(msg->getName(), "RTS") == 0) { // Check if it's the RTS message
            // Resend the dropped message request if one exists
            if (droppedMessageRequest) {
                const std::string& encryptedMessage = droppedMessageRequest->getName();
                cMessage *resendMessage = new cMessage(encryptedMessage.c_str());
                resendMessage->setContextPointer(droppedMessageRequest->getContextPointer()); // Save a copy of the original message for resending if needed

                // Retrieve the target submodule (SmartMeter3)
                cModule *target = getParentModule()->getSubmodule("SmartMeter3");

                // Send the message directly to SmartMeter3
                sendDirect(resendMessage, target, "radioIn");

                // Cleanup the dropped message request
                delete droppedMessageRequest;
                droppedMessageRequest = nullptr;
            }

            // Send the next message
            if (messageCount < maxMessageCount) {
                sendNextMessage();
            }

            // Delete the current message
            delete msg;
        } else {
            // Handle the received message if needed
            // ...

            // Delete the current message
            delete msg;
        }
    }


void client20::sendNextMessage() {
    messageCount++;

    cMessage *msg = generateCTSMessage20();

    // Encrypt the message using XOR encryption
    const char encryptionKey = 'K'; // Choose your encryption key
    const std::string& originalMessage = msg->getName();
    std::string encryptedMessage = "SM20:" + std::to_string(messageCount) + ":" + originalMessage;

    // XOR encrypt the message
    for (char& c : encryptedMessage) {
        c = c ^ encryptionKey;
        }

    // Update the message name with the encrypted message
    std::string signedMessage = encryptedMessage;
    msg->setName(encryptedMessage.c_str());

    // Retrieve the target submodule (SmartMeter3)
    cModule *target = getParentModule()->getSubmodule("SmartMeter3");

    // Send the message directly to SmartMeter3
    sendDirect(msg, target, "radioIn");

    // Schedule the next message after a 2-second interval
    simtime_t interval = 1.0; // Interval of 2 seconds
    nextMessageTime += interval;
    scheduleAt(nextMessageTime, new cMessage("RTS"));
}

void client20::handleDroppedMessage(cMessage *msg) {
    // Check if the dropped message request exists
    if (droppedMessageRequest) {
        // Resend the dropped message
        const std::string& encryptedMessage = droppedMessageRequest->getName();
        cMessage *resendMessage = new cMessage(encryptedMessage.c_str());
        resendMessage->setContextPointer(droppedMessageRequest->getContextPointer()); // Save a copy of the original message for resending if needed

        // Retrieve the target submodule (SmartMeter3)
        cModule *target = getParentModule()->getSubmodule("SmartMeter3");

        // Send the message directly to SmartMeter3
        sendDirect(resendMessage, target, "radioIn");

        // Cleanup the dropped message request
        delete droppedMessageRequest;
        droppedMessageRequest = nullptr;
    }
}















