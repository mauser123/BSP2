#include <cstdlib>
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <list>
#include <sys/times.h>
#include <fstream>

#define SIZE 256

//Dinh-Phuong Duong - Markus Stuber

using namespace std;

class Prozess {
public:
    Prozess();
    Prozess(list<string> str, int startTime, int ppid, int pid);
    list<string> anweisungen;
    int pid;
    int ppid;
    int priority;
    int value;
    int startTime;
    int timeUsed;

};

Prozess::Prozess() {
    this->priority = 0;
}

Prozess::Prozess(list<string>str, int startTime, int ppid, int pid) {
    this->priority = 0;
    this->value = 0;
    this->timeUsed = 0;
    this->ppid = ppid;
    this->pid = pid;
    this->anweisungen = str;
    this->startTime = startTime;
}
bool mode = false;
int steps = 0;
int quantum = 0;

list<Prozess*> execprozesse;
list<Prozess*> blockedprozesse;
int tmpint;
int pid = 1;
list<string> file;
string tmp;

void stps() {
    if (quantum > 4) {
        execprozesse.push_back(execprozesse.front());
        execprozesse.pop_front();
        quantum = 0;
        cout << "Quantum :" << quantum << endl;
    } else if (execprozesse.size() > 0) {
        tmp = execprozesse.front()->anweisungen.front();
        cout << tmp << endl;
        execprozesse.front()->anweisungen.pop_front();
        execprozesse.front()->timeUsed++;
        switch (tmp[0]) {
            case 'S': tmp.erase(0, 2);
                tmpint = stoi(tmp);
                execprozesse.front()->value = tmpint;
                quantum++;
                break;
            case 'A': tmp.erase(0, 2);
                tmpint = stoi(tmp);
                execprozesse.front()->value += tmpint;
                quantum++;
                break;
            case 'D': tmp.erase(0, 2);
                tmpint = stoi(tmp);
                execprozesse.front()->value -= tmpint;
                cout << "Value " << execprozesse.front()->value << endl;
                quantum++;
                break;
            case 'B':
                blockedprozesse.push_back(execprozesse.front());
                execprozesse.pop_front();
                quantum = 0;
                break; //Methode um in die Blocked liste zu schieben
            case 'E': execprozesse.pop_front();
                quantum = 0;
                break; //Beenden des Simulierten prozess
            case 'R': printf("Liesst neue datei ein\n");
                tmp.erase(0, 2);
                string filename;
                ifstream input(tmp);
                string buff;
                while (input.good()) {
                    getline(input, buff);
                    file.push_back(buff);
                    cout << buff << endl;
                }
                execprozesse.push_back(new Prozess(file, steps, execprozesse.front()->pid, pid));
                pid++;
                quantum++;
                file.clear();
                break; //Datei einlesen 				
        }
         steps++;
        cout << "Zeit: " << steps << endl;
	cout << "Quantum: " << quantum << endl;
	if(execprozesse.size()>0){
        cout << "Value: " << execprozesse.front()->value << endl;
        cout << "CPID: " << execprozesse.front()->pid << endl;}
    }
    else {		steps++;
			cout << "Zeit: " << steps << endl;}
   
    	
}

void myhandle(int mysignal) {
    if (mode == true) {
        alarm(1);
        stps();

    }
}

