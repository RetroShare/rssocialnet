#include <util/rsthreads.h>
#include <retroshare/rsplugin.h>

#include <Wt/WServer>

#include "RSWApplication.h"
#include "WebUImain.h"

uint32_t  RSWebUI::_ip         = 0 ;
uint16_t  RSWebUI::_port       = 9090 ;
RSWAppThread *RSWebUI::_thread = NULL ;

class RSWAppThread: public RsThread
{
	public:
		RSWAppThread(uint16_t port,uint32_t ip_range)
			: _port(port),_ip_range(ip_range)
		{
		}
		virtual void run()
		{
			std::string s1 ( "./Hello" );
			std::string s2 ( "--docroot" );
			std::string s3 ( "." );
			std::string s4 ( "--http-address" );

			std::ostringstream os ;
			uint32_t ip = _ip_range ;
			os << (ip & 0xff) << "." ; ip >>= 8 ;
			os << (ip & 0xff) << "." ; ip >>= 8 ;
			os << (ip & 0xff) << "." ; ip >>= 8 ;
			os << (ip & 0xff) ;

			std::string s5 ( os.str() ) ;

			std::ostringstream os2 ;
			os2 << _port ;

			std::string s6("--http-port") ;
			std::string s7( os2.str() ) ;

			std::cerr << "Using http address " << s5 << ", and port " << s7 << std::endl;

			int argc = 7 ;
			char *argv[] = {  strdup(s1.c_str()),
									strdup(s2.c_str()), 
									strdup(s3.c_str()), 
									strdup(s4.c_str()),
									strdup(s5.c_str()),
									strdup(s6.c_str()),
									strdup(s7.c_str()) } ;

			std::cerr << "In server thread. Launching..." << std::endl;

			WServer server(argv[0]);

			// WTHTTP_CONFIGURATION is e.g. "/etc/wt/wthttpd"
			server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

			// add a single entry point, at the default location (as determined by the server configuration's deploy-path)
			server.addEntryPoint(Wt::Application, createApplication, std::string(), std::string("images/webicon.png"));

			if(!server.start()) 
			{
				std::cerr << "Server failed to start. Giving up." << std::endl;
				join() ;
				stop() ;
			}

			Wt::WServer::waitForShutdown();
			server.stop();

			join() ;
			stop() ;
		}
		static WApplication *createApplication(const WEnvironment& env)
		{
			return new RSWApplication(env,*plg_interfaces);
		}

		void stopServer()
		{
			_should_stop = true ;
		}

		static const uint32_t RSWAPP_THREAD_STATUS_STOPPED = 0x01 ;
		static const uint32_t RSWAPP_THREAD_STATUS_RUNNING = 0x02 ;

		static RsPlugInInterfaces *plg_interfaces ;
	private:

		uint16_t _port;
		uint32_t _ip_range ;

		bool 		_should_stop ;
		uint32_t _status ;
};

// Needs to be constructed on heap, so that it's not copy+writed in the thread.
//
RsPlugInInterfaces *RSWAppThread::plg_interfaces = new RsPlugInInterfaces;

bool RSWebUI::isRunning() 
{ 
	return (_thread != NULL) && _thread->isRunning() ; 
}
bool RSWebUI::start(const RsPlugInInterfaces& interfaces) 
{
	if(isRunning())
		return false ;

	*RSWAppThread::plg_interfaces = interfaces ;

	_thread = new RSWAppThread(_port,_ip) ;
	_thread->start() ;

	return true ;
}
bool RSWebUI::stop() 
{
	if(_thread == NULL)
		return false ;

	std::cerr << "Stopping web server..." << std::endl;
	_thread->stopServer() ;

	while(_thread->isRunning())
	{
#ifndef WINDOWS_SYS
		usleep(500000) ;		// wait half a sec.
#else
		Sleep(500) ;		// wait half a sec.
#endif
	}
	std::cerr << "done." << std::endl;

	delete _thread ;
	_thread = NULL ;

	return true ;
};


