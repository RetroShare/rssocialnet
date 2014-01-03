#pragma once

#include <stdint.h>

class RSWAppThread ;
class RsPlugInInterfaces ;

class RSWebUI 
{
	public:
		class IPRange
		{
			public:
				std::string toStdString() const ;

				uint32_t ip ;
				uint8_t bits ;

				bool contains(const IPRange& r) const ;

				// use this syntax: a.b.c.d/e
				//
				static IPRange make_range(const std::string& str,bool& success);

			private:
				IPRange(uint32_t _ip,uint8_t _bits) : ip(_ip),bits(_bits) {}
		};
		static bool isRunning() ;
		static bool start(const RsPlugInInterfaces&) ;
		static bool stop() ;
		static bool restart() ;

		static void setPort(uint16_t port) { _port = port ; }
		static void setIPMask(const std::vector<IPRange>& ip) { _ip = ip ; }

		static uint16_t port()  { return _port ; }
		static const std::vector<IPRange>& ipMask() { return _ip ; }
	private:
		static RSWAppThread *_thread ;

		static std::vector<IPRange> _ip ;
		static uint16_t _port ;
};
