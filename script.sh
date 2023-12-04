#!/bin/bash
reg1="^[A-Z][a-zA-Z0-9, ]*[\.$\!$\?$]"
reg2=",[]*si[]"
reg=g3="[]*si[],"
count=0
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <c>"
    exit 1
fi
c=$1

while read linie
do
    if echo $linie | grep -E "$reg1" | grep -v "$reg2" | grep -v "$reg3" | grep "$c"
    then
        ((count++))        
    fi
done
echo "$count"
exit 0
