#include "Exploit.h"
#include <filesystem>
#include <vector>
#include <iostream>
#include <windows.h>
#include <sstream>
#include <limits> 
#include <curl/curl.h>
typedef Exploit* (*CreateExploitFn)();
class TerminalManager{

private:
    std::vector<Exploit*>& loaded_exploits;
    HANDLE TM;
    int CRYPTO = 1;
    int CASH = 1;
    DWORD originalMode;

public:
    TerminalManager(HANDLE, std::vector<Exploit*>&);
    ~TerminalManager(){};
    void writeText(std::string Instructions) {
        //Split the instructions by @ sign, read any color inputs
        std::istringstream ss(Instructions);
        std::string token;
        while (std::getline(ss, token, '@')) {
            //std::cout << "TOKEN IS:" << token << std::endl;
            if(token == "RED") {
                SetConsoleTextAttribute(TM, 0x0C);
            } else if (token == "WHITE") {
                SetConsoleTextAttribute(TM, 0x0F);
            } else if (token == "YELLOW") {
                SetConsoleTextAttribute(TM, 0x0E);
            } else if (token == "BLUE") {
                SetConsoleTextAttribute(TM, 0x09);
            } else if (token == "GREEN") {
                SetConsoleTextAttribute(TM, 0x0A);
            } else{
                std::cout << token;
            }
                
        }
        
    }

    void ClearConsole() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD count;
        DWORD cellCount;

        // Get the number of character cells in the current buffer
        if (!GetConsoleScreenBufferInfo(TM, &csbi)) return;
        cellCount = csbi.dwSize.X * csbi.dwSize.Y;

        // Fill the entire screen with spaces
        FillConsoleOutputCharacter(TM, ' ', cellCount, { 0, 0 }, &count);

        // Fill the entire screen with the current colors and attributes
        FillConsoleOutputAttribute(TM, csbi.wAttributes, cellCount, { 0, 0 }, &count);

        // Move the cursor to the top-left corner
        SetConsoleCursorPosition(TM, { 0, 0 });

        writeText("@BLUE@CRYPTO:@WHITE@ " + std::to_string(CRYPTO) + " " + "@YELLOW@CASH:@WHITE@ " + std::to_string(CASH));
        SetConsoleCursorPosition(TM, { 0, 1 });
    }

    bool ValidInput(){
        if (std::cin.fail()) {
            std::cin.clear(); // clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard bad input
            return false;
        }
        return true;
    }

    //Check if the command has the correct ammount of arguments, 1 is the command with no arguments
    bool CheckArguments(int Expected, int Actual, bool WriteText = true){
        if(Actual>Expected){
            if(WriteText){
                writeText("@RED@Too many arguments@WHITE@ " + std::to_string(Expected) + " argument(s) expected, " +  std::to_string(Actual) + " provided\n");
            }
            return false;
        } else if (Actual<Expected)
        {
            if(WriteText){
                writeText("@RED@Too few arguments@WHITE@ " + std::to_string(Expected) + " argument(s) expected, " +  std::to_string(Actual) + " provided\n");
            }
            return false;
        }
        return true;
    }

    void RunCommand(std::string command){
        std::istringstream ss(command);
        std::string commandlet;
        std::vector<std::string> fullCommand;

        while (ss >> commandlet){
            fullCommand.push_back(commandlet);
        }

        if(fullCommand[0] == "exploits"){
            if(!CheckArguments(1, fullCommand.size())){
             return;   
            } else{
                std::cout << "Loaded exploits:\n";
                for (size_t i = 0; i < loaded_exploits.size(); ++i) {
                    std::cout << i + 1 << ". " << loaded_exploits[i]->getName() << "\n";
                }
            }
        } else if(fullCommand[0] == "exploit"){
            if(!CheckArguments(2, fullCommand.size())){
             return;   
            } else{
                int cmdArgument;
                try {
                    cmdArgument = std::stoi(fullCommand[1]);
                } catch (const std::exception& e){
                    writeText("Second argument must be an integer value corresponding with your exploit selection");
                    return;
                }
                if (cmdArgument > 0 && cmdArgument <= (int)loaded_exploits.size()) {
                    const auto& messageData = loaded_exploits[std::stoi(fullCommand[1])-1]->instructions();
                    for (const auto& message : messageData){
                        writeText(message);
                    }
                }
            //loaded_exploits[choice - 1]->execute();
                //select exploit
            }
        } else if(fullCommand[0] == "clear"){
            if(!CheckArguments(1, fullCommand.size())){
             return;   
            } else{
                ClearConsole();
            }
        } else if(fullCommand[0] == "update"){
            if(!CheckArguments(1, fullCommand.size())){
             return;   
            } else{
                // Put console into update mode where it refreshes your crypto ammount but doesnt allow input
            }
        }  else if(fullCommand[0] == "options"){
            if(!CheckArguments(1, fullCommand.size())){
             return;   
            } else{

            }
        } else if(fullCommand[0] == "help"){
            if(CheckArguments(2, fullCommand.size(), false)){
      
                writeText("@WHITE@Reading more about command @BLUE@" + fullCommand[1] +"@WHITE@\n");   
                return;             
            } else if(CheckArguments(1, fullCommand.size(), false)){
                writeText("@WHITE@Use the help command to learn more about commands with help [name]. Available commands include:\nclear\nexploit\nexploits\n");
            } else{
                writeText("@RED@Too many arguments@WHITE@ " + std::to_string(1) + " argument(s) expected, " +  std::to_string(fullCommand.size()) + " provided\n");
            } 
        } else{
            writeText("@RED@No Command Found@WHITE@\n");
        }
    }
    void CommandExploit(){
        
    }
};


