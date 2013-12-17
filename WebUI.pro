!include("../Common/retroshare_plugin.pri"): error("Could not include file ../Common/retroshare_plugin.pri")

CONFIG += qt qrc resources

INCLUDEPATH += ../../libretroshare/src

QMAKE_CXXFLAGS *= -Wall

SOURCES = RSWApplication.cpp \
          WebUImain.cpp \
			 RSWappFriendsPage.cpp \
			 RSWappTransfersPage.cpp \
          WebUIPlugin.cpp \
    RSWappSearchFilesPage.cpp \
    RSWappSharedFilesPage.cpp

HEADERS = RSWApplication.h \
          WebUImain.h \
			 RSWappFriendsPage.h \
			 RSWappTransfersPage.h \
          WebUIPlugin.h \
    RSWappSearchFilesPage.h \
    RSWappSharedFilesPage.h

TARGET = WebUI

RESOURCES = WebUI_images.qrc

LIBS += -lwthttp -lwt

################################# Linux ##########################################

linux-* {
	INCLUDEPATH += /usr/include/Wt

	LIBS += -L../../libretroshare/src/lib -lretroshare ../../libbitdht/src/lib/libbitdht.a
}

#################################### Windows #####################################

win32 {
	WT_DIR = ../../../lib/wt-3.3.1
	BOOST_DIR = ../../../lib/boost-1.55.0

	INCLUDEPATH += $${WT_DIR}/include $${BOOST_DIR}/include/boost-1_55

	LIBS += -lws2_32 -lwsock32
	LIBS += -L"$${WT_DIR}/lib" -L"$${BOOST_DIR}/lib"
	LIBS += -lboost_date_time-mgw44-mt-1_55 -lboost_filesystem-mgw44-mt-1_55 -lboost_program_options-mgw44-mt-1_55 -lboost_random-mgw44-mt-1_55 -lboost_regex-mgw44-mt-1_55 -lboost_system-mgw44-mt-1_55 -lboost_thread-mgw44-mt-1_55
}
