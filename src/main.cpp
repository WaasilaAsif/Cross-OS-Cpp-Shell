#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <filesystem>
#include <sys/types.h> // Defines pid_t
// #include <unistd.h>    // Defines fork() and execvp()
#if defined(_WIN32)
#include <io.h>
#include <process.h>
#include <direct.h>
#include <conio.h> //Stack overflow saya this os to enable escape sequences let's test it out
#else
#include <sys/types.h> // Defines pid_t
#include <unistd.h>    // Defines fork() and execvp()
#include <termios.h>
#include <sys/wait.h> // Defines waitpid()

#endif

using namespace std;
namespace fs = filesystem;
void lsh_loop();
int lsh_cd(vector<string> &args);
int lsh_help(vector<string> &args);
int lsh_exit(vector<string> &args);
int lsh_cwd(vector<string> &args);
int lsh_man(vector<string> &args);
int lsh_tree(vector<string> &args);
int lsh_tree_all(vector<string> &args);
vector <string> history;
static int history_index = history.size() - 1;
string builtin_str[] = {
    "cd",
    "help",
    "exit",
    "cwd",
    "man",
    "tree",
    "tree_all"};
int (*builtin_func[])(vector<string> &args) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
    &lsh_cwd,
    &lsh_man,
    &lsh_tree,
    &lsh_tree_all

};
int lsh_num_builtins()
{
    return (size(builtin_str));
}
string lsh_read_line();
vector<string> lsh_split_line(string line);
int lsh_execute(vector<string> args);
int main(int argc, char **argv)
{

    lsh_loop();

    return EXIT_SUCCESS;
}
#if !defined(_WIN32)
char getch()
{
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON; // Disable buffered i/o
    old.c_lflag &= ~ECHO;   // Disable local echo
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

#endif

void lsh_loop()
{
    string line;
    vector<string> args;
    int status;

    do
    {
        cout << "> ";
        line = lsh_read_line();
        #if defined(_WIN32)

    while (status)
    {

        int ch = _getch();

        if (ch == 0 || ch == 224)
        {
            int extended_ch = _getch();
            switch (extended_ch)
            {
            case 72: // Up arrow
                if (history_index > 0)
                {
                    history_index--;
                    line = history[history_index];
                }
                break;
            case 80: // down arrow;
                ;
                break;
            default:
                cout << "Undefined Key" << endl;
                break;
            }
        }
        else if (ch == 27)
        {
            cout << "Escape key pressed. Exiting...\n";
            // execute exit work on logic
            lsh_exit();
            break;
        }
    };
#else
    char ch = getch();
    if (ch != 13 && ch != 10)     
    {

        char c1 = getch();
        if (c1 == 27)
        {
            char c2 = getch();
            if (c2 == 91)
            {
                char c3 = getch();

                switch (c3)
                {
                case 'A':
                    if (history_index > 0)
                    {
                        history_index--;
                        line = history[history_index];
                        cout<<line<<endl;
                    };
                    break;
                case 'B':
                    if (history_index < history.size() - 1)
                    {
                        history_index++;
                        line = history[history_index];
                        cout<<line<<endl;
                    };

                    break;

                default:
                    cout << "Escape key pressed exiting" << endl;
                    break;
                }
            }
        }
#endif
};

        args = lsh_split_line(line);
        status = lsh_execute(args);
    } while (status);
}

string lsh_read_line()
{
    string line;

    if (!getline(cin, line))
    {
        if (cin.eof())
        {
            history.push_back(line);
            // return line;
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("lsh: read line error");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}
vector<string> lsh_split_line(string line)
{
    vector<string> tokens;
    string current_token;

    if (line.empty())
    {
        cout << "> lsh: allocation error. No command entered" << endl
             << ">";
        return tokens;
    }

    for (int i = 0; i < line.length(); i++)
    {
        char c = line[i];

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            if (!current_token.empty())
            {
                tokens.push_back(current_token);
                current_token = "";
            }
        }
        else
        {
            current_token += c;
        }
    }
    if (!current_token.empty())
    {
        tokens.push_back(current_token);
    }

    return tokens;
}

int lsh_launch(const vector<string> args)
{
    vector<char *> c_args;
    for (const auto &arg : args)
    {
        c_args.push_back(const_cast<char *>(arg.c_str()));
    }
    c_args.push_back(nullptr);
#if defined(_WIN32)
    intptr_t status = _spawnvp(_P_WAIT, c_args[0], c_args.data());
    if (status == -1)
    {
        perror("lsh error");
    }
    return 1;
#else
        pid_t pid, wpid;
        int status;

        pid = fork();
        if (pid == 0)
        {
            // Child process is going on
            if (execvp(c_args[0], c_args.data()) == -1)
            {
                perror("lsh");
            }
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            // Error forking
            perror("lsh");
        }
        else
        {
            // Parent process
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }

        return 1;

#endif
}
int lsh_cd(vector<string> &args)
{
    if (args.size() < 2)
    {
        cout << "lsh: expected an argument to the cd command, Need an argument to move to the directory required" << endl;
    }
    else
    {
        if (args[1] == "..")
        {
            cout << "Moving back in directory\t";
            fs::path cwd = fs::current_path();
            string path = cwd.string();
            cout << path << endl;
            string ncwd;
            int pos = path.find_last_of('\\');
            ncwd = path.substr(0, pos);
            cout << "Moving to directory \t" << ncwd << endl;
#if defined(_WIN32)
            if (_chdir(ncwd.c_str()) != 0)
            {
                perror("lsh");
            }
#else
                if (chdir(ncwd.c_str()) != 0)
                {
                    perror("lsh");
                }
#endif
        }
        else
        {
            cout << "Moving to directory \t" << args[1] << endl;
#if defined(_WIN32)
            if (_chdir(args[1].c_str()) != 0)
            {
                perror("lsh");
            }
#else
                if (chdir(args[1].c_str()) != 0)
                {
                    perror("lsh");
                }
#endif
        }
    }
    try
    {

        fs::path current_dir = fs::current_path();
        std::cout << "Current working directory: " << current_dir << std::endl;
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}

int lsh_help(vector<string> &args)
{
    int i;
    cout << "My implementation of LSH inspired by Stepher Brennan's implementation" << endl;
    cout << "Most of the stuff online was in C and was only for Unix like systems. As like most people I starred coding from windows I wanted to make a cross OS shell" << endl;
    cout << "So here is my humble attempt" << endl;
    cout << "Type program names and arguments, and hit enter." << endl;
    cout << "The following are built in:" << endl;

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        cout << i << ".  " << builtin_str[i] << endl;
    }

    return 1;
}
int lsh_exit(vector<string> &args)
{
    return 0;
}
int lsh_execute(vector<string> args)
{

    if (args.size() < 1)
    {
        return 1;
    }
    for (int i = 0; i < lsh_num_builtins(); i++)
    {

        if (args[0].compare(builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}

int lsh_cwd(vector<string> &args)
{

    try
    {

        fs::path current_dir = fs::current_path();
        std::cout << "Current working directory: " << current_dir << std::endl;
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}

int lsh_man(vector<string> &args)
{
    if (args.size() < 2)
    {
        cout << "lsh: man - display info about a builtin command" << endl;
        cout << "Usage: man <command>" << endl;
        cout << "Available commands: cd, cwd, exit, help, tree, tree_all" << endl;
        return 1;
    }

    string cmd = args[1];

    if (cmd == "cd")
    {
        cout << "cd - change the current working directory" << endl;
        cout << "Usage: cd <directory>" << endl;
        cout << "       cd ..   (move up one directory)" << endl;
    }
    else if (cmd == "cwd")
    {
        cout << "cwd - print the current working directory" << endl;
        cout << "Usage: cwd" << endl;
    }
    else if (cmd == "exit")
    {
        cout << "exit - terminate the shell" << endl;
        cout << "Usage: exit" << endl;
    }
    else if (cmd == "help")
    {
        cout << "help - list all builtin commands" << endl;
        cout << "Usage: help" << endl;
    }
    else if (cmd == "tree")
    {
        cout << "tree - recursively list files/directories from current path" << endl;
        cout << "Usage: tree" << endl;
    }
    else if (cmd == "tree_all")
    {
        cout << "tree_all display hidden files aling with the regular tree function." << endl;
        cout << "Usage: tree_all" << endl;
    }
    else
    {
        cout << "lsh: man: no manual entry for '" << cmd << "'" << endl;
    }

    return 1;
}

int lsh_tree(vector<string> &args)
{

    fs::path current_dir = fs::current_path();
    cout << "Current working directory: " << current_dir << endl;
    if (fs::exists(current_dir) && fs::is_directory(current_dir))
    {
        for (const auto &entry : fs::recursive_directory_iterator(current_dir))
        {

            if (entry.path().string().find("\\.") == string::npos)
            {
                cout << entry.path().string() << endl;
            }
        }
    }
    else
    {
        cout << "lsh error: Either your current directory has no files or it is not a valid directory. Please change directory and try again" << endl;
    }

    return 1;
};

int lsh_tree_all(vector<string> &args)
{

    fs::path current_dir = fs::current_path();
    cout << "Current working directory: " << current_dir << endl;
    if (fs::exists(current_dir) && fs::is_directory(current_dir))
    {
        for (const auto &entry : fs::recursive_directory_iterator(current_dir))
        {

            cout << entry.path().string() << endl;
        }
    }
    else
    {
        cout << "lsh error: Either your current directory has no files or it is not a valid directory. Please change directory and try again" << endl;
    }

    return 1;
}