TerminalManager::TerminalManager(HANDLE HD, std::vector<Exploit*>& DLLRef) : TM(HD), loaded_exploits(DLLRef) {}

int main(){
    HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    std::vector<Exploit*> loaded_exploits;
    std::vector<HMODULE> dlls;

    // Get handle to terminal
    auto TM = new TerminalManager(stdHandle, loaded_exploits);
    TM->ClearConsole();

    for (const auto& file : std::filesystem::directory_iterator("./exploits")){
        if (file.path().extension() == ".dll") {
            HMODULE h = LoadLibraryW(file.path().c_str()); 
            // Load Library parameter is long pointer, std::string isnt a pointer 
            //but c style string is a pointer to char array with null terminating 0 so its accepted
                

            if(h){
                auto create = (CreateExploitFn)GetProcAddress(h,"create_exploit");
                //CreateExploitFn is the same as Exploit* (*)()
                if(create){
                    Exploit* exploit = create();
                    loaded_exploits.push_back(exploit);
                    dlls.push_back(h);
                }
            }
        }
    }

    std::cout << "Welcome to Cydle 1.1\n";

    while(true)
    {
        
        std::cout<<"hacker/win10>";
        std::string command;
        std::getline(std::cin,command);
        if(command == "") { TM->writeText("@RED@No Command Found@WHITE@\n"); continue; };
        //std::cout << "You entered: " << command;
        TM->RunCommand(command);

        std::cout << "\n";
        
        /*if(command == "exploits") {
            std::cout << "Loaded exploits:\n";
                for (size_t i = 0; i < loaded_exploits.size(); ++i) {
                    std::cout << i + 1 << ". " << loaded_exploits[i]->getName() << "\n";
                }
        }*/


        /*if(!TM->ValidInput())
        {
            TM->writeText("@RED@[!]@WHITE@ Invalid input.\n");
            continue;
        }*/
        

        /*if (choice > 0 && choice <= (int)loaded_exploits.size()) {
            TM->writeText(loaded_exploits[choice-1]->instructions());
            //loaded_exploits[choice - 1]->execute();
        }*/
    }
    for (auto e : loaded_exploits) delete e;
    for (auto d : dlls) FreeLibrary(d);
    return 0;
}