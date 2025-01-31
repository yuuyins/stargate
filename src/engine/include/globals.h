#ifndef SG_GLOBALS_H
#define SG_GLOBALS_H

// 0 when the application is running, 1 after the shutdown sequence has begun
extern volatile int exiting;
// Override the hardware config to force a single thread
extern int SINGLE_THREAD;
extern int UI_SEND_USLEEP;

#endif
