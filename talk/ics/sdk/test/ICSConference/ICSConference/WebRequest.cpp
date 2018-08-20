/*
 * Copyright © 2018 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// WebRequest.cpp : implementation file
//
#include "WebRequest.h"
#include <boost/asio.hpp>
#include <iostream>
#include <vector>

vector<string>& split(const string& s,
    char delim,
    vector<string>& elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string& s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

string CWebRequest::getToken(const string& addr, const string& room, const string& role, const string& user)
{
    using boost::asio::ip::tcp;
    try {
        boost::asio::io_service io_service;
        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        vector<string> list = split(addr, '/');
        string server = list[2];
        list = split(server, ':');
        tcp::resolver::query query(list[0], list[1]);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        // Body
        string content =
            "{\"room\":\""+ room +"\",\"role\":\""+ role +"\",\"username\":\""+ user +"\"}";

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        ostream request_stream(&request);
        request_stream << "POST "
            << "/createToken/"
            << " HTTP/1.1\r\n";
        request_stream << "Host: " << list[0] + ":" + list[1] << "\r\n";
        request_stream << "Accept: application/json\r\n";
        request_stream << "Content-Type: application/json\r\n";
        request_stream << "Content-Length: " << content.length() << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << content;

        // Send the request.
        boost::asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        istream response_stream(&response);
        string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        string status_message;
        getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            cout << "Invalid response\n";
            return "";
        }
        if (status_code != 200) {
            cout << "Response returned with status code " << status_code << "\n";
            return "";
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        string header;
        while (getline(response_stream, header) && header != "\r")
            cout << header << "\n";
        cout << "\n";

        ostringstream token_stream;

        // Write whatever content we already have to output.
        if (response.size() > 0) {
            token_stream << &response;
        }

        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;
        while (boost::asio::read(socket, response,
            boost::asio::transfer_at_least(1), error))
            token_stream << &response;
        if (error != boost::asio::error::eof)
            throw boost::system::system_error(error);
        return token_stream.str();
    }
    catch (exception& e) {
        cout << "Exception: " << e.what() << "\n";
    }

    return "";
}

void CWebRequest::mix(const string& addr, const string& room, const string& pub) {
    using boost::asio::ip::tcp;
    try {
        boost::asio::io_service io_service;
        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        vector<string> list = split(addr, '/');
        string server = list[2];
        list = split(server, ':');
        tcp::resolver::query query(list[0], list[1]);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        // Body
        string content =
            "[{\"op\":\"add\",\"path\":\"/info/inViews\",\"value\":\"common\"}]";

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "PATCH "
            << "/rooms/" << room
            << "/streams/" << pub
            << " HTTP/1.1\r\n";
        request_stream << "Host: " << list[0] + ":" + list[1] << "\r\n";
        request_stream << "Accept: application/json\r\n";
        request_stream << "Content-Type: application/json\r\n";
        request_stream << "Content-Length: " << content.length() << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << content;

        // Send the request.
        boost::asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cout << "Invalid response\n";
            return;
        }
        if (status_code != 200) {
            std::cout << "Response returned with status code " << status_code << "\n";
            return;
        }
        return;
    }
    catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return;
}

CWebRequest::CWebRequest()
{
}


CWebRequest::~CWebRequest()
{
}
