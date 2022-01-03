# Minimax
Generic [Minimax](https://en.wikipedia.org/wiki/Minimax) algorithm for 2 player games of [Perfect Information](https://en.wikipedia.org/wiki/Perfect_information). These are the games that are currently implemented:
- [Konane](https://mountandblade.fandom.com/wiki/Board_Games) (Mount & Blade II: Bannerlord board game)
- [MuTorere](https://mountandblade.fandom.com/wiki/Board_Games) (Mount & Blade II: Bannerlord board game)
- [BaghChal](https://mountandblade.fandom.com/wiki/Board_Games) (Mount & Blade II: Bannerlord board game)
- [Tic-tac-toe](https://en.wikipedia.org/wiki/Tic-tac-toe)
- [Connect Four](https://en.wikipedia.org/wiki/Connect_Four)

# Instructions
I am using C++17 so your `g++` version should be at least 7.0!

## Linux/Mac
### Prerequisites
1. Install `g++`:
```
sudo apt-get install g++
```
2. Install `make`:
```
sudo apt-get install make
```

### Build
```
make
```

### Run

```
make run
```

## Windows
### Prerequisites
1. Install `g++` (MinGW): http://www.mingw.org/
2. Open your Command Prompt and run this to ensure that `g++` is properly installed:
```
g++ --version
```

### Build
- Run `build.bat`.

### Run
- Run `run.bat`.

# Known Issues
- On Windows Subsystem for Linux (WSL) the AI will play as soon as possible.
- Don't run it on Windows if you want to have colorful prints!

# Notes
- Works for any 2 player games implemented as a derived class from Game<StateType, MoveType>.
- Works for games with cycles.
- Works for games that can tie.
- Takes the shortest path for the win, but the longest path for the loss.
- The AI takes at least `TIMEOUT` milliseconds to play.
- It currently does not work for games with imperfect information or chance elements (dice/card games). I'll probably code that in another project.
