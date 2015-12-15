!include("../Common/retroshare_plugin.pri"): error("Could not include file ../Common/retroshare_plugin.pri")

CONFIG += qt qrc resources

greaterThan(QT_MAJOR_VERSION, 4) {
        # Qt 5
        QT     += widgets
}

TARGET = rssocialnet

# things marked with old_witty_code are going to be removed. they are just here to copy and paste from
# this old stuff is not meant to compile
old_witty_code  {
    DENFINES *= USE_OLD_WITTY_CODE
    # stream files over http
    # compiles only with patched libretroshare
    CONFIG += filestreamer
}

INCLUDEPATH += ../../libretroshare/src

QMAKE_CXXFLAGS *= -Wall

DEFINES *= BOOST_SIGNALS_NO_DEPRECATION_WARNING

# note: equal here, because retroshare_plugin.pri adds upnp to source (no idea why)
HEADERS = WebUIPlugin.h \
        rswall.h \
        p3wallservice.h \
        rswallitems.h \

SOURCES = WebUIPlugin.cpp \
        p3wallservice.cc \
        rswallitems.cc \

# i disabled the Qt gui for now, because we don't need a gui with the http server inside main Retroshare
with_gui{
    HEADERS = \
            gui/RsWebUIConfig.h \

    SOURCES =  \
            gui/RsWebUIConfig.cpp \
}

# new webinterface
INCLUDEPATH += ../../libresapi/src ../../libresapi/src/api
LIBS += ../../libresapi/src/lib/libresapi.a -lmicrohttpd
SOURCES += \
        api/WallHandler.cpp

HEADERS += \
        api/WallHandler.h

old_witty_code {
SOURCES += RSWApplication.cpp \
		  WebUImain.cpp \
			 RSWappFriendsPage.cpp \
			 RSWappTransfersPage.cpp \
			RSWappSearchFilesPage.cpp \
			 RSWappSharedFilesPage.cpp \
	RSWappSocialNetworkPage.cpp \
	RSWappTestPage.cpp \
	sonet/NewsfeedWidget.cpp \
	sonet/FirstStepsWidget.cpp \
	sonet/GxsCircleChooserWt.cpp \
	sonet/CreatePostWidget.cpp \
	sonet/GxsIdChooserWt.cpp \
	sonet/WallWidget.cpp \
	sonet/WallRootPostWidget.cpp \
	sonet/RsGxsUpdateBroadcastWt.cpp \
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
	sonet/AvatarWidgetWt.cpp \
    apiwt/ApiServerWt.cpp

HEADERS += RSWApplication.h \
		  WebUImain.h \
			 RSWappFriendsPage.h \
			 RSWappTransfersPage.h \
			RSWappSearchFilesPage.h \
			RSWappSharedFilesPage.h \
	RSWappSocialNetworkPage.h \
	RSWappTestPage.h \
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
    apiwt/ApiServerWt.h

    filestreamer {
            DEFINES += ENABLE_FILESTREAMER
            SOURCES += apiwt/FileStreamerWt.cpp
            HEADERS += apiwt/FileStreamerWt.h
    }
}

with_gui{
FORMS += gui/RsWebUIConfig.ui
}

TARGET = rssocialnet

RESOURCES = WebUI_images.qrc

old_witty_code{
    LIBS += -lwthttp -lwt
}

with_gui{
LIBS += ../../retroshare-gui/src/lib/libretroshare-gui.a
}

################################# Linux ##########################################

linux-* {
    old_witty_code{
            INCLUDEPATH += /usr/include/Wt
    }
	LIBS += -L../../libretroshare/src/lib -lretroshare ../../libbitdht/src/lib/libbitdht.a
}

#################################### Windows #####################################

win32 {
		SQLITE_DIR = ../../../sqlcipher-2.2.0
		INCLUDEPATH += $${SQLITE_DIR}

        LIBS += -lws2_32 -lwsock32

    # libmicrohttpd sets __declspec(dllexport) on its functions
    # this causes only libmicrohttpd functions to apear in the dll
    # this switches forces the linker to export all symbols
    QMAKE_LFLAGS += -Wl,--export-all-symbols

    old_witty_code{
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
}

OTHER_FILES += \
	README.md		\
	api/README.md	\
	webui/README.md
