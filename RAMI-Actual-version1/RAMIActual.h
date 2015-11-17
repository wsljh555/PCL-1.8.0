#define _USE_MATH_DEFINES
//#include <afxwin.h>
#include <windows.h>                              // Header File For Windows
#include <iostream>
#include "resource.h"
#include <gl\gl.h>                                // Header File For The OpenGL32 Library
#include <gl\glu.h>                               // Header File For The GLu32 Library
#include <gl\glaux.h>								// Header File For The GLaux Library
//#include <gl\glut.h>                             // Header File For The GLaux Library
#include<math.h>
#include <shlobj.h>

#include <ctime>
#include ".\Library\gl2ps.h"
#include "FreeImage.h"
#include "SaveBMP.h"


#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"GLu32.lib")
#pragma comment(lib,"GLaux.lib")
#pragma comment(lib,"odbc32.lib")
#pragma comment(lib,"odbccp32.lib")
//#pragma comment(lib,"glut32.lib")
//#pragma comment(lib,"glu32.lib")
#pragma comment(lib, "FreeImage.lib")

#define SAVE_AT_ONCE		//运行程序即开始保存

//*************************************简单RAMI信息*************************************
GLfloat **fileRAMI;						//存储简单RAMI数据
GLfloat **fileLocation;					//存储简单RAMI数据
int lines;								//文件行数
int lines2;								//文件行数
GLUquadricObj *obj;
GLint slices = 3;  //表示球体纵向分割的数目（经线）,主要是设置绘制的细密度
char filePath[_MAX_PATH] = "d:\\Study\\Data\\LAI\\RAMI\\HET04";	//定义文件名
 
//*************************************Actual Canopy信息*************************************
char fileDir[_MAX_DIR]="d:\\Study\\Data\\LAI\\RAMI\\HET08_OPS_WIN";  //读取文件夹
GLint groupNum = 14;  //场景有14棵树的集合组成
GLint objNum = 1;   //14棵树组成一个场景
GLint listNums =0;

//char fileDir[_MAX_DIR]="d:\\Study\\Data\\LAI\\RAMI\\HET16_SRF";  //读取文件夹
//GLint groupNum = 6;  //场景有6棵树的集合组成
//GLint objNum = 1;   //6棵树组成一个场景
//GLint listNums =0;


//*************************************Abstract Canopy信息*************************************
char abFileDir[_MAX_DIR]="d:\\Study\\Data\\LAI\\RAMI\\HET04";  //读取文件夹
char abObjFName[_MAX_FNAME] = "HET04_cyl.def";
char abLocationFName[_MAX_FNAME] = "cyl_coord.txt";
char abObjFPath[_MAX_PATH];
char abLocationFPath[_MAX_PATH];


//*************************************保存信息*************************************

char prefix[_MAX_FNAME] = "HET04_CYL_ALL";					//保存文件文件名前缀
char saveIMGDir[_MAX_PATH] = "d:\\Study\\Temp\\LAI\\BMP\\HET04";
char saveImgPath[_MAX_PATH] ;							//像素图保存地址
char saveVecPath[_MAX_PATH] ;							//矢量图保存地址


//*************************************场景信息*************************************

//场景范围
GLdouble xMin = -135.0;
GLdouble yMin = -135.0;
GLdouble xMax =  135.0;
GLdouble yMax =  135.0;



//*************************************显示信息*************************************
//int windowsWidth = 2560;							//窗口宽度
int windowsWidth = 960;							//窗口宽度
//int windowsWidth = 960;
int windowsHeight = 640;							//窗口高度

GLdouble sideLength = 20;							//显示场景宽度
GLdouble sideLength2 ;//= sideLength / winWidth * winHeight;
GLdouble xOffset = -120;								//偏移量（图像中心为0）
GLdouble yOffset = -127.5;

GLdouble maxTreeRadius = 5.0;
GLdouble maxTreeHeight = 15+10.0;

GLint blockNum = 0;
GLint curblock = 0;

GLfloat xRotateAngle = 0.0f;						//绕x轴旋转角度
GLfloat zRotateAngle = 0.0f;						//绕z轴旋转角度


 
GLfloat rInterval = 10.0f;							//自动保存图片天顶角间隔

char banchFile[_MAX_PATH] = "filePath.ini";			//批量保存地址




//**************************************************************************
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);               // Declaration For WndProc
int SaveIMG();														//保存当前视口截图
int SaveVector();													//保存当前视口矢量数据
								
int SaveMultiAngle();												
		
int SaveWholeScene( GLdouble _xMin, GLdouble _xMax, GLdouble _yMin, GLdouble _yMax );//保存整个场景
int SaveSceneMultiAngle();															//保存整个场景的多角度		
bool IsVSyncEnabled();
void SetVSyncState(bool enable);
bool InitVSync();


/****************************************************************************/
void DrawCylinder(double baseRadius, double topRadius, int slices, int stacks, float x0, float y0, float z0, float x1, float y1, float z1);

int Execute(int index, char* _filePath, int(*func)(char*,int), int listIndex = 0);  //执行Actual Canopy文件
int ExecuteTri(int index, char* _filePath);

int drawLeaf(char *commander,int listIndex);							//层次1：画叶片
int drawFoliage(char *commander,int listIndex);							//层次2：画一棵树
int drawWood( char *commander,int listIndex);
int drawTree(char *commander,int listIndex);							//层次3：画一类树
int drawShoot_PIMO(char *commander, int listIndex);

int BuildLists(char* _fileDir, int _objNum, int _groupNum, int flag = 0);		//创建Actual Canopy的DisplayList			
void BuildlistsDisk(char* _filePath);							//为简单RAMI创建DisplayList
int BuildlistsDiskScene(char *objPath, char* locationPath, int flag = 0);

int ReadRAMI( char* filepath);										//读取简单RAMI数据
int ReadLocation( char* filepath);
void FreeMemory();													//释放简单RAMI内存
int BuildListsHET08(char* _fileDir, int _objNum, int _groupNum, int flag = 0);