int main(int argc, char** argv) {
    int mypipe[2];
    int pipename;
    //int tmpint;

    //int pid = 1;
    pid_t cpid;
    //list<Prozess*> execprozesse;
    //list<Prozess*> blockedprozesse;
    char buf[20];
    //list<string> file;
    pipename = pipe(mypipe);
    char input; //q m s p
    //string tmp;
    char buffer[SIZE]; //zur Zeitausgabe
    time_t curtime;
    struct tm *loctime;
    curtime = time(NULL);
    loctime = localtime(&curtime);

    signal(SIGALRM, myhandle);


    if (pipename == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid = fork();

    if (cpid == 0) { //1.Child
        while (true) {
            read(mypipe[0], buf, 4);

            if (buf[0] == 'p') {
                cpid = fork(); //Reporterprozesse
                if (cpid == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (cpid == 0) { //2.child

                    fputs(asctime(loctime), stdout);
                    strftime(buffer, SIZE, "Today is %A, %B %d. \n", loctime);
                    fputs(buffer, stdout);
                    strftime(buffer, SIZE, "The time is %I:%M %p. \n", loctime);
                    fputs(buffer, stdout);
                    cout << "***********************************************************************" << endl;
                    cout << "The current system state is as follows:" << endl;
                    cout << "***********************************************************************" << endl;
                    cout << "CURRENT TIME: " << steps << endl;
                    cout << "RUNNING PROCESS:" << endl;
                    cout << "pid\tppid\tpriority\tvalue\tstart time\tCPU time used so far" << endl;
                    if (execprozesse.size() > 0) {
                        cout << execprozesse.front()->pid << "\t" << execprozesse.front()->ppid << "\t"
                                << execprozesse.front()->priority << "        \t" << execprozesse.front()->value << "\t" << execprozesse.front()->startTime << "         \t" << execprozesse.front()->timeUsed << endl;
                        execprozesse.pop_front();
                    }
                    cout << "BLOCKED  PROCESSES:" << endl;
                    cout << "pid\tppid\tpriority\tvalue\tstart time\tCPU time used so far" << endl;

                    while (blockedprozesse.size() > 0) {
                        cout << blockedprozesse.front()->pid << "\t" << blockedprozesse.front()->ppid << "\t"
                                << blockedprozesse.front()->priority << "        \t" << blockedprozesse.front()->value << "\t" << blockedprozesse.front()->startTime << "         \t"
                                << blockedprozesse.front()->timeUsed << endl;
                        blockedprozesse.pop_front();
                    }

                    cout << "PROCESSES READY TO EXECUTE:" << endl;
                    cout << "pid\tppid\tpriority\tvalue\tstart time\tCPU time used so far" << endl;
                    while (execprozesse.size() > 0) {
                        cout << execprozesse.front()->pid << "\t" << execprozesse.front()->ppid << "\t"
                                << execprozesse.front()->priority << "        \t" << execprozesse.front()->value << "\t" << execprozesse.front()->startTime << "         \t" << execprozesse.front()->timeUsed << endl;
                        execprozesse.pop_front();
                    }
                    exit(0);

                } else { //2.parent
                    wait(NULL);
                }

            } else if (buf[0] == 'm') { //Toggeln mit m auto mode
                if (mode == false) {
                    mode = true;
                } else {
                    mode = false;
                }
            } else if (buf[0] == 'u') {
                while (blockedprozesse.size() > 0) {
                    execprozesse.push_back(blockedprozesse.front());
                    //execprozesse.push_front(blockedprozesse.front());
                    blockedprozesse.pop_front();
                    //quantum = 0; //Falls bei einem unblock auch ein quantum reset statfinden soll. ( popfront falls U was an den anfang schiebt)
                }
            } else if (buf[0] == 's') {
                if (mode == true) {
                    cout << "Nicht erlaubt!" << endl;
                } else {
                    stps();
                }
            } else if (buf[0] == 'T') {
                string filename;
                ifstream input("init");
                string buff;
                while (input.good()) {
                    getline(input, buff);
                    file.push_back(buff);
                    cout << buff << endl;
                }

                execprozesse.push_back(new Prozess(file, steps, 0, pid));
                pid++;
                cout << "SIZE" << execprozesse.size() << endl;
                file.clear();
                cout << "File.Size" << file.size() << endl;
                quantum = 0;
            } else if (buf[0] == 'q') {
                cout << "Bye!" << endl;
                cout << "Zeit: " << steps << endl;
                exit(0);
            }

            else {
                cout << "Wrong Input" << endl;
            }
            alarm(1);

        }

    } else if (cpid > 0) { //1.Parent
        while (true) {
            cin >> input;
            write(mypipe[1], &input, 4);

            if (input == 'q') {
                break;
            }
        }
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    return 0;
}
