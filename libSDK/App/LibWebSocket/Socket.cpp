/*
  Socket.cpp
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
#include <WinSock2.h>
#include "Socket.h"

int Socket::SetBlocking(bool blocking)
{
	u_long mode = blocking ? 0 : 1;

	if (0 == ::ioctlsocket(m_sockFd, FIONBIO, &mode)) {
		//printf("[ERROR] ioctlsocket\n");
		return -1;
	}
	return 0;
}

int Socket::Shutdown() {
	return ::shutdown(m_sockFd, SD_SEND);
}

int Socket::Close() {
	int ret = closesocket(m_sockFd);
	m_sockFd = INVALID_SOCKET;
	return ret;
}

Socket::~Socket() {
	Close();
}
