// Copyright (c) 2021, Kung-Yao Hsieh, Shang-Chen LIN, Yi-Hang Xie. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. 

#ifndef GROUP5_SRC_SOCKETSESSION_H_	
#define GROUP5_SRC_SOCKETSESSION_H_
#include <boost/asio.hpp>  
#include <boost/beast.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include "SocketUtility.h"
#include "HttpUtility.h"

/** @brief Handle the upgrading http request*/
class SocketSession : public std::enable_shared_from_this<SocketSession> {
private:
    /** @brief socket id.*/
    //boost::asio::ip::tcp::socket socket_;

    std::string handshake_msg_;
    HttpRequest http_request_;

    boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
    boost::asio::streambuf buffer_;

    std::queue<std::string> queue_;

    void handleFail(boost::system::error_code ec, char const* what) {
        //if (ec == boost::asio::error::operation_aborted) 
        //    return;
        std::cerr << what << ": " << ec.message() << "\n";
    }

    // , std::size_t bytes
    void acceptConnetction(const boost::system::error_code& ec, std::size_t bytes) {
        // Handle the error, if any
        if (ec) {
            handleFail(ec, "Socket Accept");
            close();
            return;
        }
        receiveMessage();
    }

    /** @brief Handle SocketMessage.
     *  @param message Decoded SocketMessage. This is called by receiveMessage() after the entire socket message has been received.
     */
    SocketUtility::Frame handleMessage(const std::string& frame) {
        SocketUtility::Frame wsFrame;

        int pos = 0;
        wsFrame.fin = (uint8_t)((frame[pos] >> 7) & 0x1);
        wsFrame.opcode = (uint8_t)(frame[pos] & 0xf);
        pos++;
        wsFrame.Mask = (uint8_t)((frame[pos] >> 7) & 0x1);
        wsFrame.PayloadLen = (uint8_t)(frame[pos] & 0x7f);
        pos++;

        if (wsFrame.opcode == 0x1) {
            if (wsFrame.PayloadLen == 126) {
                memcpy(&wsFrame.PayloadLen, &frame[0] + pos, 2);
                wsFrame.PayloadLen = ntohs(wsFrame.PayloadLen);
                pos += 2;
            }
            else if (wsFrame.PayloadLen == 127) {
                memcpy(&wsFrame.PayloadLen, &frame[0] + pos, 8);
                wsFrame.PayloadLen = ntohl(wsFrame.PayloadLen);
                pos += 8;
            }
            wsFrame.Payload = new char[wsFrame.PayloadLen + 1];

            if (wsFrame.Mask == 1) {
                memcpy(wsFrame.Maskingkey, &frame[0] + pos, 4);
                pos += 4;

                for (int i = 0; i < wsFrame.PayloadLen; i++) {
                    int j = i % 4;
                    wsFrame.Payload[i] = frame[pos + i] ^ wsFrame.Maskingkey[j];
                }
            }
            else {
                memcpy(wsFrame.Payload, &frame[0] + pos, wsFrame.PayloadLen);
            }
            wsFrame.Payload[wsFrame.PayloadLen] = '\0';
        }
        return wsFrame;
    }

