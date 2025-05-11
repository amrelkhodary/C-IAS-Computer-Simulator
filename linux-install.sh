#!/bin/bash
#this script installs the IAS Computer Simulator program

readonly SUCCESSFUL=0
readonly UNSUCCESSFUL=1
readonly NAME="ias"
readonly PROGRAM_PATH="/usr/local/bin"
readonly LOGS_PATH="$HOME/IAS_LOGS"
readonly GITHUB_REPO_LINK="https://github.com/amrelkhodary/C-IAS-Computer-Simulator"
readonly EMAIL="amrelkhodarybusiness@gmail.com"

# Check if required source files exist
for file in ./src/main.c ./src/ias.c ./src/program_loader.c ./src/log.c; do
    if [ ! -f "$file" ]; then
        echo "Error: Required source file $file not found."
        exit $UNSUCCESSFUL
    fi
done

# Compile the program first to a local binary
cc ./src/main.c ./src/ias.c ./src/program_loader.c ./src/log.c -lm -o ./$NAME

if [ $? -eq 0 ]; then
    # Use sudo to install the binary to system directory
    echo "Compilation successful. Installing to $PROGRAM_PATH (requires sudo)..."
    sudo install -m 755 ./$NAME $PROGRAM_PATH/
    
    if [ $? -eq 0 ]; then
        echo "Installation Complete. Run ias <filename> to run your IAS program."
        echo "All logs are saved in ${LOGS_PATH}"
        echo "Documentation is available at ${GITHUB_REPO_LINK}"
        echo "If you found any issues or bugs, please send an email to ${EMAIL}"
        
        # Clean up local binary
        rm ./$NAME
        exit $SUCCESSFUL
    else
        echo "Failed to install the binary to $PROGRAM_PATH. Do you have sudo privileges?"
        exit $UNSUCCESSFUL
    fi
else
    echo "Failed to compile IAS Computer Simulator."
    echo "If you think there is an issue with IAS, please send an email to ${EMAIL}"
    exit $UNSUCCESSFUL
fi