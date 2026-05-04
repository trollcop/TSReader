#! /bin/bash

if [ -z "$1" ]
then
	echo "Usage $0 \"path to the build dir\""
	exit $E_NOARG
fi

#echo $1


filename="alitest.cpp"
cp "$1/src/TestAppLin/$filename" "$filename"
filename="alitest.h"
cp "$1/src/TestAppLin/$filename" "$filename"
filename="msg.cpp"
cp "$1/src/TestAppLin/$filename" "$filename"
filename="msg.h"
cp "$1/src/TestAppLin/$filename" "$filename"
filename="tsrate.cpp"
cp "$1/src/TestAppLin/$filename" "$filename"
filename="tsrate.h"
cp "$1/src/TestAppLin/$filename" "$filename"
