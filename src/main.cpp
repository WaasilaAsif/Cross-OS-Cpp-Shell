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
#else
#include <sys/types.h> // Defines pid_t
#include <unistd.h>    // Defines fork() and execvp()
#include <sys/wait.h>  // Defines waitpid()
#endif

using namespace std;
namespace fs = filesystem;
void lsh_loop();
int lsh_cd(vector<string> &args);
int lsh_help(vector<string> &args);
int lsh_exit(vector<string> &args);
int lsh_cwd(vector<string> &args);
int lsh_man(vector<string> &args);
string builtin_str[] = {
    "cd",
    "help",
    "exit",
    "cwd",
    "man"};
int (*builtin_func[])(vector<string> &args) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
    &lsh_cwd

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
void lsh_loop()
{
    string line;
    vector<string> args;
    int status;

    do
    {
        cout << "> ";
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);
    } while (status);
};
string lsh_read_line()
{
    string line;

    if (!getline(cin, line))
    {
        if (cin.eof())
        {

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
            if (_chdir(ncwd.c_str()) != 0)
            {
                perror("lsh");
            }
        }
        else
        {
            cout << "Moving to directory \t" << args[1] << endl;
            if (_chdir(args[1].c_str()) != 0)
            {
                perror("lsh");
            }
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

int lsh_man(vector<string> *args)
{
    return 1;
}
