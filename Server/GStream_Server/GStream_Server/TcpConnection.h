#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "GLOBAL.h"
#include "Logger.h"

namespace Gstream {
	/// Represents a single connection from a client.
	class TcpConnection : public ENABLE_SHARED<TcpConnection>, private boost::noncopyable {
	public:
		explicit TcpConnection(IO_SERVICE& io_service,FuncPointer*);
		SOCK& socket();
		void start();
		void read();
	private:
		void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);
		void handle_write(const boost::system::error_code& e);
		IO_SERVICE::strand strand_;
		SOCK socket_;
		boost::array<char, 8192> buffer_;
		FuncPointer* func_ptr;
		boost::array<int8_t, 1> incomming_packet;
		std::string hostName;
		void invalidData();
	};

	typedef SHARED_PTR<TcpConnection> connection_ptr;
}
#endif