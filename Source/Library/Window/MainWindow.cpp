#include "Window/MainWindow.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::Initialize

      Summary:  Initializes main window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                    Is a flag that says whether the main application window
                    will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                    The window name

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName){
        RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        HRESULT hr = initialize(
            hInstance,
            nCmdShow,
            pszWindowName,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
            rc.right - rc.left,
            rc.bottom - rc.top,
            nullptr,
            nullptr
        );
        if (FAILED(hr))
        {
            return hr;
        }
        
        RAWINPUTDEVICE rid = {
            .usUsagePage = 0x01,
            .usUsage = 0x02,
            .dwFlags = 0,
            .hwndTarget = 0,
        };
        if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
        {
            return E_FAIL;
        }

        RECT ab;
        POINT p1, p2;
        GetClientRect(m_hWnd, &ab);
        p1.x = ab.left;
        p1.y = ab.top;
        p2.x = ab.right;
        p2.y = ab.bottom;

        ClientToScreen(m_hWnd, &p1);
        ClientToScreen(m_hWnd, &p2);

        ab.left = p1.x;
        ab.top = p1.y;
        ab.right = p2.x;
        ab.bottom = p2.y;

        ClipCursor(&ab);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName

      Summary:  Returns the name of the window class

      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    PCWSTR MainWindow::GetWindowClassName() const {
        return L"MainWindow";
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::HandleMessage

      Summary:  Handles the messages

      Args:     UINT uMessage
                  Message code
                WPARAM wParam
                    Additional data the pertains to the message
                LPARAM lParam
                    Additional data the pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    LRESULT MainWindow::HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {

        PAINTSTRUCT ps;
        HDC hdc;
        UINT dataSize;
        switch (uMsg)
        {
        case WM_PAINT:
            hdc = BeginPaint(m_hWnd, &ps);
            EndPaint(m_hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

            // Note that this tutorial does not handle resizing (WM_SIZE) requests,
            // so we created the window without the resize border.

        case WM_INPUT:
            dataSize = 0;
            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
            if (dataSize > 0) {
                std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
                if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize) {
                    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
                    if (raw->header.dwType == RIM_TYPEMOUSE) {
                        m_mouseRelativeMovement.X = raw->data.mouse.lLastX;
                        m_mouseRelativeMovement.Y = raw->data.mouse.lLastY;
                    }
                }
            }
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        case WM_KEYDOWN:
            switch (wParam) 
            {
                case 'W':
                case'w':
                    m_directions.bFront = true;
                    break;
                case 'S':
                case 's':
                    m_directions.bBack = true;
                    break;
                case 'A':
                case 'a':
                    m_directions.bLeft = true;
                    break;
                case 'D':
                case 'd':
                    m_directions.bRight = true;
                    break;
                case VK_SHIFT:
                    m_directions.bDown = true;
                    break;
                case VK_SPACE :
                    m_directions.bUp = true;
                    break;
                default :
                    break;



            }

            break;
        case WM_KEYUP:
            switch (wParam) 
            {
                case 'W':
                case'w':
                    m_directions.bFront = false;
                    break;
                case 'S':
                case 's':
                    m_directions.bBack = false;
                    break;
                case 'A':
                case 'a':
                    m_directions.bLeft = false;
                    break;
                case 'D':
                case 'd':
                    m_directions.bRight = false;
                    break;
                case VK_SHIFT:
                    m_directions.bDown = false;
                    break;
                case VK_SPACE:
                    m_directions.bUp = false;
                    break;
                default:
                    break;
            }
            break;

        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    const DirectionsInput& MainWindow::GetDirections() const {
        return m_directions;
    }
    const MouseRelativeMovement& MainWindow:: GetMouseRelativeMovement() const {
        return m_mouseRelativeMovement;
    }
    void MainWindow::ResetMouseMovement() {
        m_mouseRelativeMovement = { 0,0 };
    }

}