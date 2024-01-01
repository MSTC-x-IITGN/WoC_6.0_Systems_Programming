# Features
- [x] gitc - display help commands
- [x] gitc init - initialise the repo
- [x] gitc add - add files
- [x] gitc commit - commit added files
- [x] gitc rm - remove added files
- [x] gitc log - display all the commits
- [x] gitc checkout - checkout a commit
- [x] gitc revert - revert to a commit
- [x] gitc status - display the status of the repo
- [ ] support for branches

# Build
I have currently tested it out on windows (MinGW) and linux (g++).
To build the project, run the following command:
```bash
make
```
This would create a executable named `gitc` in the `./bin` directory.
