// #include <string>
#include <iostream>
#include <cstdio>
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
{}

std::string ShellCommand::parseIOArguments(const std::string &cmd)
{
    std::stringstream new_cmd;
    const char *ch = m_cmd.c_str();

    while (*ch) {
        if (*ch == '%') {
            switch (*(++ch)) {
            case 'i':
                // if (m_flags & OPEN_TO_CHILD_PIPE)
                new_cmd << "/proc/self/fd/" << pipe_stdin[0]; // getToChildPID();
                m_input_file = true;
            break;
            case 'o':
                // if (m_flags & OPEN_FROM_CHILD_PIPE)
                new_cmd << "/proc/self/fd/" << pipe_stdout[1]; // getFromChildPID();
                m_output_file = true;
            break;
            default:
                fprintf(stderr, "ERROR: Unknown identifier '%%%c'!\n", *ch);
                exit(1);
            };

            ++ch;
            continue;
        }
        new_cmd << *(ch++);
    }

    return new_cmd.str();
}

pid_t ShellCommand::execute() {
    if (m_flags & OPEN_TO_CHILD_PIPE) { // && !m_input_file) {
        pipe(pipe_stdin);
        // printf("PipeIn: %u %u\n", pipe_stdin[0], pipe_stdin[1]);
    }
    if (m_flags & OPEN_FROM_CHILD_PIPE) {
        pipe(pipe_stdout);
        // printf("PipeOut: %u %u\n", pipe_stdout[0], pipe_stdout[1]);
    }

    std::string cmd = m_cmd;
    if (m_flags & USE_IO_ARGUMENT_FLAGS)
        cmd = parseIOArguments(m_cmd);

    pid_t p = fork();
    if (p < 0) {
        fprintf(stderr, "ERROR: Failed to fork process in ShellCommand::execute!\n");
    }

    if (p == 0) { /* Child process*/
        fprintf(stdout, "%s\n", cmd.c_str());

        if (m_flags & OPEN_TO_CHILD_PIPE) {
            close(pipe_stdin[1]); // Close write part of stdin
            if (!m_input_file)
                dup2(pipe_stdin[0], 0); // Set child stdin
        }
        if (m_flags & OPEN_FROM_CHILD_PIPE ) {
            close(pipe_stdout[0]); // Close read part of stdout
            if (!m_output_file)
                dup2(pipe_stdout[1], 1); // Set child stdout
            // else
            //     open(pipe_stdout[1]);
        }


        if (m_flags & DO_NOT_FORWORD_ENVIRON) {
            execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL);
        } else {
            execle("/bin/sh", "sh", "-c", cmd.c_str(), NULL, environ);
        }

        perror("execl"); exit(99);
    }

    m_child_pid = p;
    if (m_flags & OPEN_TO_CHILD_PIPE)
        close(pipe_stdin[0]);  // Close read part of stdin
    if (m_flags & OPEN_FROM_CHILD_PIPE)
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
        output.write(buffer, bytes_read);
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
