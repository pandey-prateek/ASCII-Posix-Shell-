#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <iomanip>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <vector>
#include <stack>
#include <list>
#include <algorithm>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <sstream>
#include <sys/wait.h>
#include <fstream>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <map>
#include <unordered_map>
#include <set>

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

using namespace std;

list<int> bg;
class Node
{
public:
    char ch;
    Node *parent;
    Node *next[257];
    Node()
    {
        parent = NULL;
        for (int i = 0; i < 257; i++)
        {
            next[i] = NULL;
        }
    }
    Node(char ch)
    {
        this->ch = ch;
        for (int i = 0; i < 257; i++)
        {
            next[i] = NULL;
        }
    }
};
vector<string> split_string(string s, char ch);
class Recorder
{
public:
    ofstream recorder_stream;
    bool is_recording;
    Recorder()
    {
        recorder_stream.open("recording.txt");
        is_recording = false;
    }
    void start_recording()
    {
        is_recording = true;
    }
    void stop_recording()
    {
        is_recording = false;
    }
    void record_data(string data)
    {
        recorder_stream.seekp (0, ios::end);
        recorder_stream << data << endl;
    }
} * recorder;
class Config
{
public:
    struct termios orig_termios;
    int shell_id;
    char path[256];
    string s_path;
    string env_path;
    string username = "", hostname, symbol;
    string HOME = string(getenv("HOME"));
    string myrc_fullpath;
    string alarm = "alarm";
    int ex_status = 0;
    map<string, string> alias;
    map<string, string> env_v;
    string PATH = string(getenv("PATH"));
    unordered_map<string, pair<string, string>> extension_shorcuts;
    unordered_map<long long int, string> alarm_map;
    unordered_map<int, long long int> active;
    unordered_map<long long int, string> missed_alarm;
    string myrc_info = "";
    char myrc_path[256];
    set<long> children;
    int shell_term;
    pid_t shell_pg;
    bool isSuspended;
    int histsize = 2000;
} config;

class Trie
{
public:
    Node *root;
    Node *end;
    ofstream trie_logger;
    Trie()
    {
        root = new Node('/');
        end = new Node('$');
    }
    void initialize_trie()
    {
        trie_logger.open("trie_logger.txt");
        trie_logger << "Initializing trie" << endl;
        trie_logger << config.PATH << endl;

        vector<string> pathList = split_string(config.PATH, ':');

        for (string path : pathList)
        {
            trie_logger << "**********************************************" << endl
                        << path << endl
                        << endl;
            DIR *dir = opendir(path.c_str());
            trie_logger << "accessing files" << endl;
            if (dir == NULL)
            {
                trie_logger << "not open" << endl;
                exit(1);
            }
            struct dirent *entity;
            entity = readdir(dir);
            trie_logger << "printing files" << endl;

            while (entity != NULL)
            {

                string command_name = string(entity->d_name);
                // command_name = entity->d_name;
                if (command_name == "." || command_name == "..")
                {
                    entity = readdir(dir);

                    continue;
                }

                insert(command_name);
                trie_logger << command_name << endl;

                entity = readdir(dir);
            }
            closedir(dir);
        }
    }
    void insert(string st)
    {
        Node *cur = root;

        int n = st.length();
        for (int i = 0; i < n; i++)
        {
            int index = int(st[i]);
            if (cur->next[index] == NULL)
            {
                Node *temp = new Node(st[i]);
                temp->parent = cur;
                cur->next[index] = temp;
                cur = temp;
            }
            else
            {
                cur = cur->next[index];
            }
        }
        cur->next[256] = end;
    }
    int find(string st)
    {
        Node *cur = root;
        for (int i = 0; i < st.length(); i++)
        {
            int index = int(st[i]);

            if (cur->next[index] == NULL)
                return 0;
            cur = cur->next[index];
        }
        if (cur->next[256] == NULL) // string did not end
            return 0;
        return 1;
    }
    void getWords(Node *cur, string prefix, vector<string> &wordList)
    {
        if (cur == end)
        {
            wordList.push_back(prefix);
            return;
        }
        for (int i = 0; i < 257; i++)
        {
            if (cur->next[i] != NULL)
            {
                getWords(cur->next[i], prefix + cur->ch, wordList);
            }
        }
    }
    vector<string> auto_complete(string prefix)
    {
        vector<string> wordList;
        if (prefix == "")
            return wordList;
        Node *cur = root;
        for (int i = 0; i < prefix.length(); i++)
        {
            int index = int(prefix[i]);
            if (cur->next[index] == NULL)
            {
                // cout << -1 << endl;
                return wordList;
            }
            else
            {
                cur = cur->next[index];
            }
        }
        prefix.pop_back();
        getWords(cur, prefix, wordList);
        // cout << wordList.size() << endl;
        return wordList;
    }
    void insert_suffix(string st)
    {
        for (int i = 0; i < st.length(); i++)
        {
            insert(st.substr(i));
        }
    }
} * trie;
int getHISTSIZE();
class History
{
public:
    ofstream history_logger;
    int HISTSIZE, no_of_commands;
    string history_path;
    History()
    {
        history_logger.open("history.txt", ios_base::app);
        history_path = string(config.path) + "/history.txt";
        // read HISTSIZE from myrc

        HISTSIZE = getHISTSIZE();
        no_of_commands = 0;

        ifstream history_if;
        history_if.open("history.txt");
        string line;
        while (std::getline(history_if, line))
            ++no_of_commands;
    }
    void insert(string command)
    {
        if (command == "")
            return;
        if (no_of_commands < HISTSIZE)
        {
            string line = to_string(++no_of_commands) + "\t" + command;
            history_logger << line << endl;
        }
    }
    void print_history()
    {

        ifstream history_input_stream;
        history_input_stream.open(history_path);
        if (history_input_stream.fail())
        {
            perror("Failed to load history: ");
            // return;
        }
        string line, line_number;
        for (std::string line; getline(history_input_stream, line);)
        {
            cout << line << endl;
            if (recorder->is_recording)
                recorder->record_data(line);
        }
        history_input_stream.close();
    }

