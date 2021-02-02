mkdir FinalResult1

for (( c = 10; c<=10 ; c=c+1))
do
    echo "$c 5 1 50" > inp-params.txt
    var=""
    for(( i=1; i<=10; i=i+1))
    do
        var+="$i "
    done
    echo $var > inp-params.txt
        
    ./a.out < inp-params.txt > "./FinalResult1/$c.txt"
    echo "$c done"
done

