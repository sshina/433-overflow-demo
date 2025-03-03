#! /usr/bin/bash
gcc printAttack.c;
attack=`./a.out`
echo $attack;
$attack > ./badclient pedro
#rm ./a.out;
