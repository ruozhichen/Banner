#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include <iostream>
#include <commdlg.h>
#include <direct.h>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cv.h>
#include <highgui.h>
#include <ctime>
#include <stdlib.h>
#include <string.h>
#define VBN	10000
#define RIGHT 0
#define LEFT 1
#define ALIGNMENT_COST_PER_GROUP 200
#define ALIGNMENT_COMPLETE_POOR 45+25
#define ALIGNMENT_COMPLETE_GOOD 120
#define ALIGNMENT_COMPLETE_GREAT 150
#define ALIGNMENT_HALFWAY 45
#define ALIGNMENT_MARGINS 20
#define ALIGNMENT_INTEREST 10
#define ALIGNMENT_PERCENTAGE 0.5
#define BALANCE_PERCENTAGE 0.5
#define ALIGNMENT_COEFFICIENT 1.0
#define BALANCE_COEFFICIENT 1.0   //在cost计算时，放大的比例系数
#define PLAIN 		1.0
#define BACKGROUND  0.0
#define FALSE 	0
#define TRUE 	1
#define VERTICAL 	1
#define HORIZONTAL 	0
#define SMALL_GROWTH 0.20
#define MEDIUM_GROWTH 0.25
#define BIG_GROWTH 0.30
#define GRIDX 3
#define GRIDY 2
#define HORIZONTALLEFT 1
#define HORIZONTALRIGHT 2
#define VERTICALUP 1
#define VERTICALBOTTOM 2
#define NOMATTER 0
#define IMAGE 0
#define TITLE 1
#define EXPLAIN 2
#define KIND 3
#define TitleSpaceWidthScaleHor 0.35
#define TitleSpaceHeightScaleHor 0.40
#define ExplainSpaceWidthScaleHor 0.35
#define ExplainSpaceHeightScaleHor 0.45
#define ImageSpaceWidthScaleHor 0.50
#define ImageSpaceHeightScaleHor 0.85

#define TitleSpaceWidthScaleVer 0.8
#define TitleSpaceHeightScaleVer 0.15
#define ExplainSpaceWidthScaleVer 0.8
#define ExplainSpaceHeightScaleVer 0.20
#define ImageSpaceWidthScaleVer 0.8
#define ImageSpaceHeightScaleVer 0.45

#define GAPSCALE 0.05

#define InputAddr 0
#define OutputAddr 1
#define RADIOGROUPSIZE 18
#define SLIDERGROUPSIZE 6
#define DEFAULT_LAYOUT 0
#define USERDEFINED_LAYOUT 3
using namespace std;
/**
BannerGUI1.0(win32):
基本实现了简单的GUI，图片、标题、说明文字均可以用文件选择对话框选择，当然这些都要求是图片格式
可以选择横版或者竖版
然后单击开始按钮即可
BannerGUI2.0:
用户可以选择默认版式，或者自定义版式
默认版式附加了图片参考
*/
HINSTANCE hInst;
HWND buttonImage,buttonTitle,buttonExplain;
HWND radioHorizontal,radioVertical;
HWND radioDefaultLayout,radioUserLayout;
HWND radioGroups[RADIOGROUPSIZE];
HWND sliderGroups[SLIDERGROUPSIZE];
HWND editTextBalance,editTextAlign,editTextCost;
HWND staticScaleText[SLIDERGROUPSIZE];
HWND editBannerWidth,editBannerHeight;

OPENFILENAME ofnImage,ofnTitle,ofnExplain;
char szFileNameImage[MAX_PATH] = "";  //存储用户选择的文件路径
char szFileNameTitle[MAX_PATH] = "";  //存储用户选择的文件路径
char szFileNameExplain[MAX_PATH] = "";  //存储用户选择的文件路径
int layoutDefaultOrDefined=DEFAULT_LAYOUT;
int layoutStyle=HORIZONTAL;
int BannerHeight=420;
int BannerWidth=630;

bool fileChosen[3]={false,false,false};
const int maxn=100;
char inputAddr[50] = "input/"; //输入的相对地址
char imageName[maxn]="image.jpg";
char titleName[maxn]="title.jpg";
char explainName[maxn]="explain.jpg";

char currentDirectory[maxn];

struct Name{
    char str[maxn];
};
struct Layout{
    /*
    相对位置矩阵
    relatePos[i][j][0]表示i与j在水平方向的相对位置
    relatePos[i][j][1]表示i与j在竖直方向的相对位置
    */
    int relatePos[KIND][KIND][2];
    int imageToTitleHorizontal; //[0]=1,表示image在title左边，2表示在右边,0表示没有要求
    int imageToTitleVetrical; //[0]=1,表示image在title顶部，2表示在底部,0表示没有要求
    int imageToExplainHorizontal; //[0]=1,表示image在explain左边，2表示在右边,0表示没有要求
    int imageToExplainVertical; //[0]=1,表示image在explain顶部，2表示在底部,0表示没有要求
    int titleToExplainHorizontal; //[0]=1,表示title在explain左边，2表示在右边,0表示没有要求
    int titleToExplainVertical; //[0]=1,表示title在explain顶部，2表示在底部,0表示没有要求
    float scale[3][2]; //[kind][HORIZONTAL]表示水平即宽的比例，[kind][VERTICAL]表示垂直即高的比例
};

Layout user_defined_layout;

void PopFileInitialize (HWND hwnd,OPENFILENAME &ofn,char*szFileName);
//bool PopFileOpenDlg (HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);
void autocreatBanner(Name*elementName,int *kindMark,float &balanceCost,float &alignCost,float &totalcost);
void updateScale(int i,HWND slider,HWND editText);

