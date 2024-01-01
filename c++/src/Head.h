//
// Created by Karan Gandhi on 25-12-2023.
//

#include <string>
#include "Files.h"

#ifndef GIT_CLONE_HEAD_H
#define GIT_CLONE_HEAD_H

namespace gitc {

    class Head {
    public:
        Head() { // no support for branches yet
            read_from_file();
        }

        ~Head() {
            write_to_file();
        }

        void update_head_ref(const std::string &ref) {
            head_ref = ref;
        }

        void update_last_commit_hash(const std::string &hash) {
            last_commit_hash = hash;
        }

        static void init() {
            std::ofstream head_file(Files::join_path(Files::root_path(), ".gitc/HEAD"));
            head_file << "refs/heads/master";
            head_file.close();

            Files::make_dir(Files::join_path(Files::root_path(), ".gitc/refs/heads").c_str());

            std::ofstream master(Files::join_path(Files::root_path(), ".gitc/refs/heads/master"));
            master << "";
            master.close();
        }

        std::string get_last_commit_hash() {
            return last_commit_hash;
        }

        static bool commit_exists(const std::string &hash) {
            return Files::file_exists(Files::join_path(Files::root_path(), ".gitc/objects/" + hash));
        }

    private:
        std::string head_ref;
        std::string last_commit_hash;

        void write_to_file() {
            std::ofstream head_file(Files::join_path(Files::root_path(), ".gitc/HEAD"));
            head_file << head_ref;
            head_file.close();

            Files::make_dir(Files::join_path(Files::root_path(), ".gitc/refs").c_str());
            Files::make_dir(Files::join_path(Files::root_path(), ".gitc/refs/heads").c_str());

            std::ofstream master(Files::join_path(Files::root_path(), ".gitc/refs/heads/master"));
            master << last_commit_hash;
            master.close();
        }

        void read_from_file() {
            std::ifstream head_file(Files::join_path(Files::root_path(), ".gitc/HEAD"));
            std::getline(head_file, head_ref);
            head_file.close();

            std::ifstream master(Files::join_path(Files::root_path(), ".gitc/" + head_ref));
            std::getline(master, last_commit_hash);
            master.close();
        }
    };

} // gitc

#endif //GIT_CLONE_HEAD_H
