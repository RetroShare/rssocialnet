A new approach to build a webinterface for RS
=============================================

1. get JSON encoded data from the backend, data contains a state token
2. render data with react.js
3. ask the backend if the state token from step 1 expired. If yes, then start again with step 1.

Steps 1. and 3. are common for most things, only Step 2. differs. This allows to re-use code for steps 1. and 3.

INSTALLATION
------------

	- download and install node.js
		http://nodejs.org/
	- install dependencies
		cd webui
		npm install
	- symlink this directory to the folder where your Retrohare executable is
	- Windows:
		start cmd as admin and run
		mklink /D rs_exe_dir this_dir
	- start RS with rssocialnet plugin enabled
	- go to
		http://localhost::9090/webui/index.html
	- to automatically get the page refreshed when you change a file run
		grunt watch

API DOCUMENTATION
-----------------

	- run
		node PeersTest.js
	- this will print the expected schema of the api output, and it will try to test it with real data
	- run Retroshare with rssocialnet plugin enabled, to test if the real output of the api matches the expected schema

CONTRIBUTE
----------
	
	- if you are a web developer or want to become one
		get in contact!
	- lots of work to do, i need you!