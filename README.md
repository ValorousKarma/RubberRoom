# RubberRoom
Following guides to learn the basics of building malware in C, for educational purposes.

This respository will contain code that can function as a Remote Access Trojan (RAT) & keylogger.

I am new to ethical malware development, any help is greatly appreciated.

# Goals (Initial course)
- [x] Main function that establishes a connection with a server 
- [x] Shell function that waits for commands and iterates over options
- [x] ~~automatically start client program on reboot~~
- [x] ~~spawn other programs on client machine~~
- [x] ability to remotely navigate through client directories
- [x] ~~implement keylogger in client program~~

# NEW Goals
- [ ] re-write keylogger myself
- [ ] Allow remote script execution
- [ ] Reduce keylogger detectability
- [ ] Handle multiple client connections simultaneously
- [x] bypass anti-virus

# Compilation Process

```
i686-w64-mingw32-gcc -o RubberRat.exe backdoor.c -lwsock32 -lwininet -mwindows -O5

gcc -o ServerName server.c
```
