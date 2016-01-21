#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cv.h>
#include <highgui.h>
#define INF 0x3f3f3f3f
#define BFontWidth 21
#define BFontHeight 24
#define SFontWidth 18
#define SFontHeight 23
#define SFontWholeHeight 30
#define gapBetweenTitleandExplain 20
/*
һ��ʼ����ʱ����������Ҫ��װimagemagick
��װImageMagick-6.3.9-0-Q8-windows-dll.exe
Banner1:
������ͼƬ�ŵ�������ȥ��ͼƬλ�û�û�����ţ�����λ�����ȹ̶���

Banner2:
1.ʵ��ͼƬ�����ţ�ͼƬ��Сԭ��283*178�������picture1��Ϊ200*125,picture2��Ϊ400*251���������õ���picture1�Ĵ�С
2.�����ⶥ����ͼƬ���˶��롣
���������˵������ұ߲�����ͼƬ�����ʻ�һֱ���ұ���д���������������������
*/
using namespace std;
const int maxn=100;
int BannerHeight=343;
int BannerWidth=515;
int heightBound,widthBound; //�߽��С����ȡ���Ϳ���5%
float boundScale=0.05f; //�߽��С�ı���ȡֵ
const int disBetweenTestAndPic=20;
char inputAddr[50] = "input/"; //�������Ե�ַ
//ͼƬ�ڱ��������ʺ�λ��ʱ���������ź󣩣����Ͻ�λ�ڱ����е�xy���꣬������0��ʼ�����Լ�1���ȹ̶�������
int picInBg_x=209-1,picInBg_y=82-1;
//����һ��ʼд�뵽�����е�λ��
int textInBg_x=BannerWidth*0.06,textInBg_y=72-1+SFontWholeHeight;
/*
int r = value & 255;
int g = (value >> 8) & 255;
int bl = (value >> 16) & 255;

data(32bit): 8bit��b,g,r
*/

class Bitmap {
public:
	int w, h;
	int *data;
	Bitmap(int w_, int h_) :w(w_), h(h_) { data = new int[w*h]; }
	~Bitmap() { delete[] data; }
	int *operator[](int y) { return &data[y*w]; }
};

