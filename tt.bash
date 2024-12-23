#!/bin/bash

# Set the target file for persistent aliases (external file)
ALIAS_FILE="/tmp/ttag_aliases"  # You can change this to another file if needed

# Initialize a counter for the aliases
i=1

BLUE="\033[34m"
RESET="\033[0m"
YELLOW="\033[33m"
MAGENTA="\033[35m"
GREEN="\033[32m"

# The word to highlight is the last argument given to the script
HIGHLIGHT_WORD="${@: -1}"

# Clear previous alias file (optional, comment out if you want to append)
> "$ALIAS_FILE"

# Run ag and process each line
/usr/bin/ag --column --group $* | while IFS= read -r line; do
    # Print the original line (preserve any colors X produces)
    
    # If the line contains ":", it must be linenumber:match
    if [[ "$line" =~ ^([0-9]+):([0-9]+):(.+)$ ]]; then
        # Extract linenumber and match from the line
        linenumber="${BASH_REMATCH[1]}"
        colnumber="${BASH_REMATCH[2]}"
        match="${BASH_REMATCH[3]}"
        
        # Add the alias definition to the external file
        echo "alias e$i='$EDITOR $filename:$linenumber:$colnumber'" >> "$ALIAS_FILE"

        # Escape only the backslash character in the match
        escaped_match=$(printf "%s" "$match" | sed 's/\\/\\\\/g')

        # Increment the counter for the next alias
        echo -e "${BLUE}[${MAGENTA}$i${BLUE}]${RESET} ${YELLOW}$linenumber${RESET}:$escaped_match" | GREP_COLOR='13;30;43' grep --color=always -E "\b$HIGHLIGHT_WORD\b"
        ((i++))
    else
        # If the line is just a filename, save it for use in the alias
        filename="$line"
        echo -e "${GREEN}$line${RESET}"
    fi
done