    void search_history_file(string key)
    {
        ifstream history_input_stream;
        history_input_stream.open(history_path);
        if (history_input_stream.fail())
        {
            perror("Failed to open history: ");
            return;
        }
        string line, line_number;
        for (std::string line; getline(history_input_stream, line);)
        {
            // cout << line << endl;
            Trie *line_trie = new Trie();
            line_trie->insert_suffix(line);
            vector<string> searchResult = line_trie->auto_complete(key);
            if (searchResult.size() > 0)
            {
                cout << line << endl;

                if (recorder->is_recording)
                    recorder->record_data(line);
            }
        }
        history_input_stream.close();
    }
} * history;

class cursor
{
public:
    int win_x = 0;
    int win_y = 0;
    int x = 1;
    int y = 1;
} cursor;
class Command
{
public:
    vector<string> instructions;
} cmd;
enum KEYS
{
    ENTER = 10,
    UP = 65,
    DOWN = 66,
    LEFT = 68,
    RIGHT = 67,
    BACK_SPACE = 127,
    HOME = 104,
    TAB = 9,
    COLON = 58,
    ESC = 27,
    q = 113,
    Q = 81
};

bool set_cursor_position(int row, int col)
{
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", row, col);
    write(STDOUT_FILENO, buf, len);
    return true;
}
void clearLineMacro()
{
    cout << "\033[2K" << flush;
}
void kill_all_processes()
{
    for (auto i : config.children)
    {
        kill(i, SIGKILL);
    }
    for (auto i : bg)
    {
        kill(i, SIGKILL);
    }
}
void disable_shell()
{
    kill_all_processes();
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &config.orig_termios) != 0)
    {
        exit(EXIT_FAILURE);
    };
}
void ctrl_c(int signum)
{
    disable_shell();
    pid_t p = getpid();
    if (p < 0)
    {
    }

    if (p != config.shell_id)
    {
        return;
    }

    /* if (CHILD_ID != -1)
    {
        kill(CHILD_ID, SIGINT);
    } */

    while (true)
    {

        signal(SIGINT, ctrl_c);
        break;
    }

    return;
}

