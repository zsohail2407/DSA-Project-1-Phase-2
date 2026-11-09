// ======================= TIME-TRAVEL DEBUGGER - CLIENT TEMPLATE =======================

// The client never talks to the server directly. It only talks to the load balancer.
// The load balancer wakes the server up if it is down, so the client does not care.
// The first "start" may just take a bit longer.
//
// The network is used ONLY by "start". After the .tdbg file is downloaded,
// next / prv / jmp read that file on the local disk. They never contact the load balancer.
//
// How the pieces fit together:
//   main loop -> readCommand -> runCommand
//   start -> cmdStart()  -> requestDebugFile() (Part A) -> openTdbg() (Part B) -> shows step 1
//   next, prv, jmp -> moveBy() (Part D) -> readSnapshotAt() (Part B) -> printSnapshot() (Part E)

#include <iostream>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <sys/socket.h>
#include <sstream>
#include "helper.h"
using namespace std;

// ---- Constants ----
const char *PROGRAM_NAME = "ttdb";
const char *PROMPT = "ttdb> ";
const int IO_BUFFER_SIZE = 64 * 1024;     // fixed buffer for streaming files to and from the socket
const char *TDBG_FILE = "ttdb_last.tdbg"; // where the downloaded .tdbg is saved
const int MAX_JUMP = 1000000000;          // biggest n accepted by jmp
string LB_HOST = "127.0.0.0";
int LB_PORT = 5000;

/*
// These two must be the same as in Phase 1

const int MAX_VARS_PER_FRAME = 16;
const int MAX_STACK_DEPTH = 64;


// ---- Structs (same as Phase 1, so the file can be read back exactly as it was written) ----
struct Variable
{
    string name;
    int32_t value;
};
struct Frame
{
    string func_name;
    int32_t argc;
    Variable argv[MAX_VARS_PER_FRAME];
    int32_t returnLine;
    Variable locals[MAX_VARS_PER_FRAME];
    int32_t localCount;
};
struct Snapshot
{
    Frame callStack[MAX_STACK_DEPTH];
    int32_t stackDepth;
};
struct TTDBHeader 
{
    char magic[4]; // "TTDB"
    int32_t version;
    int32_t stepCount;
    int64_t indexOffset;
};*/

// ---- Structs only the client needs ----
struct TdbgReader
{
    ssize_t file_index;
    TTDBHeader header;
    vector<int64_t> index;
};
struct DebugSession
{
    int active_flag;     // false until a "start" has worked
    TdbgReader reader;   // the .tdbg file stays open for the whole session
    int64_t currentStep; 
};

// =============================================================================================
// PART A: TALKING TO THE LOAD BALANCER
// =============================================================================================

bool recvAll(int sock, char *buf, size_t n)
{
    // recv() can return fewer bytes than asked for, so keep calling it until exactly n bytes are read.
    // Return false if the connection closes or fails before n bytes arrive.
}
bool sendAll(int sock, const char *buf, size_t n)
{
    // send() can send fewer bytes than asked for, so keep calling it until all n bytes are sent.
    // Use the flag MSG_NOSIGNAL. Return false if the connection fails.
}

int connectToHost(const char *host, int port)
{
    // Create a TCP socket and connect it to host:port.
    // Return the socket, or -1 if the connection fails.
}
bool uploadSource(int sock, const char *sourcePath)
{
    // have to do checksum and then send
    // When the file is finished, call shutdown(sock, SHUT_WR). This is how the server knows the string is complete.
    // Return false if the file cannot be opened or any send fails.
}
bool receiveResponse(int sock, const char *outTdbgPath, string &outError)
{
    // Reply on the wire: [status][payload][CRC-32]. The CRC covers the status byte too.
    // Feed every received byte (status included) to a TrailerChecker.
    // Status 1: collect the payload into outError. Status 0: write the payload to the file.
    // At the end call verify(). If it fails, delete the file, set outError to
    // "checksum mismatch, response corrupted" and return false.
}

// =============================================================================================
// PART B: READING THE .tdbg FILE
// File layout (written by Phase 1):
//   header | snapshot 0 | snapshot 1 | ... | index (one 8 byte offset per snapshot)
// The index lets us jump straight to any step without reading the steps before it.
// =============================================================================================

