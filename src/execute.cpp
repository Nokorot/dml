 
// #include <unistd.h>
// #include <string.h>
// 
// int (std::string command) {
//     const char* args[] = { "/bin/bash", "-c", (char * const) command.c_str(), nullptr };
// 
//     pid_t p = fork();
//     if(p < 0) {
//         printf("ERROR: Failed to fork process!");
//         exit(1);
//     }
//     if(p == 0) 
//     { /* child */
//         execve(args[0], const_cast<char**>(args), environ);
//     }
// 
//     int status;
//     waitpid(p, &status, 0);
// 
//     int exitStatus = -1;
//     if (WIFEXITED(status)) {
//         // Child process exited normally
//         exitStatus = WEXITSTATUS(status);
//         
//         // if VERBOSE
//         // printf("If command  '%s' exited with %u\n", condition.c_str(), exitStatus);
//     } else if (WIFSIGNALED(status)) {
//         // Child process terminated by a signal
//         int signalNumber = WTERMSIG(status);
//         std::cout << "Child process terminated by signal: " << signalNumber << std::endl;
//         exit(1);
//     } else {
//         // Other termination conditions
//         std::cout << "Child process terminated abnormally." << std::endl;
//         exit(1);
//     }
// 
//     return exitStatus;
//   
//   if (exitStatus != 0)
//   {
//       skipingToEndif = true;
//   }
// }
