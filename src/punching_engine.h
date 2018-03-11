#ifndef PUNCHING_ENGINE_H
#define PUNCHING_ENGINE_H


typedef struct punching_info {
    const char * host;
    const int port;
    const int range;
} punching_info;

//send to each of the ports in the range and wait for a message
void * start_sending(const punching_info * info);
//create all ports in the range and listen on them
void * start_listening(const punching_info * info);
//starts up threads to begin punch
void start_punch(const punching_info * info);
#endif