void enableDefaultLayout(bool flag);
void enableUser_defined(bool flag);
void enableSlider(bool flag);
void initRadioGroups(HWND hwndDlg);
void initSliderGroups(HWND hwndDlg);
void initStaticScaleText(HWND hwndDlg);
/*窗口回调函数，一旦有事件响应，系统就会调用该函数*/
BOOL CALLBACK DlgMain(HWND hwndDlg,/*窗口句柄，32位无符号整数*/
                      UINT uMsg,/*消息类型*/
                      WPARAM wParam, /*wParam、lParam这两个是具体的事件信息*/
                      LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:/*窗口被初始化时被引发的时间*/
    {
        getcwd(currentDirectory, MAX_PATH); //获取当前窗口的路径保存下来
        buttonImage=GetDlgItem(hwndDlg,IDC_BUTTON1);
        buttonTitle=GetDlgItem(hwndDlg,IDC_BUTTON2);
        buttonExplain=GetDlgItem(hwndDlg,IDC_BUTTON3);
        radioHorizontal=GetDlgItem(hwndDlg,IDC_RADIOHORIZONTAL);
        SendMessage(radioHorizontal, BM_SETCHECK, BST_CHECKED,0); //使radioHorizontal一开始被默认选中
        radioVertical=GetDlgItem(hwndDlg,IDC_RADIOVERTICAL);
        //EnableWindow(radioVertical,false);

        radioDefaultLayout=GetDlgItem(hwndDlg,IDC_RADIODEFAULTLAYOUT);
        radioUserLayout=GetDlgItem(hwndDlg,IDC_RADIOUSERLAYOUT);
        SendMessage(radioDefaultLayout, BM_SETCHECK, BST_CHECKED,0); //使radioHorizontal一开始被默认选中

        initRadioGroups(hwndDlg);
        enableDefaultLayout(true);
        enableUser_defined(false);

        initSliderGroups(hwndDlg);
        enableSlider(false);

        initStaticScaleText(hwndDlg);

        editTextBalance=GetDlgItem(hwndDlg,IDC_EDITBALANCE);
        editTextAlign=GetDlgItem(hwndDlg,IDC_EDITALIGNMENT);
        editTextCost=GetDlgItem(hwndDlg,IDC_EDITCOST);

        editBannerWidth=GetDlgItem(hwndDlg,IDC_EDITBANNERWIDTH);
        editBannerHeight=GetDlgItem(hwndDlg,IDC_EDITBANNERHEIGHT);

        PopFileInitialize(hwndDlg,ofnImage,szFileNameImage);
        PopFileInitialize(hwndDlg,ofnTitle,szFileNameTitle);
        PopFileInitialize(hwndDlg,ofnExplain,szFileNameExplain);


    }
    return TRUE;

    case WM_CLOSE: /*窗口被关闭时的时间，即点击了“X”*/
    {
        if(MessageBoxA(hwndDlg,"关闭窗口？","提示",MB_YESNO)==IDYES){
            EndDialog(hwndDlg, 0);
        }

    }
    return TRUE;

    case WM_COMMAND:/*单击事件，就是触发按钮的事件*/
    {
        switch(LOWORD(wParam))
        {
            case IDC_BUTTON1:{
                printf("按钮被点击\n");
                SetWindowText(buttonImage,TEXT("Has chosen"));
                if(GetOpenFileName(&ofnImage))
                {
                // Do something usefull with the filename stored in szFileName
                    cout<<szFileNameImage<<endl;  //在控制台中显示选中文件的路径
                }
                fileChosen[0]=true;
                break;
            }
            case IDC_BUTTON2:{
                printf("按钮被点击\n");
                SetWindowText(buttonTitle,TEXT("Has chosen"));
                if(GetOpenFileName(&ofnTitle))
                {
                // Do something usefull with the filename stored in szFileName
                    cout<<szFileNameTitle<<endl;  //在控制台中显示选中文件的路径
                }
                fileChosen[1]=true;
                break;
            }
            case IDC_BUTTON3:{
                printf("按钮被点击\n");
                SetWindowText(buttonExplain,TEXT("Has chosen"));
                if(GetOpenFileName(&ofnExplain))
                {
                // Do something usefull with the filename stored in szFileName
                    cout<<szFileNameExplain<<endl;  //在控制台中显示选中文件的路径
                }
                fileChosen[2]=true;
                break;
            }
            case IDC_RADIODEFAULTLAYOUT:{
                enableDefaultLayout(true);
                enableUser_defined(false);
                enableSlider(false);
                layoutDefaultOrDefined=DEFAULT_LAYOUT;
                break;
            }
            case IDC_RADIOUSERLAYOUT:{
                enableDefaultLayout(false);
                enableUser_defined(true);
                enableSlider(true);
                layoutDefaultOrDefined=USERDEFINED_LAYOUT;
                layoutStyle=USERDEFINED_LAYOUT;
                break;
            }
            case IDC_RADIOHORIZONTAL:{
                layoutStyle=HORIZONTAL;
                printf("%d\n",layoutStyle);
                break;
            }
            case IDC_RADIOVERTICAL:{
                layoutStyle=VERTICAL;
                printf("%d\n",layoutStyle);
                break;
            }
            /*
            用于测试的
            */
            /*
            case IDC_BUTTONTEST:{
char widthstr[MAX_PATH],heightstr[MAX_PATH];
                GetWindowText(editBannerWidth,widthstr,MAX_PATH);
                GetWindowText(editBannerHeight,heightstr,MAX_PATH);

printf("atoi: %s %s %d %d\n",widthstr,heightstr,atoi(widthstr),atoi(heightstr));
                if(!atoi(widthstr) || !atoi(heightstr)){
                    MessageBox(hwndDlg,"广告宽和高格式错误，请重新输入","提示",MB_OKCANCEL);
                    break;
                }
                break;
            }
            */
            case IDC_BUTTONSTART:{
                if(!fileChosen[0] || !fileChosen[1] || !fileChosen[2] ){
                    MessageBox(hwndDlg,"有元素没有选择,请浏览并选择","提示",MB_OKCANCEL);
                    break;
                }
                char widthstr[MAX_PATH],heightstr[MAX_PATH];
                GetWindowText(editBannerWidth,widthstr,MAX_PATH);
                GetWindowText(editBannerHeight,heightstr,MAX_PATH);

printf("atoi: %s %s %d %d\n",widthstr,heightstr,atoi(widthstr),atoi(heightstr));
                if(!atoi(widthstr) || !atoi(heightstr)){
                    MessageBox(hwndDlg,"广告宽和高格式错误，请重新输入","提示",MB_OKCANCEL);
                    break;
                }
                BannerWidth=atoi(widthstr);
                BannerHeight=atoi(heightstr);

                SetCurrentDirectory(currentDirectory); //设置回到原来的当前目录

                char tmpstr[maxn];

                IplImage* img1 = cvLoadImage(szFileNameImage);
                strcpy(tmpstr,inputAddr);
                strcat(tmpstr,imageName);
                cvSaveImage(tmpstr,img1);  //为什么这里就保存不了呢？？？好像根本就没执行一样,也不报错

                IplImage* img2 = cvLoadImage(szFileNameTitle);
                strcpy(tmpstr,inputAddr);
                strcat(tmpstr,titleName);
                cvSaveImage(tmpstr,img2);

                IplImage* img3 = cvLoadImage(szFileNameExplain);
                strcpy(tmpstr,inputAddr);
                strcat(tmpstr,explainName);
                cvSaveImage(tmpstr,img3);

                Name elementName[3];
                strcpy(elementName[0].str,imageName);
                strcpy(elementName[1].str,titleName);
                strcpy(elementName[2].str,explainName);
                int kindMark[3]={IMAGE,TITLE,EXPLAIN};

                float balanceCost,alignCost,totalCost;
                autocreatBanner(elementName,kindMark,balanceCost,alignCost,totalCost);

                char showNum[maxn];
                sprintf(showNum,"%.3f",balanceCost);
                SetWindowText(editTextBalance,TEXT(showNum));
                sprintf(showNum,"%.3f",alignCost);
                SetWindowText(editTextAlign,TEXT(showNum));
                sprintf(showNum,"%.3f",totalCost);
                SetWindowText(editTextCost,TEXT(showNum));
                break;
            }
            case IDC_BUTTONEXIT:{
                if(MessageBox(hwndDlg,"关闭窗口？","提示",MB_YESNO)==IDYES){
                    EndDialog(hwndDlg, 0);
                }
                break;
            }
            //自定义布局
            case IDC_RADIOLEFT1:{
                user_defined_layout.relatePos[IMAGE][TITLE][HORIZONTAL]=HORIZONTALLEFT;
                user_defined_layout.relatePos[TITLE][IMAGE][HORIZONTAL]=HORIZONTALRIGHT;
                break;
            }
            case IDC_RADIOLEFT2:{
                user_defined_layout.relatePos[IMAGE][EXPLAIN][HORIZONTAL]=HORIZONTALLEFT;
                user_defined_layout.relatePos[EXPLAIN][IMAGE][HORIZONTAL]=HORIZONTALRIGHT;
                break;
            }
            case IDC_RADIOLEFT3:{
                user_defined_layout.relatePos[TITLE][EXPLAIN][HORIZONTAL]=HORIZONTALLEFT;
                user_defined_layout.relatePos[EXPLAIN][TITLE][HORIZONTAL]=HORIZONTALRIGHT;
                break;
            }
            case IDC_RADIORIGHT1:{
                user_defined_layout.relatePos[IMAGE][TITLE][HORIZONTAL]=HORIZONTALRIGHT;
                user_defined_layout.relatePos[TITLE][IMAGE][HORIZONTAL]=HORIZONTALLEFT;
                break;
            }
            case IDC_RADIORIGHT2:{
                user_defined_layout.relatePos[IMAGE][EXPLAIN][HORIZONTAL]=HORIZONTALRIGHT;
                user_defined_layout.relatePos[EXPLAIN][IMAGE][HORIZONTAL]=HORIZONTALLEFT;
                break;
            }
            case IDC_RADIORIGHT3:{
                user_defined_layout.relatePos[TITLE][EXPLAIN][HORIZONTAL]=HORIZONTALRIGHT;
                user_defined_layout.relatePos[EXPLAIN][TITLE][HORIZONTAL]=HORIZONTALLEFT;
                break;
            }
            case IDC_RADIOUP1:{
                user_defined_layout.relatePos[IMAGE][TITLE][VERTICAL]=VERTICALUP;
                user_defined_layout.relatePos[TITLE][IMAGE][VERTICAL]=VERTICALBOTTOM;
                break;
            }
            case IDC_RADIOUP2:{
                user_defined_layout.relatePos[IMAGE][EXPLAIN][VERTICAL]=VERTICALUP;
                user_defined_layout.relatePos[EXPLAIN][IMAGE][VERTICAL]=VERTICALBOTTOM;
                break;
            }
            case IDC_RADIOUP3:{
                user_defined_layout.relatePos[TITLE][EXPLAIN][VERTICAL]=VERTICALUP;
                user_defined_layout.relatePos[EXPLAIN][TITLE][VERTICAL]=VERTICALBOTTOM;
                break;
            }
            case IDC_RADIODOWN1:{
                user_defined_layout.relatePos[IMAGE][TITLE][VERTICAL]=VERTICALBOTTOM;
                user_defined_layout.relatePos[TITLE][IMAGE][VERTICAL]=VERTICALUP;
                break;
            }
            case IDC_RADIODOWN2:{
                user_defined_layout.relatePos[IMAGE][EXPLAIN][VERTICAL]=VERTICALBOTTOM;
                user_defined_layout.relatePos[EXPLAIN][IMAGE][VERTICAL]=VERTICALUP;
                break;
            }
            case IDC_RADIODOWN3:{
                user_defined_layout.relatePos[TITLE][EXPLAIN][VERTICAL]=VERTICALBOTTOM;
                user_defined_layout.relatePos[EXPLAIN][TITLE][VERTICAL]=VERTICALUP;
                break;
            }
            case IDC_RADIOANY1:{
                user_defined_layout.relatePos[IMAGE][TITLE][HORIZONTAL]=NOMATTER;
                user_defined_layout.relatePos[TITLE][IMAGE][HORIZONTAL]=NOMATTER;
                break;
            }
            case IDC_RADIOANY2:{
                user_defined_layout.relatePos[IMAGE][TITLE][VERTICAL]=NOMATTER;
                user_defined_layout.relatePos[TITLE][IMAGE][VERTICAL]=NOMATTER;
                break;
            }
            case IDC_RADIOANY3:{
                user_defined_layout.relatePos[IMAGE][EXPLAIN][HORIZONTAL]=NOMATTER;
                user_defined_layout.relatePos[EXPLAIN][IMAGE][HORIZONTAL]=NOMATTER;
                break;
            }
            case IDC_RADIOANY4:{
                user_defined_layout.relatePos[IMAGE][EXPLAIN][VERTICAL]=NOMATTER;
                user_defined_layout.relatePos[EXPLAIN][IMAGE][VERTICAL]=NOMATTER;
                break;
            }
            case IDC_RADIOANY5:{
                user_defined_layout.relatePos[TITLE][EXPLAIN][HORIZONTAL]=NOMATTER;
                user_defined_layout.relatePos[EXPLAIN][TITLE][HORIZONTAL]=NOMATTER;
                break;
            }
            case IDC_RADIOANY6:{
                user_defined_layout.relatePos[TITLE][EXPLAIN][VERTICAL]=NOMATTER;
                user_defined_layout.relatePos[EXPLAIN][TITLE][VERTICAL]=NOMATTER;
                break;
            }
        }
    }
    return TRUE;
    /**
    用于接收slider的消息
    */
    case WM_HSCROLL:{
        for(int i=0;i<SLIDERGROUPSIZE;i++){
            updateScale(i,sliderGroups[i],staticScaleText[i]);
        }
    }
    return true;
    /*
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    return TRUE;
    */
    }
    return FALSE;
}

