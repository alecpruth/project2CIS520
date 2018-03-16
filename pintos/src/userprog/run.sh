#!/bin/sh

export PATH=/root/cis520/pintos/src/utils:/root/cis520/usr/local/bin:$PATH
export BXSHARE=/root/cis520/usr/local/share/bochs

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
#pintos -p ../../userprog/sample.txt -a sample.txt -- -q
#pintos -q run 'echo x';

#pintos -p ../../userprog/build/tests/userprog/open-bad-ptr -a open-bad-ptr -- -q;
#pintos -p ../../userprog/build/tests/userprog/args-many -a args-many -- -q;
#pintos -p ../../userprog/build/tests/userprog/args-multiple -a args-multiple -- -q;
#pintos -p ../../userprog/build/tests/userprog/args-single -a args-single -- -q;
#pintos -p ../../userprog/build/tests/userprog/args-dbl-space -a args-dbl-space -- -q;
pintos -p ../../userprog/build/tests/userprog/wait-twice -a wait-twice -- -q;
pintos -p ../../userprog/build/tests/userprog/child-simple -a child-simple -- -q;
pintos -p ../../userprog/build/tests/userprog/create-bad-ptr -a create-bad-ptr -- -q;
pintos -p ../../userprog/build/tests/userprog/write-bad-fd -a write-bad-fd -- -q;
pintos -q run 'write-bad-fd';
