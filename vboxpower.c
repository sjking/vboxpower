// Purpose: used to stop all running VMs with the acpipowerbutton
// Date: Nov 4, 2013
//
// note: compile with the -gnu=c99 flag

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

const int BUF_SIZE = 256;
const char SEPARATOR = ':';
//const char *STOP_SCRIPT = "/usr/local/bin/stopvm.sh";
//const char *START_SCRIPT = "/usr/local/bin/startvm.sh";
const char *STOP_SCRIPT = "/home/vboxuser/bin/vboxpower/stopvm.sh";
const char *START_SCRIPT = "/home/vboxuser/bin/vboxpower/startvm.sh";

// return buffer containing contents of command, and length of the string
void getInfoVMs( char* buf, bool on ); 
void executePower( char* vms, bool on ); // shutdown/start the VMs called in the vms list string
void getVMs( char* buf, char* vms ); // returns ':' seperated list of running VMs in vms

int main( int argc, char *argv[] )
{
    if (argc != 2) {
        printf("Usage: %s on|off\n", argv[0]);
        return 1;
    }
    char *cmd = argv[1];
    char buf[BUF_SIZE];
    char vms[BUF_SIZE];
    bool on = false;
    bool off = false;

    if ( strcmp(cmd, "on") == 0 ) {
        getInfoVMs( buf, true );
        on = true;
    }
    else if ( strcmp(cmd, "off") == 0 ) {
        getInfoVMs( buf, false );
        off = true;
    }
    else {  
        printf("Usage: %s on|off\n", argv[0]);
        return 1;
    }

    // shutdown or power up the VMs
    if ( buf != NULL ) {
        getVMs( buf, vms );
        if (on) {
            executePower( vms, true );
//            printf("\nvms: %s\n", vms);
        }
        if (off) {
            executePower( vms, false );
//            printf("vms: %s\n", vms);
        }
    }

    return 0;
}

void executePower( char* vms, bool on )
{
    pid_t pid, wpid;
    int status;
    char *token;
    const char *DELIM = ":";

    // call the script to poweroff each VM as a separate process
    while ( (token = strsep(&vms, DELIM) ) != NULL) {
//        printf("token: *%s*\n", token);
        pid = fork();
        if (pid == -1) {
            perror("Cannot fork");
        }
        else if (pid == 0) {
            // child process, execute script
            if (on) {
                if ( execl(START_SCRIPT, START_SCRIPT,  token, (char *)0) == -1 ) {
                    char *err = strcat("Cannot execute script ", START_SCRIPT);
                    perror(err);
                }
                else {
                    printf("Starting VM \"%s\"\n", token);
                }
            }
            else {
                if ( execl(STOP_SCRIPT, STOP_SCRIPT,  token, (char *)0) == -1 ) {
                    char *err = strcat("Cannot execute script ", STOP_SCRIPT);
                    perror(err);
                }
                else {
                    printf("Stopping VM \"%s\"\n", token);
                }
            }
            break;
        }
    } 
    while ((wpid = wait(&status)) > 0) {  // keep waiting for all children
        char *msg = on ? "on." : "off.";
        if (token != NULL) printf("VM \"%s\" has been powered %s", token, msg);
    }  
}

void getVMs( char* buf, char* vms ) 
{
    size_t len = strlen(buf);
    bool word = false; // vms are listed in quotation marks: "name_of_vm"
    int j = 0;

    for (int i = 0; i < len; i++) {
        if (buf[i] == '\"') {
            if (!word) { // start of vm name string
                word = true;
                ++i;
            }
            else { // end of vm name string
                word = false;
                vms[j] = SEPARATOR;
                ++j;
            }
        }
        if (word) {
            vms[j] = buf[i];
            ++j;
        }
    }
    vms[j-1] = '\0';
}
        
void getInfoVMs( char* buf, bool on )
{
    FILE *in = NULL;
    extern FILE *popen();//, *pclose();
    char pipebuf[BUF_SIZE];
    int i = 0;
    memset(&buf[0], 0, sizeof(buf)); // clear the buffer buf

    // create a pipe to read output of command listing running VMs
    if (on) {
        if ( !(in = popen("VBoxManage list vms", "r")) )
            perror("Error executing command"); // error, could not open pipe
        else {
            while (fgets(pipebuf, sizeof(pipebuf), in) != NULL ) {
                strcat(buf, pipebuf);
            }
        }
        pclose(in);
    }
    else {
        if ( !(in = popen("VBoxManage list runningvms", "r")) )
            perror("Error executing command"); // error, could not open pipe
        else {
            while (fgets(pipebuf, sizeof(pipebuf), in) != NULL ) {
                strcat(buf, pipebuf);
            }
        }
        pclose(in);
    }
}
