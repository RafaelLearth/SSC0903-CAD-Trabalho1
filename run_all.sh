#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Clear previous results
> saidas/temposseq.txt
> saidas/tempospar.txt

# Get all input files in entradas/ sorted by number
input_files=$(ls -v entradas/input*.txt)

for input_file in $input_files; do
    filename=$(basename "$input_file")
    echo -e "${GREEN}Processando: $filename${NC}"
    
    ./studentsseq "$input_file"
    ./studentspar "$input_file"
done

