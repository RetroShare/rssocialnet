!include("../Common/retroshare_plugin.pri"): error("Could not include file ../Common/retroshare_plugin.pri")

CONFIG += qt qrc resources

INCLUDEPATH += ../../libretroshare/src

QMAKE_CXXFLAGS *= -Wall

DEFINES *= BOOST_SIGNALS_NO_DEPRECATION_WARNING

SOURCES = RSWApplication.cpp \
          WebUImain.cpp \
			 RSWappFriendsPage.cpp \
			 RSWappTransfersPage.cpp \
          WebUIPlugin.cpp \
		    RSWappSearchFilesPage.cpp \
			 RSWappSharedFilesPage.cpp \
			 gui/RsWebUIConfig.cpp \
    RSWappSocialNetworkPage.cpp \
    RSWappTestPage.cpp \
    p3wallservice.cc \
    rswallitems.cc \
    sonet/NewsfeedWidget.cpp \
    sonet/FirstStepsWidget.cpp \
    sonet/GxsCircleChooserWt.cpp \
    sonet/CreatePostWidget.cpp \
    sonet/GxsIdChooserWt.cpp \
    sonet/WallWidget.cpp \
    sonet/WallRootPostWidget.cpp \
    sonet/RsGxsUpdateBroadcastWt.cpp \
    sonet/AvatarWidget.cpp \
    sonet/CreateWallWidget.cpp \
    sonet/TokenQueueWt2.cpp \
    sonet/WallChooserWidget.cpp \
    sonet/WallTestWidget.cpp \
    sonet/CommentContainerWidget.cpp

HEADERS = RSWApplication.h \
          WebUImain.h \
			 RSWappFriendsPage.h \
			 RSWappTransfersPage.h \
          WebUIPlugin.h \
    		RSWappSearchFilesPage.h \
    		RSWappSharedFilesPage.h \
			 gui/RsWebUIConfig.h \
    RSWappSocialNetworkPage.h \
    RSWappTestPage.h \
    rswall.h \
    p3wallservice.h \
    rswallitems.h \
    sonet/FirstStepsWidget.h \
    sonet/NewsfeedWidget.h \
    sonet/GxsCircleChooserWt.h \
    sonet/CreatePostWidget.h \
    sonet/GxsIdChooserWt.h \
    sonet/WallWidget.h \
    sonet/RsGxsUpdateBroadcastWt.h \
    sonet/WallRootPostWidget.h \
    sonet/AvatarWidget.h \
    sonet/CreateWallWidget.h \
    sonet/TokenQueueWt2.h \
    sonet/WallChooserWidget.h \
    sonet/WallTestWidget.h \
    sonet/CommentContainerWidget.h

FORMS += gui/RsWebUIConfig.ui

TARGET = rssocialnet

RESOURCES = WebUI_images.qrc

LIBS += -lwthttp -lwt

################################# Linux ##########################################

linux-* {
	INCLUDEPATH += /usr/include/Wt
	LIBS += -L../../libretroshare/src/lib -lretroshare ../../libbitdht/src/lib/libbitdht.a
}

#################################### Windows #####################################

win32 {
        WT_DIR = ../../../wt-3.3.2
        BOOST_DIR = ../../../boost-build
        BOOST_NAME = mgw44-mt-1_54

        INCLUDEPATH += $${WT_DIR}/src $${WT_DIR}/build $${BOOST_DIR}/include/boost-1_54

        LIBS += -lws2_32 -lwsock32
        LIBS += -L"$${WT_DIR}/build/src" -L"$${WT_DIR}/build/src/http" -L"$${BOOST_DIR}/lib"
        LIBS += -lboost_date_time-$${BOOST_NAME} -lboost_filesystem-$${BOOST_NAME} -lboost_program_options-$${BOOST_NAME} -lboost_random-$${BOOST_NAME} -lboost_regex-$${BOOST_NAME} -lboost_system-$${BOOST_NAME} -lboost_thread-$${BOOST_NAME}
        LIBS += -lboost_signals-$${BOOST_NAME}
        #LIBS += -lboost_date_time-mgw44-mt-1_54 -lboost_filesystem-mgw44-mt-1_54 -lboost_program_options-mgw44-mt-1_54 -lboost_random-mgw44-mt-1_54 -lboost_regex-mgw44-mt-1_54 -lboost_system-mgw44-mt-1_54 -lboost_thread-mgw44-mt-1_54
        #LIBS += -lboost_signals-mgw44-mt-1_54
}
