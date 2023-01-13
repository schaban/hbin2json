#!/bin/sh

PROG_NAME=hbin2json
BUILD_DATE="$(date)"
SYS_NAME="`uname -s`"
BUILD_PATH=$PWD

PROG_DIR=prog
CORE_DIR=core

INC_DIR=inc
SRC_DIR=src
TMP_DIR=tmp

PROG_PATH=$PROG_DIR/$PROG_NAME

if [ "$SYS_NAME" = "FreeBSD" ]; then
	NO_FMT=""
fi

FMT_BOLD=${NO_FMT-"\e[1m"}
FMT_UNDER=${NO_FMT-"\e[4m"}
FMT_BLACK=${NO_FMT-"\e[30m"}
FMT_BLACK_BG=${NO_FMT-"\e[30m"}
FMT_RED=${NO_FMT-"\e[31m"}
FMT_RED_BG=${NO_FMT-"\e[41m"}
FMT_GREEN=${NO_FMT-"\e[32m"}
FMT_GREEN_BG=${NO_FMT-"\e[42m"}
FMT_YELLOW=${NO_FMT-"\e[33m"}
FMT_YELLOW_BG=${NO_FMT-"\e[43m"}
FMT_BLUE=${NO_FMT-"\e[34m"}
FMT_BLUE_BG=${NO_FMT-"\e[44m"}
FMT_MAGENTA=${NO_FMT-"\e[35m"}
FMT_MAGENTA_BG=${NO_FMT-"\e[45m"}
FMT_CYAN=${NO_FMT-"\e[36m"}
FMT_CYAN_BG=${NO_FMT-"\e[46m"}
FMT_WHITE=${NO_FMT-"\e[37m"}
FMT_WHITE_BG=${NO_FMT-"\e[47m"}
FMT_GRAY=${NO_FMT-"\e[90m"}
FMT_GRAY_BG=${NO_FMT-"\e[100m"}
FMT_B_RED=${NO_FMT-"\e[91m"}
FMT_B_RED_BG=${NO_FMT-"\e[101m"}
FMT_B_GREEN=${NO_FMT-"\e[92m"}
FMT_B_GREEN_BG=${NO_FMT-"\e[102m"}
FMT_B_YELLOW=${NO_FMT-"\e[93m"}
FMT_B_YELLOW_BG=${NO_FMT-"\e[103m"}
FMT_B_BLUE=${NO_FMT-"\e[94m"}
FMT_B_BLUE_BG=${NO_FMT-"\e[104m"}
FMT_B_MAGENTA=${NO_FMT-"\e[95m"}
FMT_B_MAGENTA_BG=${NO_FMT-"\e[105m"}
FMT_B_CYAN=${NO_FMT-"\e[96m"}
FMT_B_CYAN_BG=${NO_FMT-"\e[106m"}
FMT_B_WHITE=${NO_FMT-"\e[97m"}
FMT_B_WHITE_BG=${NO_FMT-"\e[107m"}
FMT_OFF=${NO_FMT-"\e[0m"}

if [ ! -d $PROG_DIR ]; then
        mkdir -p $PROG_DIR
fi

CORE_SRC_URL="https://raw.githubusercontent.com/schaban/crosscore_dev/main/src"
CORE_SRCS="crosscore.hpp crosscore.cpp"

DL_MODE="NONE"
DL_CMD=""

if [ -x "`command -v curl`" ]; then
	DL_MODE="CURL"
	DL_CMD="curl -o"
elif [ -x "`command -v wget`" ]; then
	DL_MODE="WGET"
	DL_CMD="wget -O"
fi

NEED_CORE=0
if [ ! -d $CORE_DIR ]; then
	mkdir -p $CORE_DIR
	NEED_CORE=1
else
	for src in $CORE_SRCS; do
		if [ $NEED_CORE -ne 1 ]; then
			if [ ! -f $CORE_DIR/$src ]; then
				NEED_CORE=1
			fi
		fi
	done
fi

if [ $NEED_CORE -ne 0 ]; then
	printf "$FMT_B_RED""-> Downloading crosscore sources...""$FMT_OFF\n"
	for src in $CORE_SRCS; do
		if [ ! -f $CORE_DIR/$src ]; then
			printf "$FMT_B_BLUE""     $src""$FMT_OFF\n"
			$DL_CMD $CORE_DIR/$src $CORE_SRC_URL/$src
		fi
	done
fi


INCS="-I $CORE_DIR"
SRCS="`ls $SRC_DIR/*.cpp` `ls $CORE_DIR/*.cpp`"

DEFS=""
LIBS=""

DEF_CXX="g++"
case $SYS_NAME in
	OpenBSD)
		INCS="$INCS -I/usr/X11R6/include"
		LIBS="$LIBS -L/usr/X11R6/lib"
		DEF_CXX="clang++"
	;;
	FreeBSD)
		INCS="$INCS -I/usr/local/include"
		LIBS="$LIBS -lpthread -L/usr/local/lib"
		DEF_CXX="clang++"
	;;
	Linux)
		LIBS="$LIBS -ldl -lpthread"
	;;
esac
CXX=${CXX:-$DEF_CXX}


printf "Compiling \"$FMT_BOLD$FMT_B_MAGENTA$PROG_PATH$FMT_OFF\" with $FMT_BOLD$CXX$FMT_OFF.\n"
rm -f $PROG_PATH
$CXX -std=c++11 -ggdb $DEFS $INCS $SRCS -o $PROG_PATH $LIBS $*