void updateScale(int i,HWND slider,HWND editText){
    int value;
    char showValue[maxn];

    value=SendMessageW(slider, TBM_GETPOS, 0, 0); //获取当前控件的值
    showValue[maxn];
    user_defined_layout.scale[i/2][i%2]=value/100.f;
    sprintf(showValue,"%.2f",value/100.0f);
    SetWindowText(editText,TEXT(showValue));
}
void initStaticScaleText(HWND hwndDlg){
    staticScaleText[0]=GetDlgItem(hwndDlg,IDC_EDITSCALE1);
    staticScaleText[1]=GetDlgItem(hwndDlg,IDC_EDITSCALE2);
    staticScaleText[2]=GetDlgItem(hwndDlg,IDC_EDITSCALE3);
    staticScaleText[3]=GetDlgItem(hwndDlg,IDC_EDITSCALE4);
    staticScaleText[4]=GetDlgItem(hwndDlg,IDC_EDITSCALE5);
    staticScaleText[5]=GetDlgItem(hwndDlg,IDC_EDITSCALE6);
}
void enableSlider(bool flag){
    for(int i=0;i<SLIDERGROUPSIZE;i++){
        EnableWindow(sliderGroups[i],flag);
    }
}

void initSlider(HWND hWndSlider,int value){
    SendMessageW(hWndSlider, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(20,70));//设置范围为20到70
    SendMessageW(hWndSlider,TBM_SETPOS, (WPARAM)1,(LPARAM)(int)value);//设置slider控件的值

}
void initSliderGroups(HWND hwndDlg){
    sliderGroups[0]=GetDlgItem(hwndDlg,IDC_SLIDER1);
    sliderGroups[1]=GetDlgItem(hwndDlg,IDC_SLIDER2);
    sliderGroups[2]=GetDlgItem(hwndDlg,IDC_SLIDER3);
    sliderGroups[3]=GetDlgItem(hwndDlg,IDC_SLIDER4);
    sliderGroups[4]=GetDlgItem(hwndDlg,IDC_SLIDER5);
    sliderGroups[5]=GetDlgItem(hwndDlg,IDC_SLIDER6);
    for(int i=0;i<SLIDERGROUPSIZE;i++){
        initSlider(sliderGroups[i],50);
    }
}

void enableDefaultLayout(bool flag){
    EnableWindow(radioHorizontal,flag);
    EnableWindow(radioVertical,flag);
}

void enableUser_defined(bool flag){
    for(int i=0;i<RADIOGROUPSIZE;i++){
        EnableWindow(radioGroups[i],flag);
    }
}
void initRadioGroups(HWND hwndDlg){
    radioGroups[0]=GetDlgItem(hwndDlg,IDC_RADIOLEFT1);
    radioGroups[1]=GetDlgItem(hwndDlg,IDC_RADIORIGHT1);
    radioGroups[2]=GetDlgItem(hwndDlg,IDC_RADIOANY1);
    SendMessage(radioGroups[2], BM_SETCHECK, BST_CHECKED,0); //使一开始被默认选中
    radioGroups[3]=GetDlgItem(hwndDlg,IDC_RADIOUP1);
    radioGroups[4]=GetDlgItem(hwndDlg,IDC_RADIODOWN1);
    radioGroups[5]=GetDlgItem(hwndDlg,IDC_RADIOANY2);
    SendMessage(radioGroups[5], BM_SETCHECK, BST_CHECKED,0); //使一开始被默认选中
    radioGroups[6]=GetDlgItem(hwndDlg,IDC_RADIOLEFT2);
    radioGroups[7]=GetDlgItem(hwndDlg,IDC_RADIORIGHT2);
    radioGroups[8]=GetDlgItem(hwndDlg,IDC_RADIOANY3);
    SendMessage(radioGroups[8], BM_SETCHECK, BST_CHECKED,0); //使一开始被默认选中
    radioGroups[9]=GetDlgItem(hwndDlg,IDC_RADIOUP2);
    radioGroups[10]=GetDlgItem(hwndDlg,IDC_RADIODOWN2);
    radioGroups[11]=GetDlgItem(hwndDlg,IDC_RADIOANY4);
    SendMessage(radioGroups[11], BM_SETCHECK, BST_CHECKED,0); //使一开始被默认选中
    radioGroups[12]=GetDlgItem(hwndDlg,IDC_RADIOLEFT3);
    radioGroups[13]=GetDlgItem(hwndDlg,IDC_RADIORIGHT3);
    radioGroups[14]=GetDlgItem(hwndDlg,IDC_RADIOANY5);
    SendMessage(radioGroups[14], BM_SETCHECK, BST_CHECKED,0); //使一开始被默认选中
    radioGroups[15]=GetDlgItem(hwndDlg,IDC_RADIOUP3);
    radioGroups[16]=GetDlgItem(hwndDlg,IDC_RADIODOWN3);
    radioGroups[17]=GetDlgItem(hwndDlg,IDC_RADIOANY6);
    SendMessage(radioGroups[17], BM_SETCHECK, BST_CHECKED,0); //使一开始被默认选中
}
int APIENTRY WinMain(HINSTANCE hInstance,  //当前应用程序的实例句柄
                     HINSTANCE hPrevInstance, //前一个实例
                     LPSTR lpCmdLine, //命令行参数,LPSTR实际就是char*
                     int nShowCmd) //主窗口的显示方式
{
    hInst=hInstance;
    InitCommonControls();

	HWND      hwnd;
	hwnd=GetForegroundWindow(); //获取前台窗口句柄。本程序中的前台窗口就是控制台窗口。


    return DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}

/*

if(GetOpenFileName(&ofn))
{
// Do something usefull with the filename stored in szFileName
}
*/
void PopFileInitialize (HWND hwnd,OPENFILENAME &ofn,char*szFileName)
{
    static TCHAR szFilter[] = TEXT ("所有图片文件\0*.bmp;*.dib;*.jpg;*.jpeg;*.jpe;*.gif;*.tiff;*.png;*.ico\0")  \
                               TEXT ("JPEG文件 (*.jpg;*.jpeg;*.jpe)\0*.jpg;*.jpeg;*.jpe\0") \
                               TEXT ("位图文件 (*.bmp;*.dib)\0*.bmp;*.dib\0") \
							   TEXT ("GIF (*.gif)\0*.gif\0") \
							   TEXT ("TIFF (*.tiff)\0*.tiff") \
							   TEXT ("PNG (*.png)\0*.png") \
							   TEXT ("ICO (*.ico)\0*.ico\0\0");

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter =  szFilter;//"Text Files (*.jpg)\0*.jpg\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "jpg";
}
/*
bool PopFileOpenDlg (HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
     ofn.hwndOwner         = hwnd ;
     ofn.lpstrFile         = pstrFileName ;
     ofn.lpstrFileTitle    = pstrTitleName ;
     ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;

     return GetOpenFileNameA(&ofn) ; //
}
*/





int heightBound,widthBound;
float boundScale=0.05f;
float BannerBalanceCost;
float BannerAlignmentCost;



struct Element{
    char name[maxn]; //不包含相对地址
    char zoomName[maxn];
	float weight;
	int x_pos;
	int y_pos;
	int width;
	int height;
	int normalGroup;  //???到底是啥
	int x_avg;		//中心坐标
	int y_avg;
	int x_gravity;
	int y_gravity;
	int alignment;
	int flag; //picture:1 title:2 explain:3
};

struct Space{
	int x;
	int y;
	int w;
	int h;
};

class Bitmap {
public:
	int w, h;
	int *data;
	Bitmap(int w_, int h_) :w(w_), h(h_) { data = new int[w*h]; }
	~Bitmap() { delete[] data; }
	int *operator[](int y) { return &data[y*w]; }
};


int l(int t);
void alpha(float *t);
void chooseNeighbour(Element *candidate, Space space, Space titileSpace,int size, float t,Layout layout);
float cost(Element *candidate, Space space, Space interestBox, int size, int designType);
int checkBorders(Element g, Space *space, int size,int &orientation);
int checkSpace(Element *candidate, Space *space, int *orientation, int size);
int getOverlapCost(Element *candidate, int size);
float getBalanceCost(Element *candidate, Space space, int size);
float getAlignmentCost(Element *candidate, Space space, Space interestBox, int size, int designType);
int isOverlapping(Element one, Element two);
char * itoa (int i);
void printfCentroid(Element *candidate, Space space, int size);

void check_im() {
	if (system("identify > null.txt") != 0) {
		fprintf(stderr, "ImageMagick must be installed, and 'convert' and 'identify' must be in the path\n"); exit(1);
	}
}



