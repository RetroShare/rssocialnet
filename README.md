Retroshare social network plugin
================================

This is a Retroshare v0.6 plugin, designed to provide social network features. It provides a webinterface based on Wt, the Qt-style library for web UIs.

This plugin extends the code of RSWebUI.

New plan:

	- deprecate RsWebUI
	- make an interface wich serves JSON data over http (see ./api)
	- make a new webinterface with react.js (see ./webui)

Please read the readmes in ./api and ./webui.

Update:

	- there are now two projects in this repository: resapi and socialnet
	- resapi is a library which implements the json interface
	- socialnet is a Retroshare plugin which contains social network functionality (wall) and the new json api

Download the sources
--------------------

	cd retroshare-trunk/plugins
	git clone https://git.gitorious.org/rssocialnet/rssocialnet.git

COMPILATION on Ubuntu
---------------------

	- compile Retroshare as usual, ses the Retroshare wiki for help: http://retroshare.sourceforge.net/wiki/index.php/UnixCompile

	- install packages 
			> sudo apt-get install witty-dev libwt-dev

	- compilation
			> cd ~/Code/retroshare/trunk/plugins/rssocialnet/
			> qmake CONFIG=debug
			> make

		This should create the following files:

          librssocialnet.so -> librssocialnet.so.1.0.0
          librssocialnet.so.1 -> librssocialnet.so.1.0.0
          librssocialnet.so.1.0 -> librssocialnet.so.1.0.0
          librssocialnet.so.1.0.0
			
	- create a link to the plugin in retroshare plugin directory
			> cd ~/.retroshare/extensions
			> ln -s ~/Code/rssocialnet/librssocialnet.so.1.0.0 librssocialnet.so

	- run Retroshare, or retroshare-nogui, and enable the librssocialnet plugin

	- In Web browser (e.g. chromium-browser) go to localhost:9090

COMPILATION on Windows
----------------------

	- compile Retroshare as usual, ses the Retroshare wiki for help: http://retroshare.sourceforge.net/wiki/index.php/Win32Compile
	
	- download and install cmake
		http://cmake.org/

	- download boost and unzip it into your development folder
		http://www.boost.org/

	- compile boost. Run these commands in Windows cmd
		cd d:\retroshare_dev\boost_1_5_55
		bootstrap.bat mingw
		bjam.exe toolset=gcc link=static threading=multi --layout=versioned --prefix=d:/retroshare_dev/boost-build --without-context install

	- download Wt and unzip it into your development folder
		http://www.webtoolkit.eu/wt

	- compile Wt. Run these command in Windows cmd
		cd d:\retroshare_dev\wt-3.3.2
		mkdir build
		cd build
		cmake -DBOOST_DIR=d:/retroshare_dev/boost-build -DBOOST_VERSION=1_55 -DBOOST_COMPILER=mgw44 -DCMAKE_INSTALL_PREFIX=d:/retroshare_dev/wt-3.3.2-install -G "MinGW Makefiles" ..
		mingw32-make
		mingw32-make install

	- you can set the versions and directories of boost and Wt in rssocialnet.pro

	- cou can add rssocialnet in retroshare_src/plugins/plugins.pro

	- compile and install rssocialnet.dll as you would with any other plugin