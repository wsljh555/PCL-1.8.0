#include "RAMILAD.h"

HGLRC           hRC=NULL;								// ������ɫ������
HDC             hDC=NULL;								// ˽��GDI�豸������
HWND            hWnd=NULL;								// �������ǵĴ��ھ�� 
HINSTANCE       hInstance;								// ��������ʵ��

bool    keys[256];										// ���ڼ������̵�����
bool    active=TRUE;									// ���ڵĻ��־��ȱʡΪTRUE 
bool    fullscreen=TRUE;								// ȫ����־ȱʡ�趨��ȫ��ģʽ


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// WndProc�Ķ��� 


#pragma region ReSizeGLScene
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// ���ò���ʼ��GL���ڴ�С 
{ 
	if (height==0)										// ��ֹ�����
	{ 
		height=1;										// ��Height��Ϊ1 
	} 
	glViewport(0, 0, width, height);					// ���õ�ǰ���ӿ�(Viewport)
	glMatrixMode(GL_PROJECTION);						// ѡ��ͶӰ���� 
	glLoadIdentity();									// ����ͶӰ���� 
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f); 
	glMatrixMode(GL_MODELVIEW);							// ѡ��ģ�͹۲���� 
	glLoadIdentity();									// ����ģ�͹۲����
}
#pragma endregion ��ʼ��GL����

#pragma region InitGL
int InitGL(GLvoid)										// �˴���ʼ��OpenGL������������ 
{
	glShadeModel(GL_SMOOTH);							// ������Ӱƽ�� 
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// ��ɫ���� 
	glClearDepth(1.0f);									// ������Ȼ��� 
	glEnable(GL_DEPTH_TEST);							// ������Ȳ��� 
	glDepthFunc(GL_LEQUAL);								// ������Ȳ��Ե�����
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // ������ϸ��͸������
	return TRUE;										// ��ʼ�� OK 
}
#pragma endregion ��OpenGL������������

#pragma region DrawGLScene
int DrawGLScene(GLvoid)									// �����￪ʼ�������еĻ��� 
{ 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // �����Ļ����Ȼ���
	glLoadIdentity();									// ���õ�ǰ��ģ�͹۲����
	return TRUE;									    // һ�� OK 
} 
#pragma endregion �������л��ƹ���

