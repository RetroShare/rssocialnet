!include("../Common/retroshare_plugin.pri"): error("Could not include file ../Common/retroshare_plugin.pri")

CONFIG += qt qrc resources

# stream files over http
# requires patched libretroshare
#CONFIG += filestreamer

INCLUDEPATH += ../../libretroshare/src

QMAKE_CXXFLAGS *= -Wall

DEFINES *= BOOST_SIGNALS_NO_DEPRECATION_WARNING

# note: equal here, because retroshare_plugin.pri adds upnp to source (no idea why)
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
    sonet/CommentContainerWidget.cpp \
    sonet/IdentityLabelWidget.cpp \
    sonet/IdentityPopupMenu.cpp \
    sonet/SonetUtil.cpp \
    sonet/ShareButton.cpp \
    sonet/AvatarGroupWidget.cpp \
    sonet/CreatePostWidget2.cpp \
    sonet/ReferenceWidget.cpp \
    WebUITimer.cpp \
    util/jpge.cpp \
    util/jpgd.cpp \
    util/imageresize.cpp \
    util/lodepng.cpp \
    api/ApiServer.cpp \
    api/json.cpp \
    api/JsonStream.cpp \
    api/ResourceRouter.cpp \
    api/PeersHandler.cpp \
    api/Operators.cpp \
    api/IdentityHandler.cpp \
    api/WallHandler.cpp \
    api/ServiceControlHandler.cpp \
    api/StateTokenServer.cpp \
	api/GxsResponseTask.cpp

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
    sonet/CommentContainerWidget.h \
    sonet/IdentityLabelWidget.h \
    sonet/IdentityPopupMenu.h \
    sonet/SonetUtil.h \
    sonet/ShareButton.h \
    sonet/AvatarGroupWidget.h \
    sonet/CreatePostWidget2.h \
    sonet/ReferenceWidget.h \
    WebUITimer.h \
    util/jpge.h \
    util/jpgd.h \
    util/imageresize.h \
    util/lodepng.h \
    api/ApiServer.h \
    api/json.h \
    api/JsonStream.h \
    api/ApiTypes.h \
    api/ResourceRouter.h \
    api/PeersHandler.h \
    api/Operators.h \
    api/IdentityHandler.h \
    api/WallHandler.h \
    api/ServiceControlHandler.h \
    api/GxsMetaOperators.h \
    api/StateTokenServer.h \
	api/GxsResponseTask.h

filestreamer {
	DEFINES += ENABLE_FILESTREAMER
	SOURCES += api/FileStreamerWt.cpp
	HEADERS += api/FileStreamerWt.h
}

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
		SQLITE_DIR = ../../../sqlcipher-2.2.0
		INCLUDEPATH += $${SQLITE_DIR}

        WT_DIR = ../../../wt-3.3.3
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

OTHER_FILES += \
	README.md		\
	api/README.md	\
	webui/README.md
