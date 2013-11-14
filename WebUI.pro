!include("../Common/retroshare_plugin.pri"): error("Could not include file ../Common/retroshare_plugin.pri")

CONFIG += qt qrc resources

INCLUDEPATH += ../../libretroshare/src

QMAKE_CXXFLAGS *= -Wall

SOURCES = RSWApplication.cpp \
          WebUImain.cpp \
			 RSWappFriendsPage.cpp \
			 RSWappTransfersPage.cpp \
          WebUIPlugin.cpp

HEADERS = RSWApplication.h \
          WebUImain.h \
			 RSWappFriendsPage.h \
			 RSWappTransfersPage.h \
          WebUIPlugin.h

TARGET = WebUI

INCLUDEPATH += /usr/include/Wt
RESOURCES = WebUI_images.qrc

LIBS += -lwthttp -lwt -L../../libretroshare/src/lib -lretroshare ../../libbitdht/src/lib/libbitdht.a
