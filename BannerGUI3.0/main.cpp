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
#define BALANCE_COEFFICIENT 1.0   //��cost����ʱ���Ŵ�ı���ϵ��
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
����ʵ���˼򵥵�GUI��ͼƬ�����⡢˵�����־��������ļ�ѡ��Ի���ѡ�񣬵�Ȼ��Щ��Ҫ����ͼƬ��ʽ
����ѡ�����������
Ȼ�󵥻���ʼ��ť����
BannerGUI2.0:
�û�����ѡ��Ĭ�ϰ�ʽ�������Զ����ʽ
Ĭ�ϰ�ʽ������ͼƬ�ο�
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
char szFileNameImage[MAX_PATH] = "";  //�洢�û�ѡ����ļ�·��
char szFileNameTitle[MAX_PATH] = "";  //�洢�û�ѡ����ļ�·��
char szFileNameExplain[MAX_PATH] = "";  //�洢�û�ѡ����ļ�·��
int layoutDefaultOrDefined=DEFAULT_LAYOUT;
int layoutStyle=HORIZONTAL;
int BannerHeight=420;
int BannerWidth=630;

bool fileChosen[3]={false,false,false};
const int maxn=100;
char inputAddr[50] = "input/"; //�������Ե�ַ
char imageName[maxn]="image.jpg";
char titleName[maxn]="title.jpg";
char explainName[maxn]="explain.jpg";

char currentDirectory[maxn];

