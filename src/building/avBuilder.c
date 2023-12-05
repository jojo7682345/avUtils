#include <AvUtils/avBuilder.h>
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <AvUtils/avLogging.h>
#include <stdio.h>

typedef struct Configuration {
    AvString str;
    void (*func) (void);
} Configuration;
uint32 defaultConfiguration;

AV_DS(AvDynamicArray, Configuration) configurations;


static avConfigCallback findConfiguration(AvString str) {

}

void printHelp(){

    
}

__attribute__((weak)) void avBuildSetup() {}

int main(int argC, const char *argV[]){
    avBuildSetup();
    argC--;
    if(argC >= 1){
        AvString config = AV_CSTR(argV[1]);
        avConfigCallback callback = findConfiguration(config);
        if(callback==nullptr){
            printHelp();
            return 1;
        }
        if(!callback()){
            printHelp();
            return 1;
        }
        return 0;
    }else{
        uint32 configurationCount = avDynamicArrayGetSize(configurations);
        if(configurationCount == 0){
            printf("no configurations");
            return -1;
        }
    }
}