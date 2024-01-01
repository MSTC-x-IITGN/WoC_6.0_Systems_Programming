//
// Created by Karan Gandhi on 24-12-2023.
//
#include <iostream>
#include <string>

#include "gitc.h"
#include "Files.h"
#include "Files.h"

int main(int argc, char *argv[]) {

    if (argc == 1) {
        // display help info
        gitc::gitc::help();
        gitc::Files::delete_file(".gitc/objects/9koyr7mhitbp");
    } else {
        std::string command = argv[1];

        if (command == "-h" || command == "--help") {
            // display help info
            gitc::gitc::help();
        }

        if (command == "init") {
            gitc::gitc::init();
            return 0;
        }

        if (command == "add") {
            if (argc == 2) {
                std::cout << "Nothing specified, nothing added." << std::endl;
                return 0;
            }

            for (int i = 2; i < argc; i++) {
                gitc::gitc().add(argv[i]);
            }
        } else if (command == "rm") {
            if (argc == 2) {
                std::cout << "Nothing specified, nothing removed." << std::endl;
                return 0;
            }

            for (int i = 2; i < argc; i++) {
                gitc::gitc().rm(argv[i]);
            }

        } else if (command == "commit") {
            if (argc != 4 || (std::string) argv[2] != "-m") {
                std::cout << "fatal: Please provide a commit message using -m flag" << std::endl;
                return 0;
            }

            gitc::gitc().commit(argv[3]);
        } else if (command == "log") {
            gitc::gitc().log();
        } else if (command == "checkout") {
            if (argc != 3) {
                std::cout << "fatal: Please provide a commit hash" << std::endl;
                return 0;
            }

            gitc::gitc().checkout(argv[2]);
        } else if(command == "revert") {
            if (argc != 3) {
                std::cout << "fatal: Please provide a commit hash" << std::endl;
                return 0;
            }

            gitc::gitc().revert(argv[2]);
        } else if (command == "status") {
            gitc::gitc().status();
        } else {
            std::cout << "gitc: '" << command << "' is not a gitc command. See 'gitc --help'." << std::endl;
        }
    }

    return 0;
}