    void receiveMessage() {
        buffer_.consume(buffer_.size());
        ws_.async_read(
            buffer_,
            std::bind(
                &SocketSession::receiveMessageHandler,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        );
    }
    /** @brief Receive stream package.
     *  @param msg stream package's content. This is a callback for read_async() in asio.
     */
    //void receiveMessage(const std::vector<boost::asio::const_buffer> &msg);
    void receiveMessageHandler(const boost::system::error_code& ec, std::size_t bytes) {
        std::cout << "Message : \n" << boost::beast::buffers_to_string(buffer_.data()) << std::endl;
        // Handle the error, if any
        if (ec) {
            handleFail(ec, "Socket Read");
            close();
            return;
        }

        //std::string message = boost::beast::buffers_to_string(buffer_.data());
        //std::cout << "Message : \n" << message << std::endl;
        //SocketUtility::Frame frame = handleMessage(message);
        //if (frame.opcode == 0x1) {
        //    std::cout << "Receive From Client : \n" << message << std::endl;
        //    //send(buffer_);
        //    //boost::asio::async_read_until(
        //    //    socket_,
        //    //    boost::asio::dynamic_buffer(buffer_),
        //    //    //boost::asio::transfer_all(), 
        //    //    "\n",
        //    //    std::bind(
        //    //        &SocketSession::receiveMessage,
        //    //        shared_from_this(),
        //    //        std::placeholders::_1,
        //    //        std::placeholders::_2
        //    //    )
        //    //);
        //}
        //else if (frame.opcode == 0x8) {
        //    std::cout << "Close Socket\n";
        //    close();
        //    return;
        //}
        ////std::cout << "Opcode is not inside the conditions\n";
        ////return;
        receiveMessage();
    }
    void sendMessageHandler(const boost::system::error_code& ec, std::size_t bytes_transferred) {
        queue_.pop();
        // Handle the error, if any
        if (ec) {
            handleFail(ec, "Socket Send");
            close();
            return;
        }

        if (!queue_.empty()) {
            sendMessage();
        }
    }
    /** @brief Package Message and Send.
     *  @param message Decoded SocketMessage.
     */
    void sendMessage() {
        std::cout << "MMMMMMMMMMMMMMMMMMMMMMMMMM : \n" << queue_.front() << std::endl;
        ws_.async_write(
            boost::asio::buffer(queue_.front()),
            std::bind(
                &SocketSession::sendMessageHandler,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        );
    }

    void handshake() {
        //use boost::asio here for custom handshake message.
        boost::asio::async_write(
            ws_.next_layer(),
            boost::asio::buffer(handshake_msg_),
            std::bind(
                &SocketSession::acceptConnetction,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        );
        //boost::asio::write(
        //    ws_.next_layer().socket(),
        //    boost::asio::buffer(handshake_msg_)
        //);
        //acceptConnetction({}, 0);
    }

public:
    // http_request_(request) maybe we do not need to do it?
    SocketSession(boost::asio::ip::tcp::socket&& socket, const HttpRequest &request)
     : ws_(std::move(socket)), http_request_(request) {
        ws_.set_option(
            boost::beast::websocket::stream_base::timeout::suggested(
                boost::beast::role_type::server));
        std::string SecWebSocketAccept = SocketUtility::generateAcceptKey(http_request_.SecWebSocketKey);
        handshake_msg_ = "";
        handshake_msg_.append("HTTP/1.1 101 Switching Protocols\r\n");
        handshake_msg_.append("Upgrade: websocket\r\n");
        handshake_msg_.append("Connection: Upgrade\r\n");
        handshake_msg_.append("Sec-WebSocket-Accept: " + SecWebSocketAccept + "\r\n");
        handshake_msg_.append("\r\n");
    }
    SocketSession(const SocketSession &) = delete;
    SocketSession& operator=(const SocketSession&) = delete;

    void send(const std::string& str) {
        queue_.push(str);
        sendMessage();
    }

    /** @brief Run socketSession after initial.*/
    void runAsync(/*boost::beast::http::request<boost::beast::http::string_body> req*/) {
        handshake();
        //ws_.async_accept(
        //    req,
        //    std::bind(
        //        &SocketSession::acceptConnetction,
        //        shared_from_this(),
        //        std::placeholders::_1
        //    )
        //);
    }

    void closeHandler(boost::system::error_code& ec) {
        std::cout << "Close Socket  " << ec << "\n";
    }

    /** @brief Close webSocket session.*/
    void close() {
        // todo : better way?
        ws_.async_close(
            {}, 
            std::bind(
                &SocketSession::closeHandler,
                shared_from_this(),
                std::placeholders::_1
            )
        );
    }
};
#endif
