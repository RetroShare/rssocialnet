#pragma once

#include <stdint.h>

class RSWAppThread ;
class RsPlugInInterfaces ;

class RSWebUI 
{
	public:
		static bool isRunning() ;
		static bool start(const RsPlugInInterfaces&) ;
		static bool stop() ;
		static bool restart() ;

		static void setPort(uint16_t port) { _port = port ; }
		static void setIPMask(uint32_t ip) { _ip = ip ; }

		static uint16_t port()  { return _port ; }
		static uint32_t ipMask() { return _ip ; }
	private:
		static RSWAppThread *_thread ;

		static uint32_t _ip ;
		static uint16_t _port ;
};
