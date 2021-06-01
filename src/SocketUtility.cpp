// Copyright (c) 2021, Kung-Yao Hsieh, Shang-Chen LIN, Yi-Hang Xie. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SocketUtility.h"

#include "StringUtility.h"

namespace SocketUtility {
    WSMessage parseMessage(const std::string& message) {
        return WSMessage{};
    }

    std::string generateAcceptKey(const std::string& seckey) {
        return StringUtility::encodeBase64(
            StringUtility::sha1(seckey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")
        );
    }
};
