g++ optimized.cpp -pthread
mkdir FinalResult2
for (( k = 10;k <= 15; k++))
do
    echo "$k 5 1 50" > inp-params.txt
    for (( i = 1; i <= $k; i++ ))      ### Outer for loop ###
    do
       
        var="$i "
        for (( j = 1 ; j <= $k; j++ )) ### Inner for loop ###
        do
            if [ $j -ne $i ]
            then
            var+="$j "
            fi
        done
        echo $var >> inp-params.txt       
    done

    #inp-params.txt > "./FinalResult2/$k.txt"
    cp inp-params.txt "./FinalResult1/$k.txt"
    echo "$c done"
done