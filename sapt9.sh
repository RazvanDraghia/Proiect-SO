#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Utilizare: $0 <caracter>"
    exit 1
fi

character=$1
counter=0
while IFS= read -r line; do
    if [[ $line =~ ^[A-Z].*[A-Za-z0-9\ \,\.\!\?]+$ && $line =~ [A-Za-z0-9\ \.\!\?]+$ && $line =~ [^\,]+[ \t]*[\.\?\!]$ ]]; then
        ((counter++))
    fi
done
echo "Numarul de propozitii corecte cu caracterul '$character' este: $counter"
