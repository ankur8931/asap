#! /bin/sh

# Strips the last ^M from each line that has two ^M's at the end

cat $1 | sed 's/$//' > $2
