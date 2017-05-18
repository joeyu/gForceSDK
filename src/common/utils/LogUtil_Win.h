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


#include <iostream>
#include <stdarg.h>
using namespace std;

#define GF_LOG_BUF_SIZE 1024

const char LevelPrompt[GF_LOG_MAX] = { 'V', 'D', 'I', 'W', 'E', 'F' };

struct win_log_print {
	static int print(GF_LOG_LEVEL level, const char* tag, const char* fmt, ...)
	{
		va_list ap;
		char buf[GF_LOG_BUF_SIZE];
		buf[0] = '\0';

		if (CURRENT_LOG_LEVEL > level)
			return 0;

		int offset = sprintf_s(buf, "[%c/%s]: ", LevelPrompt[level], tag);
		if (offset < 0)
			offset = 0;
		va_start(ap, fmt);
		vsnprintf_s(buf + offset, size_t(GF_LOG_BUF_SIZE - offset), _TRUNCATE, fmt, ap);
		va_end(ap);

		cout << buf << endl;
		return 0;//strlen(buf)
	}
};

#if defined(LOG_TO_CONSOLE)

#define gf_log_print_verbose(fmt, ...)	win_log_print::print(GF_LOG_VERBOSE, TAG, fmt, __VA_ARGS__)
#define gf_log_print_debug(fmt, ...)	win_log_print::print(GF_LOG_DEBUG, TAG, fmt, __VA_ARGS__)
#define gf_log_print_info(fmt, ...)		win_log_print::print(GF_LOG_INFO, TAG, fmt, __VA_ARGS__)
#define gf_log_print_warn(fmt, ...)		win_log_print::print(GF_LOG_WARN, TAG, fmt, __VA_ARGS__)
#define gf_log_print_error(fmt, ...)	win_log_print::print(GF_LOG_ERROR, TAG, fmt, __VA_ARGS__)
#define gf_log_print_fatal(fmt, ...)	win_log_print::print(GF_LOG_FATAL, TAG, fmt, __VA_ARGS__)

#elif defined(LOG_TO_FILE)

#define gf_log_print_verbose(fmt, ...)
#define gf_log_print_debug(fmt, ...)
#define gf_log_print_info(fmt, ...)
#define gf_log_print_warn(fmt, ...)
#define gf_log_print_error(fmt, ...)
#define gf_log_print_fatal(fmt, ...)

#else

#define gf_log_print_verbose(fmt, ...)
#define gf_log_print_debug(fmt, ...)
#define gf_log_print_info(fmt, ...)
#define gf_log_print_warn(fmt, ...)
#define gf_log_print_error(fmt, ...)
#define gf_log_print_fatal(fmt, ...)

#endif


#define GF_LOGV(fmt, ...) gf_log_print_verbose(fmt, ##__VA_ARGS__)
#define GF_LOGD(fmt, ...) gf_log_print_debug(fmt, ##__VA_ARGS__)
#define GF_LOGI(fmt, ...) gf_log_print_info(fmt, ##__VA_ARGS__)
#define GF_LOGW(fmt, ...) gf_log_print_warn(fmt, ##__VA_ARGS__)
#define GF_LOGE(fmt, ...) gf_log_print_error(fmt, ##__VA_ARGS__)
#define GF_LOGF(fmt, ...) gf_log_print_fatal(fmt, ##__VA_ARGS__)