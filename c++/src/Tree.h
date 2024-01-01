//
// Created by Karan Gandhi on 25-12-2023.
//

#include <string>
#include <vector>
#include <sstream>
#include "Files.h"

#ifndef GIT_CLONE_TREE_H
#define GIT_CLONE_TREE_H

namespace gitc {

    class Tree {
    public:
        struct Tree_entry {
            std::string path;
            std::string hash;
            std::string type;
        };


        Tree(std::string _hash) : hash(_hash) {
            read_from_file();
        }

        ~Tree() {
            write_to_file();
        }

        void add_entry(std::string path, std::string hash, std::string type) {
            Tree_entry *new_entry = new Tree_entry();
            new_entry->path = path;
            new_entry->hash = hash;
            new_entry->type = type;

            entries.push_back(new_entry);
        }

        std::vector<Tree_entry *> get_entries() {
            return entries;
        }

        std::string get_hash_of_directory(const std::string &path) {
            if (path.find('/') == std::string::npos) {
                for (Tree_entry *entry: entries) {
                    if (entry->path == path) {
                        return entry->hash;
                    }
                }

                return "";
            }

            std::string top_directory = path.substr(0, path.find('/'));
            std::string next_path = path.substr(path.find('/') + 1);

            for (Tree_entry *entry: entries) {
                if (entry->path == top_directory) {
                    Tree *tree = new Tree(entry->hash);
                    return tree->get_hash_of_directory(next_path);
                }
            }
            return "";
        }

        bool search_for_hash(const std::string &hash) {
            for (Tree_entry *entry: entries) {
                if (entry->hash == hash) {
                    return true;
                } else if(entry->type == "tree") {
                    Tree *tree = new Tree(entry->hash);
                    if (tree->search_for_hash(hash)) {
                        delete tree;
                        return true;
                    }
                    delete tree;
                }
            }

            return false;
        }

    private:
        std::string hash;
        std::vector<Tree_entry *> entries;

        void read_from_file() {
            std::string file_path = Files::join_path(Files::root_path(), ".gitc/objects/" + hash);
            std::ifstream file(file_path);

            if (!file.good()) {
                return;
            }

            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                Tree_entry *new_entry = new Tree_entry();

                iss >> new_entry->type >> new_entry->hash;
                std::getline(iss, new_entry->path);
                new_entry->path.erase(new_entry->path.begin(),
                                      std::find_if(new_entry->path.begin(), new_entry->path.end(),
                                                   [](unsigned char ch) {
                                                       return !std::isspace(ch);
                                                   }));
                entries.push_back(new_entry);
            }

            file.close();
        }

        void write_to_file() {
            std::string file_path = Files::join_path(Files::root_path(), ".gitc/objects/" + hash);
            std::ofstream file(file_path);

            for (Tree_entry *entry: entries) {
                file << entry->type << " " << entry->hash << " " << entry->path << std::endl;
            }

            file.close();
        }
    };

} // gitc

#endif //GIT_CLONE_TREE_H
