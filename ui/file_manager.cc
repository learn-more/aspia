//
// PROJECT:         Aspia Remote Desktop
// FILE:            ui/file_manager.cc
// LICENSE:         See top-level directory
// PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
//

#include "ui/file_manager.h"
#include "ui/resource.h"
#include "ui/status_code.h"
#include "ui/base/module.h"
#include "base/logging.h"

namespace aspia {

static const int kDefaultWindowWidth = 980;
static const int kDefaultWindowHeight = 700;

UiFileManager::UiFileManager(Delegate* delegate) :
    delegate_(delegate)
{
    ui_thread_.Start(MessageLoop::TYPE_UI, this);
}

UiFileManager::~UiFileManager()
{
    ui_thread_.Stop();
}

void UiFileManager::OnBeforeThreadRunning()
{
    runner_ = ui_thread_.message_loop_proxy();
    DCHECK(runner_);

    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
        WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    const UiModule& module = UiModule::Current();

    if (!Create(nullptr, style, module.string(IDS_FT_FILE_TRANSFER)))
    {
        LOG(ERROR) << "File manager window not created";
        runner_->PostQuit();
    }
    else
    {
        ScopedHICON icon(module.small_icon(IDI_MAIN));
        SetIcon(icon);
        SetCursor(LoadCursorW(nullptr, IDC_ARROW));
    }
}

void UiFileManager::OnAfterThreadRunning()
{
    DestroyWindow();
}

void UiFileManager::ReadStatusCode(proto::Status status)
{
    if (!runner_->BelongsToCurrentThread())
    {
        runner_->PostTask(std::bind(&UiFileManager::ReadStatusCode, this, status));
        return;
    }

    std::wstring message = StatusCodeToString(UiModule::Current(), status);

    MessageBoxW(hwnd(), message.c_str(), nullptr, MB_ICONWARNING | MB_OK);
}

void UiFileManager::ReadDriveList(PanelType panel_type,
                                  std::unique_ptr<proto::DriveList> drive_list)
{
    if (panel_type == UiFileManager::PanelType::REMOTE)
    {
        remote_panel_.ReadDriveList(std::move(drive_list));
    }
    else
    {
        DCHECK(panel_type == UiFileManager::PanelType::LOCAL);
        local_panel_.ReadDriveList(std::move(drive_list));
    }
}

void UiFileManager::ReadDirectoryList(PanelType panel_type,
                                      std::unique_ptr<proto::DirectoryList> directory_list)
{
    if (panel_type == UiFileManager::PanelType::REMOTE)
    {
        remote_panel_.ReadDirectoryList(std::move(directory_list));
    }
    else
    {
        DCHECK(panel_type == UiFileManager::PanelType::LOCAL);
        local_panel_.ReadDirectoryList(std::move(directory_list));
    }
}

void UiFileManager::OnDriveListRequest(UiFileManager::PanelType panel_type)
{
    delegate_->OnDriveListRequest(panel_type);
}

void UiFileManager::OnDirectoryListRequest(PanelType panel_type, const std::string& path)
{
    delegate_->OnDirectoryListRequest(panel_type, path);
}

void UiFileManager::OnCreateDirectoryRequest(PanelType panel_type,
                                             const std::string& path,
                                             const std::string& name)
{
    delegate_->OnCreateDirectoryRequest(panel_type, path, name);
}

void UiFileManager::OnRenameRequest(PanelType panel_type,
                                    const std::string& path,
                                    const std::string& old_name,
                                    const std::string& new_name)
{
    delegate_->OnRenameRequest(panel_type, path, old_name, new_name);
}

void UiFileManager::OnRemoveRequest(PanelType panel_type,
                                    const std::string& path,
                                    const std::string& item_name)
{
    delegate_->OnRemoveRequest(panel_type, path, item_name);
}

void UiFileManager::OnCreate()
{
    splitter_.CreateWithProportion(hwnd());

    local_panel_.CreatePanel(splitter_, UiFileManager::PanelType::LOCAL, this);
    remote_panel_.CreatePanel(splitter_, UiFileManager::PanelType::REMOTE, this);

    splitter_.SetPanels(local_panel_, remote_panel_);

    MoveWindow(hwnd(), 0, 0, kDefaultWindowWidth, kDefaultWindowHeight, TRUE);
    CenterWindow();

    runner_->PostTask(std::bind(&UiFileManager::OnDriveListRequest,
                                this,
                                UiFileManager::PanelType::LOCAL));

    runner_->PostTask(std::bind(&UiFileManager::OnDriveListRequest,
                                this,
                                UiFileManager::PanelType::REMOTE));
}

void UiFileManager::OnDestroy()
{
    local_panel_.DestroyWindow();
    remote_panel_.DestroyWindow();
    splitter_.DestroyWindow();
}

void UiFileManager::OnSize(int width, int height)
{
    HDWP dwp = BeginDeferWindowPos(1);

    if (dwp)
    {
        DeferWindowPos(dwp,
                       splitter_,
                       nullptr,
                       0,
                       0,
                       width,
                       height,
                       SWP_NOACTIVATE | SWP_NOZORDER);

        EndDeferWindowPos(dwp);
    }
}

void UiFileManager::OnGetMinMaxInfo(LPMINMAXINFO mmi)
{
    mmi->ptMinTrackSize.x = 500;
    mmi->ptMinTrackSize.y = 400;
}

void UiFileManager::OnClose()
{
    delegate_->OnWindowClose();
}

bool UiFileManager::OnMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT* result)
{
    switch (msg)
    {
        case WM_CREATE:
            OnCreate();
            break;

        case WM_SIZE:
            OnSize(LOWORD(lparam), HIWORD(lparam));
            break;

        case WM_GETMINMAXINFO:
            OnGetMinMaxInfo(reinterpret_cast<LPMINMAXINFO>(lparam));
            break;

        case WM_CLOSE:
            OnClose();
            break;

        case WM_DESTROY:
            OnDestroy();
            break;

        default:
            return false;
    }

    *result = 0;
    return true;
}

} // namespace aspia