Bitmap *load_bitmap(char *filename) {
	check_im();
	char rawname[256], txtname[256];

	//char relAddr[50] = "input/";
	//strcat(relAddr, filename);
	//strcpy(filename, relAddr);

	strcpy(rawname, filename);
	strcpy(txtname, filename);
	if (!strstr(rawname, ".")) { fprintf(stderr, "Error reading image '%s': no extension found\n", filename); exit(1); }
	sprintf(strstr(rawname, "."), ".raw");  //strstr(str1,str2):look for str2 in str1 and return the first location, or return null.
	sprintf(strstr(txtname, "."), ".txt");
	char buf[256];
	sprintf(buf, "convert %s rgba:%s", filename, rawname);
	if (system(buf) != 0) { fprintf(stderr, "Error reading image '%s': ImageMagick convert gave an error\n", filename); system("pause"); exit(1); }
	sprintf(buf, "identify -format \"%%w %%h\" %s > %s", filename, txtname);
	if (system(buf) != 0) { fprintf(stderr, "Error reading image '%s': ImageMagick identify gave an error\n", filename); system("pause"); exit(1); }
	FILE *f = fopen(txtname, "rt");
	if (!f) { fprintf(stderr, "Error reading image '%s': could not read output of ImageMagick identify\n", filename); system("pause"); exit(1); }
	int w = 0, h = 0;  //"a.txt" "b.txt" stores values of w,h of pixels
	if (fscanf(f, "%d %d", &w, &h) != 2) { fprintf(stderr, "Error reading image '%s': could not get size from ImageMagick identify\n", filename); system("pause"); exit(1); }
	fclose(f);
	f = fopen(rawname, "rb");
	Bitmap *ans = new Bitmap(w, h);
	unsigned char *p = (unsigned char *)ans->data;
	//Read data about the picture, but why *4???
	for (int i = 0; i < w*h * 4; i++) {
		int ch = fgetc(f);
		if (ch == EOF) { fprintf(stderr, "Error reading image '%s': raw file is smaller than expected size %dx%dx4\n", filename, w, h, 4); system("pause"); exit(1); }
		*p++ = ch;
	}
	fclose(f);
	return ans;
}

/**
store the image from a bitmap.
flag=0则是保存在input/
flag=1则是保存在output/
*/
void save_bitmap(Bitmap *bmp, char *filename,int flag) {
	check_im();
	char rawname[256];
	char tmpname[256];
	char relAddr[50];//输入输出的图片相对于代码的位置，这样防止输出输入和代码都在一个文件里，very very mess.
	strcpy(tmpname, filename);  //先将filename拷贝到tmpname中去，不能用filename进行修改，因为是指针，会对filename造成改动
	if (flag == InputAddr)
		strcpy(relAddr, inputAddr); //因为前一次结果要作为后一次的输入sheet，所以也要保存在input文件里
	else
		strcpy(relAddr, "output/"); //输出的相对位置
	strcat(relAddr, tmpname);
	strcpy(tmpname, relAddr);

	strcpy(rawname, tmpname);
	if (!strstr(rawname, ".")) { fprintf(stderr, "Error writing image '%s': no extension found\n", tmpname); system("pause"); exit(1); }
	//sprintf(strstr(rawname, "."), ".raw"); //这边注释掉的话，就不会生成.raw文件，且不影响程序运行
	char buf[256];

	FILE *f = fopen(rawname, "wb");
	if (!f) { fprintf(stderr, "Error writing image '%s': could not open raw temporary file\n", tmpname); system("pause"); exit(1); }
	unsigned char *p = (unsigned char *)bmp->data;
	for (int i = 0; i < bmp->w*bmp->h * 4; i++) {
		fputc(*p++, f);
	}
	fclose(f);
	sprintf(buf, "convert -size %dx%d -depth 8 rgba:%s %s", bmp->w, bmp->h, rawname, tmpname);
	if (system(buf) != 0) { fprintf(stderr, "Error writing image '%s': ImageMagick convert gave an error\n", tmpname); system("pause"); exit(1); }
}

/*
计算a的weight值
光累加起来，几百*几百的，光像素就有好几万，整个图片weight值真的很大。。。
1.对应像素的灰度值再除以255代表该像素的权重，不过大的话也得有好几万
2.取平均，即最后总和/S，范围在0~255
*/
float calculWeight(Bitmap* a){
    int ah=a->h,aw=a->w;
    float sum=0.0f;
    for(int ay=0;ay<ah;ay++){
        for(int ax=0;ax<aw;ax++){
            int value=(*a)[ay][ax];
            sum+=(value&255);  //像素对应的灰度值
        }
    }
    return sum/(ah*aw)/255.0f;
}

/**
将元素target划分成aw(宽)*bh(高)个网格，然后依据网格的中心和weight，加权平均求出该元素的"重心"
*/
void calGravityOfElement(Bitmap*target,int &x_gravity,int &y_gravity,int aw,int bh){
    int centroidX, centroidY, centerX, centerY;
	float weights=0, xWeights=0, yWeights=0,sum,partWeight;
	int width=target->w/aw,height=target->h/bh;
	int area,x_avg,y_avg;
//printf("start\n");
//printf("width:%d height:%d\n",width,height);
    for(int j=0;j<bh;j++){
        for(int i=0;i<aw;i++){
            sum=0.0f;
            area=0;
            for(int p=j*height;p<(j+1)*height && p<target->h;p++){
                for(int k=i*width;k<(i+1)*width && k<target->w;k++){
                    int value=(*target)[p][k];
                    sum+=(value&255);
                    area++;
                }
            }
            partWeight=sum/area/255.0f;
            weights+=partWeight;
            //注意后面x?x:x要用括号括起来！！！否则?前面的都会识别为布尔表达式
            x_avg=i*width+((i<aw-1)?width/2:(target->w-(aw-1)*width)/2);
            y_avg=j*height+((j<bh-1)?height/2:(target->h-(bh-1)*height)/2);
            xWeights+=partWeight*x_avg;
            yWeights+=partWeight*y_avg;
//printf("%d %d: %f %d %d\n",i,j,partWeight,x_avg,y_avg);
        }
    }
//printf("end\n");
    x_gravity = (int) (xWeights/weights);
	y_gravity = (int) (yWeights/weights);
}

/**
对图片pstrImageName进行缩放，按照宽度widthScale和高度heightScale的比例，结果保存为pstrSaveImageName
读取的图片和保存的图片都存放在input文件中。
*/
void zoomImage(char*imageName,char*zoomImageName,float scale){
    //const char *pstrImageName = "scale.jpg";
	//const char *pstrSaveImageName = "scale2.jpg";
	//const char *pstrWindowsSrcTitle = "原图 (http://blog.csdn.net/MoreWindows)";
	//const char *pstrWindowsDstTitle = "缩放图 (http://blog.csdn.net/MoreWindows)";

	//double fScale = 0.314;		//缩放倍数
	char tmpstr[maxn];
	strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
    strcat(tmpstr, imageName);

	CvSize czSize;			    //目标图像尺寸
	//从文件中读取图像
	IplImage *pSrcImage = cvLoadImage(tmpstr, CV_LOAD_IMAGE_UNCHANGED);
	IplImage *pDstImage = NULL;

	//计算目标图像大小
	czSize.width = pSrcImage->width * scale;
	czSize.height = pSrcImage->height * scale;
	//zoomWidth=czSize.width;
	//zoomHeight=czSize.height;
//printf("%d %d  %f\n",czSize.width,czSize.height,scale);

	//创建图像并缩放
	pDstImage = cvCreateImage(czSize, pSrcImage->depth, pSrcImage->nChannels);
	cvResize(pSrcImage, pDstImage, CV_INTER_AREA);

	//保存图片，函数返回后参数zoomImageName也会被修改
	strcpy(zoomImageName,"zoom-");
    strcat(zoomImageName,imageName);
    strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
    strcat(tmpstr,zoomImageName);
	cvSaveImage(tmpstr, pDstImage);

}

