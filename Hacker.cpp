#include "Exploit.h"
#include <filesystem>
#include <vector>
#include <iostream>
#include <windows.h>

typedef Exploit* (*CreateExploitFn)();

class TerminalManager{
private:
    HANDLE *TM;
public:
    TerminalManager(HANDLE);
    ~TerminalManager(){};
    void writeText(std::string);
};

TerminalManager::TerminalManager(HANDLE TM) : TM(&TM) {}

int main(){
    HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Get handle to terminal
    auto TM = new TerminalManager(&stdHandle);

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

        if (choice > 0 && choice <= (int)loaded_exploits.size()) {
            loaded_exploits[choice - 1]->execute();
        }
    }
    for (auto e : loaded_exploits) delete e;
    for (auto d : dlls) FreeLibrary(d);
    return 0;
}