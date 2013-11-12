#pragma once

class RSWAppThread ;

class RSWebUI 
{
	public:
		static bool isRunning() ;
		static bool start() ;
		static bool stop() ;

		static void setPort(uint16_t port) ;
		static void setIPMask(uint32_t ip) ;
	private:
		static RSWAppThread *_thread ;

		static uint32_t _ip ;
		static uint16_t _port ;
};
