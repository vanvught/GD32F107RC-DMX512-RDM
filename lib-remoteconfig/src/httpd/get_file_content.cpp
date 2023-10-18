/**
 * @file get_file_content.cpp
 *
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdint>
#include <cstring>
#include <cassert>

#include "httpd/httpd.h"
#include "../http/content/content.h"

#include "debug.h"

#if defined (CONFIG_HTTP_CONTENT_FS)
#include <cstdio>

static constexpr char supported_extensions[static_cast<int>(http::contentTypes::NOT_DEFINED)][8] = {
		"html",
		"css",
		"js",
		"json"
};

static http::contentTypes getContentType(const char *pFileName) {
	for (int i = 0; i < static_cast<int>(http::contentTypes::NOT_DEFINED); i++) {
		const auto l = strlen(pFileName);
		const auto e = strlen(supported_extensions[i]);

		if (l > (e + 2)) {
			if (pFileName[l - e - 1] == '.') {
				if (strcmp(&pFileName[l - e], supported_extensions[i]) == 0) {
					return static_cast<http::contentTypes>(i);
				}
			}
		}
	}

	return http::contentTypes::NOT_DEFINED;
}

int get_file_content(const char *fileName, char *pDst, http::contentTypes& contentType) {
	auto *pFile = fopen(fileName, "r");

	if (pFile == nullptr) {
		DEBUG_EXIT
		return -1;
	}

	contentType = getContentType(fileName);

	if (contentType == http::contentTypes::NOT_DEFINED) {
		DEBUG_EXIT
		fclose(pFile);
		return -2;
	}

	auto doRemoveWhiteSpaces = true;
	auto *p = pDst;
	int c;

	while ((c = fgetc(pFile)) != EOF) {
		if (doRemoveWhiteSpaces) {
			if (c < ' ') {
				continue;
			} else {
				doRemoveWhiteSpaces = false;
			}
		} else {
			if (c == '\n') {
				doRemoveWhiteSpaces = true;
			}
		}
		*p++ = c;
		if ((p - pDst) == http::BUFSIZE) {
			DEBUG_PUTS("File too long");
			break;
		}
	}

	fclose(pFile);

	DEBUG_PRINTF("%s -> %d", fileName, static_cast<int>(p - pDst));
	return static_cast<int>(p - pDst);
}
#else
int get_file_content(const char *pFileName, char *pDst, http::contentTypes& contentType) {
	for (auto& content : HttpContent) {
		if (strcmp(pFileName, content.pFileName) == 0) {
			assert(content.nContentLength < http::BUFSIZE);
			memcpy(pDst, content.pContent, content.nContentLength);

			DEBUG_PRINTF("%s -> %d", content.pFileName, content.nContentLength);
			contentType = content.contentType;
			return static_cast<int>(content.nContentLength);
		}
	}

	DEBUG_EXIT
	return -1;
}
#endif
