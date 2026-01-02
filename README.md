# Raven: a Stellar Blade Save Patcher
Script to modify Stellar Blade save files without manual hex editing.

This Script assumes you have the game installed.
# 🛠️ Build Instructions
Using g++: 
```
g++ src/*.cpp -I include -std=c++17 -o SavePatcher
```
Using MinGW on Windows: 
```
g++ src/*.cpp -I include -std=gnu++17 -o SavePatcher.exe
```
# 🚀 Usage 
Place the save file you want to modify in the same folder as the executable.
Then run: 
```
./SavePatcher
```
The output should be a file named `StellarBladeSave00.sav`
# ⚠️ Disclaimer
This tool is intended solely for educational, reverse-engineering, and research purposes.
Manipulating game save data may violate EULAs or terms of service.
Use responsibly.
