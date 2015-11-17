#include "RAMIActual.h"

HGLRC           hRC=NULL;                           // Permanent Rendering Context
HDC             hDC=NULL;                           // Private GDI Device Context
HWND            hWnd=NULL;                          // Holds Our Window Handle
HINSTANCE       hInstance;                          // Holds The Instance Of The Application

bool    keys[256];                              // Array Used For The Keyboard Routine
bool    active=TRUE;                                // Window Active Flag Set To TRUE By Default
bool    fullscreen=TRUE;                            // Fullscreen Flag Set To Fullscreen Mode By Default

int     g_nFPS = 0, g_nFrames = 0;                  // <b style="color:black;background-color:#a0ffff">FPS</b> and <b style="color:black;background-color:#a0ffff">FPS</b> Counter
DWORD       g_dwLastFPS = 0; 

//旋转角度
GLfloat cosx;
GLfloat tanx;
GLfloat cosz;

//关闭opengl垂直同步机制
typedef void (APIENTRY *PFNWGLEXTSWAPCONTROLPROC) (int);
typedef int (*PFNWGLEXTGETSWAPINTERVALPROC) (void);
PFNWGLEXTSWAPCONTROLPROC wglSwapIntervalEXT = NULL;
PFNWGLEXTGETSWAPINTERVALPROC wglGetSwapIntervalEXT = NULL;


//无论何时窗口（假设使用窗口模式而不是全屏模式）的尺寸改变时重新设置OpenGL场景的尺寸
//该部分主要实现的功能是【将当前的可视空间设置为正投影空间】
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)             // Resize And Initialize The GL Window
{
	if (height==0)                              // Prevent A Divide By Zero By
	{
		height=1;                           // Making Height Equal One
	}

	glViewport(0, 0, width, height);                    // Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
	glLoadIdentity();  
	// Reset The Projection Matrix

	sideLength2 = sideLength / windowsWidth * windowsHeight;
	
	//gluPerspective(50.0f,(GLfloat)width/(GLfloat)height,0.1f,500.0f);
	//gluLookAt(-140,140,40,-80,80,0,1,-1,0);

	//【glOrtho函数可以将当前的可视空间设置为正投影空间】
	// 功能：负责把视景体截取的图像按照怎样的高和宽显示到屏幕上
	glOrtho( -sideLength/2+xOffset, sideLength/2+xOffset, -sideLength2/2+yOffset, sideLength2/2+yOffset, -200, 200 );
	//glOrtho( -sideLength/5, 0, -sideLength/5, 0, -30, 30 );
	//gluOrtho2D(-sideLength/2, sideLength/2, -sideLength/2/windowsWidth*windowsHeight, sideLength/2/windowsWidth*windowsHeight);

	glRotatef(xRotateAngle, -1.0f, 0.0f, 0.0f);//绕x轴顺时针方向旋转xRotateAngle角度
	//glRotatef(xRotateAngle, xOffset, yOffset, 0.0f);
	glRotatef(zRotateAngle, 0.0f, 0.0f, 1.0f);//绕z轴逆时针方向旋转zRotateAngle角度
	
	glMatrixMode(GL_MODELVIEW);                     // Select The Modelview Matrix 选择对模型视景的操作，切换到视图矩阵来正确的画图
	glLoadIdentity();                           // Reset The Modelview Matrix 把矩阵堆栈中的在栈顶的那个矩阵置为单位矩阵，好让之前的任何变换都不影响后面的变化
} 



//在这里做所有有关OpenGL的设置
//在这里设置光照，纹理和其他需要设置的所有的东西
int InitGL(GLvoid)                              // All Setup For OpenGL Goes Here
{
	obj = gluNewQuadric();//创建2次曲面对象，用于绘制二维曲面，如圆柱体、圆锥、球体、碟盘、部分同心圆等

	//判断当前状态是否为垂直同步，如果是则关闭垂直同步
	bool isOk = InitVSync(); 
	if (isOk) {
		SetVSyncState(false);
	}

	cosx = cos(xRotateAngle/180*M_PI);
	cosz = cos(zRotateAngle/180*M_PI);
	tanx = tan(xRotateAngle/180*M_PI);

	//listNums = BuildLists(fileDir, objNum, groupNum);
	//const GLubyte* name = glGetString(GL_VENDOR); //返回负责当前OpenGL实现厂商的名字
	//const GLubyte* biaoshifu = glGetString(GL_RENDERER); //返回一个渲染器标识符，通常是个硬件平台
	//const GLubyte* OpenGLVersion =glGetString(GL_VERSION); //返回当前OpenGL实现的版本号
	//const GLubyte* gluVersion= gluGetString(GLU_VERSION); //返回当前GLU工具库版本

#ifdef SAVE_AT_ONCE
	keys[VK_SPACE] = TRUE;
#endif
	//keys['B'] = TRUE;

	//生成针叶林DisplayList 2012.3.6关闭 rhhu
	//listNums = BuildListsHET08("d:\\Study\\Data\\LAI\\RAMI\\HET08_OPS_WIN", 1, 14);

	//柱状、球状分布场景
	_makepath_s(abObjFPath,NULL, abFileDir, abObjFName,NULL);
	_makepath_s(abLocationFPath,NULL, abFileDir, abLocationFName, NULL);
	listNums = BuildlistsDiskScene(abObjFPath, abLocationFPath);

	//listNums = BuildLists("d:\\Study\\Data\\LAI\\RAMI\\HET16_SRF",1,6);	//HET16_SRF

	//ReadLocation(abLocationFPath);
	//ReadRAMI(filePath);
	//BuildlistsDisk(filePath);
	glShadeModel(GL_SMOOTH);                        // Enables Smooth Shading

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);                   // Black Background

	glClearDepth(1.0f);                         // Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Test To Do

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);          // Really Nice Perspective Calculations

	return TRUE;                                // Initialization Went OK
}



//所有的绘图代码部分
//在glLoadIdentity()函数和return TURE语句之间加入一些代码来绘制一些基本的图形
int DrawGLScene(GLvoid)                             // Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         // Clear The Screen And The Depth Buffer
	glLoadIdentity();                           // Reset The Current Modelview Matrix


	// Get FPS
	if( GetTickCount() - g_dwLastFPS >= 1000 )					// When A Second Has Passed...
	{
		g_dwLastFPS = GetTickCount();							// Update Our Time Variable
		g_nFPS = g_nFrames;										// Save The FPS
		g_nFrames = 0;											// Reset The FPS Counter

		char szTitle[256]={0};									// Build The Title String

		sprintf( szTitle, "Real Scene , %d FPS %d/%d",  g_nFPS , curblock, blockNum);
		//if( g_fVBOSupported )									// Include A Notice About VBOs
		//	strcat( szTitle, ", Using VBOs" );
		//else
		//	strcat( szTitle, ", Not Using VBOs" );
		SetWindowText( hWnd, szTitle );				// Set The Title
	}
	g_nFrames++;


	//glBegin(GL_QUADS);
	//glColor3ub(255,255,0);

	//glVertex3f(-50,0,10);
	//glVertex3f( 50,0,10);
	//glVertex3f( 50,0,0);
	//glVertex3f(-50,0,0);

	//glEnd();
	//glCallList(listNums);
	glCallList(listNums);
	/*glColor3ub(0,255,0);
	glCallList(2);
	glColor3ub(255,255,255);
	glCallList(8);*/
	//int i = 12;
	//glTranslatef(1,0,0);
	//glCallList(2);
	//glCallList(2+14);
	
	return TRUE;                        // Keep Going
}



