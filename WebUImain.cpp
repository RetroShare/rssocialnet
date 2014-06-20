#include <stdio.h>

#include <util/rsthreads.h>
#include <retroshare/rsplugin.h>

#include <Wt/WServer>
#include <Wt/WEnvironment>

#include "RSWApplication.h"
#include "WebUImain.h"

bool c ;
std::vector<RSWebUI::IPRange>  RSWebUI::_ip(1,RSWebUI::IPRange::make_range("127.0.0.1",c));
uint16_t  RSWebUI::_port       = 9090 ;
RSWAppThread *RSWebUI::_thread = NULL ;

class RSWAppThread: public RsThread
{
	public:
		RSWAppThread()
		{
			_should_stop = false ;
		}
		virtual void run()
		{
			std::string s1 ( "./Hello" );
			std::string s2 ( "--docroot" );
#ifdef WINDOWS_SYS
			std::string s3 ( "." );
#else
			std::string s3 ( "/usr/share/Wt" );
#endif
			std::string s4 ( "--http-address" );

			std::string s5 ( "0.0.0.0" ) ;

			std::ostringstream os2 ;
			os2 << RSWebUI::port()  ;

			std::string s6("--http-port") ;
			std::string s7( os2.str() ) ;

            // todo: disable for release
            // tell wt where to find conigfile
            // the configfile define sthe wt logging level
            std::string s8("-c") ;
            std::string s9("wt_config.xml") ;

			std::cerr << "RSWEBUI: Using http address " << s5 << ", and port " << s7 << std::endl;

            int argc = 9 ;
			char *argv[] = {  strdup(s1.c_str()),
									strdup(s2.c_str()), 
									strdup(s3.c_str()), 
									strdup(s4.c_str()),
									strdup(s5.c_str()),
									strdup(s6.c_str()),
                                    strdup(s7.c_str()),
                                    strdup(s8.c_str()),
                                    strdup(s9.c_str())
                           } ;

			std::cerr << "RSWEBUI: In server thread. Launching..." << std::endl;

			WServer server(argv[0]);

			// WTHTTP_CONFIGURATION is e.g. "/etc/wt/wthttpd"
			server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

			// add a single entry point, at the default location (as determined by the server configuration's deploy-path)
			server.addEntryPoint(Wt::Application, createApplication, std::string(), std::string("images/webicon.png"));

			if(!server.start()) 
			{
				std::cerr << "RSWEBUI: Server failed to start. Giving up." << std::endl;
				join() ;
				stop() ;
			}

			while(!_should_stop)
#ifdef WINDOWS_SYS
				Sleep(1000) ;
#else
				sleep(1) ;
#endif

			//Wt::WServer::waitForShutdown();

			std::cerr << "RSWEBUI: Stopping server." << std::endl;
			server.stop();

			join() ;
			stop() ;
		}
		static WApplication *createApplication(const WEnvironment& env)
		{
			bool b ;
			RSWebUI::IPRange r = RSWebUI::IPRange::make_range(env.clientAddress(),b) ;

			std::cerr << "*******************************************" << std::endl;
			std::cerr << "******* Creating application for IP " << env.clientAddress() << std::endl;
			std::cerr << "*******************************************" << std::endl;

			// Check that the IP range allows that particular IP from the client.

			bool found = false ;
			for(std::vector<RSWebUI::IPRange>::const_iterator it(RSWebUI::ipMask().begin());it!=RSWebUI::ipMask().end() && !found;++it)
				found = (*it).contains(r) ;

			if(found)
				return new RSWApplication(env,*plg_interfaces);
			else
				return NULL ;
		}

		void stopServer()
		{
			_should_stop = true ;
		}

		static const uint32_t RSWAPP_THREAD_STATUS_STOPPED = 0x01 ;
		static const uint32_t RSWAPP_THREAD_STATUS_RUNNING = 0x02 ;

		static RsPlugInInterfaces *plg_interfaces ;
	private:

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

	std::cerr << "RSWEBUI: Starting WebUI service with port=" << _port << std::endl;

	_thread = new RSWAppThread() ;
	_thread->start() ;

	return true ;
}
bool RSWebUI::restart() 
{
	std::cerr << "RSWEBUI: Scheduling webserver restart..." << std::endl;

	stop() ;
	start(*RSWAppThread::plg_interfaces) ;

	return true ;
}
bool RSWebUI::stop() 
{
	if(_thread == NULL)
		return false ;

	std::cerr << "RSWEBUI: Stopping web server..." << std::endl;
	_thread->stopServer() ;

	while(_thread->isRunning())
	{
#ifdef WINDOWS_SYS
		Sleep(500) ;		// wait half a sec.
#else
		usleep(500000) ;		// wait half a sec.
#endif
	}
	std::cerr << "RSWEBUI: done." << std::endl;

	delete _thread ;
	_thread = NULL ;

	return true ;
};

std::string RSWebUI::IPRange::toStdString() const 
{
	uint32_t tip = ip ;
	unsigned char a[4];

	for(uint32_t i=0;i<4;++i)
	{
		a[i] = tip & 0xff ;
		tip >>= 8 ;
	}
	std::ostringstream os ;
	os << (int)a[3] << "." << (int)a[2] << "." << (int)a[1] << "." << (int)a[0] ;
	os << "/" << (int)bits ; 
	os.flush();

	return os.str();
}
RSWebUI::IPRange RSWebUI::IPRange::make_range(const std::string& str,bool& success)
{
	uint32_t a[4] ;
	success = true ;
	uint32_t bts;

	if(sscanf(str.c_str(),"%u.%u.%u.%u/%u",&a[0],&a[1],&a[2],&a[3],&bts) == 5)
		if(a[0]<256 && a[1] <256 && a[2] < 256 && a[3] < 256 && bts < 33)
			return IPRange( (((((a[0]<<8)+a[1])<<8)+a[2])<<8)+a[3], bts) ;

	if(sscanf(str.c_str(),"%u.%u.%u.%u",&a[0],&a[1],&a[2],&a[3]) == 4)
		if(a[0]<256 && a[1] <256 && a[2] < 256 && a[3] < 256)
			return IPRange( (((((a[0]<<8)+a[1])<<8)+a[2])<<8)+a[3], 32) ;

	success = false ;
	return IPRange(0,0) ;
}

bool RSWebUI::IPRange::contains(const RSWebUI::IPRange& r) const
{
	// nullify the last bits.
	uint32_t rmin = ip & ~(0xffffffff >> (32 - bits)) ;
	uint32_t rmax = ip |  (0xffffffff >> bits) ;

	std::cerr << "Contain: rmin=" << std::hex << rmin << " rmax=" << rmax << std::dec << std::endl;

	return r.ip >= rmin && r.ip <= rmax ;
}

