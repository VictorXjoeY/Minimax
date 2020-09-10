# Minimax
Generic [Minimax](https://en.wikipedia.org/wiki/Minimax) algorithm for 2 player games of [Perfect Information](https://en.wikipedia.org/wiki/Perfect_information). There are 3 games currently implemented:
- Konane (Mount & Blade II: Bannerlord board game)
- MuTorere (Mount & Blade II: Bannerlord board game)
- BaghChal (Mount & Blade II: Bannerlord board game)
- Tic-tac-toe

# Notes
- Works for any 2 player games implemented as a derived class from Game<StateType, MoveType>.
- Works for games with cycles.
- Works for games that can tie.
- Takes the shortest path for the win, but the longest path for the loss.
- Uses around 1.2GB of RAM.
- It currently does not work for games with imperfect information or chance elements (dice/card games). I'll probably code that in another project.
