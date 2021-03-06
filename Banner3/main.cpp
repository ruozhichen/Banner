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
Banner3：
可以改变字体的大小，即通过改变cvInitFont中的wscale、hscale。
为方便传参数，创建类FontSize，包含wscale、hscale、thickness。

*/
using namespace std;
const int maxn=100;
int BannerHeight=343;
int BannerWidth=515;
int heightBound,widthBound; //边界大小，各取长和宽的5%
float boundScale=0.05f; //边界大小的比例取值
const int disBetweenTestAndPic=20;
char inputAddr[50] = "input/"; //输入的相对地址
//图片在背景中最适合位置时（经过缩放后），左上角位于背景中的xy坐标，索引从0开始，所以减1。先固定好试试
int picInBg_x=209-1,picInBg_y=82-1;
//文字一开始写入到背景中的位置
int textInBg_x=BannerWidth*0.06,textInBg_y=72-1+SFontWholeHeight;
/*
int r = value & 255;
int g = (value >> 8) & 255;
int bl = (value >> 16) & 255;

data(32bit): 8bit，b,g,r
*/

//字体大小
class FontSize{
public:
    float wscale,hscale,thickness;
    FontSize(float ws,float hs,float th):wscale(ws),hscale(hs),thickness(th){}
    ~FontSize(){};
};


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
        left=new int[h];//存储每行最左边的边缘像素点是第几个
        right=new int[h];//存储每行最右边的边缘像素点是第几个
        upper=new int[w];//存储每列最上边的边缘像素点是第几个
        bottom=new int[w];//存储每列最下边的边缘像素点是第几个
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
	char relAddr[50];//输入输出的图片相对于代码的位置，这样防止输出输入和代码都在一个文件里，very very mess.
	strcpy(tmpname, filename);  //先将filename拷贝到tmpname中去，不能用filename进行修改，因为是指针，会对filename造成改动
	if (flag == 0)
		strcpy(relAddr, inputAddr); //因为前一次结果要作为后一次的输入sheet，所以也要保存在input文件里
	else
		strcpy(relAddr, "output/"); //输出的相对位置
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
（后来发现该方法多此一举了。。。因为contour里面的minupper就是。）
返回图片中最顶端像素点距离图片边缘的像素距离，从而以此确定文字的位置。
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
写入说明文字到图片中去
*/
void writeExplain(char*explainFile,Bitmap*res,Contour *c,IplImage *&target,int lx,int ly,FontSize *explainSize){
    int len;
    char str[maxn];
    float wscale=explainSize->wscale,hscale=explainSize->hscale,thickness=explainSize->thickness; //字体宽度、高度大小、厚度度量
    int nowlx=lx,nowly=ly+hscale*SFontWholeHeight;
    //wordlen:估算的单词所占像素宽度，wgap：单词之间水平间隔，设为一个字母宽度。hgap；说明文字行间距设置为字体高度的1/2吧
    int wordlen,wgap=BFontWidth*(wscale+0.05),hgap=hscale*SFontHeight/2;
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
                wordlen+=wscale*BFontWidth;  //一开始忘记加上wscale了，导致单词之间间隔很大。
            }
            else{
                //小写字母以及标点符号，均按照小写字母宽度来处理
                wordlen+=wscale*SFontWidth;
            }
        }
        int dis=INF;
        /*
        判断单词右边距离图片边缘最小距离是否大于30像素
        万一单词右边没有图片呢，所以还得判断
        */
        bool flag=false;
        for(int k=SFontWholeHeight;k>=0;k--){
            int row=nowly+(SFontWholeHeight-SFontHeight)-k-picInBg_y;
            if(row>=0 && row<c->h){
                dis=min(c->left[row],dis);
                flag=true;//为true表明单词右边有图片存在
            }
        }
        if(!flag){
            dis=c->w; //即设置为图片的宽度
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
写入标题到图片中去
target:要写入文字内容的图片
l and r: 写的起始位置

注意；cvPutText中的cvPoint，是文字内容的最左下角。

picInBg_x,picInBg_y
*/
int writeTitle(char*titleFile,Bitmap*res,Contour *c,IplImage *&target,int lx,int ly,FontSize *titleSize){
    int ypos;
    int len;
    char str[maxn];
    float wscale=titleSize->wscale,hscale=titleSize->hscale,thickness=titleSize->thickness; //字体宽度、高度大小度量
    int nowlx=lx,nowly=ly;
    //wordlen:估算的单词所占像素宽度，wgap：单词之间水平间隔，设为一个字母宽度。hgap；行间距设置为字体高度的1/3吧
    int wordlen,wgap=BFontWidth*(wscale-0.05),hgap=hscale*BFontHeight/3;
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
                //小写字母以及标点符号，均按照小写字母宽度来处理
                wordlen+=wscale*SFontWidth;
            }
        }
        int dis=INF;
        /*
        判断单词右边距离图片边缘最小距离是否大于30像素
        万一单词右边没有图片呢，所以还得判断
        */
        bool flag=false;
        for(int k=SFontWholeHeight;k>=0;k--){
            int row=nowly+(SFontWholeHeight-SFontHeight)-k-picInBg_y;
            if(row>=0 && row<c->h){
                dis=min(c->left[row],dis);
                flag=true; //为true表明单词右边有图片存在
            }
        }
        if(!flag){
            dis=c->w; //即设置为图片的宽度
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

    ypos=nowly+gapBetweenTitleandExplain; //SFontWholeHeight等在writeExplain再加上，因为有比例的存在
//printf("%d %d %d %d\n",nowly,gapBetweenTitleandExplain,SFontWholeHeight,ypos);
    return ypos; //最后要返回标题的最下面的像素点纵坐标，这样好确定接下来写入的说明文字从哪开始
}

