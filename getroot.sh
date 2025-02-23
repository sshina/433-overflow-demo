#! /usr/bin/bash
gcc printAttack.c;
attack=`./a.out`
echo $attack;
rm ./a.out;
