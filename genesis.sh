git clone https://github.com/sisoputnfrba/fifa-examples;
git clone https://github.com/sisoputnfrba/so-commons-library;
cd ./so-commons-library;
make;
sudo make install;
cp -a /home/utnso/workspace/tp-2018-2c-Operativos-Crusaders/fifa-examples/. /home/utnso/workspace/;
cd ..;

cd FM9;
gcc *.c -o FM9 -lcommons -lpthread -lreadline;

cd ..;

cd CPU;
gcc *.c -o CPU -lcommons -lpthread -lreadline;

cd ..;

cd Diego;
gcc *.c -o DAM -lcommons -lpthread -lreadline;

cd ..;

cd FileSys;
gcc *.c -o MDJ -lcommons -lpthread -lreadline;

cd ..;

cd SAFA;
gcc *.c -o SAFA -lcommons -lpthread -lreadline;