struct Name{
    char str[maxn];
};
struct Layout{
    /*
    ���λ�þ���
    relatePos[i][j][0]��ʾi��j��ˮƽ��������λ��
    relatePos[i][j][1]��ʾi��j����ֱ��������λ��
    */
    int relatePos[KIND][KIND][2];
    int imageToTitleHorizontal; //[0]=1,��ʾimage��title��ߣ�2��ʾ���ұ�,0��ʾû��Ҫ��
    int imageToTitleVetrical; //[0]=1,��ʾimage��title������2��ʾ�ڵײ�,0��ʾû��Ҫ��
    int imageToExplainHorizontal; //[0]=1,��ʾimage��explain��ߣ�2��ʾ���ұ�,0��ʾû��Ҫ��
    int imageToExplainVertical; //[0]=1,��ʾimage��explain������2��ʾ�ڵײ�,0��ʾû��Ҫ��
    int titleToExplainHorizontal; //[0]=1,��ʾtitle��explain��ߣ�2��ʾ���ұ�,0��ʾû��Ҫ��
    int titleToExplainVertical; //[0]=1,��ʾtitle��explain������2��ʾ�ڵײ�,0��ʾû��Ҫ��
    float scale[3][2]; //[kind][HORIZONTAL]��ʾˮƽ����ı�����[kind][VERTICAL]��ʾ��ֱ���ߵı���
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
/*���ڻص�������һ�����¼���Ӧ��ϵͳ�ͻ���øú���*/
BOOL CALLBACK DlgMain(HWND hwndDlg,/*���ھ����32λ�޷�������*/
                      UINT uMsg,/*��Ϣ����*/
                      WPARAM wParam, /*wParam��lParam�������Ǿ�����¼���Ϣ*/
                      LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:/*���ڱ���ʼ��ʱ��������ʱ��*/
    {
        getcwd(currentDirectory, MAX_PATH); //��ȡ��ǰ���ڵ�·����������
        buttonImage=GetDlgItem(hwndDlg,IDC_BUTTON1);
        buttonTitle=GetDlgItem(hwndDlg,IDC_BUTTON2);
        buttonExplain=GetDlgItem(hwndDlg,IDC_BUTTON3);
        radioHorizontal=GetDlgItem(hwndDlg,IDC_RADIOHORIZONTAL);
        SendMessage(radioHorizontal, BM_SETCHECK, BST_CHECKED,0); //ʹradioHorizontalһ��ʼ��Ĭ��ѡ��
        radioVertical=GetDlgItem(hwndDlg,IDC_RADIOVERTICAL);
        //EnableWindow(radioVertical,false);

        radioDefaultLayout=GetDlgItem(hwndDlg,IDC_RADIODEFAULTLAYOUT);
        radioUserLayout=GetDlgItem(hwndDlg,IDC_RADIOUSERLAYOUT);
        SendMessage(radioDefaultLayout, BM_SETCHECK, BST_CHECKED,0); //ʹradioHorizontalһ��ʼ��Ĭ��ѡ��

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

    case WM_CLOSE: /*���ڱ��ر�ʱ��ʱ�䣬������ˡ�X��*/
    {
        if(MessageBoxA(hwndDlg,"�رմ��ڣ�","��ʾ",MB_YESNO)==IDYES){
            EndDialog(hwndDlg, 0);
        }

    }
    return TRUE;

    case WM_COMMAND:/*�����¼������Ǵ�����ť���¼�*/
    {
        switch(LOWORD(wParam))
        {
            case IDC_BUTTON1:{
                printf("��ť�����\n");
                SetWindowText(buttonImage,TEXT("Has chosen"));
                if(GetOpenFileName(&ofnImage))
                {
                // Do something usefull with the filename stored in szFileName
                    cout<<szFileNameImage<<endl;  //�ڿ���̨����ʾѡ���ļ���·��
                }
                fileChosen[0]=true;
                break;
            }
            case IDC_BUTTON2:{
                printf("��ť�����\n");
                SetWindowText(buttonTitle,TEXT("Has chosen"));
                if(GetOpenFileName(&ofnTitle))
                {
                // Do something usefull with the filename stored in szFileName
                    cout<<szFileNameTitle<<endl;  //�ڿ���̨����ʾѡ���ļ���·��
                }
                fileChosen[1]=true;
                break;
            }
            case IDC_BUTTON3:{
                printf("��ť�����\n");
                SetWindowText(buttonExplain,TEXT("Has chosen"));
                if(GetOpenFileName(&ofnExplain))
                {
                // Do something usefull with the filename stored in szFileName
                    cout<<szFileNameExplain<<endl;  //�ڿ���̨����ʾѡ���ļ���·��
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
            ���ڲ��Ե�
            */
            /*
            case IDC_BUTTONTEST:{
char widthstr[MAX_PATH],heightstr[MAX_PATH];
                GetWindowText(editBannerWidth,widthstr,MAX_PATH);
                GetWindowText(editBannerHeight,heightstr,MAX_PATH);

printf("atoi: %s %s %d %d\n",widthstr,heightstr,atoi(widthstr),atoi(heightstr));
                if(!atoi(widthstr) || !atoi(heightstr)){
                    MessageBox(hwndDlg,"����͸߸�ʽ��������������","��ʾ",MB_OKCANCEL);
                    break;
                }
                break;
            }
            */
            case IDC_BUTTONSTART:{
                if(!fileChosen[0] || !fileChosen[1] || !fileChosen[2] ){
                    MessageBox(hwndDlg,"��Ԫ��û��ѡ��,�������ѡ��","��ʾ",MB_OKCANCEL);
                    break;
                }
                char widthstr[MAX_PATH],heightstr[MAX_PATH];
                GetWindowText(editBannerWidth,widthstr,MAX_PATH);
                GetWindowText(editBannerHeight,heightstr,MAX_PATH);

printf("atoi: %s %s %d %d\n",widthstr,heightstr,atoi(widthstr),atoi(heightstr));
                if(!atoi(widthstr) || !atoi(heightstr)){
                    MessageBox(hwndDlg,"����͸߸�ʽ��������������","��ʾ",MB_OKCANCEL);
                    break;
                }
                BannerWidth=atoi(widthstr);
                BannerHeight=atoi(heightstr);

                SetCurrentDirectory(currentDirectory); //���ûص�ԭ���ĵ�ǰĿ¼

                char tmpstr[maxn];

                IplImage* img1 = cvLoadImage(szFileNameImage);
                strcpy(tmpstr,inputAddr);
                strcat(tmpstr,imageName);
                cvSaveImage(tmpstr,img1);  //Ϊʲô����ͱ��治���أ��������������ûִ��һ��,Ҳ������

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
                if(MessageBox(hwndDlg,"�رմ��ڣ�","��ʾ",MB_YESNO)==IDYES){
                    EndDialog(hwndDlg, 0);
                }
                break;
            }
            //�Զ��岼��
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
    ���ڽ���slider����Ϣ
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

    value=SendMessageW(slider, TBM_GETPOS, 0, 0); //��ȡ��ǰ�ؼ���ֵ
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
    SendMessageW(hWndSlider, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(20,70));//���÷�ΧΪ20��70
    SendMessageW(hWndSlider,TBM_SETPOS, (WPARAM)1,(LPARAM)(int)value);//����slider�ؼ���ֵ

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
    SendMessage(radioGroups[2], BM_SETCHECK, BST_CHECKED,0); //ʹһ��ʼ��Ĭ��ѡ��
    radioGroups[3]=GetDlgItem(hwndDlg,IDC_RADIOUP1);
    radioGroups[4]=GetDlgItem(hwndDlg,IDC_RADIODOWN1);
    radioGroups[5]=GetDlgItem(hwndDlg,IDC_RADIOANY2);
    SendMessage(radioGroups[5], BM_SETCHECK, BST_CHECKED,0); //ʹһ��ʼ��Ĭ��ѡ��
    radioGroups[6]=GetDlgItem(hwndDlg,IDC_RADIOLEFT2);
    radioGroups[7]=GetDlgItem(hwndDlg,IDC_RADIORIGHT2);
    radioGroups[8]=GetDlgItem(hwndDlg,IDC_RADIOANY3);
    SendMessage(radioGroups[8], BM_SETCHECK, BST_CHECKED,0); //ʹһ��ʼ��Ĭ��ѡ��
    radioGroups[9]=GetDlgItem(hwndDlg,IDC_RADIOUP2);
    radioGroups[10]=GetDlgItem(hwndDlg,IDC_RADIODOWN2);
    radioGroups[11]=GetDlgItem(hwndDlg,IDC_RADIOANY4);
    SendMessage(radioGroups[11], BM_SETCHECK, BST_CHECKED,0); //ʹһ��ʼ��Ĭ��ѡ��
    radioGroups[12]=GetDlgItem(hwndDlg,IDC_RADIOLEFT3);
    radioGroups[13]=GetDlgItem(hwndDlg,IDC_RADIORIGHT3);
    radioGroups[14]=GetDlgItem(hwndDlg,IDC_RADIOANY5);
    SendMessage(radioGroups[14], BM_SETCHECK, BST_CHECKED,0); //ʹһ��ʼ��Ĭ��ѡ��
    radioGroups[15]=GetDlgItem(hwndDlg,IDC_RADIOUP3);
    radioGroups[16]=GetDlgItem(hwndDlg,IDC_RADIODOWN3);
    radioGroups[17]=GetDlgItem(hwndDlg,IDC_RADIOANY6);
    SendMessage(radioGroups[17], BM_SETCHECK, BST_CHECKED,0); //ʹһ��ʼ��Ĭ��ѡ��
}
int APIENTRY WinMain(HINSTANCE hInstance,  //��ǰӦ�ó����ʵ�����
                     HINSTANCE hPrevInstance, //ǰһ��ʵ��
                     LPSTR lpCmdLine, //�����в���,LPSTRʵ�ʾ���char*
                     int nShowCmd) //�����ڵ���ʾ��ʽ
{
    hInst=hInstance;
    InitCommonControls();

	HWND      hwnd;
	hwnd=GetForegroundWindow(); //��ȡǰ̨���ھ�����������е�ǰ̨���ھ��ǿ���̨���ڡ�


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
    static TCHAR szFilter[] = TEXT ("����ͼƬ�ļ�\0*.bmp;*.dib;*.jpg;*.jpeg;*.jpe;*.gif;*.tiff;*.png;*.ico\0")  \
                               TEXT ("JPEG�ļ� (*.jpg;*.jpeg;*.jpe)\0*.jpg;*.jpeg;*.jpe\0") \
                               TEXT ("λͼ�ļ� (*.bmp;*.dib)\0*.bmp;*.dib\0") \
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
    char name[maxn]; //��������Ե�ַ
    char zoomName[maxn];
	float weight;
	int x_pos;
	int y_pos;
	int width;
	int height;
	int normalGroup;  //???������ɶ
	int x_avg;		//��������
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
flag=0���Ǳ�����input/
flag=1���Ǳ�����output/
*/
void save_bitmap(Bitmap *bmp, char *filename,int flag) {
	check_im();
	char rawname[256];
	char tmpname[256];
	char relAddr[50];//���������ͼƬ����ڴ����λ�ã�������ֹ�������ʹ��붼��һ���ļ��very very mess.
	strcpy(tmpname, filename);  //�Ƚ�filename������tmpname��ȥ��������filename�����޸ģ���Ϊ��ָ�룬���filename��ɸĶ�
	if (flag == InputAddr)
		strcpy(relAddr, inputAddr); //��Ϊǰһ�ν��Ҫ��Ϊ��һ�ε�����sheet������ҲҪ������input�ļ���
	else
		strcpy(relAddr, "output/"); //��������λ��
	strcat(relAddr, tmpname);
	strcpy(tmpname, relAddr);

	strcpy(rawname, tmpname);
	if (!strstr(rawname, ".")) { fprintf(stderr, "Error writing image '%s': no extension found\n", tmpname); system("pause"); exit(1); }
	//sprintf(strstr(rawname, "."), ".raw"); //���ע�͵��Ļ����Ͳ�������.raw�ļ����Ҳ�Ӱ���������
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
����a��weightֵ
���ۼ�����������*���ٵģ������ؾ��кü�������ͼƬweightֵ��ĺܴ󡣡���
1.��Ӧ���صĻҶ�ֵ�ٳ���255��������ص�Ȩ�أ�������Ļ�Ҳ���кü���
2.ȡƽ����������ܺ�/S����Χ��0~255
*/
float calculWeight(Bitmap* a){
    int ah=a->h,aw=a->w;
    float sum=0.0f;
    for(int ay=0;ay<ah;ay++){
        for(int ax=0;ax<aw;ax++){
            int value=(*a)[ay][ax];
            sum+=(value&255);  //���ض�Ӧ�ĻҶ�ֵ
        }
    }
    return sum/(ah*aw)/255.0f;
}

/**
��Ԫ��target���ֳ�aw(��)*bh(��)������Ȼ��������������ĺ�weight����Ȩƽ�������Ԫ�ص�"����"
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
            //ע�����x?x:xҪ����������������������?ǰ��Ķ���ʶ��Ϊ�������ʽ
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
��ͼƬpstrImageName�������ţ����տ��widthScale�͸߶�heightScale�ı������������ΪpstrSaveImageName
��ȡ��ͼƬ�ͱ����ͼƬ�������input�ļ��С�
*/
void zoomImage(char*imageName,char*zoomImageName,float scale){
    //const char *pstrImageName = "scale.jpg";
	//const char *pstrSaveImageName = "scale2.jpg";
	//const char *pstrWindowsSrcTitle = "ԭͼ (http://blog.csdn.net/MoreWindows)";
	//const char *pstrWindowsDstTitle = "����ͼ (http://blog.csdn.net/MoreWindows)";

	//double fScale = 0.314;		//���ű���
	char tmpstr[maxn];
	strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
    strcat(tmpstr, imageName);

	CvSize czSize;			    //Ŀ��ͼ��ߴ�
	//���ļ��ж�ȡͼ��
	IplImage *pSrcImage = cvLoadImage(tmpstr, CV_LOAD_IMAGE_UNCHANGED);
	IplImage *pDstImage = NULL;

	//����Ŀ��ͼ���С
	czSize.width = pSrcImage->width * scale;
	czSize.height = pSrcImage->height * scale;
	//zoomWidth=czSize.width;
	//zoomHeight=czSize.height;
//printf("%d %d  %f\n",czSize.width,czSize.height,scale);

	//����ͼ������
	pDstImage = cvCreateImage(czSize, pSrcImage->depth, pSrcImage->nChannels);
	cvResize(pSrcImage, pDstImage, CV_INTER_AREA);

	//����ͼƬ���������غ����zoomImageNameҲ�ᱻ�޸�
	strcpy(zoomImageName,"zoom-");
    strcat(zoomImageName,imageName);
    strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
    strcat(tmpstr,zoomImageName);
	cvSaveImage(tmpstr, pDstImage);

}

/**
��ʼ�������е�Ԫ��
���ı��ж�ȡ����Ԫ�ص�ͼƬ����
�ı���ʽ���£�
n
xxx.jpg
xxx.jpg
...
nΪԪ�ظ���
*/
void init(Element*current,Name*elementName,int *kindMark,int sizes,Space titleSpace,Layout layout){
    char imgName[maxn];
    char tmpstr[maxn];
    char greyImgName[maxn];
    float widthScale,heightScale,scale;
    int layoutWidth,layoutHeight;
    float spaceSmallerSizePercentage=0.8f;  //���Ԫ�صĿ�߾�С�ڶ�Ӧ�����80%������зŴ���80%
    float spaceLargerSizePercentage=1.2f;   //���Ԫ�صĿ�߾����ڶ�Ӧ�����120%���������С��120%
    int tmp;
    //FILE *f1;
    //f1=fopen(filename,"r");
    //fscanf(f1,"%d",&tmp);  //bannerWidth������Ͳ���Ҫ��
    //fscanf(f1,"%d",&tmp);  //bannerHeight,����Ͳ���Ҫ��
    //fscanf(f1,"%d",&tmp);  //sizes�Ļ���ʵ��main�������Ѿ���ȡ���ڴ���Ԫ�����飬����Ͳ���Ҫ��
    for(int i=0;i<sizes;i++){
        //fscanf(f1,"%s %d",imgName,&tmp);
        strcpy(current[i].name,elementName[i].str);
        current[i].flag=kindMark[i];
//printf("%d %s\n",i,imgName);
        strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
        strcat(tmpstr, current[i].name);

        ///��Ԫ�ؽ�������
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

        strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
        strcat(tmpstr, current[i].zoomName);
        IplImage* zoomImg=cvLoadImage(tmpstr);
        IplImage* greyImg = cvCreateImage(cvGetSize(zoomImg), IPL_DEPTH_8U, 1);
        cvCvtColor(zoomImg,greyImg,CV_BGR2GRAY);//cvCvtColor(src,des,CV_BGR2GRAY)����ɫͼƬת���ɻҶ�ͼ

        strcpy(greyImgName,"grey-");
        strcat(greyImgName,current[i].zoomName);
        strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
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
        calGravityOfElement(gryImgBit,current[i].x_gravity,current[i].y_gravity,GRIDX,GRIDY); //�ֳ�GRIDX*GRIDY������
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
��ʼ������
mark���ڱ����ˮƽ���Ǵ�ֱ����

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
��bitmap�ķ�ʽ��ͼƬbд�뵽a��ȥ
x��yΪb��a�е���ʼλ��
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
    Layout horLayout,verLayout; //ˮƽ���֣���ֱ����

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
    һ��ʼ��space��߸�ȡ������沼�ֵ�һ�룬λ�ڹ�沼�ֵ����룬����������������������������

	whiteSpace.x=BannerWidth/4; whiteSpace.y=BannerHeight/4;
	whiteSpace.w=BannerWidth/2; whiteSpace.h=BannerHeight/2;
    */
    whiteSpace.x=widthBound; whiteSpace.y=heightBound;
	whiteSpace.w=BannerWidth-2*widthBound; whiteSpace.h=BannerHeight-2*heightBound;

    titleSpace.x=widthBound;titleSpace.y=heightBound;
    if(layoutStyle==HORIZONTAL){
        titleSpace.w=BannerWidth*TitleSpaceWidthScaleHor;titleSpace.h=BannerHeight*TitleSpaceHeightScaleHor;  //ˮƽ����1
    }
    else if(layoutStyle==VERTICAL){
        titleSpace.w=BannerWidth*TitleSpaceWidthScaleVer;titleSpace.h=BannerHeight*TitleSpaceHeightScaleVer;   //��ֱ����1
    }
    else{
        titleSpace.w=BannerWidth*user_defined_layout.scale[TITLE][HORIZONTAL];  //�Զ��岼��1
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

    srand(time(NULL)); //srand��һ�ξͿ���

    initLayout(horLayout,HORIZONTAL,HORIZONTALRIGHT,NOMATTER,HORIZONTALRIGHT,NOMATTER,NOMATTER,VERTICALUP);
    initLayout(verLayout,VERTICAL,NOMATTER,VERTICALBOTTOM,NOMATTER,VERTICALBOTTOM,NOMATTER,VERTICALUP);
    if(layoutStyle==HORIZONTAL){
        init(currentSolution,elementName,kindMark,sizes,titleSpace,horLayout);  //ˮƽ����2
    }
    else if(layoutStyle==VERTICAL){
        init(currentSolution,elementName,kindMark,sizes,titleSpace,verLayout);  //��ֱ����2
    }
    else{
        init(currentSolution,elementName,kindMark,sizes,titleSpace,user_defined_layout);  //�Զ��岼��2
    }
    ///The code above is executed successfully!!!
	originalWhiteSpace = whiteSpace;

	/*****Simulated Annealing*****/
	memcpy(&bestSolution, &currentSolution, sizeof(currentSolution));
printf("the initial cost:%f\n",cost(currentSolution, whiteSpace, interestBox, sizes, designType));

	while(t >= tEnd){
		if((int)t%75 == 0 && t!=300.0 && designType==PLAIN && keepChecking==TRUE){
			//����Ƿ����Ԫ�ص�һ���ֻ���������space���棬�����ڵĻ����������space
//printf("sadasd  ");
			keepChecking = checkSpace(bestSolution, &whiteSpace, orientation, sizes);
//printf("%d\n",orientation);
		}
		for(int i = 0; i < l(t); i++){

			memcpy(&candidateSolution, &currentSolution, sizeof(currentSolution));
			if(layoutStyle==HORIZONTAL){
                chooseNeighbour(candidateSolution, whiteSpace, titleSpace, sizes, t, horLayout);  //ˮƽ����3
			}
			else if(layoutStyle==VERTICAL){
                chooseNeighbour(candidateSolution, whiteSpace, titleSpace, sizes, t, verLayout);  //��ֱ����3
			}
			else{
                chooseNeighbour(candidateSolution, whiteSpace, titleSpace, sizes, t, user_defined_layout);  //�Զ��岼��3
			}
			//Ԫ���ƶ����cost��ȥ֮ǰ��cost
			float currentCost=cost(currentSolution, whiteSpace, interestBox, sizes, designType);
			float candidateCost=cost(candidateSolution, whiteSpace, interestBox, sizes, designType);

///printf("%d %f %f ",i,candidateCost,currentCost); //ΪɶafterCost����10000������
			delta = candidateCost - currentCost;
			/*
			exp(-(delta/t))����delta���������С
			��delta<0��calc��>1�ġ�
			*/
			calc = exp(-(delta)/t);
			//random = drand48(); //����һ��double�͵������
			random=rand()*1.0f/(RAND_MAX); //32767
			//����ı���solution���ã��򸲸�֮ǰ��
			if(random<calc || delta <0)
				memcpy(&currentSolution, &candidateSolution, sizeof(currentSolution));
			//����Ŀǰ��õ�solution
            float bestCost=cost(bestSolution, whiteSpace, interestBox, sizes, designType);
///printf("%f\n",bestCost);
			if(currentCost < bestCost)
				memcpy(&bestSolution, &currentSolution, sizeof(currentSolution));
		}
		//alpha(&t); //ʵ�ʾ���t=t-1������ô���Ӹ���
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
    //������ɫ�ı���ͼ
    CvSize bgSize;
    bgSize.width=BannerWidth;
    bgSize.height=BannerHeight;
    IplImage* bgImg = cvCreateImage(bgSize, IPL_DEPTH_8U, 1);
    cvSet(bgImg,CV_RGB(255,255,255),0);
    cvSaveImage("input/background.jpg",bgImg);

    //��Ԫ��һ����д�뵽����ͼ����
    Bitmap *bg=load_bitmap("input/background.jpg");
    for(int i=0;i<sizes;i++){
printf("%s %d %d\n",bestSolution[i].zoomName,bestSolution[i].x_pos,bestSolution[i].y_pos);
        strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
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


//-----------------------------------����Դ��gaudii�Ĵ���-----------------------------------

/*
Cheking function
����Ƿ����Ԫ�ص�һ���ֻ���������space����
�ǵĻ�����true������false
���ݸ�Ԫ����ֱorˮƽ����space�ĸ���������Ӧ������space�ĸ�or��
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
	����Ͳ����˶��,��������space��Ԫ��Խ������ҲӦ��Խ��ѽ������
	�����ˣ���Ϊ����forѭ����ÿ����һ�Σ�space������һ�Ρ�
	���Ե�����Ԫ�ض��˵Ļ���ÿ�������ķ������õľ�Сһ��
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
	//��������space�����,
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
�ж�group�Ƿ���ȫ������space��
������isOut=false
����isOut=true
�����˲���orientation�����space�����ĸ�������չ
*/
int checkBorders(Element g, Space *space, int size,int &orientation){
	int i, isOut=FALSE;

	if(g.x_pos < space->x ||
		g.x_pos + g.width > space->x + space->w){
			isOut=TRUE;
			orientation=HORIZONTAL;//���Ա��ˮƽ���򳬳���space�Ŀռ䣬space��Ҫˮƽ�������չ
	}
	if(g.y_pos < space->y ||
		g.y_pos + g.height > space->y + space->h ){
			isOut=TRUE;
			orientation=VERTICAL;//���Ա�Ǵ�ֱ���򳬳���space�Ŀռ䣬space��Ҫ��ֱ�������չ
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
		return VBN; //10000   �����غϣ���costΪ�ܴ��ֵ��
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
�Ǻ��������ܹ�ע�Ĳ���ô��
*/
float getAlignmentCost(Element *candidate, Space space, Space interestBox, int size, int designType){
	int xAligns[size], yAligns[size], x2Aligns[size], y2Aligns[size];
	int i, j;
	float totalCost=0.0f, individualCost, normCost; //totalCostԴ�����ж�û�г�ʼ��������
	int xCheck, x2Check, yCheck, y2Check, marginLeft, marginRight, marginBottom, marginTop;
	int interestPointLeft, interestPointRight, interestPointUp, interestPointDown;

	//�Ը���Ԫ�ص����������ڵ�����Ϊ������
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
					candidate[i].alignment= 0;	//Ӧ����RIGHT�����Ҷ���
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
		xWeights+= candidate[i].weight*candidate[i].x_avg;  //what is x_avg and y_avg? And why do they calculate as this������
		yWeights+= candidate[i].weight*candidate[i].y_avg;
	}

	centroidX = (int) (xWeights/weights);
	centroidY = (int) (yWeights/weights);
    ///������Ļ����ϡ��������ꡱ��ֵ���ڸ�λ��������
//printf("%f %f %f %d %d\n",weights,xWeights,yWeights,centroidX,centroidY);
//printf("%d %d\n",centroidX,centroidY);
	centerX = space.w/2;
	centerY = space.h/2;

	distance = sqrt(pow(centroidX-centerX, 2) + pow(centroidY-centerY, 2));
    ///�Ҿ���Ӧ���ǶԽ��ߵ�һ�룬��Ϊ��һ�㵽���ĵ���������ǶԽ��ߵ�һ��
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
		xWeights+= candidate[i].weight+candidate[i].x_avg;  //what is x_avg and y_avg? And why do they calculate as this������
		yWeights+= candidate[i].weight+candidate[i].y_avg;
	}

	centroidX = (int) (xWeights/weights);
	centroidY = (int) (yWeights/weights);
    ///������Ļ����ϡ��������ꡱ��ֵ���ڸ�λ��������
//printf("%f %f %f %d %d\n",weights,xWeights,yWeights,centroidX,centroidY);
printf("%f %f %f\n",weights,xWeights,yWeights);
printf("�������꣺%d %d\n",centroidX,centroidY);
	centerX = space.w/2;
	centerY = space.h/2;

	distance = sqrt(pow(centroidX-centerX, 2) + pow(centroidY-centerY, 2));
    ///�Ҿ���Ӧ���ǶԽ��ߵ�һ�룬��Ϊ��һ�㵽���ĵ���������ǶԽ��ߵ�һ��
	maxDistance = sqrt(pow(centerX, 2) + pow(centerY, 2));
//printf("%f %f\n",distance,maxDistance);
	//printf("%f %f %f\n",distance,maxDistance,distance/maxDistance);
}
/*
Overlap cost
�ж��Ƿ���������������غϣ�
�ǵĻ�����1������0.
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
�ж��������Ƿ��غ�
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
��Ԫ�ؽ���������ƶ�
*/
void chooseNeighbour(Element *candidate, Space space, Space titleSpace,int size, float t,Layout layout){
	int i, widthSteps, heightSteps, xJump, yJump;
	int xmin,xmax,ymin,ymax;  //����Ԫ���ܹ����õķ�Χ������������Ԫ�ص����Ͻ�
	int x_pos2, y_pos2, test;
	int kind1,kind2;
	bool titleLimited=false; //��Ϊtrue�����ʾtitle�����Ͻ��޶���titleSpace����
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
			����6�б�֤Ԫ����space���棬�����и�ǰ�ᣬ����Ԫ�صĿ�ȡ��߶ȿ϶���С��space��
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
///Ϊɶ����31,21,40,40
//printf("%d %d %d %d\n",xmin,ymin,xmax,ymax);
            }
//printf("space:%d %d %d %d\n",space.x,space.y,space.w,space.h);
//printf("%d %d %d %d %d\n",candidate[i].flag,xmin,ymin,xmax,ymax); //why xmax<0,ymax<0 ??? because x_pos,y_pos <0
            /*
			candidate[i].x_pos = candidate[i].x_pos<space.x?space.x:candidate[i].x_pos;
			candidate[i].y_pos = candidate[i].y_pos<space.y?space.y:candidate[i].y_pos;

			candidate[i].x_pos = x_pos2 > space.x + space.w?(space.x+space.w)-candidate[i].width:candidate[i].x_pos;
			candidate[i].y_pos = y_pos2 > space.y + space.h?(space.y+space.h)-candidate[i].height:candidate[i].y_pos;

			//��֤����ֵ��20�ı�����
			//���������֮ǰ�趨Ϊspace.x���ټ�������space����ô����ֻ�п���space.x��20�ı��������Լ�ʹ��Ҳ��0
			candidate[i].x_pos -= candidate[i].x_pos%20!=0?candidate[i].x_pos%20:0;
			candidate[i].y_pos -= candidate[i].y_pos%20!=0?candidate[i].y_pos%20:0;
            */

            candidate[i].x_pos = candidate[i].x_pos<xmin?xmin:candidate[i].x_pos;
			candidate[i].y_pos = candidate[i].y_pos<ymin?ymin:candidate[i].y_pos;

            //xmax��ymax���ܻ�С��candidate[i].width��candidate[i].height!!!
			candidate[i].x_pos = x_pos2 > xmax?(xmax-candidate[i].width<xmin?xmin:xmax-candidate[i].width):candidate[i].x_pos;
			candidate[i].y_pos = y_pos2 > ymax?(ymax-candidate[i].height<ymin?ymin:ymax-candidate[i].height):candidate[i].y_pos;

			//��֤����ֵ��20�ı�����
			//���������֮ǰ�趨Ϊspace.x���ټ�������space����ô����ֻ�п���space.x��20�ı��������Լ�ʹ��Ҳ��0
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
			///!!!3.0����Ԫ�����ĵ�ʱ�����Ǹ��������˰�����
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

