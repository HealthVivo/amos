SOURCES	+= FeatureCanvasItem.cc ChromoPicker.cc CoverageCanvasItem.cc FeatureBrowser.cc LibraryPicker.cc ReadPicker.cc DataStore.cc CGraphWindow.cc CGraphContig.cc CGraphView.cc CGraphEdge.cc ContigPicker.cc Insert.cc InsertWidget.cc InsertCanvasItem.cc InsertField.cc InsertWindow.cc InsertPosition.cc UIElements.cc TilingField.cc ConsensusField.cc TilingFrame.cc RenderSeq.cc ReadInfo.cc ChromoField.cc MainWindow.cc main.cc 
HEADERS	+= FeatureCanvasItem.hh ChromoPicker.hh CoverageCanvasItem.hh FeatureBrowser.hh LibraryPicker.hh ReadPicker.hh DataStore.hh CGraphWindow.hh CGraphContig.hh CGraphView.hh CGraphEdge.hh ContigPicker.hh Insert.hh InsertWidget.hh InsertCanvasItem.hh InsertField.hh InsertWindow.hh InsertPosition.hh UIElements.hh TilingField.hh ConsensusField.hh TilingFrame.hh RenderSeq.hh ReadInfo.hh ChromoField.hh MainWindow.hh
######################################################################
# Automatically generated by qmake (1.03a) Thu Oct 7 10:24:03 2004
######################################################################


# Input

QMAKE_CFLAGS_RELEASE	= -I/usr/include/freetype2 -I/usr/include/freetype2/freetype -O2 -march=i386 -mcpu=i686 -fno-use-cxa-atexit
QMAKE_CXXFLAGS_RELEASE	= -I/usr/include/freetype2 -I/usr/include/freetype2/freetype -O2 -march=i386 -mcpu=i686 -fno-use-cxa-atexit

QMAKE_CFLAGS_WARN_ON	= -Wall -g
QMAKE_CXXFLAGS_WARN_ON	= -Wall -g

INCLUDEPATH	+= ../AMOS ../Common ../Slice /usr/local/include/io_lib/
LIBS	+=  -L../AMOS -L../Common -lAMOS -lCommon -lAMOS -lCommon 

DOMAIN = $$system(hostname -d)
contains( DOMAIN, tigr ) {
  INCLUDEPATH	+= local/devel/SE/IO_Lib/mschatz/SE/IO_Lib/install/${HOSTTYPE}/include/io_lib/
  LIBS	+=  -L/local/devel/SE/IO_Lib/mschatz/SE/IO_Lib/install/${HOSTTYPE}/lib
  QMAKE_CXX = /usr/local/bin/g++
  QMAKE_LINK  = /usr/local/bin/g++
}

LIBS += -lread

QMAKE_LIBDIR = 

