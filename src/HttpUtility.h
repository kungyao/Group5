// Copyright (c) 2021, Kung-Yao Hsieh, Shang-Chen LIN, Yi-Hang Xie. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTPLIB_SRC_HTTPUTILITY_H_	
#define HTTPLIB_SRC_HTTPUTILITY_H_	

#include <string>
#include <map>

typedef struct {
    std::string Method;
    std::string URL;
    std::string version;
    std::string Host;
    std::string Connection;
    std::string Upgrade;
    std::string cookie;
    std::map<std::string, std::string> param;
    std::string SecWebSocketKey;
    std::string content;
}HttpRequest;

#endif
