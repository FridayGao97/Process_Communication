# define _GNU_SOURCE
# include <signal.h>
# include <stdlib.h>
# include <unistd.h>
# include <limits.h>
# include <string.h>
# include <stdio.h>
# include <errno.h>
# include <time.h>
# include <sys/time.h>

pid_t otherpid;
pid_t mypid;
int typing;
//string to binary;
char *stringToBinary(char *str)
{
    if (str == NULL) {
        return NULL;
    }
    // get length of string without NUL
    size_t len = strlen(str);

    
    // allocate "slen" (number of characters in string without NUL)
    // times the number of bits in a "char" plus one byte for the NUL
    // at the end of the return value
    errno = 0;
    char *bin = malloc(len * 8 + 1);
    if(bin == NULL){
        fprintf(stderr,"malloc has failed in stringToBinary(%s): %s\n",str, strerror(errno));
        return NULL;
    }
    // receive nothing
    if (len == 0) {
        *bin = '\0';
        return bin;
    }
    //temp char for checking
    char *ptr;
    // store the beginning position
    char *start = bin;
    int i;

    // iterate the each characters
    for (ptr = str; *ptr != '\0'; ptr++) {
        // iterate each bit, perform bitwise AND for every bit of the character
        for (i = 7; i >= 0; i--, bin++) {
            *bin = (*ptr & 1 << i) ? '1' : '0';
        }
    }
    // finalize return value
    *bin = '\0';
    // reset pointer to beginning
    return start;
}

#ifdef SINGLE
//this is for single signal when SINGLE is defined in makefile

//the start time of sending signal
long start = 0;
//buffer to store bits
char buf_out[20];
//store the character
char out[4096];
//current position for bits
int curr = 0;
//current position for character
int post = 0;
//for getting time
struct timeval  tv;
//using difference in the length of time between kill signal, to identify 0 or 1(shorter for 0, longer for 1)
void SigHandler(int signo) {
    typing =1;
    //get current time
    gettimeofday(&tv, NULL);

    //first time get the signal,
    if(start == 0){
        //start time in millisenconds
        start = (tv.tv_sec) * 1000000 + (tv.tv_usec);
    }
    //sencond time get kill signal
    else{
        long now = (tv.tv_sec) * 1000000 + (tv.tv_usec);
        long diff = now - start;
        //compare the time difference between two kill signal
        if (diff < 8000 ){
            //shorter for '0'
            buf_out[curr] = '0';
        }
        else if (diff < 13000){
            //longer for '1'
            buf_out[curr] = '1';
        }
        //reset for next bit
        start = 0;
        //increase bit position for next bit
        curr++;
    }

    //check error if it is out of ascii code range(0~127)
    long int check = strtol(buf_out, 0, 2);
    if(check > 127 || check < 0){
        printf("? %c\n", (char)check);
        curr =0;
        post = 0;
    }
    
    //each character has 8 bits
    if(curr!= 0 && curr % 8 == 0){
        //convert 8 bits to a char
        char sub = strtol(buf_out, 0, 2);
        int endp = strtol(buf_out, 0, 2);
        //store the char
        out[post] = sub;
        //reset for next char
        post ++;
        curr =0;
        //clean out the buffer which store the bits
        memset(buf_out, 0, sizeof(buf_out));
        //checking by the end character, if the ascii code char to integer is 24, which is the special character we designed to receive
        if(endp == 24){
            //check the second last whether it is terminal character
            if(out[post -2] != '\0'){
                //if no, something wrong leads we did not receive the terminal character
                post = 0;
                printf("? %s\n", out);
                memset(out, 0, sizeof(out));
                return;
            }
            //if yes no error occuring
            else{
                out[post -2] = '\0';
                printf("! %s", out);
                typing =0;
            }
            memset(out, 0, sizeof(out));
            post = 0;      
        }    
    }
    return;
}

