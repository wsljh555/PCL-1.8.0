#include "RAMILAD.h"

HGLRC           hRC=NULL;								// 永久着色描述表
HDC             hDC=NULL;								// 私有GDI设备描述表
HWND            hWnd=NULL;								// 保存我们的窗口句柄 
HINSTANCE       hInstance;								// 保存程序的实例

bool    keys[256];										// 用于键盘例程的数组
bool    active=TRUE;									// 窗口的活动标志，缺省为TRUE 
bool    fullscreen=TRUE;								// 全屏标志缺省设定成全屏模式


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// WndProc的定义 


#pragma region ReSizeGLScene
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// 重置并初始化GL窗口大小 
{ 
	if (height==0)										// 防止被零除
	{ 
		height=1;										// 将Height设为1 
	} 
	glViewport(0, 0, width, height);					// 重置当前的视口(Viewport)
	glMatrixMode(GL_PROJECTION);						// 选择投影矩阵 
	glLoadIdentity();									// 重置投影矩阵 
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f); 
	glMatrixMode(GL_MODELVIEW);							// 选择模型观察矩阵 
	glLoadIdentity();									// 重置模型观察矩阵
}
#pragma endregion 初始化GL窗口

#pragma region InitGL
int InitGL(GLvoid)										// 此处开始对OpenGL进行所有设置 
{
	glShadeModel(GL_SMOOTH);							// 启用阴影平滑 
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// 黑色背景 
	glClearDepth(1.0f);									// 设置深度缓存 
	glEnable(GL_DEPTH_TEST);							// 启用深度测试 
	glDepthFunc(GL_LEQUAL);								// 所作深度测试的类型
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // 真正精细的透视修正
	return TRUE;										// 初始化 OK 
}
#pragma endregion 对OpenGL进行所有设置

#pragma region DrawGLScene
int DrawGLScene(GLvoid)									// 从这里开始进行所有的绘制 
{ 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除屏幕和深度缓存
	glLoadIdentity();									// 重置当前的模型观察矩阵
	return TRUE;									    // 一切 OK 
} 
#pragma endregion 进行所有绘制工作

