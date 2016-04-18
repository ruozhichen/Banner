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
#define GRIDX 10
#define GRIDY 6
/**
在之前2.0的基础上，可以将每个元素划分a*b个网格，根据每个小网格的weight和中心，计算元素的“重心”。
从而在计算整体“重心”时，用元素的“重心”坐标取代原先的中心坐标。
*/
using namespace std;
const int maxn=100;
char inputAddr[50] = "input/"; //输入的相对地址
int BannerHeight=640;
int BannerWidth=1000;
float BannerBalanceCost;
float BannerAlignmentCost;
struct Element{
    char name[maxn]; //包含相对地址了
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
void chooseNeighbour(Element *candidate, Space space, int size, float t);
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
初始化海报中的元素
从文本中读取各个元素的图片名称
文本格式如下：
n
xxx.jpg
xxx.jpg
...
n为元素个数
*/
void init(Element*current,char*filename,int sizes){
    char imgName[maxn];
    char tmpstr[maxn];
    char greyImgName[maxn];
    int tmp;
    FILE *f1;
    f1=fopen(filename,"r");
    fscanf(f1,"%d",&tmp);  //sizes的话其实在main函数中已经读取用于创建元素数组，这里就不需要了
    for(int i=0;i<sizes;i++){
        fscanf(f1,"%s",imgName);
//printf("%d %s\n",i,imgName);
        strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
        strcat(tmpstr, imgName);

        strcpy(current[i].name,tmpstr);

        IplImage* img = cvLoadImage(tmpstr);
        IplImage* greyImg = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
        cvCvtColor(img,greyImg,CV_BGR2GRAY);//cvCvtColor(src,des,CV_BGR2GRAY)将彩色图片转换成灰度图
//printf("1\n");
        strcpy(greyImgName,"grey-");
        strcat(greyImgName,imgName);
        strcpy(tmpstr, inputAddr);  //inputAddr存储的是相对地址，为全局变量，定义在代码最前面了。
        strcat(tmpstr,greyImgName);
        cvSaveImage(tmpstr,greyImg);
//printf("2\n");
        strcpy(tmpstr, inputAddr);
        strcat(tmpstr, greyImgName);
        Bitmap *gryImgBit = load_bitmap(tmpstr);

//printf("3\n");
        current[i].width=gryImgBit->w;
        current[i].height=gryImgBit->h;
//printf("%d %d\n",BannerWidth-current[i].width,BannerHeight-current[i].height);
        ///初始坐标限定在一开始的space区域内
        current[i].x_pos=rand()%(BannerWidth-current[i].width-BannerWidth/4)+BannerWidth/4;
        current[i].y_pos=rand()%(BannerHeight-current[i].height-BannerHeight/4)+BannerHeight/4;
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
//printf("4\n");
    }
    fclose(f1);
}
/**
用bitmap的方式将图片b写入到a中去
x、y为b在a中的起始位置
*/
void putPicture(Bitmap *&a, Bitmap *b, int x,int y){
    int ah=a->h,aw=a->w;
    int bh=b->h,bw=b->w;

    for(int by=0;by<bh;by++){
        for(int bx=0;bx<bw;bx++){
            (*a)[y+by][x+bx]=(*b)[by][bx];
        }
    }
}


int main()
{
    char filename[maxn]="input.txt";
    int sizes;
    FILE *f1;
    f1=fopen(filename,"r");
    fscanf(f1,"%d",&sizes); //元素个数
    fclose(f1);

    Element currentSolution[sizes];
    Element bestSolution[sizes];
    Element candidateSolution[sizes];

	Space whiteSpace,originalWhiteSpace,interestBox;
	int designType, orientation[4], keepChecking=TRUE;
	float tEnd= 10.0, t= 300.0, delta, calc, random;
	//char result[MAX];

    /**
    一开始的space宽高各取整个广告布局的一半，位于广告布局的中央，在生产过程中则慢慢向四周扩大
    */
	//whiteSpace.x=BannerWidth/4; whiteSpace.y=BannerHeight/4;
	//whiteSpace.w=BannerWidth/2; whiteSpace.h=BannerHeight/2;

    whiteSpace.x=0; whiteSpace.y=0;
	whiteSpace.w=BannerWidth; whiteSpace.h=BannerHeight;

	//interestBox.x=atoi(argv[5]); interestBox.y=atoi(argv[6]);
	//interestBox.w=atoi(argv[7]); interestBox.h=atoi(argv[8]);

	designType = PLAIN;
	orientation[0]=orientation[1]=orientation[2]=orientation[3]=HORIZONTAL;

    srand(time(NULL)); //srand用一次就可以

    init(currentSolution,filename,sizes);
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
			chooseNeighbour(candidateSolution, whiteSpace, sizes, t);
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
printf("%s %d %d\n",bestSolution[i].name,bestSolution[i].x_pos,bestSolution[i].y_pos);
        Bitmap *b = load_bitmap(bestSolution[i].name);
        putPicture(bg,b,bestSolution[i].x_pos,bestSolution[i].y_pos);
    }
    save_bitmap(bg,"result4.jpg",1);

    FILE*f2;
    f2=fopen("output/result4.txt","w");
    fprintf(f2,"cost:\t%f\nBalanceCost:\t%f\nAlignmentCost:\t%f\n",cost(bestSolution, whiteSpace, interestBox, sizes, designType),BannerBalanceCost,BannerAlignmentCost);
    fclose(f2);
	return 0;
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
void chooseNeighbour(Element *candidate, Space space, int size, float t){
	int i, widthSteps, heightSteps, xJump, yJump;
	int x_pos2, y_pos2, test;
	//srand(system(NULL));
	for(i=0;i<size;i++){
		if(candidate[i].normalGroup == 1){
			widthSteps = (space.w - candidate[i].width)/20;
			heightSteps = (space.h - candidate[i].height)/20;
int xrand=rand();
int yrand =rand();
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

			candidate[i].x_pos = candidate[i].x_pos<space.x?space.x:candidate[i].x_pos;
			candidate[i].y_pos = candidate[i].y_pos<space.y?space.y:candidate[i].y_pos;

			candidate[i].x_pos = x_pos2 > space.x + space.w?(space.x+space.w)-candidate[i].width:candidate[i].x_pos;
			candidate[i].y_pos = y_pos2 > space.y + space.h?(space.y+space.h)-candidate[i].height:candidate[i].y_pos;

			//保证坐标值是20的倍数。
			//那这样如果之前设定为space.x，再减不就在space外了么？那只有可能space.x是20的倍数，所以即使减也是0
			candidate[i].x_pos -= candidate[i].x_pos%20!=0?candidate[i].x_pos%20:0;
			candidate[i].y_pos -= candidate[i].y_pos%20!=0?candidate[i].y_pos%20:0;

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

