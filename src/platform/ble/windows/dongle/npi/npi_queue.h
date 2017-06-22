/*
 * Copyright 2017, OYMotion Inc.
 * All rights reserved.
 *
 * IMPORTANT: Your use of this Software is limited to those specific rights
 * granted under the terms of a software license agreement between you and
 * OYMotion.  You may not use this Software unless you agree to abide by the
 * terms of the License. The License limits your use, and you acknowledge,
 * that the Software may not be modified, copied or distributed unless used
 * solely and exclusively in conjunction with an OYMotion product.  Other
 * than for the foregoing purpose, you may not use, reproduce, copy, prepare
 * derivative works of, modify, distribute, perform, display or sell this
 * Software and/or its documentation for any purpose.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 */
#pragma once
#include <windows.h>


template<class T, unsigned max>
class NPI_Queue
{
	HANDLE space_avail; // at least one slot empty
	HANDLE data_avail;  // at least one slot full
	CRITICAL_SECTION mutex; // protect buffer, in_pos, out_pos

	T buffer[max];
	long in_pos, out_pos;
public:
	NPI_Queue() : in_pos(0), out_pos(0)
	{
		space_avail = CreateSemaphore(NULL, max, max, NULL);
		data_avail = CreateSemaphore(NULL, 0, max, NULL);
		InitializeCriticalSection(&mutex);
	}

	void Push(T data)
	{
		WaitForSingleObject(space_avail, INFINITE);
		EnterCriticalSection(&mutex);
		buffer[in_pos] = data;
		in_pos = (in_pos + 1) % max;
		LeaveCriticalSection(&mutex);
		ReleaseSemaphore(data_avail, 1, NULL);
	}

	T Pop()
	{
		WaitForSingleObject(data_avail, INFINITE);
		EnterCriticalSection(&mutex);
		T retval = buffer[out_pos];
		out_pos = (out_pos + 1) % max;
		LeaveCriticalSection(&mutex);
		ReleaseSemaphore(space_avail, 1, NULL);
		return retval;
	}

	~NPI_Queue()
	{
		DeleteCriticalSection(&mutex);
		CloseHandle(data_avail);
		CloseHandle(space_avail);
	}

};


