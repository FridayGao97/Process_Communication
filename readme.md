Description:
This projects introduced a form of very crude IPC, Unix signals, in this case I apply signals to create a channel 
for communicating messages between two processes. The two processes are running same executable code with two version.
One(the default) uses two signals (SIGUSR1 and SIGUSR2) and another (conditionally compiled as -DSINGLE) uses only onesignal (SIGUSR1)
-----------------------------------------------------------------------------------------------------------------------
Acknowledgement:
1. I have recieved help from TAs during the lab session
2. I have learned some knowledge about signal and consulted the string to binary conversion from stackoverflow: https://stackoverflow.com/questions/41384262/convert-string-to-binary-in-c
3. I have read the book named Advanced Programming in the UNIX Environment
-----------------------------------------------------------------------------------------------------------------------
Instruction to run the code:
1. Go to the correct directory of the c program.
2. Use make to compile the code to produce the executable for two signals as default.
3. Or use make covertsigs.double to produce the executable for two signals, just type "make covertsigs.double".
4. Or use make covertsigs.single to produce the executable for one signal, just type "make covertsigs.single".
5. Make clean will clear the executable.
-----------------------------------------------------------------------------------------------------------------------
Design Decision:
1. For the one signal option(only SIGUSR1 used), I first of all encode the input message into binary. Then we use different time intervals between two kills to identify 0 or 1 for one bit. If the time interval between two kills is less than 8000, we recieved 0. If the time difference is greater than 8000 and less than 13000, we say it is 1. Then decode the binary to the output message.
2. For two signals option(both SIGUSR1 and SIGUSR2 used), I simply define 1 as SIGUSR1 and 0 as SIGUSR2 in order to send the binary representation to the handler. Inside handler, we check whether the SIGUSR is 1 or 2 and decode the 
binary into char.
3. When the sender has finished their message and pressed enter. The send function will send the message by signals through binary representation. One char has 8 bits. At the end, we additionally add a specified character with symbol "CAN", 00011000 in binary. The signal handler will check each 8 bits binary and decode tham into char. If it detect the char is "CAN", which means that we reach the end of message, so we print out the characters that has been decoded before this defined termination char.
4. I send all character including the terminal character of the user input. Normally, there will be 8 bits of '0' in the end of line, which stand for the teminal characte, sending to the reciever. Then we continue to send one more fixed special characte to indicate the end of input message. In error checking, if we read the fixed special character, then we read the character before this special character. If that character is 8 bit of '0', it means all pervious character are in the same phase as sending and send completely. In addional, we get the integer of character and compare with the range of ascci code (0~127). If this int is not in the range, this will be an error message. 
5. Some unexpected error rarely happend, so I also check the message characters belong to ascci code.
6. To handle the interleaving of input/output, I will keep user input in buffer to wait until the output finish, then proceeding to sending part. we will check the status of input, if output do nothing we will get input.If we are receiving, we will wait to receive first, then send the input message.
7. There is no any other signal that I had to use for completing the design requirement. 
8. I minimize the if and while statement to ensure that do not waste the CPU cycles, further we dont set very long usleep time.

-----------------------------------------------------------------------------------------------------------------------
Testing:
I have tested both of Single & Double signal version by our own pc. Also, our code was tested (and running) on the official VM as well as on the physical hosts in lab CSC 219. 
For the performance of the double signals: around 5ms/bit.
For the performance of the single singnals: around 35ms/bit.
And I tried to send 80 characters, it only took less than 30 seconds for single signal.