bool openTdbg(const char *path, TdbgReader &r)
{
    FILE *fl=fopen(path,"rb");
    if(!fl) return false;
    fseek(fl,0, SEEK_END);

    long size=ftell(fl);

    fseek(fl,0,SEEK_SET);
    TTDBHeader head;
    if(fread(head.magic,1,4,fl)!=4 || fread(&head.version,1,4,fl)!=4
|| fread(&head.stepCount,1,4,fl)!=4 || fread(&head.indexOffset,1,8,fl)!=8)
    {
        fclose(fl);
        return false;
    }
    //data check
    if(head.version!=1 || head.stepCount<0)
    {
        fclose(fl);
        return false;
    }
    //file name
    if(head.magic[0]!= 'T' || head.magic[1]!= 'T'|| head.magic[2]!= 'D'||
    head.magic[3]!= 'B')
    {
        fclose(fl);
        return false;
    }
    // r ki validity
    vector<int64_t> idddd(head.stepCount);
    fseek(fl,head.indexOffset,SEEK_SET);
    if(fread(idddd.data(),8,head.stepCount,fl)!= size_t(head.stepCount))
{
    fclose(fl);
    return false;
}
r.header=head;
r.index=idddd;
fclose(fl);
return true;
} 
void closeTdbg(TdbgReader &r)
{
    r.index.clear();
    r.header.stepCount=0;
    r.header.indexOffset=0;
    r.file_index=-1;
    // Free r.index and close the file.
}
Snapshot *readSnapshotAt(TdbgReader &r, int32_t step)
{
}

// =============================================================================================
// PART C: THE SESSION IN MEMORY
// There is one DebugSession variable, created in main() and passed to every command by reference.
// =============================================================================================

void initSession(DebugSession &s)
{
   s.active_flag=0;
   s.currentStep=0;
   s.reader.file_index=-1;
   s.reader.index.clear();
   s.reader.header.stepCount=0;
}
void closeSession(DebugSession &s)
{
    // If active, call closeTdbg(s.reader). Then set active = false.
    //assuming keh false is 0, the rest is true
    if(s.active_flag)
    {
        closeTdbg(s.reader);
        s.active_flag=0;
        s.currentStep=0;
    }
    // Called when the user types quit and when the input ends.
}

// =============================================================================================
// PART D: THE COMMANDS
// =============================================================================================

void printSnapshot(int32_t step, int32_t totalSteps, const Snapshot &s); // defined in Part E

bool showCurrent(DebugSession &s)
{
    if(s.active_flag==0)
    {
        cout<<"No session"<<endl;
        return false;
    }
    Snapshot* temp= readSnapshotAt(s.reader,s.currentStep);
    if(temp==nullptr)
    {
        cout<<"damaged .tdbg file"<<endl;
        return false;
    }
    printSnapshot(s.currentStep+1,s.reader.header.stepCount,*temp);
    delete temp;
    return true;
}
bool moveBy(DebugSession &s, int32_t delta)
{//delta is how far you are moving
    if(s.active_flag==0)
    {
        cout<<"No session"<<endl;
        return false;
    }
    int res=s.currentStep+delta;
    if(res>=s.reader.header.stepCount)
    {
        cout<<"at last step"<<endl;
        return false;
    }
    if(res<0)
    {
         cout<<"at first step"<<endl;
        return false;
    }
    int64_t oldstep=s.currentStep;
    s.currentStep=res;
    if(!showCurrent(s))
    {
      s.currentStep=oldstep;
      return false;
    }
    return true;
}
bool cmdStart(DebugSession &s, const string &sourcePath)
{
    // 1. requestDebugFile with host and port, saving to TDBG_FILE.
    // 2. If it fails, print the error text and return false. Keep the old session as it was.
    string error;
    //if(requestDebugFile(sourcePath,error))
    //{
        cout<<error<<endl;
        return false;
    //}
    // 3. Open the new file with openTdbg into a NEW local TdbgReader.
    TdbgReader n;
    n.file_index=-1;
    if(!openTdbg(TDBG_FILE,n))
    {
        cout<<"server sent a damaged .tdbg file"<<endl;
        return false;
    }
    //    If that fails, print "server sent a damaged .tdbg file", keep the old session, return false.
    // 4. Only now close the old reader (if s.active), then put the new reader into s,
    if(s.active_flag)
    {
        closeTdbg(s.reader);
    }
    s.reader=n;
    //    set s.currentStep = 0 and s.active = true.
    s.currentStep = 0;
    s.active_flag = true;
    // 5. Show step 1 with showCurrent(s).
    return showCurrent(s);
}
bool cmdNext(DebugSession &s) { return moveBy(s,1);}
bool cmdPrv(DebugSession &s) {return moveBy(s,-1);}
bool cmdJmpNext(DebugSession &s, int32_t n) {return moveBy(s,n);}
bool cmdJmpPrv(DebugSession &s, int32_t n) { return moveBy(s,-n);}

// =============================================================================================
// PART E: PRINTING
// =============================================================================================

