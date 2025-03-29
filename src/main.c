#include "bsp_timer.h"
#include "bsp_uart_fifo.h"
#include <stdint.h>

#include "relay.h"
#include "air780e.h"

#define VERSION "0.1"

enum State {
    WAITING_FOR_CMT,
    CMT_RECEIVED,
} state;

#define RELAY 0

// Generic buffer
enum { bufsize = 512 };
static uint8_t buf[bufsize];
static uint8_t buf2[bufsize];

// Line buffer
enum { maxline = 511 }; 
char line[maxline+1];

// Function to decode a 7-bit packed PDU
void decode_7bit_pdu(const uint8_t *input, size_t input_len, char *output) {
    size_t output_index = 0;
    uint8_t carry_bits = 0;
    int carry_count = 0;

    for (size_t i = 0; i < input_len; i++) {
        uint8_t current_byte = input[i];

        // Extract the 7 bits from the current byte and carry bits
        output[output_index++] = ((current_byte << carry_count) | carry_bits) & 0x7F;

        // Update carry bits and carry count
        carry_bits = current_byte >> (7 - carry_count);
        carry_count++;

        // If carry count reaches 7, reset it and output the carry bits
        if (carry_count == 7) {
            output[output_index++] = carry_bits & 0x7F;
            carry_bits = 0;
            carry_count = 0;
        }
    }

    // Null-terminate the output string
    output[output_index] = '\0';
}

void send(char *msg) {
    comSendBuf(COM2, (uint8_t*)msg, strlen(msg));
    bsp_DelayMS(100);
}

void recv() {
    uint8_t read;
    uint32_t pos = 0;
    bsp_DelayMS(100);
    while(comGetChar(COM2,&read)) {
        if (pos < bufsize - 1) {
            buf[pos++] = read;
        }
    }
    buf[pos] = 0;
}

int hexdigit(char c) {    
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        return -1; // Invalid hex digit
    }
}

uint8_t parse_address(uint8_t *buf, char *address) {
    int outpos = 0;

    uint8_t len = buf[0];
    uint8_t t = buf[1];
    if (t == 0x91) {
        // International number
        address[outpos++] = '+';
    }    
    int i = 0;
    uint8_t *p = (uint8_t*)&buf[2];
    while (i < len) {
        uint8_t b = *(p++);
        uint8_t hi = b >> 4;
        uint8_t lo = b & 0x0F;
        if (i < len) {
            address[outpos++] = lo + '0';
            i++;
        }
        if (i < len) {
            address[outpos++] = hi + '0';
            i++;
        }
    }
    address[outpos] = 0;
    return (len+1)/2 + 2;

}

void send_sms(char *oa, char *msg) {
    send("AT+CMGF=1\r"); // Set SMS mode to text
    recv();


    snprintf((char*)buf, sizeof(buf), "AT+CMGS=\"%s\"\r", oa);
    printf("Sending: %s",buf);
    bsp_DelayMS(100);
    send(buf);
    recv();

    send((uint8_t*)msg);
    send("\x1a"); // Ctrl-Z to send the message
    recv();

    send("AT+CMGF=0\r"); // And back to PDU
    recv();

}


void send_status(char *oa) {    
    char msg[100];
    snprintf(msg, sizeof(msg), "Das Geraet ist %s", relay_state(RELAY) ? "eingeschaltet." : "ausgeschaltet."); // Too stupid to get the encoding right...
    send_sms(oa, msg);
}

void handle_message(char *oa, char *msg) {
    printf("Message from %s: %s\r\n", oa, msg);
    bsp_DelayMS(100);

    if (strcasecmp(msg, "ein") == 0) {
        relay_switch(RELAY, true);
        send_status(oa);
    } else if (strcasecmp(msg, "aus") == 0) {
        relay_switch(RELAY, false);
        send_status(oa);
    } else if (strcasecmp(msg, "status") == 0) {
        send_status(oa);
    } else {
        send_sms(oa, "Unbekannter Befehl. Befehle sind: ein, aus, status");
    }
}

void process_pdu() {
    int i = 0;
    int j = 0;
    while (i < strlen(line)) {
        uint8_t hi = hexdigit(line[i++]);
        uint8_t lo = hexdigit(line[i++]);
        buf[j++] = hi << 4 | lo;
    }

    // parse PDU
    // https://www.gsmfavorites.com/documents/sms/packetformat/ 

    int pos = 0;
    // skip SCA information, we don't care

    pos += buf[pos] + 1; // skip SCA information (+1 for length byte)
    uint8_t pdu_type = buf[pos++];

    char oa[20];
    pos += parse_address(&buf[pos], oa);
    uint8_t pid = buf[pos++];
    uint8_t dcs = buf[pos++];
    pos += 7; // skip SCTS
    uint8_t udl = buf[pos++];


    memset(buf2, 0, bufsize);
    decode_7bit_pdu(&buf[pos], udl, (char*)buf2);

    handle_message(oa, (char*)buf2);
}

void process_line() {
    printf("line received: %s\r\n", line);
    bsp_DelayMS(100);
    printf("---\r\n");
    bsp_DelayMS(100);

    switch(state) {
        case WAITING_FOR_CMT:        
            if (strncmp(line, "+CMT:", 5) == 0) {                
                state = CMT_RECEIVED;
            }
            break;
        case CMT_RECEIVED:
            process_pdu();
            state = WAITING_FOR_CMT;
            break;
    }
}

int main(void)
{

    state = WAITING_FOR_CMT;

    bsp_Init();		/* Hardware initialization */
    relay_init();

    bsp_DelayMS(100);

    printf("Welcome to 4g_relay %s, built on %s %s\r\n", VERSION, __DATE__, __TIME__);
    printf("Copyright (c) 2025 Andreas Signer <asigner@gmail.com>\r\n");

    // Air780e_Online();
    // Air780e_NITZ();
    // Air780e_Online();
   
    Air780e_NITZ();
   
    send("AT+CMGF=0\r"); // Set PDU mode
    
    bsp_StartAutoTimer(0, 1000); /* Start a 1-second auto-reloading timer */

    /* Enter the main program loop */
    int pos = 0;
    while (1)
    {
        bsp_Idle();		/* This function is in bsp.c file. Users can modify this function to implement CPU sleep and watchdog feeding */

        /* Check if the timer timeout has occurred */
        if (bsp_CheckTimer(0)) {
            // Maybe at some point we will want to do something here?
        }        
       
        uint8_t buf;
        // if(	comGetChar(COM2,&buf)) {
        //     comSendChar(COM1, buf);            
        // }
        // continue;

        if(	comGetChar(COM2,&buf)) {
            switch(buf) {                
            case '\r':
                // ignore
                break;
            case '\n':
                // Line finished!
                line[pos] = 0;
                process_line();
                pos = 0;
                break;
            default:
                if (pos < maxline) {
                    line[pos++] = buf;
                }
            }
        }

    }
}

