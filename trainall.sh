#!/usr/local/bin/zsh
make
./train $1 model_init.txt seq_model_01.txt model_01.txt > /dev/null
./train $1 model_init.txt seq_model_02.txt model_02.txt > /dev/null
./train $1 model_init.txt seq_model_03.txt model_03.txt > /dev/null
./train $1 model_init.txt seq_model_04.txt model_04.txt > /dev/null
./train $1 model_init.txt seq_model_05.txt model_05.txt > /dev/null
./test modellist.txt testing_data1.txt result1.txt
./test modellist.txt testing_data2.txt result2.txt
#make clean
#echo -ne '\007'
