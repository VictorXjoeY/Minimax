# Minimax
Generic Minimax algorithm for 2 player games. There are 3 games currently implemented:
- Konane (Mount & Blade II: Bannerlord board game)
- MuTorere (Mount & Blade II: Bannerlord board game)
- BaghChal (Mount & Blade II: Bannerlord board game)

# Notes
- Works for any 2 player games implemented as a derived class from Game<StateType, MoveType>.
- Works for games with cycles.
- Takes the shortest path for the win, but the longest path for the loss.
- Uses around 1.2GB of RAM.
- Doesn't work for games with chance elements. For that I'll probably implement another version of the algorithm which I'll call Expectiminimax.