//在程序退出之前调用的。
//KillGLWindow()函数的作用是释放渲染上下文，设备上下文，最后释放窗口句柄
GLvoid KillGLWindow(GLvoid)                         // Properly Kill The Window
{
	//gluDeleteQuadric(obj);
	if (fullscreen)                             // Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);                  // If So Switch Back To The Desktop
		ShowCursor(TRUE);                       // Show Mouse Pointer
	}

	if (hRC)                                // Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))                 // Are We Able To Release The DC And RC Contexts?
		{
			MessageBoxA(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;                           // Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))                    // Are We Able To Release The DC
	{
		MessageBoxA(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;                           // Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))                   // Are We Able To Destroy The Window?
	{
		MessageBoxA(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;                          // Set hWnd To NULL
	}

	if (!UnregisterClass("Real Scene",hInstance))               // Are We Able To Unregister Class
	{
		MessageBoxA(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;                         // Set hInstance To NULL
	}
}



//创建窗口
//形式固定，此部分不需要修改
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint      PixelFormat;                        // Holds The Results After Searching For A Match

	WNDCLASS    wc;                         // Windows Class Structure

	DWORD       dwExStyle;                      // Window Extended Style
	DWORD       dwStyle;                        // Window Style

	RECT WindowRect;                            // Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;                        // Set Left Value To 0
	WindowRect.right=(long)width;                       // Set Right Value To Requested Width
	WindowRect.top=(long)0;                         // Set Top Value To 0
	WindowRect.bottom=(long)height;                     // Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;                      // Set The Global Fullscreen Flag

	hInstance       = GetModuleHandle(NULL);            // Grab An Instance For Our Window
	wc.style        = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;       // Redraw On Move, And Own DC For Window
	wc.lpfnWndProc      = (WNDPROC) WndProc;                // WndProc Handles Messages
	wc.cbClsExtra       = 0;                        // No Extra Window Data
	wc.cbWndExtra       = 0;                        // No Extra Window Data
	wc.hInstance        = hInstance;                    // Set The Instance
	wc.hIcon        = LoadIcon(NULL, IDI_WINLOGO);          // Load The Default Icon
	wc.hCursor      = LoadCursor(NULL, IDC_ARROW);          // Load The Arrow Pointer
	wc.hbrBackground    = NULL;                     // No Background Required For GL
	wc.lpszMenuName     =  NULL; MAKEINTRESOURCE(IDR_MENU);                     // We Don't Want A Menu
	wc.lpszClassName    = "Real Scene";                 // Set The Class Name

	if (!RegisterClass(&wc))                        // Attempt To Register The Window Class
	{
		MessageBoxA(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;                           // Exit And Return FALSE
	}

	if (fullscreen)                             // Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;                   // Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));       // Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);       // Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth    = width;            // Selected Screen Width
		dmScreenSettings.dmPelsHeight   = height;           // Selected Screen Height
		dmScreenSettings.dmBitsPerPel   = bits;             // Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Run In A Window.
			if (MessageBoxA(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;               // Select Windowed Mode (Fullscreen=FALSE)
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBoxA(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;                   // Exit And Return FALSE
			}
		}
	}

	if (fullscreen)                             // Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;                  // Window Extended Style
		dwStyle=WS_POPUP;                       // Windows Style
		ShowCursor(FALSE);                      // Hide Mouse Pointer
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



//Windows应用程序的入口。
//在这里我们调用窗口创建程序，处理窗口消息，监听用户交互。
int WINAPI WinMain( HINSTANCE   hInstance,              // Instance
				   HINSTANCE   hPrevInstance,              // Previous Instance
				   LPSTR       lpCmdLine,              // Command Line Parameters
				   int     nCmdShow)               // Window Show State
{
	MSG msg;                                // Windows Message Structure
	BOOL    done=FALSE;                         // Bool Variable To Exit Loop

	//Ask The User Which Screen Mode They Prefer
	//if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	//{
	fullscreen=FALSE;                       // Windowed Mode
	//}
		
	//
	SHCreateDirectoryEx(NULL, saveIMGDir, NULL);

	// Create Our OpenGL Win[\ow
	if (!CreateGLWindow("Real Scene",windowsWidth,windowsHeight,16,fullscreen))
	{
		return 0;                           // Quit If Window Was Not Created
	}

	while(!done)                                // Loop That Runs Until done=TRUE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))           // Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)               // Have We Received A Quit Message?
			{
				done=TRUE;                  // If So done=TRUE
				
			}
			else                            // If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);             // Translate The Message
				DispatchMessage(&msg);              // Dispatch The Message
			}
		}
		else                                // If There Are No Messages
		{
			// 绘制场景。监听ESC按键和DrawGLScene() 函数发出的退出消息
			if (active)                     // Program Active?
			{
				if (keys[VK_ESCAPE])                // Was ESC Pressed?
				{
					done=TRUE;              // ESC Signalled A Quit
				}
				else                        // Not Time To Quit, Update Screen
				{
					DrawGLScene();              // Draw The Scene
					SwapBuffers(hDC);           // Swap Buffers (Double Buffering)
				}
			}


			// 通过按键来完成相关操作
			if (keys[VK_SHIFT] && keys['M'])
			{
				keys[VK_SHIFT]=FALSE;
				keys['M']=FALSE;

				if (SaveMultiAngle())
				{
					MessageBoxA(NULL,"All Succeed.","FreeImage_Save",MB_OK);
				} 
				else
				{
					MessageBoxA(NULL,saveImgPath,"FreeImage_Save error",MB_OK);
				}


			}
			if (keys[VK_SPACE])
			{
				keys[VK_SPACE] = FALSE;
				long startTime = clock();
				//SaveWholeScene(xMin, xMax, yMin, yMax);
				SaveSceneMultiAngle();
				long endTime = clock();
				long result = (endTime-startTime)/1000;
				char tmp[10];
				sprintf_s(tmp,"%ldmin%lds",result/60, result%60);
				MessageBoxA(NULL,tmp,"SaveWhole Finished",NULL);
			}

			//批处理保存图片
			if (keys[VK_SHIFT] && keys['B'])
			{
				keys[VK_SHIFT]=FALSE;
				keys['B']=FALSE;
				
				FILE* fBanch = NULL;
				if (fopen_s(&fBanch,banchFile,"r") != 0)
				{
					MessageBoxA(NULL,"File Open Failed!","ERROR",MB_OK|MB_ICONEXCLAMATION);
					return -1;
				}

				char * tmp = new char[100];
				//char tmpName[_MAX_FNAME];
				
				while (!feof(fBanch))
				{
					fscanf_s(fBanch,"%s\n",&filePath,_MAX_FNAME);
					FreeMemory();
					ReadRAMI(filePath);
					if (!SaveMultiAngle())
					{
						MessageBoxA(NULL,saveImgPath,"FreeImage_Save error",MB_OK);
						return -1;
					}
				}

				MessageBoxA(NULL,"All Succeed.","FreeImage_Save",MB_OK);

				fclose(fBanch);
				fBanch =0;

			}


			if (keys['B'])
			{
				keys['B']=FALSE;
				
				if (SaveIMG())
				{
					MessageBoxA(NULL,saveImgPath,"FreeImage_Save Succeed.",MB_OK);
				}
			}

			if (keys['D'])                    
			{
				keys['D']=FALSE;             
				xOffset += sideLength/4;	
				ReSizeGLScene(windowsWidth,windowsHeight);
				SwapBuffers(hDC);
			}

			if (keys['A'])                    
			{
				keys['A']=FALSE;             
				xOffset -= sideLength/4;	
				ReSizeGLScene(windowsWidth,windowsHeight);
			}

			if (keys['W'])                    
			{
				keys['W']=FALSE;             
				yOffset += sideLength2/4;	
				ReSizeGLScene(windowsWidth,windowsHeight);

			}

			if (keys['S'])                    
			{
				keys['S']=FALSE;             
				yOffset -= sideLength2/4;	
				ReSizeGLScene(windowsWidth,windowsHeight);

			}

			if (keys[VK_PRIOR])                    
			{
				keys[VK_PRIOR]=FALSE;             
				sideLength*=1.2;	
				ReSizeGLScene(windowsWidth,windowsHeight);

			}

			if (keys[VK_NEXT])                    
			{
				keys[VK_NEXT]=FALSE;             
				sideLength/=1.2;	
				ReSizeGLScene(windowsWidth,windowsHeight);

			}

			if (keys[VK_UP])                    
			{
				keys[VK_UP]=FALSE;             
				xRotateAngle += 5;
				ReSizeGLScene(windowsWidth,windowsHeight);	
					
			}

			if (keys[VK_DOWN])                    
			{
				keys[VK_DOWN]=FALSE;             
				xRotateAngle -= 5;
				ReSizeGLScene(windowsWidth,windowsHeight);	

			}

			if (keys[VK_LEFT])                    
			{
				keys[VK_LEFT]=FALSE;             
				zRotateAngle += 5;
				ReSizeGLScene(windowsWidth,windowsHeight);		

			}

			if (keys[VK_RIGHT])                    
			{
				keys[VK_RIGHT]=FALSE;             
				zRotateAngle -= 5;
				ReSizeGLScene(windowsWidth,windowsHeight);		

			}

			if (keys[VK_F1])                    // Is F1 Being Pressed?
			{
				keys[VK_F1]=FALSE;              // If So Make Key FALSE
				KillGLWindow();                 // Kill Our Current Window
				fullscreen=!fullscreen;             // Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("OpenGL",800,800,16,fullscreen))
				{
					return 0;               // Quit If Window Was Not Created
				}
			}

			if (keys['V'])
			{
				keys['V']=FALSE;
				switch (SaveVector())
				{
				case 0:
					break;
				case 3:
					MessageBoxA(NULL,"gl2psEndPage Failed.\nGL2PS_ERROR","ERROR",MB_OK|MB_ICONEXCLAMATION);
					break;
				case 4:
					MessageBoxA(NULL,"gl2psEndPage Failed.\nGL2PS_NO_FEEDBACK","ERROR",MB_OK|MB_ICONEXCLAMATION);
					break;
				case 5:
					MessageBoxA(NULL,"gl2psEndPage Failed.\nGL2PS_OVERFLOW","ERROR",MB_OK|MB_ICONEXCLAMATION);
					break;
				case 6:
					MessageBoxA(NULL,"gl2psEndPage Failed.\nGL2PS_UNINITIALIZED","ERROR",MB_OK|MB_ICONEXCLAMATION);
					break;		
				}

			}			

		}

	}
	// Shutdown
	if (listNums !=0)
	{
		glDeleteLists(1,listNums);
	}
	gluDeleteQuadric(obj);
	
	KillGLWindow();                             // Kill The Window
	return (msg.wParam);                            // Exit The Program
}




int SaveIMG() 
{
	char fname[_MAX_FNAME];  
	char saveImgName[_MAX_FNAME];
	_splitpath(filePath,NULL, NULL, fname, NULL);
	sprintf_s(saveImgName, 256,"%s_%02.0f_%02.0lf",fname, sideLength, xRotateAngle);
	_makepath_s(saveImgPath, _MAX_PATH,NULL,saveIMGDir, saveImgName,"bmp");

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int width = viewport[2];
	int height = viewport[3];
	
	GLubyte* pPixelData;
	pPixelData = new GLubyte[ width*height*3 ];
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	

	//读取一些像素。当前可以简单理解为“把已经绘制好的像素（它可能已经被保存到显卡的显存中）读取到内存
	//glReadPixels()前四个参数可以得到一个矩形，第1、2个参数表示矩形左下角的横纵坐标，3、4个参数表示矩形的宽度和高度
	//第5个参数表示读取的内容，第6个参数表示读取的内容保存到内存时的使用模式
	//第7个参数表示一个指针，像素被读取后将保存到这个指针所表示的地址
	glReadPixels(viewport[0],viewport[1],viewport[2],viewport[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, pPixelData);

	//glReadPixels(viewport[0],viewport[1],viewport[2],viewport[3], GL_STENCIL_INDEX, GL_BITMAP, pPixelData);
	
	/******保存二值图
	int pixnum = 0;
	for (int i =0; i< width*height; i++)
	{
		if (pPixelData[i] != 0)
		{
			pixnum = i;
			break;
		}
	}

	int offset = pixnum/width + 1;
	if (abs(xRotateAngle) > 65)
	{
		offset += 250;
	}
	height = height - offset*2;
	GLubyte* bits = pPixelData + offset*width;//*sizeof(GL_UNSIGNED_BYTE);
	
	//SaveBMP((BYTE*)bits,width, height, saveImgPath,1);
	//delete [] pPixelData;
	********************/

	//SaveBMP((BYTE*)pPixelData,width, height, saveImgPath,8);
	//glReadPixels(viewport[0],viewport[1],viewport[2],viewport[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, pPixelData);
	//SaveBMP((BYTE*)pPixelData,width, height, saveImgPath,24);
	
		
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	bool bSuccess = FALSE;

	unsigned bpp = 24;
	int pitch = ((((bpp * width) + 31) / 32) * 4);
	
	//**********************************************************************
	//【保存图片】
	FIBITMAP* dib = FreeImage_ConvertFromRawBits(pPixelData, width, height, pitch, bpp, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK  );//更加快速的保存图形
	delete [] pPixelData;
	pPixelData = 0;
	BOOL stateFI = FreeImage_Save(FIF_BMP,dib,saveImgPath);
	FreeImage_Unload( dib ) ;
	return stateFI;
	//**********************************************************************
	//return 1;
}



int SaveWholeScene( GLdouble _xMin, GLdouble _xMax, GLdouble _yMin, GLdouble _yMax )
{
	
	GLint viewport[4];
 	//glGetIntegerv(GL_VIEWPORT, viewport);
	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = windowsWidth;
	viewport[3] = windowsHeight;


   	GLint winWidth = viewport[2];
	GLint winHeight = viewport[3];
	//

	
	double yFraction = 1;
	int xNum = int( (_xMax-_xMin) / sideLength );
	int yNum = int( (_yMax-_yMin) / sideLength2 * yFraction);
	if (xNum == 0) xNum = 1;
	if (yNum == 0) yNum = 1;

	blockNum = xNum * yNum;
	curblock = 0;

	GLint wholeWidth = winWidth * xNum;
	GLint wholeHeight = winHeight * yNum;

	
   	xOffset = _xMin + sideLength/2;
	yOffset = (_yMin + _yMax)/2 -sideLength2 * (yNum-1) / 2;

	GLubyte* pPixelData = new GLubyte[ winWidth*winHeight ];
	if (NULL == pPixelData)
	{
		MessageBoxA(NULL, "Memory is not enough : pPixelData","error",MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	GLubyte* pWholeScene =new GLubyte[ wholeWidth*wholeHeight ];
	if (NULL == pWholeScene)
	{
		MessageBoxA(NULL, "Memory is not enough : pWholeScene","error",MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);

	
	
	//GLubyte* pPixelData2 = new GLubyte[ winWidth*winHeight ];
	double xOffset0 = xMin + sideLength/2;
	try
	{
		for (int y = 0; y < yNum; y++)
		{	
			xOffset = xOffset0;

			for (int x = 0; x < xNum; x++)
			{
				
				
				//long startTime = clock();

				//直接对整个场景创建DisplayList，很慢，不建议使用。已改为分块编译。
				//glDeleteLists(1, listNums);
				//listNums = BuildListsHET08("d:\\Study\\Data\\LAI\\RAMI\\HET08_OPS_WIN", 1, 14);


				//每次只对窗口中显示的部分创建DisplayList
				//HET08
				//glDeleteLists(30, 44);
				//listNums = BuildListsHET08("d:\\Study\\Data\\LAI\\RAMI\\HET08_OPS_WIN", 1, 14, 1);

				//HET04
				glDeleteLists(2, 1);
				listNums = BuildlistsDiskScene(abObjFPath, abLocationFPath, 1);


				////HET16
				//glDeleteLists(14, 7);
				//listNums = BuildLists("d:\\Study\\Data\\LAI\\RAMI\\HET16_SRF",1,6,1);


				ReSizeGLScene(winWidth,winHeight);
				DrawGLScene(); 
				//SwapBuffers(hDC);

				glReadPixels(viewport[0],viewport[1],viewport[2],viewport[3], GL_GREEN, GL_UNSIGNED_BYTE, pPixelData);

				//long endTime = clock();			
				//long result = (endTime-startTime)/1000;


				for (int h = 0; h < winHeight; h++)
				{
					CopyMemory( pWholeScene + ((y * winHeight + h) * wholeWidth  + x*winWidth),pPixelData+h*winWidth,winWidth);
					//CopyMemory( pWholeScene + ( h * wholeWidth  + x*winWidth),pPixelData+h*winWidth,winWidth);
				}		
				
				xOffset += sideLength;
				curblock ++;
			}
			
			yOffset += sideLength2;	
		}
	}
	catch (exception* e)
	{
		MessageBoxA(NULL, ERROR, e->what(), MB_OK);
	}
	
	/*int temp = sizeof(GL_UNSIGNED_BYTE);
	int temp2 = sizeof(GLubyte);
	int temp3 = sizeof(GL_BYTE);
	int temp4= sizeof(GL_BITMAP);*/
	
	delete [] pPixelData;
	pPixelData = 0;

#pragma region 剔除边缘空白部分
	int pixnum = 0;
	for (int i =0; i< wholeHeight*wholeWidth; i++)
	{
		if (pWholeScene[i] != 0)
		{
			pixnum = i;
			break;
		}

	}

	int offset = pixnum/wholeWidth;
	if (abs(xRotateAngle) > 45)
	{
		offset += 400;
	}
	wholeHeight -= offset*2;
	GLubyte* bits = pWholeScene + offset*wholeWidth;//*sizeof(GL_UNSIGNED_BYTE);
	//GLubyte* bits = pWholeScene;//*sizeof(GL_UNSIGNED_BYTE);
#pragma endregion 剔除边缘空白部分

	char saveImgName[_MAX_FNAME];
	sprintf_s(saveImgName, _MAX_FNAME,"%s_%02.0f_%02.0f",prefix, sideLength, xRotateAngle );
   	_makepath_s(saveImgPath, _MAX_PATH,NULL,saveIMGDir, saveImgName,"bmp");
	SaveBMP(bits, wholeWidth,wholeHeight, saveImgPath,1);
	bits = 0;
	delete [] pWholeScene;
	pWholeScene = 0;
	
	return 1;
}

int SaveVector() 
{
	FILE *fp;
	int state = GL2PS_OVERFLOW, buffsize = 0;

	char fname[_MAX_FNAME];  
	_splitpath(filePath,NULL, NULL, fname, NULL);
	sprintf_s(saveVecPath, 256,"d:\\Study\\Temp\\LAI\\BMP\\%s_%02.0f_%02.0lf.eps",fname, sideLength, xRotateAngle);

	fp = fopen(saveVecPath, "wb");
	buffsize += 10240*10240;
	state = gl2psBeginPage("OpenGL", "OpenGL", NULL, GL2PS_EPS, GL2PS_NO_SORT,
		//GL2PS_BEST_ROOT | 
		//GL2PS_DRAW_BACKGROUND | 
		GL2PS_USE_CURRENT_VIEWPORT,

		GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, saveVecPath);

	if (state == 3) //state=GL2PS_ERROR
	{
		MessageBoxA(NULL,"gl2psBeginPage Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return GL2PS_ERROR;
	}

	DrawGLScene();
	state = gl2psEndPage();

	fclose(fp);
	fp=0;
	return state;

}

int SaveMultiAngle() 
{
	xRotateAngle = 0.0f;
	int iState = 1;
	while(xRotateAngle < 90 && iState)
	{				
		ReSizeGLScene(windowsWidth,windowsHeight);	
		DrawGLScene(); 
		//SwapBuffers(hDC);     //实时显示
		iState = SaveIMG();				
		xRotateAngle += rInterval;
	}

	xRotateAngle =57.3f;
	ReSizeGLScene(windowsWidth,windowsHeight);	
	DrawGLScene(); 
	//SwapBuffers(hDC);     //实时显示
	iState = SaveIMG();	

	return iState;

}

int SaveSceneMultiAngle() 
{
	xRotateAngle = 0.0f;
	int iState = 1;

	GLfloat zRad = zRotateAngle/180*M_PI;
	GLfloat cos1 = cos(zRad);
	GLfloat sin1 = sin(zRad);

	GLdouble xMin2 = xMin*abs(cos(zRad))+ yMin*abs(sin(zRad));
	GLdouble xMax2 = xMax*abs(cos(zRad))+ yMax*abs(sin(zRad));
	GLdouble yMin2 = yMin*abs(cos(zRad))+ xMin*abs(sin(zRad));
	GLdouble yMax2 = yMax*abs(cos(zRad))+ xMax*abs(sin(zRad));


	while(xRotateAngle < 90 && iState)
	{				
		//SwapBuffers(hDC);     //实时显示
		cosx = cos(xRotateAngle/180*M_PI);
		cosz = cos(zRotateAngle/180*M_PI);
		tanx = tan(xRotateAngle/180*M_PI);
		iState = SaveWholeScene(xMin2, xMax2, yMin2, yMax2);;				
		xRotateAngle += rInterval;
		
	}

	xRotateAngle =57.3f;
	cosx = cos(xRotateAngle/180*M_PI);
	tanx = tan(xRotateAngle/180*M_PI);
	//SwapBuffers(hDC);     //实时显示
	iState = SaveWholeScene(xMin2, xMax2, yMin2, yMax2);	

	return iState;

}

bool InitVSync()
{
	char* extensions = (char*)glGetString(GL_EXTENSIONS);
	if (strstr(extensions,"WGL_EXT_swap_control")) {
		wglSwapIntervalEXT = (PFNWGLEXTSWAPCONTROLPROC)wglGetProcAddress("wglSwapIntervalEXT");
		wglGetSwapIntervalEXT = (PFNWGLEXTGETSWAPINTERVALPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		return true;
	}
	return false;
}

// 判断当前状态是否为垂直同步
bool IsVSyncEnabled()
{
	return (wglGetSwapIntervalEXT() > 0);
}

// 开启和关闭垂直同步
void SetVSyncState(bool enable)
{
	if (enable)
		wglSwapIntervalEXT(1);
	else 
		wglSwapIntervalEXT(0);
}


void DrawCylinder(double baseRadius, double topRadius, int _slices, int _stacks,
				  float x0, float y0, float z0, float x1, float y1, float z1)
{
	float  dir_x = x1 - x0;  
	float  dir_y = y1 - y0;  
	float  dir_z = z1 - z0;  
	float  Cylen = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );  

	glPushMatrix();  
	// 平移到起始点  
	glTranslatef( x0, y0, z0 );  
	// 计算长度  
	float length;  
	length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );  
	if ( length < 0.0001 ) {   
		dir_x = 0.0; dir_y = 0.0; dir_z = 1.0;  length = 1.0;  
	}  
	dir_x /= length;  dir_y /= length;  dir_z /= length;  
	float  up_x, up_y, up_z;  
	up_x = 0.0;  
	up_y = 1.0;  
	up_z = 0.0;  
	float  side_x, side_y, side_z;  
	side_x = up_y * dir_z - up_z * dir_y;  
	side_y = up_z * dir_x - up_x * dir_z;  
	side_z = up_x * dir_y - up_y * dir_x;  
	length = sqrt( side_x*side_x + side_y*side_y + side_z*side_z );  
	if ( length < 0.0001 ) {  
		side_x = 1.0; side_y = 0.0; side_z = 0.0;  length = 1.0;  
	}  
	side_x /= length;  side_y /= length;  side_z /= length;  

	up_x = dir_y * side_z - dir_z * side_y;  
	up_y = dir_z * side_x - dir_x * side_z;  
	up_z = dir_x * side_y - dir_y * side_x;  
	// 计算变换矩阵  
	double  m[16] = { side_x, side_y, side_z, 0.0,  
		up_x,   up_y,   up_z,   0.0,  
		dir_x,  dir_y,  dir_z,  0.0,  
		0.0,    0.0,    0.0,    1.0 };  
	glMultMatrixd( m );  
	// 圆柱体参数   
	gluCylinder( obj, baseRadius, topRadius, Cylen, _slices, _stacks );   
	glPopMatrix();
}

int ReadRAMI( char* filepath) 
{
	FILE* fp;//文件指针


	if((fp = fopen(filepath,"r"))!=NULL)//判断文件存在否
	{
		char * tmp = new char[100];
		lines = 0;
		while (!feof(fp) && fgets(tmp,100,fp))
		{
			lines ++;
		}
		fileRAMI = new float*[lines];
		for (int i = 0; i < lines; i++)
		{
			fileRAMI[i] = new float[7];
		}
		rewind(fp);


		for (int i = 0; i<lines; i++)
		{
			fscanf_s(fp,"%f %f %f %f %f %f %f\n", &fileRAMI[i][0], &fileRAMI[i][1], &fileRAMI[i][2], &fileRAMI[i][3], &fileRAMI[i][4], &fileRAMI[i][5], &fileRAMI[i][6]);
		}
		return 1;
		fclose(fp);
		fp = 0;
	}
	else
	{
		MessageBoxA(NULL,"File Open Failed!","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}
}

int ReadLocation( char* filepath) 
{
	FILE* fp;//文件指针


	if((fp = fopen(filepath,"r"))!=NULL)//判断文件存在否
	{
		char * tmp = new char[100];
		lines2 = 0;
		while (!feof(fp) && fgets(tmp,100,fp))
		{
			lines2 ++;
		}
		fileLocation = new float*[lines2];
		for (int i = 0; i < lines2; i++)
		{
			fileLocation[i] = new float[3];
		}
		rewind(fp);


		for (int i = 0; i<lines2; i++)
		{
			fscanf_s(fp,"%f %f %f\n", &fileLocation[i][0], &fileLocation[i][1], &fileLocation[i][2]);
		}
		return 1;
		fclose(fp);
		fp = 0;
	}
	else
	{
		MessageBoxA(NULL,"File Open Failed!","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}
}

void FreeMemory() 
{
	for (int i = 0; i < lines; i++)
	{
		delete [] fileRAMI[i];
	}
	delete [] fileRAMI;	
}


//执行代码
int Execute(int index, char* _filePath, int(*func)(char*,int), int listIndex) 
{
	
	glNewList(index, GL_COMPILE);

	//glPushMatrix();
	FILE* fp =  NULL;
	errno_t err;
	if (err = fopen_s(&fp,_filePath,"r"))
	{
		return err;
	}
	
	int lineMaxSize = 200;
//	char  objName[20];
	char * codes = new char[lineMaxSize];
	int comments = 0;

	while (!feof(fp)&&fgets(codes,lineMaxSize,fp) )
	{		
		
		if (strstr(codes,"/*"))
		{
			comments += 1;
		} 
		if (strstr(codes,"*/"))
		{
			comments -= 1;
			continue;
		}
		if (comments == 0)
		{		
			//if(strstr(codes,"end")==codes)
			//{	
			//	break;
			//}
			/*else if (strstr(codes,"name")==codes)
			{
				sscanf_s(codes,"%*s %s %*s\n", objName,20);
			}*/
			if( codes[0] != '\n')
			{
				(*func)(codes,listIndex);
			}
		}
	}
	//glPopMatrix();
	glEndList();
	
	fclose(fp);
	fp = 0;
	delete []codes;
	//gluDeleteQuadric(obj);
	return 0;
}


int ExecuteTri(int index, char* _filePath) 
{	
	FILE* fp =  NULL;
	errno_t err;
	if (err = fopen_s(&fp,_filePath,"r"))
	{
		return err;
	}
	
	int lineMaxSize = 200;
	char * codes = new char[lineMaxSize];

	int triLines = 0;

	int startlines = 0;
	while (!feof(fp))
	{		
		startlines ++;
		fgets(codes, lineMaxSize, fp);
		if (strstr(codes,"End of Header"))
		{
			break;
		}
	}

	while (!feof(fp) && fgets(codes, 100, fp))
	{
		if (codes[0] == 't')
		{
			triLines ++;
		} 
		else
		{
			startlines++;
		}	
	}

	rewind(fp);



	while (startlines > 0)
	{
		fgets(codes, 100, fp);
		startlines --;
	}

	int dataNums = triLines * 9;
	float* triData = new float[dataNums];
	
	for (int i = 0,j=9; i< dataNums; i+=9)
	{
		fgets(codes, lineMaxSize, fp);
		sscanf_s(codes,"%*s %f %f %f %f %f %f %f %f %f\n" ,\
			&triData[i], &triData[i + 1] ,&triData[i + 2] ,&triData[i + 3] ,&triData[i + 4] ,&triData[i + 5] ,&triData[i + 6] ,&triData[i + 7] ,&triData[i + 8]);
		
	}



	glEnableClientState(GL_VERTEX_ARRAY);
	
	glNewList(index, GL_COMPILE);

	glPushMatrix();
	glColor3ub(255,255,255);
	glVertexPointer(3, GL_FLOAT, 0, triData);
	glDrawArrays(GL_TRIANGLES, 0, triLines * 3);
	
	glPopMatrix();
	glEndList();

	glDisableClientState(GL_VERTEX_ARRAY);
	
	fclose(fp);
	fp = 0;
	delete [] codes;
	delete [] triData;
	//gluDeleteQuadric(obj);
	return 0;
}

//画叶片函数
int drawLeaf(char *commander, int listIndex)
{
	if (commander == strstr(commander,"triangle"))
	{
		float *dVertex = new float[9];
		sscanf_s(commander,"%*s %f %f %f %f %f %f %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3] ,&dVertex[4] ,&dVertex[5] ,&dVertex[6] ,&dVertex[7] ,&dVertex[8]);

		glBegin(GL_TRIANGLES);
		glColor3ub(0,255,0);
		//glColor3f(0,1.0f,0);
		glVertex3f(dVertex[0],dVertex[1],dVertex[2]);
		glVertex3f(dVertex[3],dVertex[4],dVertex[5]);
		glVertex3f(dVertex[6],dVertex[7],dVertex[8]);
		glEnd();
		delete [] dVertex;
		return GL_TRIANGLES;
		
	}
}

int drawFoliage(char *commander,int listIndex)
{
	if (commander == strstr(commander,"object"))
	{
		float *dVertex = new float[12];
		sscanf_s(commander,"%*s %*s %*s %f %f %f %f %f %f %f %f %f %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3] ,&dVertex[4] ,&dVertex[5] ,&dVertex[6] ,&dVertex[7] ,&dVertex[8] ,&dVertex[9] ,&dVertex[10] ,&dVertex[11]);

		glPushMatrix();  
		float  m[16] = { dVertex[0], dVertex[1], dVertex[2], 0,
			dVertex[3], dVertex[4], dVertex[5], 0,
			dVertex[6], dVertex[7], dVertex[8], 0,
			dVertex[9], dVertex[10], dVertex[11], 1 };
		glMultMatrixf(m);
		glCallList(listIndex);
		glPopMatrix();
		delete [] dVertex;
		return 0;
		
	}
}

int drawShoot_PIMO(char *commander, int listIndex)
{
	
	if (commander == strstr(commander,"end"))
	{
		glPopMatrix();
		
	}
	else if (commander == strstr(commander,"name nl_28_na_30_nd_50 list"))
	{
		//glColor3ub(0,255,0);
		glPushMatrix();  
		glRotatef( 172.37f, 0, 0 ,1 );
		glTranslatef(0, 0, 0.07f);
	}
	else if (commander == strstr(commander,"name nl_45_na_40_nd_70 list"))
	{
		glPushMatrix(); 
		glRotatef( 67.82f, 0, 0 ,1 );
		glTranslatef(0, 0, 0.05f);
	}
	else if (commander == strstr(commander,"name nl_28_na_40_nd_50 list"))
	{
		glPushMatrix(); 
		glRotatef( 109.39f, 0, 0 ,1 );
		glTranslatef(0, 0, 0.03f);
	}
	else if (commander == strstr(commander,"name nl_45_na_30_nd_70 list"))
	{
		glPushMatrix(); 
		glRotatef( 24.05f, 0, 0 ,1 );
	}
	else if (commander == strstr(commander,"name PIMOshoot0 list"))
	{
		//glPopMatrix();
		glPushMatrix();
		DrawCylinder(0.0025f, 0.0025f, 12, 1, 0, 0, 0.02f, 0, 0, 0.03f );
		
	}
	else if (commander == strstr(commander,"sphere PIMOfoliage"))
	{
		float *dVertex = new float[18];
		sscanf_s(commander,"%*s %*s %f %f %f %f %*s %f %f %f %*s %f %f %f %f %*s %f %f %f %f %*s %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3], 
			&dVertex[4] ,&dVertex[5] ,&dVertex[6], 
			&dVertex[7], &dVertex[8] ,&dVertex[9] ,&dVertex[10], 
			&dVertex[11] ,&dVertex[12] ,&dVertex[13] ,&dVertex[14], 
		&dVertex[15] ,&dVertex[16] ,&dVertex[17] );

		glPushMatrix();  	
			glTranslatef(dVertex[15] ,dVertex[16] ,dVertex[17]);
			glRotatef( dVertex[14], dVertex[11] ,dVertex[12] ,dVertex[13] );
			glRotatef( dVertex[10], dVertex[7] ,dVertex[8] ,dVertex[9] );
			glScalef( dVertex[4] ,dVertex[5] ,dVertex[6] );
			glTranslatef(dVertex[1] ,dVertex[2] ,dVertex[3]);
			gluSphere(obj, dVertex[0], slices, slices);
		glPopMatrix();
		delete [] dVertex;
	}
	else if (commander == strstr(commander,"sphere trunk"))
	{
		float *dVertex = new float[4];
		sscanf_s(commander,"%*s %*s %f %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3] );

		glPushMatrix();  
		//glLoadIdentity();		
		glTranslatef(dVertex[1] ,dVertex[2] ,dVertex[3]);
		gluSphere(obj, dVertex[0], slices, slices);
		glPopMatrix();
		delete [] dVertex;
	}
	else if (commander == strstr(commander,"cylinder trunk"))
	{
		float *dVertex = new float[7];
		sscanf_s(commander,"%*s %*s %f %f %f %f %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3], &dVertex[4], &dVertex[5] ,&dVertex[6]  );

		glPushMatrix();  
		//glLoadIdentity();		
			DrawCylinder(dVertex[0], dVertex[0], slices, 1, dVertex[1] ,dVertex[2] ,dVertex[3] ,dVertex[4] ,dVertex[5] ,dVertex[6] );
		glPopMatrix();
		delete [] dVertex;
	}

	return 1;

}

int drawTree( char *commander,int listIndex )
{
	if (commander == strstr(commander,"object"))
	{
		float *dVertex = new float[7];
		sscanf_s(commander,"%*s %*s %*s %f %f %f %f %*s %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3] ,&dVertex[4] ,&dVertex[5] ,&dVertex[6] );

		//float x = dVertex[4];
		//dVertex[4] = -dVertex[5] ;
		//dVertex[5] = x ;


		dVertex[4] = dVertex[4] -50;
		dVertex[5] = dVertex[5] -50;
		if ( dVertex[4] > xOffset - sideLength/2 - maxTreeRadius && dVertex[4] < xOffset + sideLength/2 + maxTreeRadius \
			&& dVertex[5] > (yOffset - sideLength2/2 )/cosx - maxTreeRadius - maxTreeHeight * tanx && dVertex[5] < (yOffset+sideLength2/2 )/cosx + maxTreeRadius ) 
		{
			glPushMatrix();  
			glLoadIdentity();		

			glTranslatef(dVertex[4] ,dVertex[5] ,dVertex[6]);
			glRotatef(dVertex[3], dVertex[0] ,dVertex[1] ,dVertex[2]);
			glColor3ub(0,255,0);
			glCallList(listIndex);
			//glCallList(listIndex+6);
			glColor3ub(255,255,255);
			glCallList(listIndex+14); //HET08
			glPopMatrix();
		}
		delete [] dVertex;
		return 0;
		
	}
}

int drawWood( char *commander,int listIndex)
{
	glColor3ub(255,255,255);
	
	if (commander == strstr(commander,"sphere"))
	{
		float *dVertex = new float[4];
		sscanf_s(commander,"%*s %f %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3]);


		glPushMatrix();  
		//glLoadIdentity();		
		glTranslatef(dVertex[1] ,dVertex[2] ,dVertex[3]);
		gluSphere(obj, dVertex[0], slices, slices);
		glPopMatrix();

		delete [] dVertex;
		return 0;
		
	}
	if (commander == strstr(commander,"cylinder"))
	{
		float *dVertex = new float[7];
		sscanf_s(commander,"%*s %f %f %f %f %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3] ,&dVertex[4] ,&dVertex[5] ,&dVertex[6] );

		DrawCylinder(dVertex[0], dVertex[0], slices, 1, dVertex[1] ,dVertex[2] ,dVertex[3] ,dVertex[4] ,dVertex[5] ,dVertex[6] );
		
		delete [] dVertex;
		return 0;
	}

	if (commander == strstr(commander,"cone"))
	{
		float *dVertex = new float[8];
		sscanf_s(commander,"%*s %f %f %f %f %f %f %f %f\n" ,\
			&dVertex[0], &dVertex[1] ,&dVertex[2] ,&dVertex[3] ,&dVertex[4] ,&dVertex[5] ,&dVertex[6] ,&dVertex[7] );

		DrawCylinder(dVertex[0], dVertex[4], slices, 1, dVertex[1] ,dVertex[2] ,dVertex[3] ,dVertex[5] ,dVertex[6] ,dVertex[7] );

		
		delete [] dVertex;
		return 0;
	}

	//
}

//建立Display List
int BuildLists(char* _fileDir, int _objNum, int _groupNum, int flag)
{
	
	int _listNum = _objNum + _groupNum*3+1;
	int lIndex;

	char tmpFName[_MAX_FNAME];
	char tmpFPath[_MAX_PATH];
	
	if (!flag)
	{
		int leaf = glGenLists(_listNum);
		strcpy_s(tmpFName,_MAX_FNAME,"PONI_leaf.def");
		_makepath_s(tmpFPath,_MAX_PATH, NULL,_fileDir,tmpFName,NULL);
		Execute(leaf, tmpFPath,drawLeaf);

		lIndex = leaf + 1;
		for (int i = 1; i <= _groupNum; i++)
		{
			sprintf_s(tmpFName, "PONI%d_foliage", i);
			_makepath_s(tmpFPath, _MAX_PATH, NULL, _fileDir, tmpFName,"dat");
			Execute(lIndex, tmpFPath,drawFoliage,leaf);

			sprintf_s(tmpFName, "PONI%d_wood", i);
			_makepath_s(tmpFPath, _MAX_PATH, NULL, _fileDir, tmpFName,"dat");
			Execute(lIndex+_groupNum, tmpFPath,drawWood);
			lIndex++;
		}
	} 
	else
	{
		lIndex = glGenLists(_groupNum+1);
	}

	lIndex = _objNum + 1;
	for (int i = 1; i <= _groupNum; i++)
	{
		
		sprintf_s(tmpFName, "PONI%d_treetransform", i);
		_makepath_s(tmpFPath, _MAX_PATH, NULL, _fileDir, tmpFName,"dat");
		Execute(lIndex+ _groupNum*2, tmpFPath,drawTree, lIndex );
		lIndex++;
	}


	glNewList(_listNum, GL_COMPILE);
	for (int i=1 ; i <= _groupNum; i++)
	{
		glCallList(_objNum+_groupNum*2+i);
	}
	glEndList();
	return _listNum;
}

//建立Display List
int BuildListsHET08(char* _fileDir, int _objNum, int _groupNum, int flag)
{
	int _listNum = _objNum + _groupNum*3+1;;
	int shoot;
	int lIndex;
	char tmpFName[_MAX_FNAME];
	char tmpFPath[_MAX_PATH];

	if(!flag)
	{
		shoot = glGenLists(_listNum);
		lIndex = shoot + 1;

		strcpy_s(tmpFName,_MAX_FNAME,"PIMO_shoot.def");
		_makepath_s(tmpFPath,_MAX_PATH, NULL,_fileDir,tmpFName,NULL);

		Execute(shoot, tmpFPath,drawShoot_PIMO);

		for (int i = 1; i <= _groupNum; i++)
		{
			sprintf_s(tmpFName, "PIMO%d_foliage", i);
			_makepath_s(tmpFPath, _MAX_PATH, NULL, _fileDir, tmpFName,"dat");
			Execute(lIndex , tmpFPath,drawFoliage,shoot);

			sprintf_s(tmpFName, "PIMO%d_stem", i);
			_makepath_s(tmpFPath, _MAX_PATH, NULL, _fileDir, tmpFName,"dat");
			ExecuteTri(lIndex + _groupNum, tmpFPath);
			lIndex++;


		}

	}
	else
	{
		//_listNum = _groupNum*2+1;
		shoot = 1;
		lIndex = glGenLists(_groupNum+1);
	}
		

	lIndex = _objNum + 1;
	for (int i = 1; i <= _groupNum; i++)
	{
	//	sprintf_s(tmpFName, "PONI%d_wood", i);
	//	_makepath_s(tmpFPath, _MAX_PATH, NULL, _fileDir, tmpFName,"dat");
	//	Execute(lIndex+6, tmpFPath,drawWood);
		sprintf_s(tmpFName, "PIMO%d_treetransform", i);
		_makepath_s(tmpFPath, _MAX_PATH, NULL, _fileDir, tmpFName,"dat");
		Execute(lIndex + _groupNum*2, tmpFPath,drawTree, lIndex );
		lIndex++;
	}


	glNewList(_listNum, GL_COMPILE);
	for (int i=1 ; i <= _groupNum; i++)
	{
		glCallList(_objNum+_groupNum+_groupNum+i);
	}
	glEndList();
	return _listNum;
}

void BuildlistsDisk(char* _filePath)
{
	if (!ReadRAMI(_filePath))
	{
		return;
	}

	int index = glGenLists(1);
	glNewList(index,GL_COMPILE);
	glPushMatrix();
	GLUquadricObj *objDisk = gluNewQuadric();
	gluQuadricDrawStyle(objDisk,GLU_FILL);
	glColor3ub(0, 255, 0);
	for (int i = 0; i < lines; i++)
	{
		glLoadIdentity();  
		glTranslated(fileRAMI[i][1],fileRAMI[i][2],(fileRAMI[i][3]-0.5));
		glRotated(180,fileRAMI[i][4],fileRAMI[i][5],fileRAMI[i][6]+1);
		gluDisk(objDisk,0.0,fileRAMI[i][0],30,1);
	}
	gluDeleteQuadric(objDisk);
	glPopMatrix();

	float sHeight = 1;
	glBegin(GL_QUADS);  //25×25地块
	glColor3ub(127, 63, 127);
	glVertex3f( 12.5,  12.5, -0.5*sHeight-fileRAMI[0][0]);
	glVertex3f(-12.5,  12.5, -0.5*sHeight-fileRAMI[0][0]);
	glVertex3f(-12.5, -12.5, -0.5*sHeight-fileRAMI[0][0]);
	glVertex3f( 12.5, -12.5, -0.5*sHeight-fileRAMI[0][0]);
	glEnd();
	glEndList();
	FreeMemory();
}

int BuildlistsDiskScene(char *objPath, char* locationPath, int flag)
{
	int objIndex, lIndex;
	if (!flag)
	{
		if (!ReadRAMI(objPath))
		{
			return -1;
		}

		float tmpAngle = 0.0f; 
		int slices = 10;
		float angleInterval = M_PI / slices;
		float radius = fileRAMI[0][0];

		objIndex = glGenLists(2);


		glNewList(objIndex,GL_COMPILE);

		//glPushMatrix();
		GLUquadricObj *objDisk = gluNewQuadric();
		gluQuadricDrawStyle(objDisk,GLU_FILL);
		glColor3ub(0, 255, 0);
		//glColor3ub(78, 255, 110);
		for (int i = 0; i < lines; i++)
		{
			glPushMatrix();
			glTranslatef(fileRAMI[i][1],fileRAMI[i][2],fileRAMI[i][3]);
			glRotatef(180,fileRAMI[i][4],fileRAMI[i][5],fileRAMI[i][6]+1);
			gluDisk(objDisk,0.0,fileRAMI[i][0],10,1);

			glPopMatrix();
		}
		gluDeleteQuadric(objDisk);
		//glPopMatrix();
		glEndList();
		FreeMemory();
		lIndex = objIndex+1;
	} 
	else
	{
		objIndex = 1;
		lIndex = glGenLists(1);
	}
	
	

	errno_t err;
	FILE* fp;
	float *location = new float[3];
	if (err = fopen_s(&fp, locationPath, "r"))
	{
		return err;
	}
	glNewList(lIndex, GL_COMPILE);
	int i = 0;
	while(!feof(fp) /*&&i < 100*/)
	{
		fscanf_s(fp,"%f %f %f\n", &location[0], &location[1], &location[2]);

		glMatrixMode(GL_MODELVIEW);                     // Select The Modelview Matrix
		glLoadIdentity();                           // Reset The Modelview Matrix
		//if ( 1/*abs(location[1]) < 10*/ )
		if ( location[0] >= (xOffset - sideLength/2 - maxTreeRadius)/cosz && location[0] <= (xOffset + sideLength/2 + maxTreeRadius)/cosz \
			&& location[1] >= (yOffset - sideLength2/2 )/cosx- maxTreeRadius - maxTreeHeight * tanx/cosz - 5 \
			&& location[1] <= ((yOffset + sideLength2/2)/cosx + maxTreeRadius)/cosz + 5 )
		{
			glPushMatrix(); 
			glTranslatef(location[0], location[1], location[2]);
			glCallList(objIndex);
			glPopMatrix();
		}
		
		i++;
	}

	//float sHeight = 3;
	//glBegin(GL_QUADS);  //25×25地块
	//glColor3ub(150, 100, 78);
	//glVertex3f( 135.0,  135.0, 0.0-sHeight);
	//glVertex3f(-135.0,  135.0, 0.0-sHeight);
	//glVertex3f(-135.0, -135.0, 0.0-sHeight);
	//glVertex3f( 135.0, -135.0, 0.0-sHeight);
	//glEnd();

	glEndList();
	fclose(fp);
	fp = 0;
	delete [] location;
	return lIndex;

}




