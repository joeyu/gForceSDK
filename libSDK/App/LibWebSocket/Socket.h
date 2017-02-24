/*
  Socket.h
  2014 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:lawliet zou(lawliet.zou@gmail.com)
  2014-2-24

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef SOCKET_H_
#define SOCKET_H_

#include <WinSock2.h>

/** Socket file descriptor and select wrapper
  */
class Socket {
public:
	virtual ~Socket();

	bool IsValid() { return m_sockFd == INVALID_SOCKET ? false : true; }
	int SetBlocking(bool blocking);
        
	// Returns what the specific OS' close function returns
	int Shutdown();
	int Close();

protected:
	Socket() : m_sockFd(INVALID_SOCKET) {}
	Socket(SOCKET socket) : m_sockFd(socket) {}

	// Socket file descriptor from the specific OS
	SOCKET m_sockFd;
private:
};


#endif /* SOCKET_H_ */
