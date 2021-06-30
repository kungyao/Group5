// Copyright (c) 2021, Kung-Yao Hsieh, Shang-Chen LIN, Yi-Hang Xie. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTPLIB_SRC_HTTPSESSION_H_	
#define HTTPLIB_SRC_HTTPSESSION_H_	

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "SocketSession.h"
#include "StringUtility.h"
#include "HttpUtility.h"
class SocketSession;

namespace {
    // "\r\n"
    typedef boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type> iterator;
    std::pair<iterator, bool> match_end(iterator begin, iterator end) {
        int index = 0;
        const std::string match = "\r\n\r\n";
        iterator i = begin;
        while (i != end) {
            if (*i == match[index])
                index++;
            else
                index = 0;
            if (index == 4)
                return std::make_pair(i, true);
            i++;
        }
        return std::make_pair(i, false);
    }
}

/** @brief Handle the incoming http request. */
class HttpSession : public std::enable_shared_from_this<HttpSession> {
private:
    /** @brief socket fd.*/
    boost::asio::ip::tcp::socket socket_;
    //std::string buffer_;
    HttpRequest request;
    std::string response;

    boost::asio::streambuf buffer_;
    //boost::beast::tcp_stream stream_;
    //boost::beast::http::request<boost::beast::http::string_body> parser_;

    void makeSocketSession() {
        std::make_shared<SocketSession>(
            std::move(socket_), request)->runAsync();
            //std::move(stream_.release_socket()), request)->runAsync(parser_);
    }
    void handleFail(boost::system::error_code ec, char const* what) {
        if (ec == boost::asio::error::operation_aborted)
            return;
        std::cerr << what << ": " << ec.message() << "\n";
    }
    /** @brief Handle HttpRequest. This is called after receiveMessage() after the entire http request has been received.
     *  @param request Decoded HttpRequest.
     */
    void handleRequest(const std::string &buffer) {
        std::vector<std::string> httpStrs = StringUtility::split(buffer, "\r\n");
        for (size_t i = 0; i < httpStrs.size(); i++) {
            if (i == 0) {
                std::vector<std::string> requestTop = StringUtility::split(httpStrs.at(i), " ");
                request.Method = requestTop.at(0);
                std::transform(request.Method.begin(), request.Method.end(), request.Method.begin(), toupper);
                request.URL = requestTop.at(1);
                request.version = requestTop.at(2);

                if (request.URL.find("?") != std::string::npos) {
                    std::string content = StringUtility::split(request.URL, "?").at(1);
                    std::vector<std::string> paramStrs = StringUtility::split(content, "&");
                    for (std::vector<std::string>::iterator iter = paramStrs.begin(); iter != paramStrs.end(); iter++)
                    {
                        std::vector<std::string> paramStr = StringUtility::split(*iter, "=");
                        request.param.insert(std::pair<std::string, std::string>(paramStr.at(0), paramStr.at(1)));
                    }
                }
            }
            else if (i > 0 && i < httpStrs.size() - 1) {
                std::vector<std::string> requestHead = StringUtility::split(httpStrs.at(i), ": ");
                if (httpStrs.at(i).find("Host") != std::string::npos) {
                    request.Host = requestHead.at(1);
                }
                else if (httpStrs.at(i).find("Connection") != std::string::npos) {
                    request.Connection = requestHead.at(1);
                }
                else if (httpStrs.at(i).find("Upgrade") != std::string::npos) {
                    request.Upgrade = requestHead.at(1);
                }
                else if (httpStrs.at(i).find("Sec-WebSocket-Key") != std::string::npos) {
                    request.SecWebSocketKey = requestHead.at(1);
                }
            }
            else if (i == httpStrs.size() - 1) {
                request.content = httpStrs.at(i);
            }
        }
    }
    std::string handleResponse(const HttpRequest &request) {
        std::string response = "";
        response.append("HTTP/1.1 200 OK\r\n");
        response.append("Content-Type: text/html;charset=ANSI\r\n");
        response.append("Server: wxj233\r\n");
        response.append("Connection: close\r\n");
        response.append("\r\n");
        return response;
    }
    /** @brief Receive stream package.This is a callback for read_async() in asio
     *  @param msg Stream package's content.
     */
    void receiveMessage(const boost::system::error_code& ec, std::size_t bytes) {
        // Handle the error, if any
        if (ec) {
            handleFail(ec, "Http Read");
            close(ec);
            return;
        }

        // convert parser data to string data.
        //std::stringstream ss;
        //ss << parser_;
        //std::string buffer = ss.str();

        std::string buffer((std::istreambuf_iterator<char>(&buffer_)), std::istreambuf_iterator<char>());
        //std::string buffer(boost::asio::buffers_begin(buffer_), boost::asio::buffers_end(buffer_));
        std::cout << buffer << std::endl;
        handleRequest(buffer);

        if (request.Connection.find("Upgrade") == std::string::npos) {
            std::cout << "http�u��\n";
            response = handleResponse(request);
            boost::asio::async_write(
                socket_,
                boost::asio::buffer(response),
                std::bind(
                    &HttpSession::sendMessage,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2
                )
            );
            //boost::asio::async_write(
            //    stream_,
            //    boost::asio::buffer(response),
            //    std::bind(
            //        &HttpSession::sendMessage,
            //        shared_from_this(),
            //        std::placeholders::_1,
            //        std::placeholders::_2
            //    )
            //);
        }
        else if (request.Connection.find("Upgrade") != std::string::npos && request.Upgrade.find("websocket") != std::string::npos) {
            makeSocketSession();
        }
        else {
            close(ec);
        }
    }
    /** @brief Package Response and Send.
     *  @param response Decoded HttpResponse.
     */
    void sendMessage(const boost::system::error_code& ec, std::size_t bytes_transferred) {
        if (ec) {
            handleFail(ec, "Http Send");
        }
        return close(ec);
    }
public:
    explicit HttpSession(boost::asio::ip::tcp::socket&& socket)
        : socket_(std::move(socket))/*, stream_(std::move(socket))*/ {}
    HttpSession(const HttpSession&) = delete;
    HttpSession& operator=(const HttpSession&) = delete;

    /** @brief Run HttpSession after initial.*/
    void runAsync() {
        //buffer_ = "";
        //buffer_
        boost::asio::async_read_until(
            socket_,
            //boost::asio::dynamic_buffer(buffer_),
            buffer_,
            //boost::asio::transfer_all(), 
            match_end,
            std::bind(
                &HttpSession::receiveMessage,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        ); 
        //buffer_.consume(buffer_.size());
        //boost::beast::http::async_read(
        //    stream_,
        //    buffer_,
        //    //boost::asio::dynamic_buffer(buffer_),
        //    parser_,
        //    std::bind(
        //        &HttpSession::receiveMessage,
        //        shared_from_this(),
        //        std::placeholders::_1,
        //        std::placeholders::_2
        //    )
        //);
    }

    /** @brief Close HttpSession.*/
    void close(boost::system::error_code ec) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        socket_.close();
        //stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    }
};

#endif
