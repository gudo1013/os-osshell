#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <filesystem>
#include <sys/wait.h>
#include <ctype.h>

void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);
void addToHistoryFile(std::string command, int *number_saved, int command_size);
void history(int number, int filesize);
void findAndExecute(char **command, std::vector<std::string> path_list);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);

    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    std::vector<std::string> command_list; // to store command user types in, split into its various parameters
    char **command_list_exec; // command_list converted to an array of character arrays

    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)

    // History file counting
    std::ifstream readHistoryFile("history.txt", std::ios::out);
    int history_number;
    history_number = 0;
    std::string templine;
    while(std::getline(readHistoryFile, templine))
    {
        history_number++;
    }//while
    readHistoryFile.close();
    
    // Prompt user for input until 'exit' is entered
    while(true)
    {
        // Print prompt for user to input
        std::cout << "osshell> ";

        // Get user input for next command
        std::string user_input;
        std::getline(std::cin, user_input);
        splitString(user_input, ' ', command_list);
        vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);

        // Empty command handling
        if(command_list_exec[0]!=NULL)
        {
            // History command
            int history_clear = 0;
            if(strcmp(command_list_exec[0], "history") == 0)
            {
                // 'history'
                if(command_list_exec[1] == NULL)
                {
                    // Print the entire history
                    history(history_number, history_number);
                }//if
                // 'history n'
                else if(atoi(command_list_exec[1]) > 0)
                {
                    // Check that the whole second arg is a number
                    bool isnum = true;
                    for(int j = 0; j<strlen(command_list_exec[1]); j++)
                    {
                        if(!(isdigit(command_list_exec[1][j])))
                        {
                            isnum = false;
                        }
                    }//while
                    if(isnum)
                    {
                        // Print the last number of command specified
                        history(atoi(command_list_exec[1]), history_number);
                    }//if
                    else
                    {
                        std::cout << "Error: history expects an integer > 0 (or 'clear')\n";
                    }
                }//else if
                // 'history clear'
                else if(strcmp(command_list_exec[1], "clear")==0)
                {
                    std::remove("history.txt");
                    history_clear = 1;
                }//else if
                // Incorrect formatting for history
                else
                {
                    std::cout << "Error: history expects an integer > 0 (or 'clear')\n";
                }//else
            }//if
            // Break shell when command 'exit' is inputed
            else if(strcmp(command_list_exec[0], "exit") == 0)
            {
                addToHistoryFile(user_input, &history_number, command_list.size());
                break;
            }//else if
            // Any other possible command entered
            else
            {
                findAndExecute(command_list_exec, os_path_list);
            }//else

            // Add command to history if not 'history clear'
            if(history_clear == 0)
            {
                addToHistoryFile(user_input, &history_number, command_list.size());
            }//if

            
        
        }//if
        // free array
        freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
        
    }//while
    
    return 0;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}

/*
    command: the command to be saved
    number_saved: number of commands saved in the history file already
    command_size: number of parameters in the command
*/
void addToHistoryFile(std::string command, int *number_saved, int command_size)
{
    // If the number of commands saved exceeds 128, delete the first
    if(*number_saved == 128)
    {
        std::ofstream tempFile("temp.txt", std::ios::app);
        std::ifstream historyFile("history.txt", std::ios::out);
        std::string line;
        std::getline(historyFile, line);
        while(std::getline(historyFile, line))
        {
            tempFile << line << "\n";
        }//while
        tempFile.close();
        historyFile.close();
        std::remove("history.txt");
        std::rename("temp.txt", "history.txt");
        *number_saved = *number_saved - 1;
    }//if
    // Add the command to the history file
    std::ofstream historyFile("history.txt", std::ios::app);
    historyFile << command;
    historyFile << "\n";
    historyFile.close();
    *number_saved = *number_saved + 1;
}

/*
number: the number of previous commands to be printed
filesize: how many commands currently stored in the history file
*/
void history(int number, int filesize)
{
    //print correct lines in history
    std::ifstream readHistoryFile("history.txt", std::ios::out);
    std::string line;
    int command_number = 1;
    while(std::getline(readHistoryFile, line))
    {
        if(command_number > filesize - number)
        {
            printf("%3d: ", command_number);
            std::cout << line << "\n"; 
        }//if
        command_number++;
    }//while
    readHistoryFile.close();
}//history

/*
command: the command to be executed
path_list: list of PATH variables to search through for executables
*/
void findAndExecute(char **command, std::vector<std::string> path_list)
{
    bool exists = false;
    std::string full_path;
    pid_t process;
    // Search in the current directory
    if( command[0][0] == '.' || command[0][0] == '/' )
    {
        if(std::filesystem::exists(command[0]) && !exists)
        {
            exists = true;
            full_path = command[0];
        }//if
    }
    // Search in the PATH enviroment variable location
    else
    {
        for (int i = 0; i < path_list.size(); i++)
        {
            
            if(std::filesystem::exists(path_list[i] + "/" + command[0]) && !exists)
            {
                exists = true;
                full_path = (path_list[i] + "/" + command[0]).c_str();
            }//if
        }//for
    }
    if(exists)
    {
        process = fork();
        // Child process
        if(process == 0)
        {
            execv(full_path.c_str(), command);
            exit(0);
            
        }//if
        else
        {
            wait(NULL);
        }//else
    }//if
    else
    {
        printf("%s: Error command not found\n", command[0]);
    }//else
}//findAndExecute

