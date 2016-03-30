#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cv.h>
#include <highgui.h>
/**
ʵ����������������
1����ͼƬת��Ϊ�Ҷ�ͼ
2�����Ҷ�ȡ�ûҶ�ͼ��bitmap����ȡÿ�����صĻҶ�ֵ
*/
using namespace std;
const int maxn=100;
char inputAddr[50] = "input/"; //�������Ե�ַ


class Bitmap {
public:
	int w, h;
	int *data;
	Bitmap(int w_, int h_) :w(w_), h(h_) { data = new int[w*h]; }
	~Bitmap() { delete[] data; }
	int *operator[](int y) { return &data[y*w]; }
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


int main()
{
    char imgAname[maxn] = "imageA.jpg";  //ԭʼͼƬA
    char imgBname[maxn]="imageB.jpg";
    char greyImgAname[maxn]="grayImgA.jpg";  //A�ĻҶ�ͼ
    char greyImgBname[maxn]="grayImgB.jpg";
    char tmpstr[maxn];
    strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
    strcat(tmpstr, imgAname);
    IplImage* imgA = cvLoadImage(tmpstr);
    IplImage* greyimgA = cvCreateImage(cvGetSize(imgA), IPL_DEPTH_8U, 1);
    cvCvtColor(imgA,greyimgA,CV_BGR2GRAY);//cvCvtColor(src,des,CV_BGR2GRAY)����ɫͼƬת���ɻҶ�ͼ

    strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
    strcat(tmpstr, greyImgAname);
    cvSaveImage(tmpstr,greyimgA);

    strcpy(tmpstr, inputAddr);
    strcat(tmpstr, greyImgAname);
    Bitmap *gryimgBitA = load_bitmap(tmpstr);

    int HimgA=gryimgBitA->h,WimgA=gryimgBitA->w;
    int greyarray[HimgA][WimgA];
    for(int ay=0;ay<HimgA;ay++){
        for(int ax=0;ax<WimgA;ax++){
            int value=(*gryimgBitA)[ay][ax];
            greyarray[ay][ax]=value&255;  //���ض�Ӧ�ĻҶ�ֵ
        }
    }
    printf("x:120 y:30 Grey Value:%d\n",greyarray[29][119]);
    printf("x:94 y:78 Grey Value:%d\n",greyarray[77][93]);
    printf("x:279 y:134 Grey Value:%d\n",greyarray[133][278]);
    return 0;
}