/**
初始化海报中的元素
从文本中读取各个元素的图片名称
文本格式如下：
n
xxx.jpg
xxx.jpg
...
n为元素个数
*/
void init(Element*current,Name*elementName,int *kindMark,int sizes,Space titleSpace,Layout layout){
    char imgName[maxn];
    char tmpstr[maxn];
    char greyImgName[maxn];
    float widthScale,heightScale,scale;
    int layoutWidth,layoutHeight;
    float spaceSmallerSizePercentage=0.8f;  //如果元素的宽高均小于对应区域的80%，则进行放大至80%
    float spaceLargerSizePercentage=1.2f;   //如果元素的宽高均大于对应区域的120%，则进行缩小至120%
    int tmp;
    //FILE *f1;
    //f1=fopen(filename,"r");
    //fscanf(f1,"%d",&tmp);  //bannerWidth，这里就不需要了
    //fscanf(f1,"%d",&tmp);  //bannerHeight,这里就不需要了
    //fscanf(f1,"%d",&tmp);  //sizes的话其实在main函数中已经读取用于创建元素数组，这里就不需要了
    for(int i=0;i<sizes;i++){
        //fscanf(f1,"%s %d",imgName,&tmp);
        strcpy(current[i].name,elementName[i].str);
        current[i].flag=kindMark[i];
//printf("%d %s\n",i,imgName);
        strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
        strcat(tmpstr, current[i].name);

        ///对元素进行缩放
        IplImage* img = cvLoadImage(tmpstr);
        CvSize imgSize=cvGetSize(img);
        layoutWidth=BannerWidth*layout.scale[current[i].flag][HORIZONTAL];
        layoutHeight=BannerHeight*layout.scale[current[i].flag][VERTICAL];
printf("%s Size: %d %d %d %d\n",current[i].name,imgSize.width,imgSize.height,layoutWidth,layoutHeight);

        if(imgSize.height<layoutHeight*spaceSmallerSizePercentage&&imgSize.width<layoutWidth*spaceSmallerSizePercentage){
//printf("scale:%f %f\n",layout.scale[current[i].flag][HORIZONTAL],layout.scale[current[i].flag][VERTICAL]);
            widthScale=(float)layoutWidth*spaceSmallerSizePercentage/imgSize.width;
            heightScale=(float)layoutHeight*spaceSmallerSizePercentage/imgSize.height;
            scale=min(widthScale,heightScale);
        }
        else if(imgSize.height>layoutHeight*spaceLargerSizePercentage&&imgSize.width>layoutWidth*spaceLargerSizePercentage){
            widthScale=(float)layoutWidth*spaceLargerSizePercentage/imgSize.width;
            heightScale=(float)layoutHeight*spaceLargerSizePercentage/imgSize.height;
            scale=min(widthScale,heightScale);
        }
        else{
            scale=1.0f;
        }
printf("scale: %f\n",scale);
//printf("zoom:%d %d %d %d %d %f\n",current[i].flag,layoutWidth,layoutHeight,imgSize.width,imgSize.height,scale);
        zoomImage(current[i].name,current[i].zoomName,scale);

        strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
        strcat(tmpstr, current[i].zoomName);
        IplImage* zoomImg=cvLoadImage(tmpstr);
        IplImage* greyImg = cvCreateImage(cvGetSize(zoomImg), IPL_DEPTH_8U, 1);
        cvCvtColor(zoomImg,greyImg,CV_BGR2GRAY);//cvCvtColor(src,des,CV_BGR2GRAY)将彩色图片转换成灰度图

        strcpy(greyImgName,"grey-");
        strcat(greyImgName,current[i].zoomName);
        strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
        strcat(tmpstr,greyImgName);
        cvSaveImage(tmpstr,greyImg);

        strcpy(tmpstr, inputAddr);
        strcat(tmpstr, greyImgName);
        Bitmap *gryImgBit = load_bitmap(tmpstr);


        current[i].width=gryImgBit->w;
        current[i].height=gryImgBit->h;

        if(current[i].flag!=TITLE){
            current[i].x_pos=rand()%(BannerWidth-current[i].width-2*widthBound)+widthBound;
            current[i].y_pos=rand()%(BannerHeight-current[i].height-2*heightBound)+heightBound;
        }
        else{
            current[i].x_pos=rand()%titleSpace.w+titleSpace.x;
            current[i].y_pos=rand()%titleSpace.h+titleSpace.y;
        }
        //int x_gravity,y_gravity;
        calGravityOfElement(gryImgBit,current[i].x_gravity,current[i].y_gravity,GRIDX,GRIDY); //分成GRIDX*GRIDY个网格
        //current[i].x_avg=current[i].x_pos+current[i].width/2;
        //current[i].y_avg=current[i].y_pos+current[i].height/2;
        current[i].x_avg=current[i].x_pos+current[i].x_gravity;
        current[i].y_avg=current[i].y_pos+current[i].y_gravity;
        current[i].alignment=LEFT;
        current[i].weight=calculWeight(gryImgBit);
        current[i].normalGroup=1;

printf("gravity in element: %d %d\nweight:%f x:%d y:%d\n\n",current[i].x_gravity,current[i].y_gravity,current[i].weight,current[i].x_pos,current[i].y_pos);
    }
    //fclose(f1);
}
/**
初始化布局
mark用于标记是水平还是垂直布局

*/
void initLayout(Layout &layout,int mark,int ith,int itv,int ieh,int iev,int teh,int tev){
    layout.imageToTitleHorizontal=ith;
    layout.relatePos[IMAGE][TITLE][HORIZONTAL]=ith%3;
    layout.relatePos[TITLE][IMAGE][HORIZONTAL]=(3-ith)%3;

    layout.imageToTitleVetrical=itv;
    layout.relatePos[IMAGE][TITLE][VERTICAL]=itv%3;
    layout.relatePos[TITLE][IMAGE][VERTICAL]=(3-itv)%3;

    layout.imageToExplainHorizontal=ieh;
    layout.relatePos[IMAGE][EXPLAIN][HORIZONTAL]=ieh%3;
    layout.relatePos[EXPLAIN][IMAGE][HORIZONTAL]=(3-ieh)%3;

    layout.imageToExplainVertical=iev;
    layout.relatePos[IMAGE][EXPLAIN][VERTICAL]=iev%3;
    layout.relatePos[EXPLAIN][IMAGE][VERTICAL]=(3-iev)%3;

    layout.titleToExplainHorizontal=teh;
    layout.relatePos[TITLE][EXPLAIN][HORIZONTAL]=teh%3;
    layout.relatePos[EXPLAIN][TITLE][HORIZONTAL]=(3-teh)%3;

    layout.titleToExplainVertical=tev;
    layout.relatePos[TITLE][EXPLAIN][VERTICAL]=tev%3;
    layout.relatePos[EXPLAIN][TITLE][VERTICAL]=(3-tev)%3;

    if(mark==HORIZONTAL){
        layout.scale[TITLE][HORIZONTAL]=TitleSpaceWidthScaleHor;
        layout.scale[TITLE][VERTICAL]=TitleSpaceHeightScaleHor;

        layout.scale[IMAGE][HORIZONTAL]=ImageSpaceWidthScaleHor;
        layout.scale[IMAGE][VERTICAL]=ImageSpaceHeightScaleHor;

        layout.scale[EXPLAIN][HORIZONTAL]=ExplainSpaceWidthScaleHor;
        layout.scale[EXPLAIN][VERTICAL]=ExplainSpaceHeightScaleHor;
    }
    else if(mark==VERTICAL){
        layout.scale[TITLE][HORIZONTAL]=TitleSpaceWidthScaleVer;
        layout.scale[TITLE][VERTICAL]=TitleSpaceHeightScaleVer;

        layout.scale[IMAGE][HORIZONTAL]=ImageSpaceWidthScaleVer;
        layout.scale[IMAGE][VERTICAL]=ImageSpaceHeightScaleVer;

        layout.scale[EXPLAIN][HORIZONTAL]=ExplainSpaceWidthScaleVer;
        layout.scale[EXPLAIN][VERTICAL]=ExplainSpaceHeightScaleVer;
    }
}
/**
用bitmap的方式将图片b写入到a中去
x、y为b在a中的起始位置
*/
void putPicture(Bitmap *&a, Bitmap *b, int x,int y){
    int ah=a->h,aw=a->w;
    int bh=b->h,bw=b->w;
int cnt=0;
    for(int by=0;by<bh;by++){
        for(int bx=0;bx<bw;bx++){
if(y+by>=ah || x+bx>=aw){
    cnt++;
}
else
            (*a)[y+by][x+bx]=(*b)[by][bx];
        }
    }

printf("cnt:%d\n",cnt);
}



