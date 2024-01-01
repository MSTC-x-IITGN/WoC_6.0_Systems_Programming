//
// Created by Karan Gandhi on 25-12-2023.
//
#include "Files.h"
#include "Index.h"
#include "Head.h"
#include "Commit.h"

#ifndef GIT_CLONE_GITC_H
#define GIT_CLONE_GITC_H

namespace gitc {
    class gitc {
    public:
        gitc() {
#ifndef __linux__
            srand(time(NULL));
#endif
            index = new Index();
            head = new Head();

            if (!Files::in_repo()) {
                std::cout << "fatal: not a gitc repository (or any of the parent directories): .gitc" << std::endl;
                std::exit(1);
            }
        }

        ~gitc() {
            delete index;
            delete head;
        }

        static void init() {
            if (Files::in_repo()) {
                std::cout << "Already a gitc repository in " << Files::get_cwd() << "/.gitc" << std::endl;
                return;
            }

            // create the .gitc directory
            Files::create_gitc_dir(Files::get_cwd());
            Files::create_file(Files::join_path(Files::get_cwd(), ".gitc/HEAD"));
            Head::init();
            std::cout << "Initialized empty gitc repository in " << Files::get_cwd() << "/.gitc" << std::endl;
        }

        void add(const std::string &path) {
            std::vector<std::string> added_files = Files::ls_recursive(path);

            if (added_files.empty()) {
                std::cout << path << " did not match any files" << std::endl;
                return;
            }

            for (std::string &file: added_files) {
                // update the file in the index
                index->update(file, ADD);
            }
        }

        void rm(const std::string &path) {
            std::vector<std::string> removed_files = Files::ls_recursive(path);

            if (removed_files.empty()) {
                std::cout << path << " did not match any files" << std::endl;
                return;
            }

            for (std::string &file: removed_files) {
                // update the file in the index
                index->update(file, REMOVE);
            }
        }

        void status() {
            std::cout << "On branch master" << std::endl;

            if (index->is_staged()) {
                std::cout << "Changes to be commited:" << std::endl;
                std::cout << "   (use the rm command to unstage the files)" << std::endl;

                for (auto entry: index->get_entries()) {
                    if (entry->stage_number == STAGED) {
                        std::cout << "  \t" << entry->path << std::endl;
                    }
                }
            }

            if (index->has_untracked_files()) {
                std::cout << "Changes not staged for commit:" << std::endl;
                std::cout << "   (use \"add/rm <file>...\" to update what will be committed)" << std::endl;

                for (auto entry: index->get_entries()) {
                    if (entry->stage_number == UNTRACKED) {
                        std::cout << "  \t" << entry->path << std::endl;
                    }
                }
            }
        }

        void commit(const std::string &message) {
            if (!index->is_staged()) {
                std::cout << "Please add files first to commit" << std::endl;
                return;
            }
            // make a commit object, object tree (which is the snapshot of index), and update the head
            Commit *new_commit = Commit::create_commit_from_index(*index, head->get_last_commit_hash(), message);
            head->update_last_commit_hash(new_commit->get_commit_hash());
            delete new_commit;
        }

        void checkout(const std::string &commit_hash) {
            if (!Head::commit_exists(commit_hash)) {
                std::cout << "fatal: commit " << commit_hash << " does not exist" << std::endl;
                return;
            }

            if (index->has_untracked_files()) {
                std::cout << "fatal: Your local changes to the following files would be overwritten by checkout:"
                          << std::endl;

                for (auto &entry : index->get_entries()) {
                    if (entry->stage_number == UNTRACKED)
                        std::cout << "   " << entry->path << std::endl;
                }

                std::cout << "Aborting" << std::endl;
                return;
            }

            Commit *commit = new Commit(commit_hash);
            commit->update_working_directory();
            delete commit;
        }

        void revert(const std::string &commit_hash) {
            if (!Head::commit_exists(commit_hash)) {
                std::cout << "fatal: commit " << commit_hash << " does not exist" << std::endl;
                return;
            }

            Commit *commit = new Commit(commit_hash);
            commit->update_working_directory();
            delete commit;

            // delete the current commit and update the head
            while (head->get_last_commit_hash() != commit_hash) {
                std::string last_commit_hash = head->get_last_commit_hash();
                commit = new Commit(last_commit_hash);
                commit->delete_commit();
                head->update_last_commit_hash(commit->get_parent_commit_hash());
                delete commit;
                Files::delete_file(Files::join_path(Files::relative_root_path(), ".gitc/objects/" + last_commit_hash));
            }
        }

        void log() {
            if (head->get_last_commit_hash().empty()) {
                std::cout << "No commits to display" << std::endl;
                return;
            }

            Commit *current_commit = new Commit(head->get_last_commit_hash());
            current_commit->print_commit(true);

            while (!current_commit->get_parent_commit_hash().empty()) {
                std::string next_commit_hash = current_commit->get_parent_commit_hash();
                delete current_commit;
                current_commit = new Commit(next_commit_hash);
                current_commit->print_commit(false);
            }
        }

        static void help() {
            std::cout << "usage: gitc [-h | --help]" << std::endl;
            std::cout << "These are common gitc commands used in various situations: " << std::endl << std::endl;

            std::cout << "start a working area" << std::endl;
            std::cout << "   init              Create an empty gitc repository or reinitialize an existing one"
                      << std::endl << std::endl;

            std::cout << "work on the current change\n"
                      << "   add               Add file contents to the index\n"
                      << "   rm                Remove files from the working tree and from the index\n\n"
                      << "examine the history and state\n"
                      << "   log               Show commit logs\n"
                      << "   status            Show the working tree status\n"
                      << "   checkout          Checkout a commit\n\n"
                      << "grow, mark and tweak your common history\n"
                      << "   commit            Record changes to the repository\n"
                      << "   revert            Revert a commit\n\n\n"
                      << "'gitc --help' and 'gitc -h' list available subcommands"
                      << std::endl << std::endl;

            std::cout << "Created by Karan Gandhi" << std::endl;
        }

    private:
        Files files;
        Head *head;
        Index *index;

    };

} // gitc

#endif //GIT_CLONE_GITC_H
