!/bin/bash                                                                     
if [ "$#" -ne 1 ]; then
    echo "$0 <caracter>"
    exit 1
fi
character="$1"
correct_sentences=0
while IFS= read -r line || [[ -n "$line" ]];do
    if [[ "$line" =~ ^[A-Z].*[A-Za-z0-9\ \,\.\!\?]+[\.!\?]+$ && ! "$line" =~ ,\\
 (si\ ) ]]; then
        if [[ "$line" == *"$character"* ]]; then
            ((correct_sentences++))
        fi
    fi
done
echo "$correct_sentences"


