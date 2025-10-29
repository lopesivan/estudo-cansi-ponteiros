#!/usr/bin/env bash

prefix=ex-

ls ${prefix}*.c | sort -n -t'-' -k 2 |
    tail -1 |
    sed -e "s/${prefix}\([0-9]\+\).c/\1/" -e 's/^0//' |
    while read n; do printf "cp ${prefix}%02d.c ${prefix}%02d.c\n" "$n" "$((n + 1))"; done
