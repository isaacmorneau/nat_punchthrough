#ifndef PUNCHING_ENGINE_H
#define PUNCHING_ENGINE_H

//create all ports in the range and listen on them
void start_listening(const int port, const int range);

//send to each of the ports in the range and wait for a message
void start_sending(const char * host, const int port, const int range);

#endif
