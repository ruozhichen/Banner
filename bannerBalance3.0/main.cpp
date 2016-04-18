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
#define GRIDX 10
#define GRIDY 6
/**
��֮ǰ2.0�Ļ����ϣ����Խ�ÿ��Ԫ�ػ���a*b�����񣬸���ÿ��С�����weight�����ģ�����Ԫ�صġ����ġ���
�Ӷ��ڼ������塰���ġ�ʱ����Ԫ�صġ����ġ�����ȡ��ԭ�ȵ��������ꡣ
*/
using namespace std;
const int maxn=100;
char inputAddr[50] = "input/"; //�������Ե�ַ
int BannerHeight=640;
int BannerWidth=1000;
float BannerBalanceCost;
float BannerAlignmentCost;
struct Element{
    char name[maxn]; //������Ե�ַ��
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
flag=0���Ǳ�����input/
flag=1���Ǳ�����output/
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
��ʼ�������е�Ԫ��
���ı��ж�ȡ����Ԫ�ص�ͼƬ����
�ı���ʽ���£�
n
xxx.jpg
xxx.jpg
...
nΪԪ�ظ���
*/
void init(Element*current,char*filename,int sizes){
    char imgName[maxn];
    char tmpstr[maxn];
    char greyImgName[maxn];
    int tmp;
    FILE *f1;
    f1=fopen(filename,"r");
    fscanf(f1,"%d",&tmp);  //sizes�Ļ���ʵ��main�������Ѿ���ȡ���ڴ���Ԫ�����飬����Ͳ���Ҫ��
    for(int i=0;i<sizes;i++){
        fscanf(f1,"%s",imgName);
//printf("%d %s\n",i,imgName);
        strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
        strcat(tmpstr, imgName);

        strcpy(current[i].name,tmpstr);

        IplImage* img = cvLoadImage(tmpstr);
        IplImage* greyImg = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
        cvCvtColor(img,greyImg,CV_BGR2GRAY);//cvCvtColor(src,des,CV_BGR2GRAY)����ɫͼƬת���ɻҶ�ͼ
//printf("1\n");
        strcpy(greyImgName,"grey-");
        strcat(greyImgName,imgName);
        strcpy(tmpstr, inputAddr);  //inputAddr�洢������Ե�ַ��Ϊȫ�ֱ����������ڴ�����ǰ���ˡ�
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
        ///��ʼ�����޶���һ��ʼ��space������
        current[i].x_pos=rand()%(BannerWidth-current[i].width-BannerWidth/4)+BannerWidth/4;
        current[i].y_pos=rand()%(BannerHeight-current[i].height-BannerHeight/4)+BannerHeight/4;
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
//printf("4\n");
    }
    fclose(f1);
}
/**
��bitmap�ķ�ʽ��ͼƬbд�뵽a��ȥ
x��yΪb��a�е���ʼλ��
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
    fscanf(f1,"%d",&sizes); //Ԫ�ظ���
    fclose(f1);

    Element currentSolution[sizes];
    Element bestSolution[sizes];
    Element candidateSolution[sizes];

	Space whiteSpace,originalWhiteSpace,interestBox;
	int designType, orientation[4], keepChecking=TRUE;
	float tEnd= 10.0, t= 300.0, delta, calc, random;
	//char result[MAX];

    /**
    һ��ʼ��space��߸�ȡ������沼�ֵ�һ�룬λ�ڹ�沼�ֵ����룬����������������������������
    */
	//whiteSpace.x=BannerWidth/4; whiteSpace.y=BannerHeight/4;
	//whiteSpace.w=BannerWidth/2; whiteSpace.h=BannerHeight/2;

    whiteSpace.x=0; whiteSpace.y=0;
	whiteSpace.w=BannerWidth; whiteSpace.h=BannerHeight;

	//interestBox.x=atoi(argv[5]); interestBox.y=atoi(argv[6]);
	//interestBox.w=atoi(argv[7]); interestBox.h=atoi(argv[8]);

	designType = PLAIN;
	orientation[0]=orientation[1]=orientation[2]=orientation[3]=HORIZONTAL;

    srand(time(NULL)); //srand��һ�ξͿ���

    init(currentSolution,filename,sizes);
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
			chooseNeighbour(candidateSolution, whiteSpace, sizes, t);
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
			����6�б�֤Ԫ����space���棬�����и�ǰ�ᣬ����Ԫ�صĿ�ȡ��߶ȿ϶���С��space��
			*/
			x_pos2 = candidate[i].x_pos + candidate[i].width;
			y_pos2 = candidate[i].y_pos + candidate[i].height;

			candidate[i].x_pos = candidate[i].x_pos<space.x?space.x:candidate[i].x_pos;
			candidate[i].y_pos = candidate[i].y_pos<space.y?space.y:candidate[i].y_pos;

			candidate[i].x_pos = x_pos2 > space.x + space.w?(space.x+space.w)-candidate[i].width:candidate[i].x_pos;
			candidate[i].y_pos = y_pos2 > space.y + space.h?(space.y+space.h)-candidate[i].height:candidate[i].y_pos;

			//��֤����ֵ��20�ı�����
			//���������֮ǰ�趨Ϊspace.x���ټ�������space����ô����ֻ�п���space.x��20�ı��������Լ�ʹ��Ҳ��0
			candidate[i].x_pos -= candidate[i].x_pos%20!=0?candidate[i].x_pos%20:0;
			candidate[i].y_pos -= candidate[i].y_pos%20!=0?candidate[i].y_pos%20:0;

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