void ctrl_z(int signum)
{
    disable_shell();
}
int getCursorPosition(int *rows, int *cols)
{
    string s;
    s.resize(32);
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;
    while (i < s.length() - 1)
    {
        if (read(STDIN_FILENO, &s[i], 1) != 1)
            break;
        if (s[i] == 'R')
            break;
        i++;
    }
    s[i] = '\0';
    if (s[0] != '\x1b' || s[1] != '[')
        return -1;
    if (sscanf(&s[2], "%d;%d", rows, cols) != 2)
        return -1;
    return 0;
}
int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        // editorReadKey();
        return getCursorPosition(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

bool fileExists(const std::string &filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}
void env_var_modify()
{
    string line;
    ofstream fout;
    fout.open(config.myrc_fullpath.c_str());
    fout << "PATH=" + config.env_path << "\n";
    fout << "HOME=" + config.HOME << "\n";
    fout << "USER=" + config.username << "\n";
    fout << "HOSTNAME=" + config.hostname << "\n";
    fout << "PS1=" + config.symbol
         << "\n";
    fout << "HISTSIZE=" + to_string(config.histsize)
         << "\n";
    for (auto i : config.alias)
    {
        fout << "alias " + i.first + "=" + "\'" + i.second + "\'\n";
    }
    for (auto i : config.env_v)
    {
        fout << i.first + "=" + i.second + "\n";
    }
    fout.close();
}

void export_new_var(string comm)
{
    vector<string> s = split_string(comm, '=');
    if (s.size() == 1)
        return;
    config.env_v[s[0]] = s[1];
    if (config.username != "")
    {
        env_var_modify();
    }
    config.ex_status = 0;
}
void handle_alias(vector<Command>, int);
vector<Command> process_commands(string);
void create_myrc()
{
    if (fileExists(".myrc"))
    {
        fstream file;
        file.open(".myrc", ios::in);
        string x;
        int count = 0;
        while (getline(file, x))
        {
            if (count < 6)
            {
                if (x.substr(0, 8) == "HISTSIZE")
                {
                    config.histsize = stoi(x.substr(9, x.length() - 9));
                }
                count++;
                continue;
            }
            if (x.substr(0, 5) == "alias")
            {
                vector<Command> var = process_commands(x);
                handle_alias(var, 0);
            }
            else
            {
                export_new_var(x);
            }
        }
    }
    fstream file;
    file.open(".myrc", ios::out);
    getcwd(config.myrc_path, 256);
    config.myrc_fullpath = string(config.myrc_path) + "/.myrc";
    file << "PATH=" + string(getenv("PATH")) << "\n";
    config.env_path = string(getenv("PATH"));
    file << "HOME=" + string(getenv("HOME")) << "\n";
    config.username = string(getenv("USER"));
    file << "USER=" + config.username << "\n";
    char host[1024];
    memset(host, 0, 1024);
    gethostname(host, 1024);
    config.hostname = string(host);
    file << "HOSTNAME=" + config.hostname << "\n";
    config.symbol = "$";
    file << "PS1=$"
         << "\n";
    file << "HISTSIZE=" + to_string(config.histsize)
         << "\n";
    for (auto i : config.alias)
    {
        file << "alias " + i.first + "=" + "\'" + i.second + "\'\n";
    }
    for (auto i : config.env_v)
    {
        file << i.first + "=" + i.second + "\n";
    }
    file.close();
}
void enable_shell()
{
    tcgetattr(STDIN_FILENO, &config.orig_termios);
    struct termios raw = config.orig_termios;
    raw.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int getHISTSIZE()
{
    fstream file;
    file.open(config.myrc_fullpath.c_str(), ios::in);
    string x;
    int count = 0;
    int histsize;
    while (file >> x)
    {
        if (count == 5)
        {
            int ind = x.find("=");
            histsize = stoi(x.substr(ind + 1, 4));
            break;
        }
        count++;
    }
    return histsize;
}
vector<string> split_string(string s, char ch)
{
    vector<string> tokens;

    stringstream ss(s);
    string token;

    while (getline(ss, token, ch))
    {
        tokens.push_back(token);
    }
    return tokens;
}
void read_file()
{
    vector<string> tokens;
    ofstream off(".myrc", ios::app);
    off.close();
    ifstream iff(".myrc");
    string token;

    while (getline(iff, token, '\n'))
    {
        vector<string> v = split_string(token, '=');
        if (v[0] == "ext")
        {
            vector<string> vec = split_string(v[1], ' ');
            config.extension_shorcuts[vec[vec.size() - 1]] = make_pair(vec[0], vec[1]);
        }
    }
}
string getParent_dir(string s)
{
    vector<string> tokens = split_string(s, '/');
    string ret;
    for (int i = 0; i < tokens.size() - 1; i++)
    {
        if (i > 0)
            ret += "/" + tokens[i];
    }
    return ret;
}
string read_command(bool *exec,bool *remainder)
{
    set_cursor_position(cursor.win_y - 1, 1);
    string s;
    s = BOLDRED + config.username + "@" + config.hostname + ": \x1b[0m" + BOLDYELLOW + config.s_path + " " + config.symbol + " \x1b[0m";
    cout << '\n'
         << s << flush;
    if (config.missed_alarm.size() > 0)
    {
        cout << endl
             << "Missed Alarm: " << endl;

        for (auto i : config.missed_alarm)
        {
            cout << abs(i.first) << "seconds ago   message:" << i.second << endl;
        }
        config.missed_alarm.clear();
        
        return "";
    }
    if (config.alarm_map.size() > 0&&*remainder)
    {
        cout << endl
             << "Remainders Alarm: " << endl;
        std::time_t result = std::time(nullptr);
        for (auto i : config.alarm_map)
        {
            cout << abs(i.first)-result << " seconds later   message:" << i.second << endl;
        }
        *remainder=false;
        return "";
    }
    cursor.x = s.length();
    int label = s.length();
    while (1)
    {
        char key[3] = {'\0'};
        read(STDIN_FILENO, &key, 3);
        if (iscntrl(key[0]))
        {
            if (int(key[0]) == BACK_SPACE)
            {
                if (!s.empty() && cursor.x > label)
                {
                    clearLineMacro();
                    set_cursor_position(cursor.win_y - 1, 1);
                    s.pop_back();
                    cursor.x--;
                    cout << '\n'
                         << s << flush;
                }
                else
                {
                    clearLineMacro();
                    set_cursor_position(cursor.win_y - 1, 1);
                    cout << '\n'
                         << s << flush;
                }
            }
            else if (int(key[0]) == 27)
            {

                if (int(key[1]) == 91)
                {

                    if (int(key[2]) == RIGHT)
                    {
                        clearLineMacro();
                        set_cursor_position(cursor.win_y - 1, 1);
                        cout << '\n'
                             << s << flush;
                    }
                    else if (int(key[2]) == LEFT)
                    {
                        clearLineMacro();
                        set_cursor_position(cursor.win_y - 1, 1);
                        cout << '\n'
                             << s << flush;
                    }
                    else if (int(key[2]) == UP)
                    {
                        clearLineMacro();
                        set_cursor_position(cursor.win_y - 1, 1);
                        cout << '\n'
                             << s << flush;
                    }
                    else if (int(key[2]) == DOWN)
                    {
                        clearLineMacro();
                        set_cursor_position(cursor.win_y - 1, 1);
                        cout << '\n'
                             << s << flush;
                    }
                }
            }
            else if (int(key[0]) == ENTER)
            {
                cursor.y++;
                break;
            }
            else if (int(key[0]) == TAB)
            {
                string prefix = "";
                int k;
                for (k = label; k < cursor.x; k++)
                {
                    prefix += s[k];
                }
                int prefix_len = prefix.length();
                vector<string> commandList = trie->auto_complete(prefix);
                clearLineMacro();
                set_cursor_position(cursor.win_y - 1, 1);
                cout << '\n'
                     << s << flush;
                if (commandList.size() == 1)
                {

                    for (int j = prefix_len; j < commandList[0].length(); j++)
                    {
                        s += commandList[0][j];
                        cursor.x++;
                    }
                    clearLineMacro();
                    set_cursor_position(cursor.win_y - 1, 1);
                    cout << '\n'
                         << s << flush;
                }
                else if (commandList.size() > 1)
                {
                    cout << endl;
                    int count = 0;
                    cursor.y++;
                    for (auto ele : commandList)
                    {

                        cout << left << setw(30) << ele;
                        count++;
                        if (count % 4 == 0)
                        {
                            cursor.y++;
                            cout << endl;
                        }
                    }
                    cursor.y++;
                    cout << endl;
                    *exec = false;
                    break;
                }
            }
        }
        else
        {
            s.push_back(key[0]);
            cursor.x++;
        }
    }

    return s.substr(label, cursor.x - label);
}
vector<Command> process_commands(string shell_i)
{
    vector<Command> cmds;
    vector<string> tokens = split_string(shell_i, '|');
    for (int i = 0; i < tokens.size(); i++)
    {
        Command cmd;
        vector<string> v = split_string(tokens[i], ' ');
        for(auto i:v){
            if(i.empty())
                continue;
            cmd.instructions.push_back(i);
        }
        cmds.push_back(cmd);
    }
    return cmds;
}

void clear_screen()
{

    write(STDOUT_FILENO, "\x1b[H", 3);
    write(STDOUT_FILENO, "\x1b[2J", 4);
}
void export_print()
{
    fstream file;
    file.open(config.myrc_fullpath.c_str(), ios::in);
    string x;
    while (getline(file, x))
    {
        if (x == "" || x == "\n")
        {
            continue;
        }
        else if (x.substr(0, 5) == "alias")
        {
            cout << "declare -x " << x << endl;
            if (recorder->is_recording)
            {
                recorder->record_data("declare -x " + x);
            }
        }
        else
        {
            vector<string> var = split_string(x, '=');
            cout << "declare -x " << var[0] << "="
                 << "\"" << var[1] << "\"" << endl;
            if (recorder->is_recording)
            {
                recorder->record_data("declare -x " + var[0] + "=" + "\"" + var[1] + "\"");
            }
        }
    }
    file.close();
}

void export_delete(string str, int flag)
{
    if (flag == 0)
    {
        auto search = config.env_v.find(str);
        if (config.env_v.find(str) != config.env_v.end())
        {
            config.ex_status = 0;
            config.env_v.erase(str);
            env_var_modify();
        }
        else
        {
            cout << "Not found" << endl;
            config.ex_status = 1;
            if (recorder->is_recording)
            {
                recorder->record_data("Not found");
            }
        }
    }
    else
    {
        auto search = config.alias.find(str);
        if (config.alias.find(str) != config.alias.end())
        {
            config.ex_status = 0;
            config.alias.erase(str);
            env_var_modify();
        }
        else
        {
            cout << "Not found" << endl;
            config.ex_status = 1;
            if (recorder->is_recording)
            {
                recorder->record_data("Not found");
            }
        }
    }
}
bool pathexists(string pathname)
{
    struct stat buff;
    return (stat(pathname.c_str(), &buff) == 0);
}

string display_path(string s)
{
    vector<string> tokens = split_string(s, '/');
    if (tokens.size() <= 2)
    {
        return s;
    }
    string res = "~";
    for (int i = 3; i < tokens.size(); i++)
    {
        res += "/" + tokens[i];
    }
    return res;
}

string handle_path(string s)
{
    vector<string> tokens = split_string(s, '/');
    string dir;
    vector<string> format;
    if (tokens[0] == "~")
    {
        dir = getenv("HOME");
        for (int i = 1; i < tokens.size(); i++)
        {
            dir += "/" + tokens[i];
        }
    }
    else if (tokens[0] == ".")
    {
        dir = getcwd(config.path, 256);
        for (int i = 1; i < tokens.size(); i++)
        {
            dir += "/" + tokens[i];
        }
    }
    else if (tokens[0] == "..")
    {
        dir = getParent_dir(getcwd(config.path, 256));
        for (int i = 1; i < tokens.size(); i++)
        {
            dir += "/" + tokens[i];
        }
    }
    else
    {
        if (s[0] == '/')
            dir = s;
        else
            dir = string(getcwd(config.path, 256)) + "/" + s;
    }
    if (!dir.empty())
    {
        vector<string> new_tokens = split_string(dir, '/');
        for (int i = 1; i < new_tokens.size(); i++)
        {
            format.push_back("/" + new_tokens[i]);
        }
    }
    vector<string> v;
    for (int i = 0; i < format.size(); i++)
    {

        if (format[i] == "/..")
        {
            if (v.empty())
            {
                continue;
            }
            v.pop_back();
        }
        else if (format[i] == "/.")
        {
            continue;
        }
        else
        {
            v.push_back(format[i]);
        }
    }
    string x;
    for (int i = 0; i < v.size(); i++)
    {
        x += v[i];
    }
    if (x == "")
    {
        x = "/";
    }
    return x;
}

int export_change(string str)
{
    int stat = 0;
    if (str.substr(0, 3) == "PS1")
    {
        config.symbol = str.substr(4, str.length() - 4);
        env_var_modify();
        config.ex_status = 0;
        stat = 1;
    }
    else if (str.substr(0, 3) == "HISTSIZE")
    {
        config.histsize = stoi(str.substr(9, str.length() - 9));
        config.ex_status = 0;
        env_var_modify();
        stat = 1;
    }
    else if (config.env_v.find(str) != config.env_v.end())
    {
        auto search = config.env_v.find(str);
        config.env_v[search->first] = search->second;
        config.ex_status = 0;
        stat = 1;
        env_var_modify();
    }
    else
    {
        stat = 0;
    }
    return stat;
}

void path_commands(vector<Command> cmds, int ind)
{
    if (strcmp(cmds[ind].instructions[0].c_str(), "cd") == 0)
    {
        if (cmds[ind].instructions.size() == 1)
        {
            config.ex_status = 0;
            return;
        }
        if (cmds[ind].instructions.size() > 2)
        {
            cout << "Too many arguments" << endl;
            if (recorder->is_recording)
            {
                recorder->record_data("Too many arguments\n");
            }
            config.ex_status = 1;
            return;
        }
        string s = (handle_path(cmds[ind].instructions[1]));
        if (!pathexists(s))
        {
            cout << "No such file or directory" << endl;
            if (recorder->is_recording)
            {
                recorder->record_data("No such file or directory\n");
            }
            config.ex_status = 1;
            return;
        }
        chdir(s.c_str());
        getcwd(config.path, 256);
        config.ex_status = 0;
        config.s_path = display_path(string(config.path));
    }

    else if (strcmp(cmds[ind].instructions[0].c_str(), "pwd") == 0)
    {
        cout << config.path << endl;
        if (recorder->is_recording)
        {
            recorder->record_data(string(config.path));
        }
        config.ex_status = 0;
    }
    else
    {
        if (cmds[ind].instructions.size() == 1 || (cmds[ind].instructions.size() == 2 && strcmp(cmds[ind].instructions[1].c_str(), "-p") == 0))
        {
            cout << 1 << endl;
            export_print();
            config.ex_status = 0;
        }
        else if (cmds[ind].instructions.size() == 2)
        {
            if (!export_change(cmds[ind].instructions[1]))
            {
                export_new_var(cmds[ind].instructions[1]);
            }
        }
        else if (cmds[ind].instructions.size() == 3 && cmds[ind].instructions[1] == "-n")
        {
            export_delete(cmds[ind].instructions[2], 0);
        }
        else if (cmds[ind].instructions.size() == 4 && strcmp(cmds[ind].instructions[1].c_str(), "-n") == 0)
        {
            // export_delete(cmds[ind].instructions[2], cmds[ind].instructions[3]);
            if (cmds[ind].instructions[2].substr(0, 5) == "alias")
            {
                export_delete(cmds[ind].instructions[3], 1);
            }
        }
        else if (cmds[ind].instructions[1].substr(0, 5) == "alias")
        {
            auto it = cmds[ind].instructions.begin();
            cmds[ind].instructions.erase(it);
            handle_alias(cmds, ind);
        }
    }
}

void printmyrc()
{
    fstream file;
    file.open(config.myrc_fullpath.c_str(), ios::in);
    string x;
    while (getline(file, x))
    {
        if (x == "" || x == "\n")
        {
            continue;
        }
        cout << x << endl;
        if (recorder->is_recording)
        {
            recorder->record_data(x);
        }
    }
    file.close();
    config.ex_status = 0;
}

int printenv_value(string envar)
{
    int stat = 0;
    if (envar == "USER")
    {
        cout << config.username << endl;
        if (recorder->is_recording)
        {
            recorder->record_data(config.username);
        }
    }
    else if (envar == "HOSTNAME")
    {
        cout << config.hostname << endl;
        if (recorder->is_recording)
        {
            recorder->record_data(config.hostname);
        }
    }
    else if (envar == "PS1")
    {
        cout << config.symbol << endl;
        if (recorder->is_recording)
        {
            recorder->record_data(config.symbol);
        }
    }
    else if (config.env_v.find(envar) != config.env_v.end())
    {
        cout << config.env_v[envar] << endl;
        if (recorder->is_recording)
        {
            recorder->record_data(config.env_v[envar]);
        }
    }
    else if (envar.substr(0, 5) == "alias")
    {
        string al_name = envar.substr(6, envar.length() - 6);
        if (config.alias.find(al_name) != config.alias.end())
        {
            cout << config.alias[al_name] << endl;
            if (recorder->is_recording)
            {
                recorder->record_data(config.alias[al_name]);
            }
        }
        else
        {
            cout << "Not found" << endl;
            if (recorder->is_recording)
            {
                recorder->record_data("Not found");
            }
            return 1;
        }
    }
    else
    {
        fstream file;
        file.open(config.myrc_fullpath.c_str(), ios::in);
        string x;
        if (envar == "PATH")
        {
            file >> x;
            cout << x.substr(5, x.length() - 5) << endl;
            if (recorder->is_recording)
            {
                recorder->record_data(x.substr(5, x.length() - 5));
            }
        }
        else if (envar == "HOME")
        {
            file >> x;
            file >> x;
            cout << x.substr(5, x.length() - 5) << endl;
            if (recorder->is_recording)
            {
                recorder->record_data(x.substr(5, x.length() - 5));
            }
        }
        else
        {
            stat = 1;
        }
        file.close();
    }
    return stat;
}

void handle_echo(vector<Command> commands, int ind)
{
    string s = "";
    for (int i = 1; i < commands[ind].instructions.size(); i++)
    {
        for (int j = 0; j < commands[ind].instructions[i].length(); j++)
        {
            if (commands[ind].instructions[i][j] == '$')
            {
                if (commands[ind].instructions[i][j + 1])
                {
                    if (commands[ind].instructions[i][j + 1] == '$')
                    {
                        int x = getpid();
                        cout << to_string(x);
                        s += x;
                        j++;
                        continue;
                    }
                    else if (commands[ind].instructions[i][j + 1] == '?')
                    {
                        cout << config.ex_status;
                        s += to_string(config.ex_status);
                        j++;
                        continue;
                    }
                }
            }
            cout << commands[ind].instructions[i][j];
            s += commands[ind].instructions[i][j];
        }
        cout << " ";
        s += " ";
    }
    cout << endl;
    if (recorder->is_recording)
    {
        recorder->record_data(s);
    }
    config.ex_status = 0;
}
void print_alarm(string message)
{

    cout << endl
         << message << endl;

    _exit(EXIT_SUCCESS);
}

void set_alarm(vector<string> messages, int seconds)
{

    string message;
    for (int i = 2; i < messages.size(); i++)
    {
        message += messages[i];
        message += " ";
    }
    std::time_t result = std::time(nullptr);
    config.alarm_map[result + seconds] = message;
    pid_t pid = fork();
    config.children.insert(pid);
    config.active[pid] = result + seconds;
    if (pid == 0)
    {

        sleep(seconds);
        print_alarm(message);
    }
}
void alarm_map_to_file()
{
    ofstream off(config.alarm, ios::out);
    for (auto i : config.alarm_map)
    {
        off << i.first << " " << i.second << endl;
    }
}
void file_to_alarm_map()
{
    ofstream off(config.alarm, ios::app);
    off.close();
    ifstream iff(config.alarm);
    string token;
    while (getline(iff, token, '\n'))
    {
        vector<string> v = split_string(token, ' ');
        std::time_t result = std::time(nullptr);
        long long t = stoll(v[0]) - result;
        if (t < 0)
        {
            config.missed_alarm[t] = v[1];
        }
        else
        {   v[1]="alarm 10 "+v[1];
            set_alarm(split_string(v[1], ' '), t);
        }
    }
    iff.close();
    ofstream offf(config.alarm);
    offf.close();
}
void alias_modify()
{
    string line;
    ofstream fout;
    fout.open(config.myrc_fullpath.c_str());
    fout << config.myrc_info;
    for (auto i : config.alias)
    {
        fout << "alias " + i.first + "=" + "\'" + i.second + "\'\n";
    }
}

void handle_alias(vector<Command> commands, int ind)
{
    int len = commands[ind].instructions.size();
    if (len == 1)
    {
        for (auto i : config.alias)
        {
            cout << "alias " + i.first + "=" + "\'" + i.second + "\'" << endl;
            if (recorder->is_recording)
            {
                recorder->record_data("alias " + i.first + "=" + "\'" + i.second + "\' \n");
            }
        }
        config.ex_status = 0;
        return;
    }
    else
    {
        int j, qflag = 0;
        string al_name = "", al_comm = "";
        for (j = 0; j < commands[ind].instructions[1].length(); j++)
        {
            if (commands[ind].instructions[1][j] != '=')
            {
                al_name += commands[ind].instructions[1][j];
            }
            else
            {
                break;
            }
        }
        if (j == commands[ind].instructions[1].length())
        {
            auto search = config.alias.find(commands[ind].instructions[1]);
            if (config.alias.find(commands[ind].instructions[1]) == config.alias.end())
            {
                cout << "Not found" << endl;
                config.ex_status = 1;
                if (recorder->is_recording)
                {
                    recorder->record_data("Not Found");
                }
                return;
            }
            else
            {
                cout << "alias " + search->first + "=" + "\'" + search->second + "\'" << endl;
                config.ex_status = 0;
                return;
            }
        }
        if (commands[ind].instructions[1][j + 1] == '\"' || commands[ind].instructions[1][j + 1] == '\'')
        {
            j += 2;
        }
        else
        {
            j++;
            qflag = 1;
        }
        if (len > 2 && qflag == 1)
        {
            cout << "Not found"
                 << endl;
            if (recorder->is_recording)
            {
                recorder->record_data("Not Found");
            }
            config.ex_status = 1;
            return;
        }
        while (commands[ind].instructions[1][j])
        {
            al_comm += commands[ind].instructions[1][j];
            j++;
        }
        if (len == 2 && qflag == 0)
        {
            al_comm = al_comm.substr(0, al_comm.length() - 1);
        }
        else
        {
            if (al_comm != "")
                al_comm += " ";
        }
        for (int i = 2; i < len - 1; i++)
        {
            al_comm += commands[ind].instructions[i] + " ";
        }

        j = 0;
        if (commands[ind].instructions[len - 1].length() != 1 && len != 2)
        {
            while (j < commands[ind].instructions[len - 1].length() - 1)
            {
                al_comm += commands[ind].instructions[len - 1][j];
                j++;
            }
        }

        config.alias[al_name] = al_comm;
        if (config.username != "")
        {
            env_var_modify();
        }
        config.ex_status = 0;
    }
}
void init()
{
    enable_shell();
    atexit(disable_shell);
    if (getWindowSize(&cursor.win_y, &cursor.win_x) == -1)
    {
        cout << "No Window size available" << endl;
        return;
    }
    file_to_alarm_map();
    trie = new Trie();
    trie->initialize_trie();
    getcwd(config.path, 256);
    config.s_path = display_path(string(config.path));
    create_myrc();
    clear_screen();
    read_file();
    history = new History();
    recorder = new Recorder();

    config.shell_pg = getpid();
    config.isSuspended = false;
    setpgid(config.shell_pg, config.shell_pg);
}
void bghandler(int sig)
{
    int bgstatus = 0;

    int corpse = waitpid((pid_t)-1, &bgstatus, WNOHANG);

    if (corpse > 0)
    {
        cout << corpse << " done with status " << bgstatus << endl;
        bg.remove(corpse);
        tcsetpgrp(config.shell_term, config.shell_pg);
        config.isSuspended = false;
    }
}
void alarm_child(int sig)
{
    int bgstatus = 0;

    int corpse = waitpid((pid_t)-1, &bgstatus, WNOHANG);

    if (corpse > 0)
    {
        config.alarm_map.erase(config.active[corpse]);
        config.active.erase(corpse);
        config.children.erase(corpse);
    }
}
bool check_extension_exist(string file_name)
{
    vector<string> v = split_string(file_name, '.');
    if (config.extension_shorcuts.find("." + v[v.size() - 1]) != config.extension_shorcuts.end())
    {
        return true;
    }
    return false;
}
void execute_runnable(vector<Command> commands)
{
    if (commands.size() == 1 && commands[0].instructions.size() == 2)
    {
        string s = (handle_path(commands[0].instructions[1]));
        if (!pathexists(s))
        {
            cout << "No such file or directory" << endl;
            config.ex_status = 1;
        }
        pid_t pid = fork();
        if (pid == 0)
        {
            vector<string> v = split_string(s, '.');
            close(2);
            if (check_extension_exist(commands[0].instructions[1]))
                execl(config.extension_shorcuts["." + v[v.size() - 1]].second.c_str(), config.extension_shorcuts["." + v[v.size() - 1]].first.c_str(), s.c_str(), (char *)0);
            else
                execl("/usr/bin/xdg-open", "xdg-open", s.c_str(), (char *)0);
            exit(0);
        }
        else
        {
        }
    }
}

int check_builinCommands(vector<Command> commands, vector<Command> &alias,bool* remainder)
{
    int ind = 0;
    auto search = config.alias.find(commands[ind].instructions[0]);
    if (search != config.alias.end())
    {
        if (search->second == "exit")
        {
            kill_all_processes();
            exit(EXIT_SUCCESS);
        }
        alias = process_commands(search->second);
        return 1;
    }
    if (commands[ind].instructions[0] == "$$")
    {
        string x = to_string(getpid());
        cout << x + ": Command not found" << endl;
        if (recorder->is_recording)
        {
            recorder->record_data(x + ": Command not found");
        }
        config.ex_status = 127;
        return 0;
    }
    if (commands[ind].instructions[0] == "$?")
    {
        string x = to_string(config.ex_status);
        cout << x + ": Command not found" << endl;
        if (recorder->is_recording)
        {
            recorder->record_data(x + ": Command not found");
        }
        config.ex_status = 127;
        return 0;
    }
    if (strcmp(commands[ind].instructions[0].c_str(), "cd") == 0 || strcmp(commands[ind].instructions[0].c_str(), "pwd") == 0 || strcmp(commands[ind].instructions[0].c_str(), "export") == 0)
    {
        path_commands(commands, ind);
        return 0;
    }

    if ((strcmp(commands[ind].instructions[0].c_str(), "env") == 0 || strcmp(commands[ind].instructions[0].c_str(), "printenv") == 0) && commands[ind].instructions.size() == 1)
    {
        printmyrc();
        config.ex_status = 0;
        return 0;
    }

    if (strcmp(commands[ind].instructions[0].c_str(), "printenv") == 0)
    {
        if (commands[ind].instructions.size() == 2)
        {
            if (printenv_value(commands[ind].instructions[1]))
            {
                config.ex_status = 1;
            }
            else
            {
                config.ex_status = 0;
            }
        }
        else if (commands[ind].instructions.size() == 3)
        {
            if (printenv_value(commands[ind].instructions[1] + " " + commands[ind].instructions[2]))
            {
                config.ex_status = 1;
            }
            else
            {
                config.ex_status = 0;
            }
        }
        return 0;
    }
    if (strcmp(commands[ind].instructions[0].c_str(), "echo") == 0)
    {
        handle_echo(commands, ind);
        return 0;
    }

    if (strcmp(commands[ind].instructions[0].c_str(), "alias") == 0)
    {
        handle_alias(commands, ind);
        return 0;
    }
    if (strcmp(commands[ind].instructions[0].c_str(), "open") == 0)
    {
        execute_runnable(commands);
        return 0;
    }
    if (strcmp(commands[ind].instructions[0].c_str(), "alarm") == 0)
    {
        set_alarm(commands[ind].instructions, stoi(commands[ind].instructions[1]));
        *remainder=true;
        return 0;
    }
    return -1;
}
void setFg(int pid)
{
    if (pid > 0)
    {
        int status;
        cout << " + " << pid << "\t" << endl;
        tcsetpgrp(config.shell_term, pid);
        config.isSuspended = true;
    }
}
int start_command(vector<Command> commands)
{

    for (int i = 0; i < commands.size() - 1; i++)
    {
        pid_t pid, wpid;
        int status;

        int pd[2];

        if (pipe(pd) < 0)
        {
            cout << "pipe failed" << endl;
        }

        char *args[commands[i].instructions.size() + 1];

        for (int j = 0; j < commands[i].instructions.size(); j++)
        {
            args[j] = new char[commands[i].instructions[j].length()];
            strcpy(args[j], commands[i].instructions[j].c_str());
        }
        args[commands[i].instructions.size()] = NULL;

        pid = fork();
        bool redirectFlag = 0;

        switch (pid)
        {
        case -1:
            cout << "fork failed" << endl;
            break;

        case 0:
            dup2(pd[1], 1);
            close(pd[0]);
            close(pd[1]);

            if (args[1] != NULL)
            {
                int index;
                for (int j = 0; j < commands[i].instructions.size(); j++)
                {
                    if (strcmp(args[j], ">") == 0 || strcmp(args[j], ">>") == 0)
                    {
                        index = j;
                        redirectFlag = 1;
                        break;
                    }
                }

                if (redirectFlag)
                {
                    int fd;
                    if (strcmp(args[index], ">") == 0)
                    {
                        fd = open(args[index + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
                    }
                    else
                    {
                        fd = open(args[index + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
                    }
                    dup2(fd, 1);
                    args[index] = 0;
                    if (execvp(args[0], args) == -1)
                    {

                        if (recorder->is_recording)
                        {
                            recorder->record_data("Command Failed");
                        }
                        cout << "command failed" << flush;
                        config.ex_status = 127;
                        _exit(EXIT_FAILURE);
                    }
                    close(fd);
                }
            }

            if (!redirectFlag)
            {
                if (execvp(args[0], args) == -1)
                {
                    if (recorder->is_recording)
                    {
                        recorder->record_data("Command Failed");
                    }
                    cout << "command failed" << endl;
                    config.ex_status = 127;
                    _exit(EXIT_FAILURE);
                }
            }

            for (int i = 0; i < commands[i].instructions.size() + 1; i++)
                delete (args[i]);
            break;

        default:
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            break;
        }

        dup2(pd[0], 0);
        close(pd[1]);
        close(pd[0]);
    }

    int n = commands.size() - 1;

    char *args[commands[n].instructions.size() + 1];

    for (int i = 0; i < commands[n].instructions.size(); i++)
    {
        args[i] = new char[commands[n].instructions[i].length()];
        strcpy(args[i], commands[n].instructions[i].c_str());
    }
    args[commands[n].instructions.size()] = NULL;

    bool redirectFlag = 0;

    if (args[1] != NULL)
    {
        int index;
        for (int i = 0; i < commands[n].instructions.size(); i++)
        {
            if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0)
            {
                index = i;
                redirectFlag = 1;
                break;
            }
        }

        if (redirectFlag)
        {
            int fd;
            if (strcmp(args[index], ">") == 0)
            {
                fd = open(args[index + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            }
            else
            {
                fd = open(args[index + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
            }
            dup2(fd, 1);
            args[index] = 0;
            if (execvp(args[0], args) == -1)
            {
                if (recorder->is_recording)
                {
                    recorder->record_data("Command Failed");
                }
                cout << "command failed" << flush;
                config.ex_status = 127;
                _exit(EXIT_FAILURE);
            }
            close(fd);
        }
    }

    if (!redirectFlag)	
    {	
        if (recorder -> is_recording && freopen ("recording.txt", "a", stdout)){	
            if (execvp(args[0], args) == -1)	
            {	
                cout << "command failed" << endl;	
                _exit(EXIT_FAILURE);	
            }	
        } else {	
            if (execvp(args[0], args) == -1)	
            {	
                cout << "command failed" << endl;	
                _exit(EXIT_FAILURE);	
            }	
        }	
        for (int i = 0; i < commands[n].instructions.size() + 1; i++)	
            delete (args[i]);	
    }

    return 1;
}

int main(int argc, char const *argv[])
{
    init();
    bool remainder = true;
    signal(SIGINT, ctrl_c);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, alarm_child);
    int newy, newx;
    while (1)
    {
        bool exec = true;

        getWindowSize(&newy, &newx);
        if (newy != cursor.win_y || newx != cursor.win_x)
        {

            cursor.win_y = newy;
            cursor.win_x = newx;
        }
        if (config.isSuspended)
            continue;
        string command = read_command(&exec,&remainder);
        bool isBg = 0;
        for (int i = command.length() - 1; i >= 0; i--)
        {
            if (command[i] == '&')
            {
                isBg = 1;
                command.pop_back();
                break;
            }

            if (command[i] != ' ')
                break;
        }

        if (command[command.length() - 1] == ' ')
        {
            int i = command.length() - 1;
            while (command[i] == ' ')
            {
                command.pop_back();
            }
        }
        if (!command.empty())
        {
            history->insert(command);
            if (command == "record stop")
            {
                recorder->stop_recording();
                continue;
            }
            if (recorder->is_recording)
            {
                recorder->record_data(command);
            }
            if (command == "record start")
            {
                recorder->start_recording();
                continue;
            }
            if (command == "exit")
            {
                alarm_map_to_file();
                kill_all_processes();
                exit(EXIT_SUCCESS);
                return 0;
            }
            if (command == "history")
            {
                history->print_history();
                continue;
            }
            if (command.substr(0, 8) == "history " && command.length() > 8)
            {
                string keyword = command.substr(8, command.length() - 8);
                history->search_history_file(keyword);
                continue;
            }
            if (command == "fg")
            {
                setFg(bg.front());
                continue;
            }
            vector<Command> cmds = process_commands(command);
            vector<Command> builin_commands;
            int val = check_builinCommands(cmds, builin_commands,&remainder);
            if (val == 1)
            {
                int bval = check_builinCommands(builin_commands, builin_commands,&remainder);
                if (bval == 0)
                {
                    continue;
                }
                cmds = builin_commands;
            }
            else if (val == 0)
            {
                continue;
            }

            if (!cmds.empty() && exec)
            {
                if (isBg)
                    signal(SIGCHLD, bghandler);

                signal(SIGTTIN, SIG_DFL);
                pid_t pid = fork();
                pid_t wpid;
                int status = 0;

                switch (pid)
                {
                case -1:
                    cout << "fork error" << endl;
                    break;

                case 0:
                    if (isBg)
                    {
                        setpgrp();
                        isBg = 0;
                    }
                    start_command(cmds);
                    break;

                default:
                    if (!isBg)
                    {
                        do
                        {
                            wpid = waitpid(pid, &status, WUNTRACED);
                        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                    }
                    if (WIFEXITED(status))
                    {
                        int exit_status = WEXITSTATUS(status);
                        config.ex_status = exit_status;
                    }
                    if (isBg)
                    {
                        bg.push_front(pid);
                        cout << bg.size() << " " << pid << endl;
                        isBg = 0;
                    }

                    setpgid(pid, pid);
                    tcsetpgrp(config.shell_term, config.shell_pg);
                }
            }
        }
    }
    return 0;
}
