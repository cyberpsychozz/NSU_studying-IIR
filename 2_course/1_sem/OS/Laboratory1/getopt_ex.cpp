#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>
#include <errno.h>

using namespace std;

int main(int argc, char *argv[]){
    char options = "ispducC:vV:"
    int c, invalid;
    int iflg = 0, sflg = 0, pflg = 0, uflg = 0, cflg = 0, dflg = 0, vflg =0;
    string U_arg, C_arg, V_arg;

    cout << "args equals" << argc;
    while(getopt(argc, argv, options) != EOF){
        switch (c)
        {
        case 'i':
            iflg++;
            cout << "Real UID: " << getuid() << "Effective UID: " << geteuid() << endl;
            cout << "Real GID: " << getgid() << "Effective GID: " << getegid() << endl;
            break;

        case 's':
            sflg++;
            if(setpgid(0,0) == -1){
                cerr << "setgpid failed" << stderror(errno) << endl;
            }else{
                cout << "Process set as group leader" << endl;
            }
            break;

        case 'p':
            pflg++;
            cout << "Process ID: " << getpid() <<endl;
            cout << "Parent Process ID: " << getppid() <<endl;
            cout << "Process Group ID: " << getpgrp() <<endl;

            break;

        case 'd':
            
            break;
        case 'u':
            uflg++;
            {
                struct rlimit rlim;
                if (getrlimit(RLIMIT_FSIZE, &rlim) == -1) {
                    std::cerr << "getrlimit RLIMIT_FSIZE failed: " << strerror(errno) << std::endl;
                } else {
                    std::cout << "Current ulimit: " << rlim.rlim_cur << " bytes" << std::endl;
                }
            }
            break;

        case 'U':
            uflg++;
            U_arg = optarg;
            try {
                size_t pos;
                long new_ulimit = std::stol(optarg, &pos);
                if (pos != std::strlen(optarg)) {
                    std::cerr << "Invalid ulimit value: " << optarg << std::endl;
                    invalid++;
                } else {
                    struct rlimit rlim;
                    rlim.rlim_cur = new_ulimit;
                    rlim.rlim_max = new_ulimit;
                    if (setrlimit(RLIMIT_FSIZE, &rlim) == -1) {
                        std::cerr << "setrlimit RLIMIT_FSIZE failed: " << strerror(errno) << std::endl;
                    } else {
                        std::cout << "ulimit set to " << new_ulimit << " bytes" << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Invalid ulimit value: " << optarg << std::endl;
                invalid++;
            }
            break;

        case 'c':
            cflg++;
            {
                struct rlimit rlim;
                if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
                    std::cerr << "getrlimit RLIMIT_CORE failed: " << strerror(errno) << std::endl;
                } else {
                    std::cout << "Core file size limit: " << rlim.rlim_cur << " bytes" << std::endl;
                }
            }
            break;
        
        case 'C':
            cflg++;
            C_arg = optarg;
            try {
                size_t pos;
                long new_core = std::stol(optarg, &pos);
                if (pos != std::strlen(optarg)) {
                    std::cerr << "Invalid core file size: " << optarg << std::endl;
                    invalid++;
                } else {
                    struct rlimit rlim;
                    rlim.rlim_cur = new_core;
                    rlim.rlim_max = new_core;
                    if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
                        std::cerr << "setrlimit RLIMIT_CORE failed: " << strerror(errno) << std::endl;
                    } else {
                        std::cout << "Core file size set to " << new_core << " bytes" << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Invalid core file size: " << optarg << std::endl;
                invalid++;
            }
            break;
        
        case 'v':
            vflg++;
            {
                extern char **environ;
                for (char **env = environ; *env != NULL; env++) {
                std::cout << *env << std::endl;
                }
            }
            break;

        case 'V':
            vflg++;
            V_arg = optarg;
            if (putenv(optarg) != 0) {
                std::cerr << "putenv failed: " << strerror(errno) << std::endl;
            } else {
                std::cout << "Environment variable set: " << optarg << std::endl;
            }
            break;

        case '?':
            std::cerr << "Invalid option: " << static_cast<char>(optopt) << std::endl;
            invalid++;
            break;
        }
    }

    std::cout << "i flag count: " << iflg << std::endl;
    std::cout << "s flag count: " << sflg << std::endl;
    std::cout << "p flag count: " << pflg << std::endl;
    std::cout << "u flag count: " << uflg << std::endl;
    std::cout << "c flag count: " << cflg << std::endl;
    std::cout << "d flag count: " << dflg << std::endl;
    std::cout << "v flag count: " << vflg << std::endl;
    if (!U_arg.empty()) std::cout << "U_arg: " << U_arg << std::endl;
    if (!C_arg.empty()) std::cout << "C_arg: " << C_arg << std::endl;
    if (!V_arg.empty()) std::cout << "V_arg: " << V_arg << std::endl;
    std::cout << "Invalid options count: " << invalid << std::endl;
    std::cout << "optind equals " << optind << std::endl;

    
    if (optind < argc) {
        std::cout << "Non-option arguments: ";
        for (int i = optind; i < argc; ++i) {
            std::cout << argv[i] << " ";
        }
        std::cout << std::endl;
    }

    return 0;

}