void printSnapshot(int32_t step, int32_t totalSteps, const Snapshot &s)
{
    //func name, steps, main
    cout<<step<<" of "<< totalSteps<<endl;
    for(int i=0;i<s.stackDepth;i++)
    {
    Frame f=s.callStack[i];
    cout<<f.func_name;
    if(i==0)
    {
        cout<<" curr ";
    }
    //Frame:func_name, argc,argv[16],returnLine,locals[16],local count
    if(f.argc==0 && f.localCount==0)
    {
        cout<<" Empty(no vars)";
    }
    //argv
    if(f.argc>0)
    {
        cout<<" arguments: ";
        for(int idx=0;idx<f.argc;idx++)
        {
            cout<< " "<<f.argv[idx].name<< "="<<f.argv[idx].value<<" ";
        }
    }
    //locals
    if(f.localCount>0)
    {
        cout<<" locals: ";
        for(int idx=0;idx<f.localCount;idx++)
        {
            cout<< " "<<f.locals[idx].name<< "="<<f.locals[idx].value<<" ";
        }
    }
    cout<<endl;
    }
}
void printHelp()
{
    // Print the commands
    cout<<"Commands:"<<endl;
    cout<<"start<file>  : Upload a C-- program, then record"<<endl;
    cout<<"next  : Move one step forward"<<endl;
    cout<<"prv  : Move one step backwards"<<endl;
    cout<<"jmp next <n>  : jump n steps forward"<<endl;
    cout<<"jmp prv <n>  : jump n steps backwards"<<endl;
    cout<<"help  : view/show this list"<<endl;
    cout<<"quit, exit  : leave"<<endl; 
}

// =============================================================================================
// PART F: THE INFINITE LOOP
// =============================================================================================

bool runCommand(DebugSession &s, const string &line, bool &quitRequested)
{
    // 1. Split the line into words.
    stringstream ss(line);
    string word;
    vector<string> words;
    while(ss>>word)
    {
        words.push_back(word);
    }
    if(words.empty())
    {
        return true;
    }
    // 2. If the first word is "ttdb", drop it.
    if(words[0]=="ttdb")
    {
       words.erase(words.begin());
       if(words.empty())
       {
        return true;
       }
    }
    // 3. Match the first word:
    //      start      -> needs a path after it, then cmdStart
    if(words[0]=="start"&& words.size()==2)
    {
        cmdStart(s,words[1]);
         return true;
    }
    //      next       -> no more words, then cmdNext
    if(words[0]=="next"&& words.size()==1)
    {
        cmdNext(s);
         return true;
    }
    //      prv        -> no more words, then cmdPrv
    if(words[0]=="prv" && words.size()==1)
    {
        cmdPrv(s);
         return true;
    }
    //      jmp        -> second word "next" or "prv", third word a positive number ,
    //                    then cmdJmpNext or cmdJmpPrv


    if(words[0]=="jmp") //wrong: idk how to do it, check again
    {
        if(words.size()==3)
        {
         return false;
        }
        int n=stoi(words[2]);
        if(n>0)
        {
        if(n<1)
        {
            cout<<"n should be atleast 1 and it cant be neg or 0"<<endl;
            return false;
        }
        if(words[1]=="next")
        {
          cmdJmpNext(s,n);
           return true;
        }
        else if(words[1]=="prv")
        {
          cmdJmpPrv(s,n);
           return true;
        }
        else{
            cout<<"Unknown Command"<<endl;
             return false;
        }
    }
    }
    //      help       -> printHelp
    if(words[0]=="help" && words.size()==1)
    {
        printHelp();
         return true;
    }
    //      quit, exit -> set quitRequested = true
    if(words[0]=="quit" || words[0]=="exit")
    {
        if(words.size()==1)
        {
        quitRequested = true;
         return true;
        }
    }
    cout<<"Unknown Command"<<endl;
    printHelp();
    return false;
    // 4. For anything else, or a wrong number of words, print "unknown command" and the help text, and return false.
}
int main(int argc, char *argv[])
{
    // you get string LB_HOST and int LB_PORT = 0 in arguments of this main function.
    
    
    // main host port
    if(argc==3)
    {
        LB_HOST=argv[1];
        LB_PORT=atoi(argv[2]);
    }
    DebugSession session;
    initSession(session);

    // Print a short welcome and the help text.
    cout<<"Welcome to the Time Travel Debugger"<<endl;
    printHelp();
    string aaaa;
    bool q=false;
    while(true)
    {
        cout<<PROMPT;
        if(!getline(cin,aaaa))
        {
            break;
        }
        runCommand(session,aaaa,q);
        if(q)
        {
            break;
        }
    }
    closeSession(session);
    return 0;
    //
    // Infinite loop:
    //   if quitRequested, break
    //
    // After the loop: closeSession(session) and return 0.
    // Never exit the loop because of an error. Only quit or the end of the input leaves it.

}