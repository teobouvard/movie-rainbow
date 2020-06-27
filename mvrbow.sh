#!/bin/bash

usage (){
    printf "Usage: %s: [−i input_movie] [−o output_file]\n" $0
}

# variables
input_movie=
output_file="out.pdf"
verbose=0

while getopts "vi:o:" opt
do
    case $opt in
        i)  input_movie="$OPTARG";;
        o)  output_file="$OPTARG";;
        v)  verbose=1;;
        ?)  usage
            exit 2;;
    esac
done

if [ -z "$input_movie" ]; then
  printf "Missing input movie\n"
  usage
  exit 2
fi

printf "Remaining arguments are: %s\n$*"


tmpfile=$(mktemp)
echo $tmpfile
rm $tmpfile