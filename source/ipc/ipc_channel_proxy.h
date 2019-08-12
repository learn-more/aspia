//
// Aspia Project
// Copyright (C) 2019 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#ifndef IPC__IPC_CHANNEL_PROXY_H
#define IPC__IPC_CHANNEL_PROXY_H

#include "ipc/ipc_channel.h"

#include <mutex>

namespace ipc {

class ChannelProxy
{
public:
    bool setListener(Listener* listener);
    bool connectToServer(const QString& channel_name);
    bool isConnected() const;
    bool start();

private:
    friend class Channel;
    explicit ChannelProxy(Channel* channel);

    // Called directly by Channel::~Channel.
    void willDestroyCurrentChannel();

    Channel* channel_;
    mutable std::mutex channel_lock_;

    DISALLOW_COPY_AND_ASSIGN(ChannelProxy);
};

} // namespace ipc

#endif // IPC__IPC_CHANNEL_PROXY_H