class Contour{
public:
    int w,h;
    int *left,*right,*upper,*bottom;
    int minleft,maxright,minupper,maxbottom;
    Contour(int w_,int h_):w(w_), h(h_) {
        left=new int[h];//�洢ÿ������ߵı�Ե���ص��ǵڼ���
        right=new int[h];//�洢ÿ�����ұߵı�Ե���ص��ǵڼ���
        upper=new int[w];//�洢ÿ�����ϱߵı�Ե���ص��ǵڼ���
        bottom=new int[w];//�洢ÿ�����±ߵı�Ե���ص��ǵڼ���
        minleft=minupper=INF;
        maxright=maxbottom=0;
    }
    ~Contour(){
        delete[] left;
        delete[] right;
        delete[] upper;
        delete[] bottom;
    }
    int getLeft(int x){
        return left[x];
    }
    int getRight(int x){
        return right[x];
    }
    int getUpper(int x){
        return upper[x];
    }
    int getBottom(int x){
        return bottom[x];
    }
};
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
*/
void save_bitmap(Bitmap *bmp, char *filename,int flag) {
	check_im();
	char rawname[256];
	char tmpname[256];
	char relAddr[50];//���������ͼƬ����ڴ����λ�ã�������ֹ�������ʹ��붼��һ���ļ��very very mess.
	strcpy(tmpname, filename);  //�Ƚ�filename������tmpname��ȥ��������filename�����޸ģ���Ϊ��ָ�룬���filename��ɸĶ�
	if (flag == 0)
		strcpy(relAddr, inputAddr); //��Ϊǰһ�ν��Ҫ��Ϊ��һ�ε�����sheet������ҲҪ������input�ļ���
	else
		strcpy(relAddr, "output/"); //��������λ��
	strcat(relAddr, tmpname);
	strcpy(tmpname, relAddr);

	strcpy(rawname, tmpname);
	if (!strstr(rawname, ".")) { fprintf(stderr, "Error writing image '%s': no extension found\n", tmpname); system("pause"); exit(1); }
	sprintf(strstr(rawname, "."), ".raw");
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

/**
look for '.' in "xxxx.png"
*/
int find_pos(char*src){
	int len = strlen(src);
	for (int i = 0; i < len; i++){
		if (src[i] == '.')
			return i;
	}
	return -1;
}

/**
get the Contour  Pixel Position of the picture
*/
void getContourPixel(Bitmap *a,Contour *&c){
    int ah=a->h,aw=a->w;
    c=new Contour(aw,ah);

    for(int ay=0;ay<ah;ay++){
        for(int ax=0;ax<aw;ax++){
            int value = (*a)[ay][ax];
            int dr = (value & 255);  //red
            int dg = ((value >> 8) & 255) ;  //green
            int db = (value >> 16)&255;  //blue
            //int dd = value >> 24; //depth
            if(dr<=250||dg<=250||db<=250){
                //if it is not white
                c->left[ay]=ax+1;
                c->minleft=min(c->minleft,ax+1);
                break;
            }
        }

        for(int ax=aw-1;ax>=0;ax--){
            int value = (*a)[ay][ax];
            int dr = (value & 255);  //red
            int dg = ((value >> 8) & 255) ;  //green
            int db = (value >> 16)&255;  //blue
            //int dd = value >> 24; //depth
            if(dr<=250||dg<=250||db<=250){
                //if it is not white
                c->right[ay]=ax+1;
                c->maxright=max(c->maxright,ax+1);
                break;
            }
        }
    }

    for(int ax=0;ax<aw;ax++){
        for(int ay=0;ay<ah;ay++){
            int value = (*a)[ay][ax];
            int dr = (value & 255);  //red
            int dg = ((value >> 8) & 255) ;  //green
            int db = (value >> 16)&255;  //blue
            //int dd = value >> 24; //depth
            if(dr<=250||dg<=250||db<=250){
                //if it is not white
                c->upper[ax]=ay+1;
                c->minupper=min(c->minupper,ay+1);
                break;
            }
        }

        for(int ay=ah-1;ay>=0;ay--){
            int value = (*a)[ay][ax];
            int dr = (value & 255);  //red
            int dg = ((value >> 8) & 255) ;  //green
            int db = (value >> 16)&255;  //blue
            //int dd = value >> 24; //depth
            if(dr<=250||dg<=250||db<=250){
                //if it is not white
                c->bottom[ax]=ay+1;
                c->maxbottom=max(c->maxbottom,ay+1);
                break;
            }
        }
    }
}

/**
����ͼƬ��������ص����ͼƬ��Ե�����ؾ��룬�Ӷ��Դ�ȷ�����ֵ�λ�á�
*/
int getImageTopPixel(Contour *c){
    int res=INF;
    for(int i=0;i<c->w;i++){
        res=min(res,c->getUpper(i));
    }
    return res;
}
void putPicture(Bitmap *a, Bitmap *b, Bitmap *&res){
    int ah=a->h,aw=a->w;
    int bh=b->h,bw=b->w;

    res = new Bitmap(a->w, a->h);

    for(int ay=0;ay<ah;ay++){
        for(int ax=0;ax<aw;ax++){
            (*res)[ay][ax] = (*a)[ay][ax];
        }
    }

    for(int by=0;by<bh;by++){
        for(int bx=0;bx<bw;bx++){
            (*res)[picInBg_y+by][picInBg_x+bx]=(*b)[by][bx];
        }
    }
}

/**
д��˵�����ֵ�ͼƬ��ȥ
*/
void writeExplain(char*explainFile,Bitmap*res,Contour *c,IplImage *&target,int lx,int ly){
    int len;
    char str[maxn];
    float wscale=0.5f,hscale=0.5f,thickness=1.0f; //������ȡ��߶ȴ�С����ȶ���
    int nowlx=lx,nowly=ly+hscale*SFontWholeHeight;
    //wordlen:����ĵ�����ռ���ؿ��ȣ�wgap������֮��ˮƽ�������Ϊһ����ĸ���ȡ�hgap��˵�������м������Ϊ����߶ȵ�1/2��
    int wordlen,wgap=BFontWidth*wscale,hgap=hscale*SFontHeight/2;
    //bool isLine=false;
    CvFont font;
    cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX,wscale,hscale,0,thickness,8);

    FILE *f1;
    f1=fopen(explainFile,"r");

    while(fscanf(f1,"%s",str)!=EOF){
        len=strlen(str);
        wordlen=0;
        for(int i=0;i<len;i++){
            char ch=str[i];
            if(ch-'A'>=0 && ch-'A'<26){
                wordlen+=wscale*BFontWidth;  //һ��ʼ���Ǽ���wscale�ˣ����µ���֮�����ܴ�
            }
            else{
                //Сд��ĸ�Լ������ţ�������Сд��ĸ����������
                wordlen+=wscale*SFontWidth;
            }
        }
        int dis=INF;
        /*
        �жϵ����ұ߾���ͼƬ��Ե��С�����Ƿ����30����
        ��һ�����ұ�û��ͼƬ�أ����Ի����ж�
        */
        bool flag=false;
        for(int k=SFontWholeHeight;k>=0;k--){
            int row=nowly+(SFontWholeHeight-SFontHeight)-k-picInBg_y;
            if(row>=0 && row<c->h){
                dis=min(c->left[row],dis);
                flag=true;//Ϊtrue���������ұ���ͼƬ����
            }
        }
        if(!flag){
            dis=c->w; //������ΪͼƬ�Ŀ���
        }
        if(nowlx+wordlen+disBetweenTestAndPic>picInBg_x+dis){
            //isLine=true;
            nowlx=lx;
            nowly=nowly+hgap+hscale*SFontWholeHeight;
        }
//printf("%s %d %d  %d %d\n",str,nowlx,nowly,wordlen,wgap);
        cvPutText(target,str,cvPoint(nowlx,nowly),&font,cvScalar(0,0,0));

        nowlx+=wordlen+wgap;
    }
    fclose(f1);
}
/**
д����⵽ͼƬ��ȥ
target:Ҫд���������ݵ�ͼƬ
l and r: д����ʼλ��

ע�⣻cvPutText�е�cvPoint�����������ݵ������½ǡ�

picInBg_x,picInBg_y
*/
int writeTitle(char*titleFile,Bitmap*res,Contour *c,IplImage *&target,int lx,int ly){
    int ypos;
    int len;
    char str[maxn];
    float wscale=1.0f,hscale=1.0f,thickness=2.0f; //������ȡ��߶ȴ�С����
    int nowlx=lx,nowly=ly;
    //wordlen:����ĵ�����ռ���ؿ��ȣ�wgap������֮��ˮƽ�������Ϊһ����ĸ���ȡ�hgap���м������Ϊ����߶ȵ�1/3��
    int wordlen,wgap=BFontWidth*wscale,hgap=hscale*BFontHeight/3;
    //bool isLine=false;
    CvFont font;
    cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX,wscale,hscale,0,thickness,8);

    FILE *f1,*f2;
    f1=fopen(titleFile,"r");

    while(fscanf(f1,"%s",str)!=EOF){
        len=strlen(str);
        wordlen=0;
        for(int i=0;i<len;i++){
            char ch=str[i];
            if(ch-'A'>=0 && ch-'A'<26){
                wordlen+=wscale*BFontWidth;
            }
            else{
                //Сд��ĸ�Լ������ţ�������Сд��ĸ����������
                wordlen+=wscale*SFontWidth;
            }
        }
        int dis=INF;
        /*
        �жϵ����ұ߾���ͼƬ��Ե��С�����Ƿ����30����
        ��һ�����ұ�û��ͼƬ�أ����Ի����ж�
        */
        bool flag=false;
        for(int k=SFontWholeHeight;k>=0;k--){
            int row=nowly+(SFontWholeHeight-SFontHeight)-k-picInBg_y;
            if(row>=0 && row<c->h){
                dis=min(c->left[row],dis);
                flag=true; //Ϊtrue���������ұ���ͼƬ����
            }
        }
        if(!flag){
            dis=c->w; //������ΪͼƬ�Ŀ���
        }
        if(nowlx+wordlen+disBetweenTestAndPic>picInBg_x+dis){
            //isLine=true;
            nowlx=lx;
            nowly=nowly+hgap+hscale*SFontWholeHeight;
        }

        cvPutText(target,str,cvPoint(nowlx,nowly),&font,cvScalar(0,0,0));

        nowlx+=wordlen+wgap;
    }

    fclose(f1);

    ypos=nowly+gapBetweenTitleandExplain; //SFontWholeHeight����writeExplain�ټ��ϣ���Ϊ�б����Ĵ���
