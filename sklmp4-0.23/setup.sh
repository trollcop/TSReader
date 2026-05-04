#!/bin/sh

echo "**************************************************"
echo "* Going into /util dir, launching 'genmake.pl' ..."
echo "**************************************************"
cd util; perl genmake.pl; cd ..

echo "********************************************************"
echo "* Going into each build dirs, launching 'make subdir'..."
echo "********************************************************"

cd build/alpha-cxx;  make -k subdir; cd ../..
cd build/hp-acc;     make -k subdir; cd ../..
cd build/hp-gcc;     make -k subdir; cd ../..
cd build/irix-cc;    make -k subdir; cd ../..
cd build/irix-gcc;   make -k subdir; cd ../..
cd build/linux-icc;  make -k subdir; cd ../..
cd build/linux-gcc;  make -k subdir; cd ../..
cd build/linux-gcc3; make -k subdir; cd ../..
cd build/sunos-cc;   make -k subdir; cd ../..
cd build/sunos-gcc;  make -k subdir; cd ../..

echo "*************************************************************"
echo "* Ready. "
echo "* You can go in /build/<platform> and type  'make tmp4', e.g."
echo "*************************************************************"