//sending the bits which is converted by characters reading from input.
void send(){
    printf("*System: You are entering Single Signal Mode.\n");
    //get my own pid
    mypid = getpid();
    printf("Own PID: %d\n", mypid);
    //get the target pid
    scanf("%d \n", &otherpid);
    //get the input and store into buffer
    char buf_in[4096];
    //special character for checking " Dec: 24; Bin: 00011000; Symbol:CAN; Description:Cancel"
    char Can[] = {'0','0','0','1','1','0','0','0'};
    while(1){
        fgets(buf_in, 4096, stdin);
        // handle the interleaving of input and output, holding input until last output prints.
        while (typing != 0){ ; }
        //convert characters to binary
        char *out = stringToBinary(buf_in);
        //clean out the buffer store characters
        memset(buf_in, 0, sizeof(buf_in));
        //get the size of bits, be used for sending
        size_t len = strlen(out);
        int slen = (int)len;
        
        int i;
        for (i = 0;i<slen;i++){
            //sending each bit
            if (out[i] == '1'){
                //'1' will seperate by 10 ms.
                kill(otherpid,SIGUSR1);
                usleep(10000);
                kill(otherpid, SIGUSR1);
                usleep(25000);
            }
            else{
                //'0' seperate by 5 ms
                kill(otherpid, SIGUSR1);
                usleep(5000);
                kill(otherpid, SIGUSR1);
                usleep(30000);
            }           
        }

        //send the checking character '0' == '00000000'
        for (i=0; i<8; i++){
            kill(otherpid, SIGUSR1);
            usleep(5000);
            kill(otherpid, SIGUSR1);
            usleep(30000);
            //printf("send: 0|");
            
        }

        for (i=0; i<8; i++){
            //sending each bit of special character.
            if (Can[i] == '1'){
                //'1' will seperate by 10 ms.
                kill(otherpid,SIGUSR1);
                usleep(10000);
                kill(otherpid, SIGUSR1);
                usleep(25000);
            }
            else{
                //'0' seperate by 5 ms
                kill(otherpid, SIGUSR1);
                usleep(5000);
                kill(otherpid, SIGUSR1);
                usleep(30000);
            }
        }

        //free the mememory and clean the buffer
        free(out);
        memset(buf_in, 0, sizeof(buf_in));       
    }
    return;
}

#else 
//Double Signal
char buf_out[20];
char out[4096];
int curr = 0;
int post = 0;
void SigHandler(int signo) {
    typing = 1;
    //kill SIGUSR1 to send '1'
    if (signo == SIGUSR1){
        buf_out[curr] = '1';
        
    }
    //kill SIGUSR2 to send '0'
    else if (signo == SIGUSR2){
        buf_out[curr] = '0';
        
    }
    //for next bit
    curr++;

    //check error if it is out of ascii code range
    long int check = strtol(buf_out, 0, 2);
    if(check > 127 || check < 0){
        printf("?:error: %c\n", (char)check);
        curr =0;
        post = 0;
        memset(buf_out, 0, sizeof(buf_out));
        return;
    }
    //convert and check terminal character for every 8 bits.
    if(curr!= 0 && curr % 8 == 0){
        char sub = strtol(buf_out, 0, 2);
        int endp = strtol(buf_out, 0, 2);
        out[post] = sub;
        post ++;
        curr =0;
        memset(buf_out, 0, sizeof(buf_out));

        if(endp == 24){
            if(out[post -2] != '\0'){
                post = 0;
                printf("? %s\n", out);
                memset(out, 0, sizeof(out));
                return;
            }
            else{
                out[post -2] = '\0';
                printf("! %s", out);
                typing = 0;
            }
            memset(out, 0, sizeof(out));
            post = 0;      
        } 
    }
    return;
}
//send via two signals
void send(){
    printf("*System: You are entering Double Signal Mode.\n");
    //get my own pid
    mypid = getpid();
    printf("Own PID: %d\n", mypid);
    //get the target pid
    scanf("%d \n", &otherpid);
    char buf_in[4096];
    //special character for checking " Dec: 24; Bin: 00011000; Symbol:CAN; Description:Cancel"
    char Can[] = {'0','0','0','1','1','0','0','0'};
    char *out;
    while(1){
        fgets(buf_in, 4096, stdin);
        // handle the interleaving of input and output
        while (typing != 0){ ; }
        out = stringToBinary(buf_in);
        memset(buf_in, 0, sizeof(buf_in));
        size_t len = strlen(out);
        int slen = (int)len;

        int i; 
        for (i = 0;i<slen;i++){
            if (out[i] == '1'){
                //SIGUSR1 for '1'
                kill(otherpid,SIGUSR1);
            }
            else {
                //SIGUSR2 for '0'
                kill(otherpid, SIGUSR2);
            }
            usleep(5000);
        }
        //sending '\0'
        for (i=0; i<8; i++){
            kill(otherpid, SIGUSR2);
            usleep(5000);
        }

        for (i=0; i<8; i++){
            if (Can[i] == '1'){
                //SIGUSR1 for '1'
                kill(otherpid,SIGUSR1);
            }
            else{
                //SIGUSR2 for '0'
                kill(otherpid, SIGUSR2);
            }
            usleep(5000);
        }

        free(out);
        memset(buf_in, 0, sizeof(buf_in));

    }
    return;
}
#endif


int main(int argc, char *argv[]){
    //build signal
    struct sigaction act;
    act.sa_handler = SigHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    //checking signal error
    if (sigaction(SIGUSR1, &act, NULL) < 0 ||
        sigaction(SIGUSR2, &act, NULL) < 0 ||
        0){
        printf("sigaction error!\n");
        return 1;
    }

    //begin to talk
    send();
    
    return 0;
}



