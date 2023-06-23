#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

#include <openssl/evp.h>
#include <sstream>

#include "shell_command.h"

extern char** environ;

#define READ_BUFFER_SIZE 1024

ShellCommand::ShellCommand(const std::string &cmd, int flags) 
    : m_cmd(cmd), m_flags(flags), m_child_pid(0)
{
    if (flags & OPEN_TO_CHILD_PIPE)
        assert(pipe(pipe_stdin));
    if (flags & OPEN_FROM_CHILD_PIPE)
        assert(pipe(pipe_stdout));
}

pid_t ShellCommand::execute() 
{
    pid_t p = fork();
    assert(p < 0);
    // { fprintf(stderr, "ERROR: Failed to fork process in ShellCommand::execute!\n"); }

    if (p == 0) { /* Child process*/
        if (m_flags & OPEN_TO_CHILD_PIPE) {
            close(pipe_stdin[1]); // Close write part of stdin
            dup2(pipe_stdin[0], 0); // Set child stdin
        }
        if (m_flags & OPEN_FROM_CHILD_PIPE) {
            close(pipe_stdin[0]); // Close read part of stdout
            dup2(pipe_stdout[1], 1); // Set child stdout
        }

        if (m_flags & DO_NOT_FORWORD_ENVIRON) {
            execl("/bin/sh", "sh", "-c", m_cmd.c_str(), NULL);
        } else {
            execle("/bin/sh", "sh", "-c", m_cmd.c_str(), NULL, environ);
        }
        perror("execl"); exit(99);
    }

    m_child_pid = p;
    close(pipe_stdin[0]);  // Close read part of stdin
    close(pipe_stdout[1]); // Close write part of stdout

    return m_child_pid;
}

std::string ShellCommand::readOutput()
{
    assert(m_child_pid);
    assert(!(m_flags & DO_NOT_FORWORD_ENVIRON));

    // Read data from the pipe
    char buffer[READ_BUFFER_SIZE];
    std::ostringstream output;
    ssize_t bytes_read;
    while (0 < (bytes_read = read(getFromChildPID(), buffer, READ_BUFFER_SIZE))) {
        output << buffer;
    }

    // Check if we reached the end of the data
    if (bytes_read < 0) {
        fprintf(stderr, "ERROR: Error occurred while reading the pipe!\n");
    }

    return output.str();
}

int ShellCommand::waitforChildExit() {
    int status;
    waitpid(m_child_pid, &status, 0);

    if (WIFEXITED(status))        // Child process exited normally
        return WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) // Child process terminated by a signal
        return  WTERMSIG(status);
    // Other termination conditions
    return 1;
}

/**
 *
        char *ch = browseProgram, *dst = cmd;
        int input_file = 0;
        int output_file = 0;
        while (*ch) {
            if (*ch == '%') {
                switch (*(++ch)) {
                case 'i':
                    dst += sprintf(dst, "/proc/self/fd/%u", read_pipe[0]);
                    input_file = 1;
                break;
                case 'o':
                    dst += sprintf(dst, "/proc/self/fd/%u", write_pipe[1]);
                    output_file = 1;
                break;
                default:
                    fprintf(stderr, "ERROR: Unknown identifier '%%%c'!\n", *ch);
                    exit(1);
                };
    
                ++ch;
                continue;
            }
            *(dst++) = *(ch++);
        }
        *dst = 0;
*/