#pragma region KillGLWindow
GLvoid KillGLWindow(GLvoid)								// �������ٴ��� 
{
	//gluDeleteQuadric(obj);
	if (fullscreen)										// ���Ǵ���ȫ��ģʽ��? 
	{
		ChangeDisplaySettings(NULL,0);                  // �ǵĻ����л������� 
		ShowCursor(TRUE);								// ��ʾ���ָ�� 
	}

	if (hRC)											// ����ӵ����ɫ��������?
	{
		if (!wglMakeCurrent(NULL,NULL))                 // �����ܷ��ͷ�DC��RC������?
		{
			MessageBoxA(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// �� hInstance ��Ϊ NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))                    // �����ܷ��ͷ� DC? 
	{
		MessageBoxA(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// �� DC ��Ϊ NULL 
	}

	if (hWnd && !DestroyWindow(hWnd))                   // �ܷ����ٴ���?
	{
		MessageBoxA(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// �� hWnd ��Ϊ NULL
	}

	if (!UnregisterClass("Real Scene",hInstance))       // �ܷ�ע����?
	{
		MessageBoxA(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// �� hInstance ��Ϊ NULL
	}
}
#pragma endregion �������ٴ���

#pragma region CreateGLWindow
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint      PixelFormat;							// �������ƥ��Ľ��

	WNDCLASS    wc;										// ������ṹ 

	DWORD       dwExStyle;								// ��չ���ڷ�� 
	DWORD       dwStyle;								// ���ڷ�� 

	RECT WindowRect;									// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;							// Set Left Value To 0
	WindowRect.right=(long)width;                       // Set Right Value To Requested Width
	WindowRect.top=(long)0;								// Set Top Value To 0
	WindowRect.bottom=(long)height;                     // Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;							// ����ȫ��ȫ����־

	hInstance = GetModuleHandle(NULL);                  // ȡ�����Ǵ��ڵ�ʵ��
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;      // �ƶ�ʱ�ػ�����Ϊ����ȡ��DC 
	wc.lpfnWndProc      = (WNDPROC) WndProc;            // WndProc������Ϣ
	wc.cbClsExtra       = 0;							// �޶��ⴰ������ 
	wc.cbWndExtra       = 0;							// �޶��ⴰ������ 
	wc.hInstance        = hInstance;                    // ����ʵ�� 
	wc.hIcon        = LoadIcon(NULL, IDI_WINLOGO);      // װ��ȱʡͼ��
	wc.hCursor      = LoadCursor(NULL, IDC_ARROW);      // װ�����ָ��
	wc.hbrBackground    = NULL;							// GL����Ҫ����
	wc.lpszMenuName     =  NULL;						// ����Ҫ�˵�
	wc.lpszClassName    = "Real Scene";                 // �趨������

	if (!RegisterClass(&wc))							// ����ע�ᴰ����
	{
		MessageBoxA(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Exit And Return FALSE
	}

	if (fullscreen)										// Ҫ����ȫ��ģʽ��?
	{
		DEVMODE dmScreenSettings;						// �豸ģʽ
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));       // ȷ���ڴ����
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		    // Devmode �ṹ�Ĵ�С
		dmScreenSettings.dmPelsWidth    = width;					// ��ѡ��Ļ���
		dmScreenSettings.dmPelsHeight   = height;					// ��ѡ��Ļ�߶�
		dmScreenSettings.dmBitsPerPel   = bits;						// ÿ������ѡ��ɫ�����
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// ����������ʾģʽ�����ؽ����ע: CDS_FULLSCREEN ��ȥ��״̬���� 
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// ��ģʽʧ�ܣ��ṩ����ѡ��˳����ڴ��������С� 
			if (MessageBoxA(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;               //����û�ѡ�񴰿�ģʽ������fullscreen ��ֵ��ΪFALSE,����������С�
			}
			else
			{
				//����û�ѡ���˳���������Ϣ���ڸ�֪�û����򽫽�����������FALSE���߳��򴰿�δ�ܳɹ������������˳���
				MessageBoxA(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;                   //�˳������� FALSE
			}
		}
	}

	if (fullscreen)                             // �Դ���ȫ��ģʽ��?
	{
		dwExStyle=WS_EX_APPWINDOW;              // ��չ������
		dwStyle=WS_POPUP;                       // ������
		ShowCursor(FALSE);                      // �������ָ��
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
#pragma endregion ����OpenGL����

#pragma region LRESULT CALLBACK WndProc
//���������еĴ�����Ϣ����ĵط���
//������ע���괰����֮������֪ͨ����ת���ⲿ�ִ�������������Ϣ
LRESULT CALLBACK WndProc(   HWND    hWnd,                   // Handle For This Window
						 UINT    uMsg,                   // Message For This Window
						 WPARAM  wParam,                 // Additional Message Information
						 LPARAM  lParam)                 // Additional Message Information
{
	//uMsg ����������������Ҫ�������Ϣ������
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
#pragma endregion �������еĴ�����Ϣ

#pragma region WinMain
//WindowsӦ�ó������ڡ�
//���������ǵ��ô��ڴ������򣬴�������Ϣ�������û�������
int WINAPI WinMain( HINSTANCE   hInstance,										//ʵ��
				   HINSTANCE   hPrevInstance,									// ǰһ��ʵ��
				   LPSTR       lpCmdLine,										// �����в���
				   int     nCmdShow)											// ������ʾ״̬
{
	MSG msg;																	// Windowsx��Ϣ�ṹ
	BOOL    done=FALSE;															// �����˳�ѭ����Bool ����

	fullscreen=FALSE;															//����ģʽ

																				// ����OpenGL����
	if (!CreateGLWindow("Real Scene",windowsWidth,windowsHeight,16,fullscreen))
	{
		return 0;																// ʧ���˳�
	}

	while(!done)																// ����ѭ��ֱ�� done=TRUE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))								//����Ϣ�ڵȴ���?
		{
			if (msg.message==WM_QUIT)											//�յ��˳���Ϣ? 
			{
				done=TRUE;                  

			}
			else    // ��������˳���Ϣ�����Ƿ�����Ϣ��Ȼ������Ϣ��ʹ��WndProc() �� Windows�ܹ���������
			{
				TranslateMessage(&msg);											// ������Ϣ
				DispatchMessage(&msg);              // Dispatch The Message
			}
		}
		else																	// ���û����Ϣ
		{
			// ���Ƴ���������ESC������DrawGLScene() �����������˳���Ϣ
			if (active)															// ���򼤻��ô? 
			{
				if (keys[VK_ESCAPE])											// ESC ������ô?
				{
					done=TRUE;              // ESC Signalled A Quit
				}
				else															// �����˳���ʱ��ˢ����Ļ 
				{
					DrawGLScene();												// ���Ƴ���
					SwapBuffers(hDC);											// �������� (˫����)
				}
			}

			if (keys[VK_F1])													
			{
				keys[VK_F1]=FALSE;												// ���ǣ�ʹ��Ӧ��Key�����е�ֵΪ FALSE 
				KillGLWindow();													// ���ٵ�ǰ�Ĵ���
				fullscreen=!fullscreen;											// �л� ȫ�� / ���� ģʽ 
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("OpenGL",800,800,16,fullscreen))
				{
					return 0;               // Quit If Window Was Not Created
				}
			}

		}

	}

	KillGLWindow();																// ���ٴ���
	return (msg.wParam);														// �˳�����
}
#pragma endregion Windows��������