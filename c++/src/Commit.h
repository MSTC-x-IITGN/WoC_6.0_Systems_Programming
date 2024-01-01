//
// Created by Karan Gandhi on 25-12-2023.
//

#include <string>
#include <algorithm>
#include <time.h>
#include "Index.h"
#include "Files.h"
#include "Tree.h"

#ifndef GIT_CLONE_COMMIT_H
#define GIT_CLONE_COMMIT_H

namespace gitc {

    class Commit {
    public:
        Commit(std::string _commit_hash) : commit_hash(_commit_hash) {
            read_from_file();
        }


        ~Commit() {
            write_to_file();
        }

        std::string get_commit_hash() {
            return commit_hash;
        }

        std::string get_parent_commit_hash() {
            return parent_hash;
        }

        static Commit *create_commit_from_index(Index &index, std::string previous_commit_hash, std::string message) {
            Commit *new_commit = new Commit();

            new_commit->commit_hash = Files::create_hash(HASH_LENGTH);
            new_commit->tree_hash = Files::create_hash(HASH_LENGTH);
            new_commit->parent_hash = previous_commit_hash;
            new_commit->commit_message = message;
            new_commit->timestamp = time(nullptr);

            int files_changed = create_tree_recursively(index.get_entries(), new_commit->tree_hash, previous_commit_hash,
                                                        ".");

            std::cout << "[master] " << new_commit->commit_hash << ": " << message << std::endl;
            std::cout << files_changed << " files changed" << std::endl;

            for (auto entry: index.get_entries()) {
                if (entry->stage_number == STAGED) {
                    std::cout << "  " << entry->path << std::endl;
                }
            }
            index.unsatge_entries();

            return new_commit;
        }

        void print_commit(bool is_head) {
            std::cout << "commit: " << commit_hash << (is_head ? " (HEAD -> master)" : "") << std::endl;
            std::cout << "time: " << timestamp << std::endl << std::endl;
            std::cout << "\t" << commit_message << std::endl << std::endl;
        }

        void update_working_directory() {
            // update the current working directory to the state of the commit
            Files::clear_working_dir_recursively();
            update_working_directory_recursively(tree_hash, "");
        }

        void delete_commit() {
            // delete the commit from the .gitc/objects directory
            recursively_delete_tree(tree_hash, new Commit(parent_hash));
        }

    private:
        std::string commit_hash;
        std::string tree_hash;
        std::string parent_hash;
        std::string commit_message;
        unsigned long timestamp;

        Commit() {}

        void read_from_file() {
            std::string file_path = Files::join_path(Files::root_path(), ".gitc/objects/" + commit_hash);
            std::ifstream file(file_path);

            std::string line;
            std::getline(file, line);
            std::istringstream iss(line);

            std::string type;
            iss >> type >> tree_hash;

            std::getline(file, line);
            iss = std::istringstream(line);
            iss >> type >> parent_hash;

            std::getline(file, line);
            iss = std::istringstream(line);
            iss >> type >> timestamp;

            std::getline(file, commit_message);

            file.close();
        }

        void write_to_file() {
            std::string file_path = Files::join_path(Files::root_path(), ".gitc/objects/" + commit_hash);
            std::ofstream file(file_path);

            file << "tree " << tree_hash << std::endl;
            file << "parent " << parent_hash << std::endl;
            file << "time " << timestamp << std::endl;
            file << commit_message << std::endl;
        }

        static int create_tree_recursively(std::vector<Index_entry *> entries, const std::string &current_tree_hash,
                                           const std::string &last_commit_hash, const std::string &current_path) {
            std::vector<std::string> directories;
            std::vector<std::string> files;
            int files_changed = 0;

            for (Index_entry *entry: entries) {
                if (entry->stage_number == UNTRACKED)
                    continue;

                if (entry->path.find('/') != std::string::npos) {
                    directories.push_back(entry->path.substr(0, entry->path.find('/')));
                } else {
                    files.push_back(entry->path);
                }
            }

            directories.erase(std::unique(directories.begin(), directories.end()), directories.end());
            files.erase(std::unique(files.begin(), files.end()), files.end());

            Tree *new_tree = new Tree(current_tree_hash);

            for (auto &directory: directories) {
                std::vector<Index_entry *> directory_entries;
                bool changed = false;
                for (Index_entry *entry: entries) {
                    if (entry->path.find(directory) == 0) {
                        Index_entry *new_entry = new Index_entry();

                        new_entry->path = entry->path.substr(entry->path.find('/') + 1);
                        new_entry->hash = entry->hash;
                        new_entry->stage_number = entry->stage_number;

                        if (entry->stage_number == STAGED) {
                            changed = true;
                        }

                        directory_entries.push_back(new_entry);
                    }
                }

                if (!changed) {
                    // get the hash of the tree for the same directory from the prev commit
//                    std::string prev_tree_hash = new_tree->get_hash_of_directory(directory);
                    Commit *prev_commit = new Commit(last_commit_hash);
                    Tree *prev_root_tree = new Tree(prev_commit->tree_hash);
                    std::string prev_tree_hash = prev_root_tree->get_hash_of_directory(
                            Files::join_path(current_path, directory));

                    new_tree->add_entry(directory, prev_tree_hash, "tree");
                    continue;
                }

                std::string next_tree_hash = Files::create_hash(HASH_LENGTH);
                files_changed += create_tree_recursively(directory_entries, next_tree_hash, last_commit_hash,
                                                         Files::join_path(current_path, directory));
                new_tree->add_entry(directory, next_tree_hash, "tree");
            }

            for (auto &file: files) {
                for (Index_entry *entry: entries) {
                    if (entry->path == file) {
                        new_tree->add_entry(file, entry->hash, "blob");

                        if (entry->stage_number == STAGED) {
                            files_changed++;
                        }
                    }
                }
            }

            delete new_tree;

            return files_changed;
        }


        static void update_working_directory_recursively(const std::string &current_tree_hash,
                                                         const std::string &path) {
            Tree *current_tree = new Tree(current_tree_hash);
            std::vector<Tree::Tree_entry *> entries = current_tree->get_entries();

            for (auto entry: entries) {
                if (entry->type == "tree") {
                    Files::make_dir(Files::join_path(Files::root_path(), path + "/" + entry->path).c_str());
                    update_working_directory_recursively(entry->hash, path + "/" + entry->path);
                } else {
                    // copy the file from the .gitc/objects directory to the current working directory
                    Files::copy_file_contents(Files::join_path(Files::root_path(), ".gitc/objects/" + entry->hash),
                                              Files::join_path(Files::root_path(), path + "/" + entry->path));
                }
            }
        }

        static void recursively_delete_tree(const std::string &current_tree_hash, Commit* parent_commit) {
            Tree *current_tree = new Tree(current_tree_hash);
            std::vector<Tree::Tree_entry *> entries = current_tree->get_entries();

            for (auto entry: entries) {
                if (parent_commit->depends_on(entry->hash)) { // can do this more efficiently
                    continue;
                }
                if (entry->type == "tree") {
                    recursively_delete_tree(entry->hash, parent_commit);
                } else {
                    Files::delete_file(Files::join_path(Files::root_path(), ".gitc/objects/" + entry->hash));
                }
            }

            Files::delete_file(Files::join_path(Files::root_path(), ".gitc/objects/" + current_tree_hash));
        }

        bool depends_on(const std::string &hash) {
            // check if the current commit has a file with the given hash
            Tree *current_tree = new Tree(tree_hash);
            bool res = current_tree->search_for_hash(hash);
            delete current_tree;
            return res;
        }
    };

} // gitc

#endif //GIT_CLONE_COMMIT_H