#pragma region KillGLWindow
GLvoid KillGLWindow(GLvoid)								// 正常销毁窗口 
{
	//gluDeleteQuadric(obj);
	if (fullscreen)										// 我们处于全屏模式吗? 
	{
		ChangeDisplaySettings(NULL,0);                  // 是的话，切换回桌面 
		ShowCursor(TRUE);								// 显示鼠标指针 
	}

	if (hRC)											// 我们拥有着色描述表吗?
	{
		if (!wglMakeCurrent(NULL,NULL))                 // 我们能否释放DC和RC描述表?
		{
			MessageBoxA(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// 将 hInstance 设为 NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))                    // 我们能否释放 DC? 
	{
		MessageBoxA(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// 将 DC 设为 NULL 
	}

	if (hWnd && !DestroyWindow(hWnd))                   // 能否销毁窗口?
	{
		MessageBoxA(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// 将 hWnd 设为 NULL
	}

	if (!UnregisterClass("Real Scene",hInstance))       // 能否注销类?
	{
		MessageBoxA(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// 将 hInstance 设为 NULL
	}
}
#pragma endregion 正常销毁窗口

#pragma region CreateGLWindow
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint      PixelFormat;							// 保存查找匹配的结果

	WNDCLASS    wc;										// 窗口类结构 

	DWORD       dwExStyle;								// 扩展窗口风格 
	DWORD       dwStyle;								// 窗口风格 

	RECT WindowRect;									// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;							// Set Left Value To 0
	WindowRect.right=(long)width;                       // Set Right Value To Requested Width
	WindowRect.top=(long)0;								// Set Top Value To 0
	WindowRect.bottom=(long)height;                     // Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;							// 设置全局全屏标志

	hInstance = GetModuleHandle(NULL);                  // 取得我们窗口的实例
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;      // 移动时重画，并为窗口取得DC 
	wc.lpfnWndProc      = (WNDPROC) WndProc;            // WndProc处理消息
	wc.cbClsExtra       = 0;							// 无额外窗口数据 
	wc.cbWndExtra       = 0;							// 无额外窗口数据 
	wc.hInstance        = hInstance;                    // 设置实例 
	wc.hIcon        = LoadIcon(NULL, IDI_WINLOGO);      // 装入缺省图标
	wc.hCursor      = LoadCursor(NULL, IDC_ARROW);      // 装入鼠标指针
	wc.hbrBackground    = NULL;							// GL不需要背景
	wc.lpszMenuName     =  NULL;						// 不需要菜单
	wc.lpszClassName    = "Real Scene";                 // 设定类名字

	if (!RegisterClass(&wc))							// 尝试注册窗口类
	{
		MessageBoxA(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Exit And Return FALSE
	}

	if (fullscreen)										// 要尝试全屏模式吗?
	{
		DEVMODE dmScreenSettings;						// 设备模式
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));       // 确保内存分配
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		    // Devmode 结构的大小
		dmScreenSettings.dmPelsWidth    = width;					// 所选屏幕宽度
		dmScreenSettings.dmPelsHeight   = height;					// 所选屏幕高度
		dmScreenSettings.dmBitsPerPel   = bits;						// 每象素所选的色彩深度
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// 尝试设置显示模式并返回结果。注: CDS_FULLSCREEN 移去了状态条。 
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// 若模式失败，提供两个选项：退出或在窗口内运行。 
			if (MessageBoxA(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;               //如果用户选择窗口模式，变量fullscreen 的值变为FALSE,程序继续运行。
			}
			else
			{
				//如果用户选择退出，弹出消息窗口告知用户程序将结束。并返回FALSE告诉程序窗口未能成功创建。程序退出。
				MessageBoxA(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;                   //退出并返回 FALSE
			}
		}
	}

	if (fullscreen)                             // 仍处于全屏模式吗?
	{
		dwExStyle=WS_EX_APPWINDOW;              // 扩展窗体风格
		dwStyle=WS_POPUP;                       // 窗体风格
		ShowCursor(FALSE);                      // 隐藏鼠标指针
	}
	else
	{
		dwExStyle=WS_EX_WINDOWEDGE | WS_EX_APPWINDOW;         // Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW; //| ~WS_CAPTION;                    // Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);     // Adjust Window To True Requested Size

	if (!(hWnd=CreateWindowEx(  dwExStyle,              // Extended Style For The Window
		"Real Scene",               // Class Name
		title,                  // Window Title
		WS_CLIPSIBLINGS |           // Required Window Style
		WS_CLIPCHILDREN |           // Required Window Style
		dwStyle,                // Selected Window Style
		0, 0,                   // Window Position
		WindowRect.right-WindowRect.left,   // Calculate Adjusted Window Width
		WindowRect.bottom-WindowRect.top,   // Calculate Adjusted Window Height
		NULL,                   // No Parent Window
		NULL,                   // No Menu
		hInstance,              // Instance
		NULL)))                 // Don't Pass Anything To WM_CREATE
	{
		KillGLWindow();                         // Reset The Display
		MessageBoxA(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	static  PIXELFORMATDESCRIPTOR pfd=                  // pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),                  // Size Of This Pixel Format Descriptor
		1,                              // Version Number
		PFD_DRAW_TO_WINDOW |                        // Format Must Support Window
		PFD_SUPPORT_OPENGL |                        // Format Must Support OpenGL
		PFD_DOUBLEBUFFER,                       // Must Support Double Buffering
		PFD_TYPE_RGBA,                          // Request An RGBA Format
		bits,                               // Select Our Color Depth
		0, 0, 0, 0, 0, 0,                       // Color Bits Ignored
		0,                              // No Alpha Buffer
		0,                              // Shift Bit Ignored
		0,                              // No Accumulation Buffer
		0, 0, 0, 0,                         // Accumulation Bits Ignored
		16,                             // 16Bit Z-Buffer (Depth Buffer)
		0,                              // No Stencil Buffer
		0,                              // No Auxiliary Buffer
		PFD_MAIN_PLANE,                         // Main Drawing Layer
		0,                              // Reserved
		0, 0, 0                             // Layer Masks Ignored
	};

	if (!(hDC=GetDC(hWnd)))                         // Did We Get A Device Context?
	{
		KillGLWindow();                         // Reset The Display
		MessageBoxA(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))             // Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();                         // Reset The Display
		MessageBoxA(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))               // Are We Able To Set The Pixel Format?
	{
		KillGLWindow();                         // Reset The Display
		MessageBoxA(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))                   // Are We Able To Get A Rendering Context?
	{
		KillGLWindow();                         // Reset The Display
		MessageBoxA(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))                        // Try To Activate The Rendering Context
	{
		KillGLWindow();                         // Reset The Display
		MessageBoxA(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);                       // Show The Window
	SetForegroundWindow(hWnd);                      // Slightly Higher Priority
	SetFocus(hWnd);                             // Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);                       // Set Up Our Perspective GL Screen

	if (!InitGL())                              // Initialize Our Newly Created GL Window
	{
		KillGLWindow();                         // Reset The Display
		MessageBoxA(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	return TRUE;                                // Success
}
#pragma endregion 创建OpenGL窗口

#pragma region LRESULT CALLBACK WndProc
//这里是所有的窗口信息处理的地方。
//当我们注册完窗口类之后，我们通知它跳转到这部分代码来处理窗口信息
LRESULT CALLBACK WndProc(   HWND    hWnd,                   // Handle For This Window
						 UINT    uMsg,                   // Message For This Window
						 WPARAM  wParam,                 // Additional Message Information
						 LPARAM  lParam)                 // Additional Message Information
{
	//uMsg 变量保存了我们想要处理的消息的名字
	switch (uMsg)                               // Check For Windows Messages
	{
	case WM_ACTIVATE:                       // Watch For Window Activate Message
		{
			if (!HIWORD(wParam))                    // Check Minimization State
			{
				active=TRUE;                    // Program Is Active
			}
			else
			{
				active=FALSE;                   // Program Is No Longer Active
			} 
			return 0;                       // Return To The Message Loop
		}
	case WM_SYSCOMMAND:                     // Intercept System Commands
		{
			switch (wParam)                     // Check System Calls
			{
			case SC_SCREENSAVE:             // Screensaver Trying To Start?
			case SC_MONITORPOWER:               // Monitor Trying To Enter Powersave?
				return 0;                   // Prevent From Happening
			}
			break;                          // Exit
		}
	case WM_CLOSE:                          // Did We Receive A Close Message?
		{
			PostQuitMessage(0);                 // Send A Quit Message
			return 0;                       // Jump Back
		}
	case WM_KEYDOWN:                        // Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;                    // If So, Mark It As TRUE
			return 0;                       // Jump Back
		}
	case WM_KEYUP:                          // Has A Key Been Released?
		{
			keys[wParam] = FALSE;                   // If So, Mark It As FALSE
			return 0;                       // Jump Back
		}
	case WM_SIZE:                           // Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));       // LoWord=Width, HiWord=Height
			return 0;                       // Jump Back
		}

	}
	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
#pragma endregion 处理所有的窗口消息

#pragma region WinMain
//Windows应用程序的入口。
//在这里我们调用窗口创建程序，处理窗口消息，监听用户交互。
int WINAPI WinMain( HINSTANCE   hInstance,										//实例
				   HINSTANCE   hPrevInstance,									// 前一个实例
				   LPSTR       lpCmdLine,										// 命令行参数
				   int     nCmdShow)											// 窗口显示状态
{
	MSG msg;																	// Windowsx消息结构
	BOOL    done=FALSE;															// 用来退出循环的Bool 变量

	fullscreen=FALSE;															//窗口模式

																				// 创建OpenGL窗口
	if (!CreateGLWindow("Real Scene",windowsWidth,windowsHeight,16,fullscreen))
	{
		return 0;																// 失败退出
	}

	while(!done)																// 保持循环直到 done=TRUE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))								//有消息在等待吗?
		{
			if (msg.message==WM_QUIT)											//收到退出消息? 
			{
				done=TRUE;                  

			}
			else    // 如果不是退出消息，我们翻译消息，然后发送消息，使得WndProc() 或 Windows能够处理他们
			{
				TranslateMessage(&msg);											// 翻译消息
				DispatchMessage(&msg);              // Dispatch The Message
			}
		}
		else																	// 如果没有消息
		{
			// 绘制场景。监听ESC按键和DrawGLScene() 函数发出的退出消息
			if (active)															// 程序激活的么? 
			{
				if (keys[VK_ESCAPE])											// ESC 按下了么?
				{
					done=TRUE;              // ESC Signalled A Quit
				}
				else															// 不是退出的时候，刷新屏幕 
				{
					DrawGLScene();												// 绘制场景
					SwapBuffers(hDC);											// 交换缓存 (双缓存)
				}
			}

			if (keys[VK_F1])													
			{
				keys[VK_F1]=FALSE;												// 若是，使对应的Key数组中的值为 FALSE 
				KillGLWindow();													// 销毁当前的窗口
				fullscreen=!fullscreen;											// 切换 全屏 / 窗口 模式 
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("OpenGL",800,800,16,fullscreen))
				{
					return 0;               // Quit If Window Was Not Created
				}
			}

		}

	}

	KillGLWindow();																// 销毁窗口
	return (msg.wParam);														// 退出程序
}
#pragma endregion Windows程序的入口