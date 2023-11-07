#include <string>

const static int OPEN_TO_CHILD_PIPE = 1;
const static int OPEN_FROM_CHILD_PIPE = 2;
const static int DO_NOT_FORWORD_ENVIRON = 4;
const static int USE_IO_ARGUMENT_FLAGS = 8;

class ShellCommand
{
public:
    ShellCommand(const std::string &cmd, int flags);

    inline int getToChildPID() { return pipe_stdin[1]; }
    inline int getFromChildPID()  { return pipe_stdout[0]; }

    int waitforChildExit();

    std::string readOutput();
    pid_t execute();


private:
    std::string m_cmd;
    int m_flags;

    pid_t m_child_pid;
    int pipe_stdin[2], pipe_stdout[2];

    bool m_input_file = false, m_output_file = false;

    std::string parseIOArguments(const std::string &cmd);
};
