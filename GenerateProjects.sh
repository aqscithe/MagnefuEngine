#!/bin/bash

# Navigate to the script's directory (optional, if you want to ensure the script is run from the right location)
cd "$(dirname "$0")"

# Call premake5 to generate Xcode projects
vendor/bin/premake/premake5 xcode4

# For Visual Studio Code
#vendor/bin/premake/premake5 gmake2

# Pause the script to review the output (optional)
read -p "Press [Enter] key to continue..."