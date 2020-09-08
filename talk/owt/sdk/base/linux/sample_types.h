// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __SAMPLE_TYPES_H__
#define __SAMPLE_TYPES_H__

#ifdef UNICODE
#define msdk_cout std::wcout
#define msdk_err std::wcerr
#else
#define msdk_cout std::cout
#define msdk_err std::cerr
#endif

typedef std::basic_string<msdk_char> msdk_string;
typedef std::basic_stringstream<msdk_char> msdk_stringstream;
typedef std::basic_ostream<msdk_char, std::char_traits<msdk_char>> msdk_ostream;
typedef std::basic_istream<msdk_char, std::char_traits<msdk_char>> msdk_istream;
typedef std::basic_fstream<msdk_char, std::char_traits<msdk_char>> msdk_fstream;

#if defined(_UNICODE)
#define MSDK_MAKE_BYTE_STRING(src, dest)       \
  {                                            \
    std::wstring wstr(src);                    \
    std::string str(wstr.begin(), wstr.end()); \
    strcpy_s(dest, str.c_str());               \
  }
#else
#define MSDK_MAKE_BYTE_STRING(src, dest) msdk_strcopy(dest, src);
#endif

#endif  //__SAMPLE_TYPES_H__