void autocreatBanner(Name*elementName,int *kindMark,float &balanceCost,float &alignCost,float &totalcost){
    char filename1[maxn]="inputHor.txt",filename2[maxn]="inputVer.txt";
    char tmpstr[maxn];
    int sizes=3;
    //int layoutStyle=HORIZONTAL;
    //int layoutStyle=VERTICAL;
    Layout horLayout,verLayout; //水平布局，垂直布局

    Element currentSolution[sizes];
    Element bestSolution[sizes];
    Element candidateSolution[sizes];

	Space whiteSpace,originalWhiteSpace,interestBox,titleSpace,imageSpace,explainSpace;
	int designType, orientation[4], keepChecking=TRUE;
	float tEnd= 10.0, t= 300.0, delta, calc, random;
	//char result[MAX];
    heightBound=BannerHeight*boundScale;
    widthBound=BannerWidth*boundScale;
    /**
    一开始的space宽高各取整个广告布局的一半，位于广告布局的中央，在生产过程中则慢慢向四周扩大

	whiteSpace.x=BannerWidth/4; whiteSpace.y=BannerHeight/4;
	whiteSpace.w=BannerWidth/2; whiteSpace.h=BannerHeight/2;
    */
    whiteSpace.x=widthBound; whiteSpace.y=heightBound;
	whiteSpace.w=BannerWidth-2*widthBound; whiteSpace.h=BannerHeight-2*heightBound;

    titleSpace.x=widthBound;titleSpace.y=heightBound;
    if(layoutStyle==HORIZONTAL){
        titleSpace.w=BannerWidth*TitleSpaceWidthScaleHor;titleSpace.h=BannerHeight*TitleSpaceHeightScaleHor;  //水平布局1
    }
    else if(layoutStyle==VERTICAL){
        titleSpace.w=BannerWidth*TitleSpaceWidthScaleVer;titleSpace.h=BannerHeight*TitleSpaceHeightScaleVer;   //垂直布局1
    }
    else{
        titleSpace.w=BannerWidth*user_defined_layout.scale[TITLE][HORIZONTAL];  //自定义布局1
        titleSpace.h=BannerHeight*user_defined_layout.scale[TITLE][VERTICAL];
    }
    //imageSpace.w=bannerwidth*ImageSpaceWidthScaleHor;imageSpace.h=BannerHeight*ImageSpaceHeightScaleHor;
    //imageSpace.w=BannerWidth*ImageSpaceWidthScaleVer;imageSpace.h=BannerHeight*ImageSpaceHeightScaleVer;

    //explainSpace.w=BannerWidth*ExplainSpaceWidthScaleHor;explainSpace.h=BannerHeight*ExplainSpaceHeightScaleHor;
    //explainSpace.w=BannerWidth*ExplainSpaceWidthScaleVer;explainSpace.h=BannerHeight*ExplainSpaceHeightScaleVer;

	//interestBox.x=atoi(argv[5]); interestBox.y=atoi(argv[6]);
	//interestBox.w=atoi(argv[7]); interestBox.h=atoi(argv[8]);

	designType = PLAIN;
	orientation[0]=orientation[1]=orientation[2]=orientation[3]=HORIZONTAL;

    srand(time(NULL)); //srand用一次就可以

    initLayout(horLayout,HORIZONTAL,HORIZONTALRIGHT,NOMATTER,HORIZONTALRIGHT,NOMATTER,NOMATTER,VERTICALUP);
    initLayout(verLayout,VERTICAL,NOMATTER,VERTICALBOTTOM,NOMATTER,VERTICALBOTTOM,NOMATTER,VERTICALUP);
    if(layoutStyle==HORIZONTAL){
        init(currentSolution,elementName,kindMark,sizes,titleSpace,horLayout);  //水平布局2
    }
    else if(layoutStyle==VERTICAL){
        init(currentSolution,elementName,kindMark,sizes,titleSpace,verLayout);  //垂直布局2
    }
    else{
        init(currentSolution,elementName,kindMark,sizes,titleSpace,user_defined_layout);  //自定义布局2
    }
    ///The code above is executed successfully!!!
	originalWhiteSpace = whiteSpace;

	/*****Simulated Annealing*****/
	memcpy(&bestSolution, &currentSolution, sizeof(currentSolution));
printf("the initial cost:%f\n",cost(currentSolution, whiteSpace, interestBox, sizes, designType));

	while(t >= tEnd){
		if((int)t%75 == 0 && t!=300.0 && designType==PLAIN && keepChecking==TRUE){
			//检查是否存在元素的一部分或者整个在space外面，若存在的话，则得扩大space
//printf("sadasd  ");
			keepChecking = checkSpace(bestSolution, &whiteSpace, orientation, sizes);
//printf("%d\n",orientation);
		}
		for(int i = 0; i < l(t); i++){

			memcpy(&candidateSolution, &currentSolution, sizeof(currentSolution));
			if(layoutStyle==HORIZONTAL){
                chooseNeighbour(candidateSolution, whiteSpace, titleSpace, sizes, t, horLayout);  //水平布局3
			}
			else if(layoutStyle==VERTICAL){
                chooseNeighbour(candidateSolution, whiteSpace, titleSpace, sizes, t, verLayout);  //垂直布局3
			}
			else{
                chooseNeighbour(candidateSolution, whiteSpace, titleSpace, sizes, t, user_defined_layout);  //自定义布局3
			}
			//元素移动后的cost减去之前的cost
			float currentCost=cost(currentSolution, whiteSpace, interestBox, sizes, designType);
			float candidateCost=cost(candidateSolution, whiteSpace, interestBox, sizes, designType);

///printf("%d %f %f ",i,candidateCost,currentCost); //为啥afterCost都是10000。。。
			delta = candidateCost - currentCost;
			/*
			exp(-(delta/t))随着delta的增大而减小
			当delta<0，calc是>1的。
			*/
			calc = exp(-(delta)/t);
			//random = drand48(); //产生一个double型的随机数
			random=rand()*1.0f/(RAND_MAX); //32767
			//如果改变后的solution更好，则覆盖之前的
			if(random<calc || delta <0)
				memcpy(&currentSolution, &candidateSolution, sizeof(currentSolution));
			//更新目前最好的solution
            float bestCost=cost(bestSolution, whiteSpace, interestBox, sizes, designType);
///printf("%f\n",bestCost);
			if(currentCost < bestCost)
				memcpy(&bestSolution, &currentSolution, sizeof(currentSolution));
		}
		//alpha(&t); //实际就是t=t-1，搞那么复杂干嘛
		t--;
	}

	/*Printing the output. This will be get by the main process*/
printf("size:%d\n",sizes);
	for(int i=0;i<sizes;i++){
		if(bestSolution[i].normalGroup == 1){
			printf("%d %d %d %d\n", bestSolution[i].x_pos,bestSolution[i].y_pos,bestSolution[i].x_avg,bestSolution[i].y_avg);
		}
	}

	/*Cost*/
	printf("cost:\t%f\nBalanceCost:\t%f\nAlignmentCost:\t%f\n",cost(bestSolution, whiteSpace, interestBox, sizes, designType),BannerBalanceCost,BannerAlignmentCost);
	/*Size modifications*/
	printf("size modifications--width:%d ", whiteSpace.w - originalWhiteSpace.w); 	/*Width*/
	printf("height:%d \n", whiteSpace.h - originalWhiteSpace.h); 	/*Height*/

    printfCentroid(bestSolution,whiteSpace,sizes);
    //创建白色的背景图
    CvSize bgSize;
    bgSize.width=BannerWidth;
    bgSize.height=BannerHeight;
    IplImage* bgImg = cvCreateImage(bgSize, IPL_DEPTH_8U, 1);
    cvSet(bgImg,CV_RGB(255,255,255),0);
    cvSaveImage("input/background.jpg",bgImg);

    //将元素一个个写入到背景图里面
    Bitmap *bg=load_bitmap("input/background.jpg");
    for(int i=0;i<sizes;i++){
printf("%s %d %d\n",bestSolution[i].zoomName,bestSolution[i].x_pos,bestSolution[i].y_pos);
        strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
        strcat(tmpstr, bestSolution[i].zoomName);
        Bitmap *b = load_bitmap(tmpstr);
        putPicture(bg,b,bestSolution[i].x_pos,bestSolution[i].y_pos);
    }

    totalcost=cost(bestSolution, whiteSpace, interestBox, sizes, designType);
    balanceCost=BannerBalanceCost;
    alignCost=BannerAlignmentCost;
    char resultName[maxn]="result.jpg";
    char horizontalName[maxn]="horizontal-";
    char verticalName[maxn]="vertical-";
    char tmpName[maxn];
    if(layoutStyle==HORIZONTAL){
        strcpy(tmpName,horizontalName);
        strcat(tmpName,resultName);
        strcpy(resultName,tmpName);
        save_bitmap(bg,tmpName,OutputAddr);

        FILE*f2;
        f2=fopen("output/HorResult.txt","w");
        fprintf(f2,"cost:\t%f\nBalanceCost:\t%f\nAlignmentCost:\t%f\n",cost(bestSolution, whiteSpace, interestBox, sizes, designType),BannerBalanceCost,BannerAlignmentCost);
        fclose(f2);
    }
    else{
        strcpy(tmpName,verticalName);
        strcat(tmpName,resultName);
        strcpy(resultName,tmpName);
        save_bitmap(bg,tmpName,OutputAddr);

        FILE*f2;
        f2=fopen("output/VerResult.txt","w");
        fprintf(f2,"cost:\t%f\nBalanceCost:\t%f\nAlignmentCost:\t%f\n",cost(bestSolution, whiteSpace, interestBox, sizes, designType),BannerBalanceCost,BannerAlignmentCost);
        fclose(f2);
    }
    strcpy(tmpName,"output/");
    strcat(tmpName,resultName);
    IplImage* imgResult = cvLoadImage(tmpName);
    cvShowImage("result",imgResult);

}


//-----------------------------------下面源自gaudii的代码-----------------------------------

/*
Cheking function
检查是否存在元素的一部分或者整个在space外面
是的话返回true，否则false
根据各元素竖直or水平超出space的个数，来对应的扩大space的高or宽
*/
int checkSpace(Element *solution, Space *space, int *orientation, int size){
	int i=0, isOut[size], nOut=0;
	float factor, growx=0,growy=0;

	for(i = 0; i < size; i++){
		if(checkBorders(solution[i], space, size,orientation[i])==TRUE){
			nOut++;
			isOut[i]=TRUE;
		}
		else
			isOut[i]=FALSE;
	}

//printf("%d\n",nOut);
	/*
	下面就不懂了额额,按道理超出space的元素越多增长也应该越大呀？？？
	明白了，因为下面for循环，每超出一次，space就增加一次。
	所以当超出元素多了的话，每次增长的幅度设置的就小一点
	*/
	if(nOut == 1)
		factor = BIG_GROWTH; //0.3
	else if(nOut == 2)
		factor = MEDIUM_GROWTH; //0.25
	else if(nOut >= 3)
		factor = SMALL_GROWTH; //0.2

	for(i=0;i<size;i++){
		if(isOut[i]==TRUE){
            if(orientation[i]==VERTICAL){
                growy+=solution[i].height*factor;
            }
            else{
                growx+=solution[i].width*factor;
            }
		}
			//grow+= orientation[i]==VERTICAL? solution[i].height*factor : solution[i].width*factor;
	}
	growy += (int)growy%20==0?0: 20-(int)growy%20;
	growx += (int)growx%20==0?0: 20-(int)growx%20;
	//用于扩大space的面积,
	if(space->h+growy<BannerHeight){
            space->h+=(int)growy;
//printf("%d ",space->y);
            space->y-=(int)(growy/2);
//printf("%f y:%d\n",growy,space->y);
    }

	if(space->w+growx<BannerWidth){
            space->w+=(int)growx;
//printf("%d ",space->x);
            space->x-=(int)(growx/2);
//printf("%f x:%d\n",growx,space->x);
    }

	if(nOut!=0){
		return TRUE;
	}
	else
		return FALSE;
}
/*
判断group是否完全包含在space内
包含，isOut=false
否则，isOut=true
增加了参数orientation，标记space该往哪个方向扩展
*/
int checkBorders(Element g, Space *space, int size,int &orientation){
	int i, isOut=FALSE;

	if(g.x_pos < space->x ||
		g.x_pos + g.width > space->x + space->w){
			isOut=TRUE;
			orientation=HORIZONTAL;//用以标记水平方向超出了space的空间，space需要水平方向的扩展
	}
	if(g.y_pos < space->y ||
		g.y_pos + g.height > space->y + space->h ){
			isOut=TRUE;
			orientation=VERTICAL;//用以标记垂直方向超出了space的空间，space需要垂直方向的扩展
	}
	return isOut;
}

