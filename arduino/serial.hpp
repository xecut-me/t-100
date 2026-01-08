#ifndef __SERIAL_HPP__
#define __SERIAL_HPP__

// Delays and width
#define LINE_WIDTH 69             // FIXME - check if it is correct
#define CARRIAGE_RETURN_DELAY 500 // in milliseconds
#define LINE_FEED_DELAY 500       // in milliseconds
//

void handleTTYtx();
void handleTTYrx();
void handleNetworkRX(uint8_t byte);

#endif // __SERIAL_HPP__
