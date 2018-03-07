#!/bin/sh
clear
clear
echo Entering directory /examples ...
cd ../examples/
make

echo Entering directory /userprog ...
cd ../userprog/
make
# Stop for user to press enter key
read -rp 'Press enter to continue... ' second </dev/tty

echo Entering directory /userprog/build ... 
cd build
pintos-mkdisk filesys.dsk --filesys-size=2;
pintos -f -q;
pintos -p ../../examples/echo -a echo -- -q;
pintos -q run 'echo x';