/**
file:已经插入图片后的背景图
titleFile：title的txt
explainFile:说明文字的txt
res：已经插入图片后的背景图的Bitmap
c：图片的边缘像素信息
*/
void writeText(char*file,char*titleFile,char*explainFile,Bitmap*res,Contour *c,char*resultName,FontSize *titleSize,FontSize *explainSize){
    char tmpstr[maxn];
    strcpy(tmpstr, inputAddr);
    strcat(tmpstr, file);

    IplImage * target;
	target = cvLoadImage(tmpstr, -1);

    int ly=writeTitle(titleFile,res,c,target,textInBg_x,textInBg_y,titleSize);
    writeExplain(explainFile,res,c,target,textInBg_x,ly,explainSize);

    cvSaveImage(resultName, target);
}

/**
对图片pstrImageName进行缩放，按照宽度widthScale和高度heightScale的比例，结果保存为pstrSaveImageName
读取的图片和保存的图片都存放在input文件中。
*/
void zoomImage(char*pstrImageName,char*pstrSaveImageName,float scale,int &zoomWidth,int &zoomHeight){
    //const char *pstrImageName = "scale.jpg";
	//const char *pstrSaveImageName = "scale2.jpg";
	//const char *pstrWindowsSrcTitle = "原图 (http://blog.csdn.net/MoreWindows)";
	//const char *pstrWindowsDstTitle = "缩放图 (http://blog.csdn.net/MoreWindows)";

	//double fScale = 0.314;		//缩放倍数
	char tmpstr[maxn];
	strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
    strcat(tmpstr, pstrImageName);

	CvSize czSize;			    //目标图像尺寸

	//从文件中读取图像
	IplImage *pSrcImage = cvLoadImage(tmpstr, CV_LOAD_IMAGE_UNCHANGED);
	IplImage *pDstImage = NULL;

	//计算目标图像大小
	czSize.width = pSrcImage->width * scale;
	czSize.height = pSrcImage->height * scale;
	zoomWidth=czSize.width;
	zoomHeight=czSize.height;
//printf("%d %d  %f\n",czSize.width,czSize.height,scale);

	//创建图像并缩放
	pDstImage = cvCreateImage(czSize, pSrcImage->depth, pSrcImage->nChannels);
	cvResize(pSrcImage, pDstImage, CV_INTER_AREA);

	//创建窗口
	//cvNamedWindow(pstrWindowsSrcTitle, CV_WINDOW_AUTOSIZE);
	//cvNamedWindow(pstrWindowsDstTitle, CV_WINDOW_AUTOSIZE);

	//在指定窗口中显示图像
	//cvShowImage(pstrWindowsSrcTitle, pSrcImage);
	//cvShowImage(pstrWindowsDstTitle, pDstImage);

	//等待按键事件
	//cvWaitKey();

	//保存图片
	strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
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
    char zoomImageDest[maxn]="ImageZoom1.jpg"; //缩放后的图片

    //char resultName[maxn]="ImageNo-zoomAndNo-align.jpg";
    //char resultName[maxn]="ImageNo-zoomAndAlign.jpg";
    //char resultName[maxn]="ImageZoomAndNo-align.jpg";
    char resultName[maxn]="ImageZoomAndalign.jpg";
    //char resultName[maxn]="ImageWithZoomSmaller.jpg";
    strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
    strcat(tmpstr, bg);
    Bitmap *a = load_bitmap(tmpstr);  //背景图

    //-------------------------Banner2新增内容------------------------------------
    BannerHeight=a->h;
    BannerWidth=a->w;
    heightBound=BannerHeight*boundScale;
    widthBound=BannerWidth*boundScale;
//printf("%d %d\n",heightBound,widthBound);
    int left=(BannerWidth-2*widthBound)*0.4f; //0.4为左右尺寸的比例，左:右=4:6
    int right=BannerWidth-left-2*widthBound;
//printf("%d %d\n",left,right);

    //-----------------------------Banner2-----------------------------------------


    strcpy(tmpstr, inputAddr);
    strcat(tmpstr, picture);
    Bitmap *image1 = load_bitmap(tmpstr);  //图片的位图

    //--------------------------Banner2新增内容------------------------------------
    float widthScale=(float)right/image1->w;
    float heightScale=(float)(BannerHeight-2*heightBound)/image1->h;
    //缩放比例取两者最小值
    float scale=min(widthScale,heightScale);
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
    save_bitmap(bgWithPic,halfDest,0); //还是保存到input目录下

    Contour *contorPixel=NULL;
    getContourPixel(image2,contorPixel);

    //--------------------------Banner2新增内容------------------------------------
    //int imageTopPixel=getImageTopPixel(contorPixel);
//printf("%d %d\n",imageTopPixel,contorPixel->minupper);
    //标题顶端与图片顶端对齐
    textInBg_y=picInBg_y+contorPixel->minupper+SFontWholeHeight; //注意标题位置参照是以字母最左下角

    //--------------------------------Banner2-------------------------------------


    //--------------------------------Banner3新增内容-----------------------------
    float titlewscale=1.0f,titlehscale=1.0f,titlethickness=2.0f;
    float explainwscale=0.4f,explainhscale=0.4f,explainthickness=1.0f;
    FontSize *titleSize=new FontSize(titlewscale,titlehscale,titlethickness);
    FontSize *explainSize=new FontSize(explainwscale,explainhscale,explainthickness);
    //--------------------------------Banner3-------------------------------------

    writeText(halfDest,titleFile,explainFile,bgWithPic,contorPixel,resultName,titleSize,explainSize);

    return 0;
}