//printf("%d %d %d %d\n",nowly,gapBetweenTitleandExplain,SFontWholeHeight,ypos);
    return ypos; //���Ҫ���ر��������������ص������꣬������ȷ��������д���˵�����ִ��Ŀ�ʼ
}

/**
file:�Ѿ�����ͼƬ��ı���ͼ
titleFile��title��txt
explainFile:˵�����ֵ�txt
res���Ѿ�����ͼƬ��ı���ͼ��Bitmap
c��ͼƬ�ı�Ե������Ϣ
*/
void writeText(char*file,char*titleFile,char*explainFile,Bitmap*res,Contour *c,char*resultName){
    char tmpstr[maxn];
    strcpy(tmpstr, inputAddr);
    strcat(tmpstr, file);

    IplImage * target;
	target = cvLoadImage(tmpstr, -1);

    int ly=writeTitle(titleFile,res,c,target,textInBg_x,textInBg_y);
    writeExplain(explainFile,res,c,target,textInBg_x,ly);

    cvSaveImage(resultName, target);
}

/**
��ͼƬpstrImageName�������ţ����տ���widthScale�͸߶�heightScale�ı������������ΪpstrSaveImageName
��ȡ��ͼƬ�ͱ����ͼƬ�������input�ļ��С�
*/
void zoomImage(char*pstrImageName,char*pstrSaveImageName,float scale,int &zoomWidth,int &zoomHeight){
    //const char *pstrImageName = "scale.jpg";
	//const char *pstrSaveImageName = "scale2.jpg";
	//const char *pstrWindowsSrcTitle = "ԭͼ (http://blog.csdn.net/MoreWindows)";
	//const char *pstrWindowsDstTitle = "����ͼ (http://blog.csdn.net/MoreWindows)";

	//double fScale = 0.314;		//���ű���
	char tmpstr[maxn];
	strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
    strcat(tmpstr, pstrImageName);

	CvSize czSize;			    //Ŀ��ͼ��ߴ�

	//���ļ��ж�ȡͼ��
	IplImage *pSrcImage = cvLoadImage(tmpstr, CV_LOAD_IMAGE_UNCHANGED);
	IplImage *pDstImage = NULL;

	//����Ŀ��ͼ���С
	czSize.width = pSrcImage->width * scale;
	czSize.height = pSrcImage->height * scale;
	zoomWidth=czSize.width;
	zoomHeight=czSize.height;
//printf("%d %d  %f\n",czSize.width,czSize.height,scale);

	//����ͼ������
	pDstImage = cvCreateImage(czSize, pSrcImage->depth, pSrcImage->nChannels);
	cvResize(pSrcImage, pDstImage, CV_INTER_AREA);

	//��������
	//cvNamedWindow(pstrWindowsSrcTitle, CV_WINDOW_AUTOSIZE);
	//cvNamedWindow(pstrWindowsDstTitle, CV_WINDOW_AUTOSIZE);

	//��ָ����������ʾͼ��
	//cvShowImage(pstrWindowsSrcTitle, pSrcImage);
	//cvShowImage(pstrWindowsDstTitle, pDstImage);

	//�ȴ������¼�
	//cvWaitKey();

	//����ͼƬ
	strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
    strcat(tmpstr, pstrSaveImageName);
	cvSaveImage(tmpstr, pDstImage);
}
int main()
{

    char tmpstr[maxn];
    char bg[maxn]="background.jpg";
    char picture[maxn]="picture1.jpg";  //200*125
    //char picture[maxn]="picture2.jpg";    //400*251
    char halfDest[maxn]="res1.jpg";
    char titleFile[maxn]="title.txt";
    char explainFile[maxn]="explain.txt";
    char zoomImageDest[maxn]="ImageZoom1.jpg";
    //char resultName[maxn]="ImageNo-zoomAndNo-align.jpg";
    //char resultName[maxn]="ImageNo-zoomAndAlign.jpg";
    //char resultName[maxn]="ImageZoomAndNo-align.jpg";
    char resultName[maxn]="ImageZoomAndalign.jpg";
    //char resultName[maxn]="ImageWithZoomSmaller.jpg";
    strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
    strcat(tmpstr, bg);
    Bitmap *a = load_bitmap(tmpstr);  //����ͼ

    //-------------------------Banner2��������------------------------------------
    BannerHeight=a->h;
    BannerWidth=a->w;
    heightBound=BannerHeight*boundScale;
    widthBound=BannerWidth*boundScale;
//printf("%d %d\n",heightBound,widthBound);
    int left=(BannerWidth-2*widthBound)*0.4f;
    int right=BannerWidth-left-2*widthBound;
//printf("%d %d\n",left,right);

    //-----------------------------Banner2-----------------------------------------


    strcpy(tmpstr, inputAddr);
    strcat(tmpstr, picture);
    Bitmap *image1 = load_bitmap(tmpstr);  //ͼƬ��λͼ

    //--------------------------Banner2��������------------------------------------
    float widthScale=(float)right/image1->w;
    float heightScale=(float)(BannerHeight-2*heightBound)/image1->h;
    float scale=min(widthScale,heightScale);  //���ű���ȡ������Сֵ
    //scale=1.0f;
    int zoomWidth,zoomHeight;
    zoomImage(picture,zoomImageDest,scale,zoomWidth,zoomHeight);

    picInBg_x=(right-zoomWidth)/2+left+widthBound;
    picInBg_y=(BannerHeight-2*heightBound-zoomHeight)/2+heightBound;

    strcpy(tmpstr, inputAddr);
    strcat(tmpstr, zoomImageDest);
    Bitmap *image2 = load_bitmap(tmpstr);
    //--------------------------------Banner2-------------------------------------

    Bitmap *bgWithPic=NULL;
    putPicture(a,image2,bgWithPic);
    save_bitmap(bgWithPic,halfDest,0); //���Ǳ��浽inputĿ¼��

    Contour *contorPixel=NULL;
    getContourPixel(image2,contorPixel);

    //--------------------------Banner2��������------------------------------------

    int imageTopPixel=getImageTopPixel(contorPixel);
    textInBg_y=picInBg_y+imageTopPixel+SFontWholeHeight; //ע�����λ�ò���������ĸ�����½�

    //--------------------------------Banner2-------------------------------------

    writeText(halfDest,titleFile,explainFile,bgWithPic,contorPixel,resultName);

    return 0;
}