/*Cost function. It returns the final cost*/
float cost(Element *candidate, Space space, Space interestBox, int size, int designType){
	int overlapCost;
	float balanceCost, alignmentCost;

	/*Overlap Cost*/
	overlapCost = getOverlapCost(candidate, size);

	/*If it's overlapping*/
	if(overlapCost == 1){
		return VBN; //10000   存在重合，则cost为很大的值。
	}
	/*If not...*/
	else{
		/*Balance Cost*/
		BannerBalanceCost=balanceCost=getBalanceCost(candidate, space, size);
		/*Alignment Cost*/
		BannerAlignmentCost=alignmentCost = getAlignmentCost(candidate, space, interestBox, size, designType);
//printf("bal:%f ali:%f\n",balanceCost,alignmentCost);
		return ((balanceCost*BALANCE_PERCENTAGE*BALANCE_COEFFICIENT)+(alignmentCost*ALIGNMENT_PERCENTAGE*ALIGNMENT_COEFFICIENT));
	}
}

/*
Alignment cost
what is interestBox???
是海报中最受关注的部分么？
*/
float getAlignmentCost(Element *candidate, Space space, Space interestBox, int size, int designType){
	int xAligns[size], yAligns[size], x2Aligns[size], y2Aligns[size];
	int i, j;
	float totalCost=0.0f, individualCost, normCost; //totalCost源代码中都没有初始化。。。
	int xCheck, x2Check, yCheck, y2Check, marginLeft, marginRight, marginBottom, marginTop;
	int interestPointLeft, interestPointRight, interestPointUp, interestPointDown;

	//以各个元素的四条边所在的线作为对齐线
	for(i=0;i<size;i++){
		xAligns[i]= candidate[i].x_pos;
		yAligns[i]= candidate[i].y_pos;
		x2Aligns[i]= candidate[i].x_pos+candidate[i].width;
		y2Aligns[i]= candidate[i].y_pos+candidate[i].height;
	}

	for(i=0;i<size;i++){
		xCheck=0; x2Check=0; yCheck=0; y2Check=0;
		marginLeft=0; marginRight=0; marginBottom=0; marginTop=0;
		interestPointLeft=0; interestPointRight=0; interestPointDown=0; interestPointUp=0;

		for(j=0;j<size;j++){
			if(i!=j && candidate[i].normalGroup==1 && candidate[j].normalGroup==1){
				/*X2 Y2 check*/
				if(x2Aligns[i]==x2Aligns[j]){
					x2Check=1;
					candidate[i].alignment= 0;	//应该是RIGHT，即右对齐
				}
				else if(y2Aligns[i]==y2Aligns[j])
					y2Check=1;

				/*X Y check*/
				if(xAligns[i]==xAligns[j]){
					xCheck=1;
					candidate[i].alignment= 1;
				}
				else if(yAligns[i]==yAligns[j])
					yCheck=1;

				/*Margins check */
				if(xAligns[i]==space.x){
					marginLeft=1;
					candidate[i].alignment= 1;
				}
				else if(x2Aligns[i]==space.x+space.w){
					marginRight=1;
					candidate[i].alignment= 0;
				}
				else if(yAligns[i]==space.y){
					marginTop=1;
				}
				else if(y2Aligns[i]==space.y+space.h){
					marginBottom=1;
				}
				/*
				Interest points check
				???
				*/
				if(xAligns[i]==interestBox.x && designType==BACKGROUND){
					interestPointLeft=1;
					candidate[i].alignment= 1;
				}
				else if(x2Aligns[i]==interestBox.x+interestBox.w  && designType==BACKGROUND){
					interestPointRight=1;
					candidate[i].alignment= 0;
				}
				else if(yAligns[i]==interestBox.y && designType==BACKGROUND)
					interestPointUp=1;
				else if(y2Aligns[i]==interestBox.y+interestBox.h  && designType==BACKGROUND)
					interestPointDown=1;

			}//if
		}//for

		individualCost = ALIGNMENT_COST_PER_GROUP;

		if(((xCheck || x2Check) && (marginLeft || marginRight)) && ((yCheck || y2Check) && (marginTop || marginBottom)))
			individualCost -= ALIGNMENT_COMPLETE_GREAT;
		else if( (marginLeft || marginRight) && (marginTop || marginBottom) )
			individualCost -= ALIGNMENT_COMPLETE_GOOD;
		else if( (xCheck || x2Check) && (yCheck || y2Check) )
			individualCost -= ALIGNMENT_COMPLETE_POOR;
		else if( (xCheck || x2Check || marginLeft || marginRight) || (yCheck || y2Check || marginTop || marginBottom) )
			individualCost -= ALIGNMENT_HALFWAY;

		if(marginLeft || marginRight)
			individualCost -= ALIGNMENT_MARGINS;
		else if(interestPointDown || interestPointUp || interestPointRight || interestPointLeft)
			individualCost -= ALIGNMENT_INTEREST;

		totalCost += individualCost>0? individualCost:0;
	}//for

	normCost = totalCost/(ALIGNMENT_COST_PER_GROUP*size);

	return normCost;

}

/*
Balance cost

*/
float getBalanceCost(Element *candidate, Space space, int size){
	int i, centroidX, centroidY, centerX, centerY;
	float weights=0, xWeights=0, yWeights=0, maxDistance, distance;
	for(i=0;i<size;i++){
		weights+=candidate[i].weight;
		xWeights+= candidate[i].weight*candidate[i].x_avg;  //what is x_avg and y_avg? And why do they calculate as this？？？
		yWeights+= candidate[i].weight*candidate[i].y_avg;
	}

	centroidX = (int) (xWeights/weights);
	centroidY = (int) (yWeights/weights);
    ///算出来的基本上“重心坐标”的值都在个位数。。。
//printf("%f %f %f %d %d\n",weights,xWeights,yWeights,centroidX,centroidY);
//printf("%d %d\n",centroidX,centroidY);
	centerX = space.w/2;
	centerY = space.h/2;

	distance = sqrt(pow(centroidX-centerX, 2) + pow(centroidY-centerY, 2));
    ///我觉得应该是对角线的一半，因为任一点到中心的最大距离就是对角线的一半
	maxDistance = sqrt(pow(centerX, 2) + pow(centerY, 2));
//printf("%f %f\n",distance,maxDistance);
	return distance/maxDistance;
}

/*
Balance cost

*/
void printfCentroid(Element *candidate, Space space, int size){
	int i, centroidX, centroidY, centerX, centerY;
	float weights=0, xWeights=0, yWeights=0, maxDistance, distance;
	for(i=0;i<size;i++){
		weights+=candidate[i].weight;
		xWeights+= candidate[i].weight+candidate[i].x_avg;  //what is x_avg and y_avg? And why do they calculate as this？？？
		yWeights+= candidate[i].weight+candidate[i].y_avg;
	}

	centroidX = (int) (xWeights/weights);
	centroidY = (int) (yWeights/weights);
    ///算出来的基本上“重心坐标”的值都在个位数。。。
//printf("%f %f %f %d %d\n",weights,xWeights,yWeights,centroidX,centroidY);
printf("%f %f %f\n",weights,xWeights,yWeights);
printf("重心坐标：%d %d\n",centroidX,centroidY);
	centerX = space.w/2;
	centerY = space.h/2;

	distance = sqrt(pow(centroidX-centerX, 2) + pow(centroidY-centerY, 2));
    ///我觉得应该是对角线的一半，因为任一点到中心的最大距离就是对角线的一半
	maxDistance = sqrt(pow(centerX, 2) + pow(centerY, 2));
//printf("%f %f\n",distance,maxDistance);
	//printf("%f %f %f\n",distance,maxDistance,distance/maxDistance);
}
/*
Overlap cost
判断是否存在两个物体有重合，
是的话返回1，否则0.
*/
int getOverlapCost(Element *candidate, int size){
	int i, j;
	int somethingWrong=0;

	/*For every two groups, it checks if there is overlapping*/
	for(i = 0; i < size && somethingWrong == 0; i++)
		for(j = 0; j < size && somethingWrong == 0; j++)
			if(i!=j)
				somethingWrong = isOverlapping(candidate[i], candidate[j]);

	return somethingWrong;
}

