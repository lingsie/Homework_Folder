Text Blackjack RPG Framework (Project 1)


1) Game Concept (2–4 sentences)


This program is a text-based Blackjack-style game framework where the player plays simplified 21-point rounds against a dealer. The project focuses on clean system design: a Player profile system, an Inventory that stores Item objects, and commands that allow inspecting and applying items. The Blackjack round logic is intentionally simplified because this project builds the foundation of a game, not a complete game.





2) Player Archetypes


At the start of the program, the user selects an archetype. The archetype initializes player attributes using constant values (tables/constants in code).


Player Stats
- Bankroll: the player’s money (affects ability to keep playing)
- Luck: small advantage in gameplay (implementation described below)
- Risk: affects reward/penalty (implementation described below)


Archetypes
1. Gambler
	Bankroll = 1000
	Luck = 5
	Risk = 5
	Specialty: balanced with higher luck
2. Analyst
	Bankroll = 800
	Luck = 2
	Risk = 8
	Specialty: higher risk/reward style
3. High Roller
	Bankroll = 2000
	Luck = 1
	Risk = 3
	Specialty: starts with more money but lower luck





3) Item Database (10 Items)


The inventory stores Item objects (not strings). Each item has:
    name
    description (full sentence)
    one numeric trait value


Note on trait usage: In this version, the item’s numeric trait modifies a player stat (Bankroll or Luck or Risk) when applied.
1. Lucky Coin
Description: A worn coin that makes you feel confident.
Trait: +2 Luck
2. Bonus Chip
Description: A promotional chip that adds extra funds.
Trait: +75 Bankroll
3. Focus Charm
Description: A charm that helps you stay calm under pressure.
Trait: +1 Luck
4. Emergency Loan
Description: A risky loan that gives instant cash but increases risk.
Trait: +200 Bankroll (and/or +1 Risk depending on implementation)
5. VIP Badge
Description: A badge that makes the dealer treat you slightly better.
Trait: +1 Luck
6. Heat Shield
Description: A tool that helps you keep a low profile at the table.
Trait: -1 Risk
7. High Limit Token
Description: A token that increases your confidence to bet bigger.
Trait: +100 Bankroll
8. Reroll Chip
Description: A chip that lets you feel like you got a second chance.
Trait: +1 Luck (or modifies a reroll counter if implemented)
9. Insurance Pass
Description: A pass that reduces losses in a bad round.
Trait: +50 Bankroll (or reduces penalty if implemented)
10. Steady Hands Glove
Description: A glove that improves consistency in tense moments.
Trait: +1 Luck


(If your code uses slightly different items or effects, update this list to match exactly.)





4) Game State Variables (Beyond Player Stats)


To avoid a trivial project, the game tracks at least one numeric game variable beyond player stats:
    - roundNumber: increases by 1 each time the player uses the play command.
    - (Optional) winStreak: increases on a win and resets on a loss.


These variables change during gameplay and demonstrate game-state progression.





5) User Manual (Commands)


The program runs in a loop until the user quits. Available commands:
--help
Displays all available commands.
--profile
Displays player name, archetype, and stats (Bankroll, Luck, Risk, etc.).
--inventory
Displays the player’s inventory using a dedicated inventory display function.
--inspect-<item>
Shows the item’s description and numeric trait value.
Example: inspect Lucky Coin
--apply-<item>
Applies/uses the item and updates a player stat or a game variable through a function.
Example: apply Bonus Chip
--play
Advances the game state by playing one simplified round against the dealer. This updates roundNumber (and may update bankroll based on win/loss).
--quit
Exits the program.




6) Design Notes / Handoff Readiness


This project is structured for readability and handoff to another developer:
	- Player handles player data, archetype initialization, and stat changes.
	- Item represents item data (name/description/trait).
	- Inventory stores Item objects in a fixed-size array, supports add/remove/find, and displays contents through a dedicated function.
	- main handles input parsing and control flow (the command loop).
	- Blackjack/Game module contains gameplay logic so main stays clean.


All constants (inventory capacity and archetype starting stats) are defined as constants in the code.

