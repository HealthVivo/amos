SOURCES	+= UIElements.cc TilingField.cc ConsensusField.cc TilingFrame.cc RenderSeq.cc ReadInfo.cc ChromoField.cc MainWindow.cc main.cc 
HEADERS	+= UIElements.hh TilingField.hh ConsensusField.hh TilingFrame.hh RenderSeq.hh ReadInfo.hh ChromoField.hh MainWindow.hh
######################################################################
# Automatically generated by qmake (1.03a) Thu Oct 7 10:24:03 2004
######################################################################


# Input

QMAKE_CFLAGS_RELEASE	= -I/usr/include/freetype2 -I/usr/include/freetype2/freetype -O2 -march=i386 -mcpu=i686 -fno-use-cxa-atexit
QMAKE_CXXFLAGS_RELEASE	= -I/usr/include/freetype2 -I/usr/include/freetype2/freetype -O2 -march=i386 -mcpu=i686 -fno-use-cxa-atexit

QMAKE_CFLAGS_WARN_ON	= -Wall -g
QMAKE_CXXFLAGS_WARN_ON	= -Wall -g

QMAKE_CXX = /usr/bin/g++
QMAKE_LINK  = /usr/bin/g++


INCLUDEPATH	+= ../AMOS ../Common ../Slice /local/devel/SE/IO_Lib/mschatz/SE/IO_Lib/install/i386-linux/include/io_lib/
LIBS	+= -L../../build/lib/AMOS -L/local/devel/SE/IO_Lib/mschatz/SE/IO_Lib/install/i386-linux/lib -lAMOS -lCommon -lread