/*
判断两物体是否重合
*/
int isOverlapping(Element one, Element two){
	int xCheck, xCheck1, xCheck2, xCheck3, xCheck4;
	int yCheck, yCheck1, yCheck2, yCheck3, yCheck4;
	int final;

	/*X checks*/
	xCheck1 = (one.x_pos >= two.x_pos && one.x_pos <= two.x_pos + two.width)?1:0;
	xCheck2 = (one.x_pos+one.width >= two.x_pos && one.x_pos+one.width <= two.x_pos + two.width)?1:0;
	xCheck3 = (two.x_pos >= one.x_pos && two.x_pos<=one.x_pos+one.width)?1:0;
	xCheck4 = (two.x_pos+two.width >= one.x_pos && two.x_pos+two.width<=one.x_pos+one.width)?1:0;

	xCheck = xCheck1 + xCheck2 + xCheck3 + xCheck4 > 0?1:0;

	/*Y checks*/
	yCheck1 = (one.y_pos >= two.y_pos && one.y_pos <= two.y_pos + two.height)?1:0;
	yCheck2 = (one.y_pos+one.height >= two.y_pos && one.y_pos+one.height <= two.y_pos + two.height)?1:0;
	yCheck3 = (two.y_pos >= one.y_pos && two.y_pos<=one.y_pos+one.height)?1:0;
	yCheck4 = (two.y_pos+two.height >= one.y_pos && two.y_pos+two.height<=one.y_pos+one.height)?1:0;

	yCheck = yCheck1 + yCheck2 + yCheck3 + yCheck4 > 0?1:0;

	final = xCheck + yCheck > 1?1:0;

	return final;
}


/*
Neighbour fuction
对元素进行随机的移动
*/
void chooseNeighbour(Element *candidate, Space space, Space titleSpace,int size, float t,Layout layout){
	int i, widthSteps, heightSteps, xJump, yJump;
	int xmin,xmax,ymin,ymax;  //整个元素能够放置的范围，而不仅仅是元素的左上角
	int x_pos2, y_pos2, test;
	int kind1,kind2;
	bool titleLimited=false; //若为true，则表示title的左上角限定在titleSpace里面
	//srand(system(NULL));
	for(i=0;i<size;i++){
		if(candidate[i].normalGroup == 1){
            if(candidate[i].flag==TITLE){
                widthSteps = (titleSpace.w)/20;
                heightSteps = (titleSpace.h)/20;
            }
            else{
                widthSteps = (space.w - candidate[i].width-2*widthBound)/20;
                heightSteps = (space.h - candidate[i].height-2*heightBound)/20;
            }
int xrand=rand();
int yrand=rand();
			xJump = widthSteps==0?0:(xrand%widthSteps)*20;
			yJump = heightSteps==0?0:(yrand%heightSteps)*20;
//printf("%d %d    ",xJump,yJump);
			/*Long distances in case Temperature is high; low distances in case it isnt*/
			xJump = (int) (xJump*(t/300.0));
			yJump = (int) (xJump*(t/300.0));
//printf("%d %d    ",xJump,yJump);
			/*Checking the jump is multiple of 20*/
			xJump -= xJump%20!=0?xJump%20:0;
			yJump -= yJump%20!=0?yJump%20:0;

			/*Random negative jumps*/
			xJump -= rand()%2==0?0:xJump*2;
			yJump -= rand()%2==0?0:yJump*2;

//printf("%d %d\n",xJump,yJump);
			/*Now we move the group with that jumping distance*/
			candidate[i].x_pos += xJump;
			candidate[i].y_pos += yJump;

			/*
			We run some checks on the calculations
			下面6行保证元素在space里面，这里有个前提，就是元素的宽度、高度肯定是小于space的
			*/
			x_pos2 = candidate[i].x_pos + candidate[i].width;
			y_pos2 = candidate[i].y_pos + candidate[i].height;

            xmin=space.x;ymin=space.y;
            xmax=space.x+space.w;
            ymax=space.y+space.h;
//printf("%d %d %d %d\n",xmin,ymin,xmax,ymax);
            kind1=candidate[i].flag;
            if(kind1==TITLE && titleLimited){
                xmin=titleSpace.x;
                ymin=titleSpace.y;
                xmax=xmin+titleSpace.w+candidate[i].weight;
                ymax=ymin+titleSpace.h+candidate[i].height;
            }
            else{
                for(int k=0;k<size;k++){
                    if(k!=i){
                        kind2=candidate[k].flag;
//printf("kind:%d %d\n",kind1,kind2);
//printf("%d %d %d\n",kind2,candidate[k].x_pos,candidate[k].y_pos);
                        if(layout.relatePos[kind1][kind2][HORIZONTAL]==HORIZONTALLEFT){
//printf("1.%d %d\n",xmax,candidate[k].x_pos);
                            xmax=min(max(space.x,candidate[k].x_pos-(int)(BannerWidth*GAPSCALE)),xmax);
                        }
                        else if(layout.relatePos[kind1][kind2][HORIZONTAL]==HORIZONTALRIGHT){
//printf("2.%d %d\n",xmin,candidate[k].x_pos+candidate[k].width);
                            xmin=max(min(space.x+space.w,candidate[k].x_pos+candidate[k].width+(int)(BannerWidth*GAPSCALE)),xmin);
                        }
                        if(layout.relatePos[kind1][kind2][VERTICAL]==VERTICALUP){
//printf("3.%d %d\n",ymax,candidate[k].y_pos);
                            ymax=min(max(space.y,candidate[k].y_pos-(int)(BannerHeight*GAPSCALE)),ymax);
                        }
                        else if(layout.relatePos[kind1][kind2][VERTICAL]==VERTICALBOTTOM){
//printf("4.%d %d\n",ymin,candidate[k].y_pos+candidate[k].height);
                            ymin=max(min(space.y+space.h,candidate[k].y_pos+candidate[k].height+(int)(BannerHeight*GAPSCALE)),ymin);
                        }
                    }
                }
///为啥都是31,21,40,40
//printf("%d %d %d %d\n",xmin,ymin,xmax,ymax);
            }
//printf("space:%d %d %d %d\n",space.x,space.y,space.w,space.h);
//printf("%d %d %d %d %d\n",candidate[i].flag,xmin,ymin,xmax,ymax); //why xmax<0,ymax<0 ??? because x_pos,y_pos <0
            /*
			candidate[i].x_pos = candidate[i].x_pos<space.x?space.x:candidate[i].x_pos;
			candidate[i].y_pos = candidate[i].y_pos<space.y?space.y:candidate[i].y_pos;

			candidate[i].x_pos = x_pos2 > space.x + space.w?(space.x+space.w)-candidate[i].width:candidate[i].x_pos;
			candidate[i].y_pos = y_pos2 > space.y + space.h?(space.y+space.h)-candidate[i].height:candidate[i].y_pos;

			//保证坐标值是20的倍数。
			//那这样如果之前设定为space.x，再减不就在space外了么？那只有可能space.x是20的倍数，所以即使减也是0
			candidate[i].x_pos -= candidate[i].x_pos%20!=0?candidate[i].x_pos%20:0;
			candidate[i].y_pos -= candidate[i].y_pos%20!=0?candidate[i].y_pos%20:0;
            */

            candidate[i].x_pos = candidate[i].x_pos<xmin?xmin:candidate[i].x_pos;
			candidate[i].y_pos = candidate[i].y_pos<ymin?ymin:candidate[i].y_pos;

            //xmax、ymax可能会小于candidate[i].width、candidate[i].height!!!
			candidate[i].x_pos = x_pos2 > xmax?(xmax-candidate[i].width<xmin?xmin:xmax-candidate[i].width):candidate[i].x_pos;
			candidate[i].y_pos = y_pos2 > ymax?(ymax-candidate[i].height<ymin?ymin:ymax-candidate[i].height):candidate[i].y_pos;

			//保证坐标值是20的倍数。
			//那这样如果之前设定为space.x，再减不就在space外了么？那只有可能space.x是20的倍数，所以即使减也是0
			int xremainder=candidate[i].x_pos%20;
			int yremainder=candidate[i].y_pos%20;

			if(xremainder!=0){
                if(candidate[i].x_pos-xremainder<xmin)
                    candidate[i].x_pos+=20-xremainder;
                else
                    candidate[i].x_pos-=xremainder;
			}
			if(yremainder!=0){
                if(candidate[i].y_pos-yremainder<ymin)
                    candidate[i].y_pos+=20-yremainder;
                else
                    candidate[i].y_pos-=yremainder;
			}
			//candidate[i].x_pos -= candidate[i].x_pos%20!=0?candidate[i].x_pos%20:0;
			//candidate[i].y_pos -= candidate[i].y_pos%20!=0?candidate[i].y_pos%20:0;
//printf("%d %d\n",candidate[i].x_pos,candidate[i].y_pos);
			/*We too calculate the average point*/
			///!!!3.0计算元素重心的时候忘记更改这里了啊啊啊
			//candidate[i].x_avg = candidate[i].x_pos + candidate[i].width/2;
			//candidate[i].y_avg = candidate[i].y_pos + candidate[i].height/2;
			candidate[i].x_avg = candidate[i].x_pos + candidate[i].x_gravity;
			candidate[i].y_avg = candidate[i].y_pos + candidate[i].y_gravity;

		}
	}
}

void alpha(float *t){
	*t=*t-1;
}

int l(int t){
	int k= 100, times;
	times = k - (((k - t)/10)*5);
	return times;
}

char * itoa (int i){
	char str_val [maxn] ;

	snprintf (str_val, sizeof (str_val), "%d", i) ;

	return strdup (str_val) ;
}

