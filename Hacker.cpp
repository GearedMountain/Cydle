#include "Exploit.h"
#include <filesystem>
#include <vector>
#include <iostream>
#include <windows.h>
#include <sstream>
#include <limits> 
typedef Exploit* (*CreateExploitFn)();

class TerminalManager{
private:
    HANDLE TM;
public:
    TerminalManager(HANDLE);
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

    int CRYPTO = 5492;
    int CASH = 5492;
    writeText("@BLUE@CRYPTO:@WHITE@ " + std::to_string(CRYPTO) + " " + "@YELLOW@CASH:@WHITE@ " + std::to_string(CASH));
    SetConsoleCursorPosition(TM, { 0, 1 });
}
};

TerminalManager::TerminalManager(HANDLE HD) : TM(HD) {}

int main(){
    HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get handle to terminal
    auto TM = new TerminalManager(stdHandle);
    TM->ClearConsole();

    std::vector<Exploit*> loaded_exploits;
    std::vector<HMODULE> dlls;

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
    while(true)
    {
        std::cout << "Loaded exploits:\n";
        for (size_t i = 0; i < loaded_exploits.size(); ++i) {
            std::cout << i + 1 << ". " << loaded_exploits[i]->getName() << "\n";
        }

        int choice;
        std::cout << "Choose an exploit to run: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear(); // clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard bad input
            TM->writeText("@RED@[!]@WHITE@ Invalid input. Please enter a number.\n");
            continue;
        }

        if (choice > 0 && choice <= (int)loaded_exploits.size()) {
            TM->writeText(loaded_exploits[choice-1]->instructions());
            //loaded_exploits[choice - 1]->execute();
        }
    }
    for (auto e : loaded_exploits) delete e;
    for (auto d : dlls) FreeLibrary(d);
    return 0;
}