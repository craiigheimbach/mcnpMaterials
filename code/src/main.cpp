 #include <iostream>
#include <cstdio>
#include "utilities.h"
#include "controller.h"

//  ***************************************************************************
int main(int argc, char** argv)
{
    Controller controller;
    auto defaultFileName = "commands.txt";

    auto commandFileName = (argc < 2) ? defaultFileName : argv[1];

    if (!controller.setControlFileName(commandFileName))
    {
        std::cout << "Default file name is " << defaultFileName << "." << std::endl;
        std::cout << "Start program with McnpMaterials fileName" << std::endl;
        return 2;
    }
    controller.execute();
    return 0;
}