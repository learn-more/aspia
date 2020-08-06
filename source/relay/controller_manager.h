//
// Aspia Project
// Copyright (C) 2020 Dmitry Chapyshev <dmitry@aspia.ru>
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

#ifndef RELAY__CONTROLLER_MANAGER_H
#define RELAY__CONTROLLER_MANAGER_H

#include "base/memory/byte_array.h"
#include "base/net/network_server.h"
#include "relay/controller.h"

namespace base {
class TaskRunner;
} // namespace base

namespace relay {

class SessionManager;
class SharedPool;

class ControllerManager
    : public base::NetworkServer::Delegate,
      public Controller::Delegate
{
public:
    explicit ControllerManager(std::shared_ptr<base::TaskRunner> task_runner);
    ~ControllerManager();

    bool start();

protected:
    // net::Server::Delegate implementation.
    void onNewConnection(std::unique_ptr<base::NetworkChannel> channel) override;

    // Controller::Delegate implementation.
    void onControllerFinished(Controller* controller) override;

private:
    std::shared_ptr<base::TaskRunner> task_runner_;
    std::unique_ptr<base::NetworkServer> server_;
    std::unique_ptr<SharedPool> shared_pool_;
    std::unique_ptr<SessionManager> session_manager_;
    std::vector<std::unique_ptr<Controller>> controllers_;
    base::ByteArray session_key_;

    uint32_t current_controller_ = 0;

    DISALLOW_COPY_AND_ASSIGN(ControllerManager);
};

} // namespace relay

#endif // RELAY__CONTROLLER_MANAGER_H