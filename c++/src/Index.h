//
// Created by Karan Gandhi on 25-12-2023.
//
#include <string>
#include <sstream>
#include <algorithm>

#ifndef GIT_CLONE_INDEX_H
#define GIT_CLONE_INDEX_H

namespace gitc {

    enum Index_updates {
        REMOVE, ADD
    };

    enum Stage_number {
        UNMODIFIED, STAGED, UNTRACKED
    };

    struct Index_entry {
        std::string path;
        std::string hash;
        Stage_number stage_number = UNTRACKED;
    };


    class Index {
    public:
        Index() {
            readFromFiles();

            std::vector<std::string> files = Files::ls_recursive(Files::relative_root_path());
            for (std::string &file: files) {
                if (!has_entry(file)) {
                    Index_entry *new_entry = new Index_entry();

                    new_entry->path = file;
                    new_entry->stage_number = UNTRACKED;
                    new_entry->hash = Files::create_hash(HASH_LENGTH); // now come up with a hash function

//                    std::cout << new_entry->path << (int) new_entry->stage_number << new_entry->hash << std::endl;
                    entries.push_back(new_entry);
                }
            }

            Files::make_dir(".gitc/objects");
        }

        ~Index() {
            writeToFile();
            for (Index_entry *entry: entries)
                delete entry;

            entries.clear();
        }

        void update(const std::string &path, Index_updates updates) {
            if (updates == ADD) {
                for (Index_entry *entry: entries) {
                    if (entry->path == path) {
                        const std::string current_object_path = Files::join_path(Files::root_path(Files::get_cwd()),
                                                                                 ".gitc/objects/" + entry->hash);

                        if (Files::check_for_changes(path, current_object_path)) {
                            entry->hash = Files::create_hash(HASH_LENGTH);
                            entry->stage_number = STAGED;

                            const std::string new_object_path = Files::join_path(Files::root_path(Files::get_cwd()),
                                                                                 ".gitc/objects/" + entry->hash);
                            Files::copy_file_contents(path, new_object_path);
                        }
                    }
                }
            } else if (updates == REMOVE) {
                // make the file untracked
                for (Index_entry *entry: entries) {
                    if (entry->path == path) {
                        entry->stage_number = UNTRACKED;
                        Files::delete_file(
                                Files::join_path(Files::root_path(Files::get_cwd()), ".gitc/objects/" + entry->hash));
                    }
                }
            }
        }

        bool has_entry(const std::string &path) {
//            std::cout << "Has entry: " << path << std::endl;
            for (Index_entry *entry: entries) {
                if (entry->path == path) {
                    return true;
                }
            }

//            std::cout << "false" << std::endl;
            return false;
        }

        void unsatge_entries() {
            staged = false;
            for (auto entry : entries) {
                if (entry->stage_number == STAGED) {
                    entry->stage_number = UNMODIFIED;
                }
            }
        }

        std::vector<Index_entry *> get_entries() {
            return entries;
        }

        bool is_staged() {
            return staged;
        }

        bool has_untracked_files() {
            bool res = false;

            for (auto entry : entries)
                res |= entry->stage_number == UNTRACKED;

            return res;
        }

    private:
        bool staged;
        std::vector<Index_entry *> entries;

        void writeToFile() {
            // filepath stage_number hash
            const std::string index_file_path = Files::join_path(Files::root_path(Files::get_cwd()), ".gitc/index");
            std::ofstream index_file(index_file_path);

            index_file << entries.size() << " gitc_version_1.0" << std::endl;
            for (Index_entry *entry: entries) {
                index_file << entry->path << " " << entry->stage_number << " " << entry->hash << std::endl;
//                std::cout << entry->path << " " << entry->stage_number << " " << entry->hash << std::endl;
            }

            index_file.close();
        }

        void readFromFiles() {
            // read from the index file
            const std::string index_file_path = Files::join_path(Files::root_path(Files::get_cwd()), ".gitc/index");
            std::ifstream index_file(index_file_path);

            int size;
            std::string version;

            std::string line;
            std::getline(index_file, line);
            std::istringstream iss(line);
            iss >> size >> version;

            for (int i = 0; i < size; i++) {
                Index_entry *entry = new Index_entry();
                int stage_number;

                std::getline(index_file, line);
                std::reverse(line.begin(), line.end());
                std::istringstream current_iss(line);
                current_iss >> entry->hash >> stage_number;
                std::getline(current_iss, entry->path);

                std::reverse(entry->hash.begin(), entry->hash.end());
                std::reverse(entry->path.begin(), entry->path.end());

                while(entry->path[(int) entry->path.size() - 1] == ' ') entry->path.pop_back();

                entry->stage_number = static_cast<Stage_number>(stage_number);

                if (stage_number == STAGED) staged = true;
                if (Files::file_exists(entry->path)) entries.push_back(entry);
            }
        }
    };

} // gitc

#endif //GIT_CLONE_INDEX_H
