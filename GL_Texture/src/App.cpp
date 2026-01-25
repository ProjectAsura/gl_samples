//-----------------------------------------------------------------------------
// File : App.cpp
// Desc : Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <vector>
#include <App.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <MiscGL.h>
#include <asdxMath.h>


#ifndef DLOG
    #if defined(DEBUG) || defined(_DEBUG)
        #define DLOG(x, ...) printf_s(x "\n", ##__VA_ARGS__)
    #else
        #define DLOG(x, ...) 
    #endif
#endif//DLOG

#ifndef ILOG
#define ILOG(x, ...) printf_s(x "\n", ##__VA_ARGS__)
#endif//ILOG

#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif//ELOG

// ウィンドウクラス名です.
#ifndef APP_WND_CLASSNAME
#define APP_WND_CLASSNAME      TEXT("WindowClass")
#endif//APP_WND_CLAASNAME


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/SimpleVS.inc"
#include "../res/shaders/Compiled/SimplePS.inc"

struct SceneParam
{
    asdx::Matrix    World;
    asdx::Matrix    View;
    asdx::Matrix    Proj;
};

} // namespace


///////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
App::App()
: App(L"Triangle", 960, 540, nullptr, nullptr, nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
App::App(LPCWSTR title, uint32_t width, uint32_t height, HICON hIcon, HMENU hMenu, HACCEL hAccel)
: m_hInst       (nullptr)
, m_hWnd        (nullptr)
, m_hDC         (nullptr)
, m_hGLContext  (nullptr)
, m_Width       (width)
, m_Height      (height)
, m_AspectRatio (float(width) / float(height))
, m_Title       (title)
, m_hIcon       (hIcon)
, m_hMenu       (hMenu)
, m_hAccel      (hAccel)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
App::~App()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      アプリケーションを実行します.
//-----------------------------------------------------------------------------
int App::Run()
{
    auto ret = -1;

    if (InitApp())
    { ret = MainLoop(); }
    
    TermApp();
    
    return ret;
}

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool App::InitApp()
{
    // COMライブラリの初期化.
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        DLOG( "Error : Com Library Initialize Failed." );
        return false;
    }

    // ウィンドウの初期化.
    if (!InitWnd())
    {
        ELOG("Error : InitWnd() Failed.");
        return false;
    }

    // OpenGLの初期化.
    if (!InitGL())
    {
        ELOG("Error : InitGL() Failed.");
        return false;
    }

    // アプリケーション固有の初期化.
    if (!OnInit())
    {
        ELOG("Error : OnInit() Failed.");
        return false;
    }

    // ウィンドウを表示します.
    UpdateWindow(m_hWnd);
    ShowWindow(m_hWnd, SW_SHOWNORMAL);

    // フォーカスを設定します.
    SetFocus(m_hWnd);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void App::TermApp()
{
    // アプリケーション固有の終了処理.
    OnTerm();

    // OpenGLの終了処理.
    TermGL();

    // ウィンドウの終了処理.
    TermWnd();

    // COMライブラリの終了処理.
    CoUninitialize();
}

//-----------------------------------------------------------------------------
//      ウィンドウの初期化.
//-----------------------------------------------------------------------------
bool App::InitWnd()
{
   // インスタンスハンドルを取得.
    HINSTANCE hInst = GetModuleHandle(nullptr);
    if (!hInst)
    {
        DLOG( "Error : GetModuleHandle() Failed. ");
        return false;
    }

    // アイコンなしの場合はロード.
    if (m_hIcon == nullptr)
    {
        // 最初にみつかったものをアイコンとして設定する.
        WCHAR exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        m_hIcon = ExtractIconW(hInst, exePath, 0);

        // それでも見つからなった場合.
        if (m_hIcon == nullptr)
        { m_hIcon = LoadIcon(hInst, IDI_APPLICATION); }
    }

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEXW wc = {};
    wc.cbSize           = sizeof( WNDCLASSEXW );
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MsgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = m_hIcon;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = APP_WND_CLASSNAME;
    wc.hIconSm          = m_hIcon;

    // ウィンドウクラスを登録します.
    if (!RegisterClassExW(&wc))
    {
        // エラーログ出力.
        DLOG( "Error : RegisterClassEx() Failed." );

        // 異常終了.
        return false;
    }

    // インスタンスハンドルを設定.
    m_hInst = hInst;

    // 矩形の設定.
    RECT rc = {0, 0, LONG(m_Width), LONG(m_Height)};

    DWORD style = WS_OVERLAPPEDWINDOW;

    // 指定されたクライアント領域を確保するために必要なウィンドウ座標を計算します.
    AdjustWindowRect(&rc, style, FALSE);

    // ウィンドウを生成します.
    m_hWnd = CreateWindowW(
        APP_WND_CLASSNAME,
        m_Title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ( rc.right - rc.left ),
        ( rc.bottom - rc.top ),
        NULL,
        m_hMenu,
        hInst,
        this);

    // 生成チェック.
    if (!m_hWnd)
    {
        // エラーログ出力.
        DLOG( "Error : CreateWindow() Failed." );

        // 異常終了.
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      ウィンドウの終了処理.
//-----------------------------------------------------------------------------
void App::TermWnd()
{
    // ウィンドウクラスの登録を解除.
    if ( m_hInst != nullptr )
    { UnregisterClassW(APP_WND_CLASSNAME, m_hInst); }

    if ( m_hAccel )
    { DestroyAcceleratorTable(m_hAccel); }

    if ( m_hMenu )
    { DestroyMenu(m_hMenu); }

    if ( m_hIcon )
    { DestroyIcon(m_hIcon); }

    // タイトル名をクリア.
    m_Title = nullptr;

    // ハンドルをクリア.
    m_hInst  = nullptr;
    m_hWnd   = nullptr;
    m_hIcon  = nullptr;
    m_hMenu  = nullptr;
    m_hAccel = nullptr;
    m_hDC    = nullptr;
}

//-----------------------------------------------------------------------------
//      OpenGLの初期化.
//-----------------------------------------------------------------------------
bool App::InitGL()
{
    // デバイスコンテキストを取得.
    m_hDC = GetDC(m_hWnd);
    if (!m_hDC)
    {
        ELOG("Error : GetDC() Failed.");
        return false;
    }

    // ピクセルフォーマットを設定.
    {
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize       = sizeof(pfd);
        pfd.nVersion    = 1;
        pfd.dwFlags     = LPD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.iPixelType  = PFD_TYPE_RGBA;
        pfd.cColorBits  = 32;
        pfd.cDepthBits  = 32;
        pfd.iLayerType  = PFD_MAIN_PLANE;

        auto format = ChoosePixelFormat(m_hDC, &pfd);
        if (SetPixelFormat(m_hDC, format, &pfd) == FALSE)
        {
            auto ret = GetLastError();
            ELOG("Error : SetPixelFormat() Failed. errcode = %d", ret);
            return false;
        }
    }

    // GLレンダリングコンテキストを生成.
    m_hGLContext = wglCreateContext(m_hDC);

    // カレントに設定しておく.
    wglMakeCurrent(m_hDC, m_hGLContext);

    // GLEWの初期化.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        ELOG("Error : glewInit() Failed.");
        return false;
    }

    // WGLEWの初期化.
    if (wglewInit() != GLEW_OK)
    {
        ELOG("Error : wglewInit() Failed.");
        return false;
    }

    // OpenGL 4.6 Core Profileの設定.
    static const int attr[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MAJOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB
    };

    // バージョン指定したGLレンダリングコンテキストを生成.
    auto hGLRC = wglCreateContextAttribsARB(m_hDC, nullptr, attr);
    if (hGLRC)
    {
        // カレントを変更.
        wglMakeCurrent(m_hDC, hGLRC);

        // 古いやつを破棄.
        wglDeleteContext(m_hGLContext);

        // 差し替える.
        m_hGLContext = hGLRC;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      OpenGLの終了処理.
//-----------------------------------------------------------------------------
void App::TermGL()
{
    // カレントを無効化.
    if (m_hDC)
    {
        wglMakeCurrent(m_hDC, nullptr);
    }

    // GLレンダリングコンテキストを破棄.
    if (m_hGLContext)
    {
        wglDeleteContext(m_hGLContext);
        m_hGLContext = nullptr;
    }

    // デバイスコンテキストを破棄.
    if (m_hDC && m_hWnd)
    {
        ReleaseDC(m_hWnd, m_hDC);
        m_hDC = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      メインループ処理.
//-----------------------------------------------------------------------------
int App::MainLoop()
{
    MSG msg = {};

    auto frameCount = 0;
    LARGE_INTEGER ticksPerSec = {};
    LARGE_INTEGER currTime = {};
    LARGE_INTEGER prevTime = {};
    QueryPerformanceFrequency(&ticksPerSec);
    QueryPerformanceCounter(&currTime);

    while(WM_QUIT != msg.message)
    {
        auto hasMsg = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);

        if (hasMsg)
        {
            auto ret = TranslateAccelerator(m_hWnd, m_hAccel, &msg);
            if (0 == ret)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            QueryPerformanceCounter(&currTime);
            auto time = (currTime.QuadPart - prevTime.QuadPart) / double(ticksPerSec.QuadPart);

            // 0.5秒ごとにFPSを更新.
            if (time > 0.5f)
            {
                // FPSを算出.
                m_FPS = float(frameCount / time);

                // 更新時間を設定.
                prevTime = currTime;

                frameCount = 0;
            }

            // フレーム遷移処理.
            OnFrameMove();

            // フレーム描画処理.
            OnFrameRender();

            // フレームカウントをインクリメント.
            m_FrameCount++;
        }
    }

    return (int)msg.wParam;
}

//-----------------------------------------------------------------------------
//      フレームカウントを取得します.
//-----------------------------------------------------------------------------
uint64_t App::GetFrameCount() const
{ return m_FrameCount; }

//-----------------------------------------------------------------------------
//      FPSを取得します.
//-----------------------------------------------------------------------------
float App::GetFPS() const
{ return m_FPS; }

//-----------------------------------------------------------------------------
//      初期化時の処理です.
//-----------------------------------------------------------------------------
bool App::OnInit()
{
    // 情報を表示.
    ILOG("OpenGL Version : %s", glGetString(GL_VERSION));
    ILOG("Vendor         : %s", glGetString(GL_VENDOR));
    ILOG("Renderer       : %s", glGetString(GL_RENDERER));

    // 頂点シェーダ生成.
    if (!m_VS.Init(Shader::VS, SimpleVS, sizeof(SimpleVS)))
    {
        ELOG("Error : Shader::Init() Failed.");
        return false;
    }

    // ピクセルシェーダ生成.
    if (!m_PS.Init(Shader::PS, SimplePS, sizeof(SimplePS)))
    {
        ELOG("Error : Shader::Init() Failed.");
        return false;
    }

    // シェーダプログラム生成.
    const Shader* shaders[] = {
        &m_VS,
        &m_PS,
    };
    if (!m_Program.Init(shaders, _countof(shaders)))
    {
        ELOG("Error : ShaderProgram::Init() Failed.");
        return false;
    }

    // 頂点バッファ生成
    {
        struct Vertex
        {
            asdx::Vector3 Position;
            asdx::Vector2 TexCoord;
        };

        const Vertex vertices[] = {
            { asdx::Vector3(-1.0f,  1.0f, 0.0f), asdx::Vector2(0.0f, 0.0f) },
            { asdx::Vector3( 1.0f,  1.0f, 0.0f), asdx::Vector2(1.0f, 0.0f) },
            { asdx::Vector3( 1.0f, -1.0f, 0.0f), asdx::Vector2(1.0f, 1.0f) },
            { asdx::Vector3(-1.0f, -1.0f, 0.0f), asdx::Vector2(0.0f, 1.0f) },
        };

        if (!m_VB.Init(vertices, _countof(vertices), sizeof(Vertex)))
        {
            ELOG("Error : m_PosVB Init Failed.");
            return false;
        }
    }

    // インデックスバッファ生成
    {
        const uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };

        if (!m_IB.Init(indices, _countof(indices)))
        {
            ELOG("Error : m_IB Init Failed.");
            return false;
        }
    }

    // ユニフォームバッファ生成.
    {
        auto aspectRatio = float(m_Width) / float(m_Height);

        SceneParam param = {};
        param.World = asdx::Matrix::CreateIdentity();
        param.View  = asdx::Matrix::CreateLookAt(asdx::Vector3(0.0f, 0.0f, -5.0f), asdx::Vector3(0.0f, 0.0f, 0.0f), asdx::Vector3(0.0f, 1.0f, 0.0f));
        param.Proj  = asdx::Matrix::CreatePerspectiveFieldOfView(asdx::ToRadian(37.5f), aspectRatio, 0.1f, 1000.0f);

        if (!m_UB.Init(&param, sizeof(param)))
        {
            ELOG("Error : m_UB Init Failed.");
            return false;
        }
    }

    // テクスチャ生成.
    {
        if (!m_Texture.InitFromFile("../res/texture/SampleTexture.png"))
        {
            ELOG("Error : m_Texture Init Failed.");
            return false;
        }
    }

    // サンプラー生成.
    {
        auto desc = Sampler::LinearClamp;
        if (!m_Sampler.Init(desc))
        {
            ELOG("Error : m_Sampler Init Failed.");
            return false;
        }
    }

    if (HasGLError())
        return false;

    return true;
}

//-----------------------------------------------------------------------------
//      終了時の処理です.
//-----------------------------------------------------------------------------
void App::OnTerm()
{
    m_Sampler.Term();
    m_Texture.Term();
    m_Program.Term();
    m_PS.Term();
    m_VS.Term();
    m_VB.Term();
    m_IB.Term();
}

//-----------------------------------------------------------------------------
//      フレーム遷移時の処理です.
//-----------------------------------------------------------------------------
void App::OnFrameMove()
{
    m_RotAngle += 0.05f;
    auto world = asdx::Matrix::CreateRotationY(m_RotAngle);
    m_UB.Update(&world, sizeof(world), 0);
}

//-----------------------------------------------------------------------------
//      フレーム描画時の処理です.
//-----------------------------------------------------------------------------
void App::OnFrameRender()
{
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[2]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, GLsizei(m_Width), GLsizei(m_Height));
    glScissor(0, 0, m_Width, m_Height);

    m_Program.Bind();
    {
        const Attribute attributes[] = {
            { 0, 3, GL_FLOAT, GL_FALSE, 0  },
            { 1, 2, GL_FLOAT, GL_FALSE, 12 },
        };

        InputLayout layout = { attributes, _countof(attributes) };

        m_VB.Bind(layout);
        m_IB.Bind();
        m_UB.Bind(0);
        m_Texture.Bind(1);
        m_Sampler.Bind(1);

        glDrawElements(GL_TRIANGLES, m_IB.GetIndexCount(), GL_UNSIGNED_INT, 0);

        m_Sampler.Unbind(1);
        m_Texture.Unbind(1);
        m_IB.Unbind();
        m_VB.Unbind(layout);
    }
    m_Program.Unbind();

    wglSwapIntervalEXT(1);
    SwapBuffers(m_hDC);
}

//-----------------------------------------------------------------------------
//      リサイズ時の処理です.
//-----------------------------------------------------------------------------
void App::OnResize(uint32_t w, uint32_t h)
{
}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void App::OnKey(uint32_t keyCode, bool isKeyDown, bool isAltDown)
{
}

//-----------------------------------------------------------------------------
//      マウス処理です.
//-----------------------------------------------------------------------------
void App::OnMouse(int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR)
{
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void App::OnTyping(uint32_t keyCode)
{
}

//-----------------------------------------------------------------------------
//      メッセージプロシージャの処理です.
//-----------------------------------------------------------------------------
void App::OnMsgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
}

//-----------------------------------------------------------------------------
//      ウィンドウプロシージャ.
//-----------------------------------------------------------------------------
LRESULT CALLBACK App::MsgProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
   auto pInstance = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    PAINTSTRUCT ps;
    HDC         hdc;

    if ( (uMsg == WM_KEYDOWN)
      || (uMsg == WM_SYSKEYDOWN)
      || (uMsg == WM_KEYUP)
      || (uMsg == WM_SYSKEYUP) )
    {
        if (pInstance != nullptr)
        {
            bool isKeyDown = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN);

            DWORD mask = (1 << 29);
            bool isAltDown = !!(lp & mask);

            pInstance->OnKey(uint32_t(wp), isKeyDown, isAltDown);
        }
    }

    if ( (uMsg == WM_LBUTTONDOWN)
      || (uMsg == WM_LBUTTONUP)
      || (uMsg == WM_LBUTTONDBLCLK)
      || (uMsg == WM_MBUTTONDOWN)
      || (uMsg == WM_MBUTTONUP)
      || (uMsg == WM_MBUTTONDBLCLK)
      || (uMsg == WM_RBUTTONDOWN)
      || (uMsg == WM_RBUTTONUP)
      || (uMsg == WM_RBUTTONDBLCLK)
      || (uMsg == WM_XBUTTONDOWN)
      || (uMsg == WM_XBUTTONUP)
      || (uMsg == WM_XBUTTONDBLCLK)
      || (uMsg == WM_MOUSEHWHEEL)
      || (uMsg == WM_MOUSEMOVE)
      || (uMsg == WM_MOUSEWHEEL) )
    {
        if (pInstance != nullptr)
        {
            int x = int(LOWORD(lp));
            int y = int(HIWORD(lp));

            int wheelDelta = 0;
            if ((uMsg == WM_MOUSEHWHEEL)
             || (uMsg == WM_MOUSEWHEEL))
            {
                POINT pt = {};
                pt.x = x;
                pt.y = y;

                ScreenToClient(hWnd, &pt);
                x = pt.x;
                y = pt.y;

                wheelDelta += int(HIWORD(wp));
            }

            int  mask = LOWORD( wp );
            bool isDownL  = !!(mask & MK_LBUTTON );
            bool isDownR  = !!(mask & MK_RBUTTON );
            bool isDownM  = !!(mask & MK_MBUTTON );
            //bool isDownX1 = !!(mask & MK_XBUTTON1);
            //bool isDownX2 = !!(mask & MK_XBUTTON2);

            //auto isAltDown   = !!(GetKeyState(VK_MENU)    & 0x8000);
            //auto isCtrlDown  = !!(GetKeyState(VK_CONTROL) & 0x8000);
            //auto isShiftDown = !!(GetKeyState(VK_SHIFT)   & 0x8000);

            pInstance->OnMouse(x, y, wheelDelta, isDownL, isDownM, isDownR);
        }
    }

    switch( uMsg )
    {
    case WM_CREATE:
        {
            auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lp);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));

            // ドラッグアンドドロップ可能.
            DragAcceptFiles(hWnd, TRUE);
        }
        break;

    case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        { PostQuitMessage(0); }
        break;

    case WM_SIZE:
        {
            auto w = uint32_t(LOWORD(lp));
            auto h = uint32_t(HIWORD(lp));

            if (pInstance != nullptr)
            {
                pInstance->m_Width  = w;
                pInstance->m_Height = h;
                pInstance->OnResize(w, h);
            }
        }
        break;

    case WM_CHAR:
        {
            if (pInstance != nullptr)
            {
                auto keyCode = uint32_t(wp);
                pInstance->OnTyping(keyCode);
            }
        }
        break;
    }

    // ユーザーカスタマイズ用に呼び出し.
    if (pInstance != nullptr)
    { pInstance->OnMsgProc(hWnd, uMsg, wp, lp); }

    return DefWindowProc( hWnd, uMsg, wp, lp );
}

