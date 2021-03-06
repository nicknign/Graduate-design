// Android serverView.cpp : CAndroidserverView 类的实现
//

#include "stdafx.h"

#include "Android server.h"

#include "Android serverDoc.h"
#include "Android serverView.h"

#include <winsock2.h>

#include<iostream>  

#include<math.h>  

#include<time.h>  

using namespace std;  


#define N 20*20 //栅格个数

//蚂蚁数量  

#define M 75  

//最大循环次数NcMax  

int NcMax =500; 

//信息启发因子，期望启发式因子，全局信息素挥发参数，局部信息素挥发参数, 状态转移公式中的q0  

double alpha = 2, beta = 5, rou = 0.1, alpha1 = 0.1,  qzero = 0.1; 

double allDistance[N][N]; 

double Lnn; 

int C[N]={0}; //障碍点坐标


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAndroidserverView

IMPLEMENT_DYNCREATE(CAndroidserverView, CFormView)

BEGIN_MESSAGE_MAP(CAndroidserverView, CFormView)
	ON_COMMAND(ID_32771, &CAndroidserverView::OnstartServer)	

	ON_MESSAGE(WM_LINK,&CAndroidserverView::OnLink)	
	ON_MESSAGE(WM_INPUT2,&CAndroidserverView::SetInput)	
	ON_MESSAGE(WM_OUTPUT,&CAndroidserverView::SetOutput)
	ON_MESSAGE(WM_CAMERA,&CAndroidserverView::OnCamera)	

	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_COMMAND(ID_32772, &CAndroidserverView::Oncloseserver)
	ON_BN_CLICKED(IDC_BUTTON6, &CAndroidserverView::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CAndroidserverView::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CAndroidserverView::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_CHECK2, &CAndroidserverView::OnBnClickedCheck2)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_32774, &CAndroidserverView::OnImageAnalyzing)


	ON_BN_CLICKED(IDC_FIND, &CAndroidserverView::OnBnClickedFind)
	ON_COMMAND(ID_32775, &CAndroidserverView::OnCameraSet)
	ON_BN_CLICKED(IDC_Canculate_DIS, &CAndroidserverView::OnBnClickedCanculateDis)
	ON_COMMAND(ID_32776, &CAndroidserverView::OnMenushishijianshi)
	ON_BN_CLICKED(IDC_PictureProcess, &CAndroidserverView::OnBnClickedPictureprocess)
	ON_BN_CLICKED(IDC_advanceinline, &CAndroidserverView::OnBnClickedadvanceinline)
	ON_BN_CLICKED(IDC_gototarget, &CAndroidserverView::OnBnClickedgototarget)
	ON_BN_CLICKED(IDC_Qianjin, &CAndroidserverView::OnBnClickedQianjin)
	ON_BN_CLICKED(IDC_Houtui, &CAndroidserverView::OnBnClickedHoutui)
	ON_BN_CLICKED(IDC_Youzhuan, &CAndroidserverView::OnBnClickedYouzhuan)
	ON_BN_CLICKED(IDC_Zuozhuan, &CAndroidserverView::OnBnClickedZuozhuan)
	ON_BN_CLICKED(IDC_Tingzhi, &CAndroidserverView::OnBnClickedTingzhi)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, &CAndroidserverView::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_RUNTarget, &CAndroidserverView::OnBnClickedRuntarget)
	ON_BN_CLICKED(IDC_RoadCreat, &CAndroidserverView::OnBnClickedRoadcreat)
END_MESSAGE_MAP()

//障碍物检测函数
bool obtacleBetween(int x,int y) //TRUE表示有障碍物，FALSE表示没有
{	
	if(x == y)
		return false;
	int maxX,minX,maxY,minY;
	if( abs(x%20 - y%20) > abs(x/20 - y/20) )
	{
		if(x%20 > y%20)
		{
			maxX = x%20;
			maxY = x/20;
			minX = y%20;
			minY = y/20;
		}
		else
		{
			maxX = y%20;
			maxY = y/20;
			minX = x%20;
			minY = x/20;
		}
		if(maxX != minX)
		{
			double k = (double)(maxY - minY)/(maxX - minX);
			for(int i=minX;i!=maxX;i++)
			{
				int j=(int)((i-minX)*k + minY);
				//是否经过障碍物		
				for(int x = 0; x < N ; x++)
				{
					if(C[x] == -1)
						break;
					if(C[x] == j*20 + i)
						return true;
				}		
			}
			return false;
		}
	}
	else
	{
		if(x/20 > y/20)
		{
			maxX = x%20;
			maxY = x/20;
			minX = y%20;
			minY = y/20;
		}
		else
		{
			maxX = y%20;
			maxY = y/20;
			minX = x%20;
			minY = x/20;
		}
		if(maxY != minY)
			{
				double k = (double)(maxX - minX)/(maxY - minY);
				for(int i=minY;i!=maxY;i++)
				{

					int j=(int)((i-minY)*k + minX);
					//是否经过障碍物		
					for(int x = 0; x < N ; x++)
						{
							if(C[x] == -1)
								break;
							if(C[x] == i*20 + j)
								return true;
					}		
				}
				return false;
			}
	}
	return true;
}

class AntColonySystem  

{  

private:      

    double info[N][N], visible[N][N];//坐标点的信息素强度,坐标点的能见度  

public:   

    AntColonySystem()  

    {  

    }  

    //计算当前节点到下一节点转移的概率  

    double Transition(int i, int j);      

    //局部更新规则  

    void UpdateLocalPathRule(int i, int j);   

    //初始化  

    void InitParameter(double value);     

    //全局信息素更新  

    void UpdateGlobalPathRule(int* bestTour, int globalBestLength,int end);  

};  

//计算当前节点到下一节点转移的概率  

double AntColonySystem::Transition(int i, int j)  

{  

    if (i != j)  

    {  

        return (pow(info[i][j],alpha) * pow(visible[i][j], beta));  

    }  

    else  

    {  

        return 0.0;  

    }     

}  

//局部更新规则  

void AntColonySystem::UpdateLocalPathRule(int i, int j)  

{ 

	if(i<0)
		return;

    info[i][j] = (1.0 - alpha1) * info[i][j] + alpha1 * (1.0 / (N * Lnn));  

    info[j][i] = info[i][j];  

}  

//初始化  

void AntColonySystem::InitParameter(double value)  

{  

  //初始化路径上的信息素强度tao0  

    for(int i = 0; i < N; i++)  

    {  

        for(int j = 0; j < N; j++)  

        {                 

            info[i][j] = value;  

            info[j][i] = value;  

            if (i != j)  

            {  

                visible[i][j] = 1.0 / allDistance[i][j];  

                visible[j][i] = visible[i][j]; 

            }

        }  		

    }

	//障碍物可见度为0
	for(int i = 0; i < N ; i++)
	{
		if(C[i] == -1)
			break;
		for(int j=0; j<N ;j++)
		{
			visible[C[i]][j] = 0;
			visible[j][C[i]] = 0;
		}
	}

}  

//全局信息素更新  

void AntColonySystem::UpdateGlobalPathRule(int* bestTour, int globalBestLength,int end)  

{  

    for(int i = 0; i < N; i++)  

    {  

        int row = *(bestTour + 2 * i);  

        int col = *(bestTour + 2* i + 1); 

		if(row<0)
			continue;

		info[row][col] = (1.0 - rou) * info[row][col] + rou * (1.0 / globalBestLength);  

        info[col][row] =info[row][col];  

		if(col == end)

			break;

    }  

}  
//----------------------------------------------蚁群系统类

class ACSAnt  

{  

private:  

    AntColonySystem* antColony;  

protected:  

    int startCity, cururentCity,endCity;//初始坐标编号，当前坐标编号。化二维为一维处理。  

    int allowed[N];//禁忌表 

	bool visibletoEnd[N];//直线可见终点

    int Tour[N][2];//当前路径  

    int currentTourIndex;//当前路径索引，从0开始，存储蚂蚁经过城市的编号  

public:   

    ACSAnt(AntColonySystem* acs, int start,int end)  

    {  

        antColony = acs;   

        startCity = start;  

		endCity = end;

		//初始化直线可见终点

		for(int i=0; i<N ; i++)

			visibletoEnd[i] = !obtacleBetween(i,endCity);//TRUE表示可见，FALSE表示不可见
		int i=0;

    }     

    //开始搜索  

    int* Search();  

    //选择下一节点  

    int Choose();  

    //移动到下一节点  

    void MoveToNextCity(int nextCity);  

};  

//开始搜索  

int* ACSAnt::Search()  

{  

    cururentCity = startCity;  

    int toCity;  

    currentTourIndex = 0;  

    for(int i  = 0; i < N; i++)  

    {  

        allowed[i] = 1;  

    }  

    allowed[cururentCity] = 0;  

    int count = 0;  

    do  

    {  

        count++;      

        toCity = Choose();        

        if (toCity != endCity)  

        {             

            MoveToNextCity(toCity);  

            antColony->UpdateLocalPathRule(cururentCity, toCity);  

            cururentCity = toCity; 

			if( visibletoEnd[cururentCity] == true && toCity>=0 )
			{
				MoveToNextCity(endCity);
				antColony->UpdateLocalPathRule(cururentCity, endCity);
				break;
			}

        }		

    }while(toCity >= 0);      

    return *Tour;  

}  

//选择下一节点  

int ACSAnt::Choose()  

{  

    int nextCity = -1;        

    double q = rand()/(double)RAND_MAX;  

    //如果 q <= q0,按先验知识，否则则按概率转移，  

    if (q <= qzero)  

    {  

        double probability = -1.0;//转移到下一节点的概率  

        for(int i = 0; i < N; i++)  

        {  

            //去掉禁忌表中已走过的节点,从剩下节点中选择最大概率的可行节点  

            if (1 == allowed[i])  

            {  

                double prob = antColony->Transition(cururentCity, i);  

                if (prob  > probability)  

                {  

                    nextCity = i;  

                    probability = prob;  

                }  

            }  

        }  

    }  

    else  

    {  

        //按概率转移           

        double p = rand()/(double)RAND_MAX;//生成一个随机数,用来判断落在哪个区间段  

        double sum = 0.0;             

        double probability = 0.0;//概率的区间点，p 落在哪个区间段，则该点是转移的方向  

        //计算概率公式的分母的值  

        for(int i = 0; i < N; i++)  

        {  

            if (1 == allowed[i])  

            {  

                sum += antColony->Transition(cururentCity, i);  

            }  

        }  

        for(int j = 0; j < N; j++)  

        {  

            if (1 == allowed[j] && sum > 0)  

            {  

                probability += antColony->Transition(cururentCity, j)/sum;  

                if (probability >= p || (p > 0.9999 && probability > 0.9999))  

                {  

                    nextCity = j;  

                    break;  

                }  

            }  

        }     

    }  	
		return nextCity;  

}  

//移动到下一节点  

void ACSAnt::MoveToNextCity(int nextCity)  

{  

    allowed[nextCity]=0;  

    Tour[currentTourIndex][0] = cururentCity;  

    Tour[currentTourIndex][1] = nextCity;  

    currentTourIndex++;  

    cururentCity = nextCity;  

}  

//------------------------------------------蚁群算法执行类  

void calculateAllDistance()  

{  

    for(int i = 0; i < N; i++)  

    {  

        for(int j = 0; j < N; j++)  

        {  

            if (i != j)  

            {  

                allDistance[i][j] = sqrt(pow((i/20 - j/20),2.0) + pow((i%20 - j%20),2.0));  

                allDistance[j][i] = allDistance[i][j];  

            }  

        }  

    }  

}  



double calculateSumOfDistance(int* tour,int end)  

{  

    double sum = 0;  

    for(int i = 0; i< N ;i++)  

    {  

        int row = *(tour + 2 * i);  

        int col = *(tour + 2* i + 1);  

		if(row<0)
			continue;

        sum += allDistance[row][col]; 

		if( col == end )
			break;

    }  

    return sum;  

}  

	

	//------------------------------------------  

//选择下一个节点，配合下面的函数来计算的长度  

int ChooseNextNode(int currentNode, int visitedNode[])  

{  

    int nextNode = -1;        

    double shortDistance = 0.0;  

    for(int i = 0; i < N; i++)  

    {  

        //去掉已走过的节点,从剩下节点中选择距离最近的节点  

        if (1 == visitedNode[i])  

        {             

            if (shortDistance == 0.0)  

            {  

                shortDistance = allDistance[currentNode][i];  

                nextNode = i;  

            }  

            if(shortDistance < allDistance[currentNode][i])  

            {  

                nextNode = i;  

            }  

        }  

    }  

    return nextNode;  

}  

//给一个节点由最近邻距离方法计算长度  

double CalAdjacentDistance(int node)  

{  

    double sum = 0.0;  

    int visitedNode[N];  

    for(int j = 0; j < N; j++)  

    {  

        visitedNode[j] = 1;   

    }  

    visitedNode[node] = 0;  

    int currentNode = node;  

    int nextNode;  

    do  

    {  

        nextNode = ChooseNextNode(currentNode, visitedNode);  

        if (nextNode >= 0)  

        {  

            sum += allDistance[currentNode][nextNode];  

            currentNode= nextNode;  

            visitedNode[currentNode] = 0;  

        }         

    }while(nextNode >= 0);  

    sum += allDistance[currentNode][node];  

    return sum;  

}  

//---------------------------------结束---------------------------------------------  

// CAndroidserverView 构造/析构

CAndroidserverView::CAndroidserverView()
	: CFormView(CAndroidserverView::IDD)
	, m_speed(_T(""))
	, m_cameracheck(FALSE)
	, InputOrOutput(false)
	, m_trackshow(FALSE)
	, m_distance(0)
	, m_width(0)
	, Flag(FALSE)
{
	// TODO: 在此处添加构造代码
	frame = 0,target = 0; 
	image = 0,image2 = 0,hsv = 0,hsv2 = 0,hue = 0, hue2 = 0, mask = 0, mask2 = 0, backproject = 0, histimg = 0;
	speed=50;

	Focus = 1.78;  //通过查手机得到

	biaoding = 0;
	m_advanceinline = 0;

	m_speed.Format(_T("%d"),speed);	
	m_cameracheck=0;
	InputOrOutput=0;
	TheImage = 0;
	TheTarget = 0;
	TheBack = 0;
	track_object = 0;
	int hist_size[]={255};          //将H分量的值量化到[0,255]
	float ranges[]={0,360};
	float* rang = ranges;
	hist = cvCreateHist(1, hist_size, CV_HIST_ARRAY, &rang, 1);
	m_trackshow = TRUE;
	p_Car.x = 10;
	p_Car.y = 0;
	p_Target.x = -1;
	p_Target.y = -1;
	direction = 0;
	m_gototarget = FALSE;
	Shade = NULL;
	D1 = 0;

	for(int i=0;i<400;i++)
	{
		p_obstacle[i].x=-1;
		p_obstacle[i].y=-1;
	}
	for(int i=0;i<400;i++)
	{
		p_road[i].x=-1;
		p_road[i].y=-1;
	}
}

CAndroidserverView::~CAndroidserverView()
{
	cvReleaseImage( &TheImage );                        // 释放 ipl 占用的内存
	cvReleaseImage( &TheTarget );                        // 释放 ipl 占用的内存
	//delete pTD;
	delete[] Shade;
	exit(0);
}

void CAndroidserverView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_speed);
	DDX_Check(pDX, IDC_CHECK2, m_cameracheck);
	DDX_Check(pDX, IDC_CHECK1, m_trackshow);
	DDX_Text(pDX, IDC_Distance, m_distance);
	DDX_Text(pDX, IDC_width, m_width);
	DDX_Control(pDX, IDC_MAP, m_mapcontrol);
}

BOOL CAndroidserverView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	return CFormView::PreCreateWindow(cs);
}

void CAndroidserverView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	TheImage = cvLoadImage( "aa.jpg", 1 );
	TheImage1 = cvLoadImage("aa.jpg",1);
	/*CvSize ImgSize;
    ImgSize.height = 256;
    ImgSize.width =256;
    TheImage = cvCreateImage( ImgSize, IPL_DEPTH_8U, IMAGE_CHANNELS );
	IplImage* ipl = cvLoadImage( "aa.jpg", 1 );
	ResizeImage( ipl );*/

}


// CAndroidserverView 诊断

#ifdef _DEBUG
void CAndroidserverView::AssertValid() const
{
	CFormView::AssertValid();
}

void CAndroidserverView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CAndroidserverDoc* CAndroidserverView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAndroidserverDoc)));
	return (CAndroidserverDoc*)m_pDocument;
}
#endif //_DEBUG


// CAndroidserverView 消息处理程序

void CAndroidserverView::OnstartServer()
{
	// TODO: 在此添加命令处理程序代码
	RECVPARAM1 *pRecvparam1 = new RECVPARAM1;	
	pRecvparam1->psock=&sockTemp;
	pRecvparam1->psocksend=&sockSend;
	pRecvparam1->hwnd=m_hWnd;
	hThread=CreateThread(NULL,0,fun2proc,(LPVOID)pRecvparam1,0,NULL);
	CloseHandle(hThread);	//关闭该接受线程句柄，释放其引用计数
	
	GetDlgItem(IDC_LINK_STATE)->SetWindowText(_T("等待连接"));

	 CRect rect;
	 GetDlgItem(IDC_LINK_STATE)->GetClientRect(rect);
	 ClientToScreen(rect); //转换到对话框坐标系下
	 InvalidateRect(rect);

	
}

LRESULT CAndroidserverView::OnLink(WPARAM wParam,LPARAM lParam)
{
	//取出接收到的数据
	GetDlgItem(IDC_LINK_STATE)->SetWindowText(_T("已连接"));
	CRect rect;
	 GetDlgItem(IDC_LINK_STATE)->GetClientRect(rect);
	 ClientToScreen(rect); //转换到对话框坐标系下
	 InvalidateRect(rect);
	sockConn = (CSocket*)lParam;

	TheImage = cvLoadImage( "aa.jpg", 1 );    // 读取图片、缓存到一个局部变量 ipl 中
	cvCopy(TheImage,TheImage1);	
	Sleep(50);

	if(m_advanceinline)
	{
		OnBnClickedPictureprocess();
		Flag = Rundirect();
		//Flag = TRUE;
	}
	else if(m_gototarget)
	{
		Gototarget();
		Flag = true;
		//Flag = Rundirect();
	}
	else
		Flag = TRUE;

    if( !TheImage )                                    // 判断是否成功载入图片
        return 0;

	UpdateData(TRUE);
	if(m_trackshow)
		OnBnClickedFind();
	else
		ShowImage( TheImage, IDC_STC_DUMMY );            // 调用显示图片函数  	
	
	return 0;
}
DWORD WINAPI CAndroidserverView::fun1proc(LPVOID lpParameter)
{	
	CSocket* sock;
	sock=((RECVPARAM1*)lpParameter)->psocksend;
	HWND hwnd1=((RECVPARAM1*)lpParameter)->hwnd;
	delete lpParameter;	
	
	while(1)
	{
		::PostMessage(hwnd1,WM_CAMERA,0,0);
		Sleep(50);
	}
	
}
DWORD WINAPI CAndroidserverView::fun2proc(LPVOID lpParameter1)
{
	
	CSocket* sockCon,* sock;	
	DWORD dwTemp = 0;	
	sockCon=((RECVPARAM1*)lpParameter1)->psock;
	sock=((RECVPARAM1*)lpParameter1)->psocksend;
	HWND hwnd1=((RECVPARAM1*)lpParameter1)->hwnd;			
	delete lpParameter1;
	sockCon->Create(6800);
	sockCon->Listen(1);//只接受一个连接	
	sockCon->Accept(*sock);//注意，sockTemp已交了自己的指针地址到sockSend，故不用Close
	::PostMessage(hwnd1,WM_LINK,0,(LPARAM)sock);
	//BOOL bTest = sock->AsyncSelect(0);//由于CSocket实际是异步，将它变为同步（阻塞）方式。
	//sock->IOCtl( FIONBIO, &dwTemp);//用IOCtl要将AsyncSelect的第一个参数为0，参看MSDN
	sock->AsyncSelect(0);
	sock->IOCtl( FIONBIO, &dwTemp);//变为阻塞方式
	CString str;	
	str = "hello!";
	sock->Send(str, 6);//传送字符串到接收方（Client端）		
	CString str1,str2;	
	str2="aa.jpg";
		
		
	UINT uiLength;	
	char len[6];
	
while(true)
{
	sock->Receive(len,6);		
	//AfxMessageBox(len);
	::PostMessage(hwnd1,WM_INPUT2,0,0);
	uiLength=atoi(len);
	int  iBufSize = 1024  * 200;
	int  iSize = iBufSize;
	LPBYTE  pBuf = new BYTE[iBufSize];
	int  iNumByte;
	UINT uiTotal = 0;
	CFile file;
	file.Open(str2, CFile::modeCreate | CFile::modeWrite);
	BOOL bFileFail=FALSE;
	while(uiTotal < uiLength)
	{
		int iEnd=0;	
		iNumByte=sock->Receive(&iEnd, sizeof(byte));
		if(iNumByte == SOCKET_ERROR)
		{
			AfxMessageBox(_T("接收信号错误！"));
			goto ExitLable2;
		}
		if(iEnd==1)
		{
			AfxMessageBox(_T("发送端终止！"));
			goto ExitLable2;
		}

		if((int)(uiLength - uiTotal) < iBufSize)
			iSize = uiLength - uiTotal;
		int iCount=0;
		while(iCount<iSize)
		{
			iNumByte = sock->Receive(pBuf, iSize-iCount);
			if(iNumByte == SOCKET_ERROR)
			{
				AfxMessageBox(_T("接收错误！"));
				goto ExitLable2;
			}
			iCount+=iNumByte;
			file.Write(pBuf, iNumByte);
		}
		uiTotal += iCount;//以实际接收字节为准	
		str.Format(_T("接收进度：%d%%"),int(((double)uiTotal/uiLength)*100));	
	}
	//AfxMessageBox(_T("接收文件成功！"));
	bFileFail=TRUE;
ExitLable2:	
	delete[] pBuf;		
	file.Close();
	//文件接收失败，则删除接收文件
	::PostMessage(hwnd1,WM_OUTPUT,0,0);
	if(!bFileFail)
	{
		CFile::Remove( str2 );
	}	
	::PostMessage(hwnd1,WM_LINK,0,(LPARAM)sock);
	Sleep(10);
}

	sock->Close();	
	return 0;
}
void CAndroidserverView::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	char buf[3];
	char* ch=m_speed.GetBuffer(0);
	buf[0]='0';
	buf[1]=ch[0];
	buf[2]=ch[1];
	if(!InputOrOutput)
		sockConn->Send(buf,3);
	//send(sockConn,(char*)buf,sizeof(buf)-1,0);//返回数据
}

void CAndroidserverView::OnPaint()
{
	CPaintDC dc(this); // device context for painting	
	
	CRect rect;
	m_mapcontrol.GetWindowRect(rect);     // ListCtrl矩形
	//m_listCtrl.GetClientRect(rect);// 对话框客户区矩形
	ScreenToClient(rect);             // 屏幕坐标转换为客户区坐标
	// 取得原来的矩形，在此基础上修改。这里我直接设置个坐标
	//rect.top = 122;
	//rect.left = 226;
	rect.right = rect.left + 400;
	rect.bottom = rect.top + 400;
	m_mapcontrol.MoveWindow(rect);

	DrawMap();
	
	ShowImage( TheImage1, IDC_STC_DUMMY );            // 调用显示图片函数   
	ShowImage( TheTarget, IDC_TARGETMAIN );            // 调用显示图片函数
	ShowImage( TheBack,IDC_BACKMAIN );
   


	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CFormView::OnPaint()
}

/*UINT CAndroidserverView::ReceiveDataThread(LPVOID lpParam)
{
	CSocket* sockRecv = (CSocket*)lpParam;
	//CFileDialog dlg(FALSE,NULL,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
	//	_T("所有文件 (*.*)|*.*||"));
	//while(dlg.DoModal()!=IDOK)
	//{
	//	AfxMessageBox(_T("选择文件出错，请重新选择！"));
	//}		
	CString str,str1,str2;
	//CSocket  sockRecv;
	//sockRecv.Create();
	//pDlg->m_CtrlIPSend.GetWindowText(str);
	//pDlg->GetDlgItem(IDC_BUTTON_RECEIVE)->EnableWindow(FALSE);
	//while(sockRecv.Connect(str,pDlg->m_iDataPort2)==0)//接收方地址，若上网，可改为实际IP地址，端口要跟Server端相同。
	//{
	//	Sleep(50);
	//}
	//pDlg->GetDlgItem(IDC_BUTTON_REC_END)->EnableWindow(TRUE);
	//pDlg->m_bRecRun=TRUE;
	//str2=dlg.GetPathName();
	str2="aa.jpg";
	CFile file;
	file.Open(str2, CFile::modeCreate | CFile::modeWrite);
	BOOL bFileFail=FALSE;
	DWORD dwTemp =  0;
	sockRecv->AsyncSelect(0);
	sockRecv->IOCtl( FIONBIO, &dwTemp);//变为阻塞方式
	
	UINT uiLength;
	//sockRecv->Receive(&uiLength, 5);//接收发方（Server端）的文件大小
	char len[5];
	sockRecv->Receive(len,5);
	uiLength=atoi(len);
	int  iBufSize = 1024  * 200;
	int  iSize = iBufSize;
	LPBYTE  pBuf = new BYTE[iBufSize];
	int  iNumByte;
	UINT uiTotal = 0;
	while(uiTotal < uiLength)
	{
		int iEnd=0;
	//	if(pDlg->m_bRecEnd)
	//	{
	//		AfxMessageBox("接收端终止！");
	//		goto ExitLable2;
	//	}
		iNumByte=sockRecv->Receive(&iEnd, sizeof(byte));
		if(iNumByte == SOCKET_ERROR)
		{
			AfxMessageBox(_T("接收信号错误！"));
			goto ExitLable2;
		}
		if(iEnd==1)
		{
			AfxMessageBox(_T("发送端终止！"));
			goto ExitLable2;
		}

		if((int)(uiLength - uiTotal) < iBufSize)
			iSize = uiLength - uiTotal;
		int iCount=0;
		while(iCount<iSize)
		{
			iNumByte = sockRecv->Receive(pBuf, iSize-iCount);
			if(iNumByte == SOCKET_ERROR)
			{
				AfxMessageBox(_T("接收错误！"));
				goto ExitLable2;
			}
			iCount+=iNumByte;
			file.Write(pBuf, iNumByte);
		}
		uiTotal += iCount;//以实际接收字节为准
		//pDlg->m_CtrlProgressRec.SetPos(int(((double)uiTotal/uiLength)*100));
		str.Format(_T("接收进度：%d%%"),int(((double)uiTotal/uiLength)*100));
		//pDlg->GetDlgItem(IDC_STATIC_REC)->GetWindowText(str1);
		//if(str1!=str)
			//pDlg->GetDlgItem(IDC_STATIC_REC)->SetWindowText(str);
	}
	AfxMessageBox(_T("接收文件成功！"));
	bFileFail=TRUE;
ExitLable2:
	//pDlg->m_bRecRun=FALSE;
	delete[] pBuf;
	delete[] len;
	file.Close();
	//文件接收失败，则删除接收文件
	if(!bFileFail)
	{
		CFile::Remove( str2 );
	}
	sockRecv->Close();
	//pDlg->m_CtrlProgressRec.SetPos(0);
	//pDlg->GetDlgItem(IDC_BUTTON_REC_END)->EnableWindow(FALSE);
	//pDlg->GetDlgItem(IDC_BUTTON_RECEIVE)->EnableWindow(TRUE);
	//pDlg->GetDlgItem(IDC_STATIC_REC)->SetWindowText("接收进度：");
	return 0;
}*/

void CAndroidserverView::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CFormView::OnClose();
}

void CAndroidserverView::Oncloseserver()
{
	// TODO: 在此添加命令处理程序代码
	
	sockConn->Close();
	sockSend.Close();
	sockTemp.Close();
}

void CAndroidserverView::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	if(speed<100)
	{
		speed++;
		m_speed.Format(_T("%d"),speed);
		UpdateData(FALSE);
	}
}

void CAndroidserverView::OnBnClickedButton7()
{
	// TODO: 在此添加控件通知处理程序代码
	if(speed>0)
	{
		speed--;
		m_speed.Format(_T("%d"),speed);
		UpdateData(FALSE);
	}
}

void CAndroidserverView::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码
	char buf='T';
	if(!InputOrOutput)
		sockConn->Send(&buf,1);
}

void CAndroidserverView::OnBnClickedCheck2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(m_cameracheck)
	{
		RECVPARAM1 *pRecvparam1 = new RECVPARAM1;		
		pRecvparam1->psocksend=sockConn;	
		pRecvparam1->hwnd=m_hWnd;
		Flag = TRUE;
		hThread1=CreateThread(NULL,0,fun1proc,(LPVOID)pRecvparam1,0,NULL);
	}
	else
	{
		TerminateThread(hThread1,0);
	}
}

LRESULT CAndroidserverView::SetInput(WPARAM wParam,LPARAM lParam)
{
	InputOrOutput = 1;
	return 0;
}

LRESULT CAndroidserverView::SetOutput(WPARAM wParam,LPARAM lParam)
{
	InputOrOutput = 0;
	return 0;
}
LRESULT CAndroidserverView::OnCamera(WPARAM wParam,LPARAM lParam)
{
	if(Flag)
	{
		char buf='T';
		if(!InputOrOutput)
		sockConn->Send(&buf,1);
		Flag = FALSE;
	}
	return 0;
}
void CAndroidserverView::ShowImage( IplImage* img, UINT ID )    // ID 是Picture Control控件的ID号
{
	if(!img)
		return;	
    CDC* pDC = GetDlgItem( ID ) ->GetDC();        // 获得显示控件的 DC
    HDC hDC = pDC ->GetSafeHdc();                // 获取 HDC(设备句柄) 来进行绘图操作
    CRect rect;
    GetDlgItem(ID) ->GetClientRect( &rect );
    int rw = rect.right - rect.left;            // 求出图片控件的宽和高
    int rh = rect.bottom - rect.top;
    int iw = img->width;                        // 读取图片的宽和高
    int ih = img->height;
    int tx = (int)(rw - iw)/2;                    // 使图片的显示位置正好在控件的正中
    int ty = (int)(rh - ih)/2;
    //SetRect( rect, tx, ty, tx+iw, ty+ih );

	double scale;
	if(rw > rh)
		scale = (double)rw/img->width;
	else
		scale = (double)rh/img->height;
	CvSize dstSize;
	dstSize.width = 200;//scale*img->width;
	dstSize.height = 200;//scale*img->height;	
	theshow = cvCreateImage(dstSize, IPL_DEPTH_8U, IMAGE_CHANNELS);
	cvResize(img, theshow, CV_INTER_LINEAR);
    CvvImage cimg;
    cimg.CopyOf( theshow );                            // 复制图片
    cimg.DrawToHDC( hDC, &rect );                // 将图片绘制到显示控件的指定区域内
    ReleaseDC( pDC );
	cvReleaseImage(&theshow);

	
	/*CWnd *pWin = GetDlgItem(IDC_STC_DUMMY);
    CDC *pDC1 = pWin->GetDC();
	CPen pen(PS_DOT,1,RGB(255, 0, 0));
	pDC1->SelectObject(&pen);
	GetDlgItem(IDC_STC_DUMMY) ->GetClientRect( &rect );
	pDC1->MoveTo(0,rect.Height()/2);
	pDC1->LineTo(rect.Width(),rect.Height()/2);*/
	

}
void CAndroidserverView::OnImageAnalyzing()
{
	// TODO: 在此添加命令处理程序代码
	ImageAnalyzingDLG IA;
	IA.DoModal();
	TheTarget = IA.TheTarget;
	TheBack = IA.TheBack;
	selectrect = IA.select;
	track_object = -1;
	ShowImage( TheTarget, IDC_TARGETMAIN ); 
	ShowImage( TheBack,IDC_BACKMAIN );
	/*pTD = new ImageAnalyzingDLG(); //给指针分配内存  
	pTD->Create(IDD_ImageAnalyzing); //创建一个非模态对话框  
	pTD->ShowWindow(SW_SHOWNORMAL); //显示非模态对话框*/  

}

void CAndroidserverView::OnBnClickedFind()
{
	// TODO: 在此添加控件通知处理程序代码		
	int backproject_mode = 0;
	int select_object = 0;	
	int show_hist = 1; 	
	CvRect track_window=selectrect;	
	CvConnectedComp track_comp;
	int hdims = 48;     // 划分HIST的个数，越高越精确
	float hranges_arr[] = {0,180};
	float* hranges = hranges_arr;
	int vmin = 10, vmax = 256, smin = 30;
    int bin_w;
	int hist_size[]={255};          //将H分量的值量化到[0,255]
	float ranges[]={0,360};    //H分量的取值范围是[0,360)
	float* rang = ranges;
	//CvHistogram* hist=cvCreateHist(1, hist_size, CV_HIST_ARRAY, &rang, 1);
        frame = TheImage1;
		target = TheTarget;

        if( !frame )
           return;
		if( !target )
           return;
	
        if( !image )
        {
             /* allocate all the buffers */
            image = cvCreateImage( cvGetSize(frame), 8, 3 );
            image->origin = frame->origin;
            hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
            hue = cvCreateImage( cvGetSize(frame), 8, 1 );
            mask = cvCreateImage( cvGetSize(frame), 8, 1 );
            backproject = cvCreateImage( cvGetSize(frame), 8, 1 );
            hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );  // 计算直方图
            histimg = cvCreateImage( cvSize(320,200), 8, 3 );
            cvZero( histimg );        
            
        }

        cvCopy( frame, image, 0 );
        cvCvtColor( image, hsv, CV_BGR2HSV );  // 彩色空间转换 BGR to HSV          
		
         if( track_object )
        {
            int _vmin = vmin, _vmax = vmax;

            cvInRangeS( hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
                        cvScalar(180,256,MAX(_vmin,_vmax),0), mask );  // 得到二值的MASK
            cvSplit( hsv, hue, 0, 0, 0 );  // 只提取 HUE 分量

            if( track_object < 0 )
            {
                float max_val = 0.f;
                cvSetImageROI( hue, selectrect );  // 得到选择区域 for ROI
                cvSetImageROI( mask, selectrect ); // 得到选择区域 for mask
                cvCalcHist( &hue, hist, 0, mask ); // 计算直方图
                cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );  // 只找最大值
                cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 ); // 缩放 bin 到区间 [0,255] 
                cvResetImageROI( hue );  // remove ROI
                cvResetImageROI( mask );
                track_window = selectrect;
                track_object = 1;
				//histyuan = hist;

                cvZero( histimg );
                bin_w = histimg->width / hdims;  // hdims: 条的个数，则 bin_w 为条的宽度
                            
            }

            cvCalcBackProject( &hue, backproject, hist );  // 使用 back project 方法
            cvAnd( backproject, mask, backproject, 0 );
            
            // calling CAMSHIFT 算法模块
            cvCamShift( backproject, track_window,
                        cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
                        &track_comp, &track_box );
            track_window = track_comp.rect;
            
           if( image->origin )
                track_box.angle = -track_box.angle;
		   if(track_window.width > 0)
            //cvEllipseBox( TheImage, track_box, CV_RGB(255,0,0), 2, CV_AA, 0 );
			DrawBox(track_box,TheImage1);
        }

        
			ShowImage( TheImage1, IDC_STC_DUMMY );            // 调用显示图片函数 
		
}

void CAndroidserverView::DrawBox(CvBox2D box,IplImage* img) 
  { 
      CvPoint2D32f point[4]; 
      int i; 
      for ( i=0; i<4; i++) 
      { 
          point[i].x = 0; 
          point[i].y = 0; 
      } 
     cvBoxPoints(box, point); //计算二维盒子顶点 
     CvPoint pt[4]; 
     for ( i=0; i<4; i++) 
     { 
         pt[i].x = (int)point[i].x; 
         pt[i].y = (int)point[i].y; 
     } 
     cvLine( img, pt[0], pt[1],CV_RGB(255,0,0), 2, 8, 0 ); 
     cvLine( img, pt[1], pt[2],CV_RGB(255,0,0), 2, 8, 0 ); 
     cvLine( img, pt[2], pt[3],CV_RGB(255,0,0), 2, 8, 0 ); 
     cvLine( img, pt[3], pt[0],CV_RGB(255,0,0), 2, 8, 0 ); 
 } 

void CAndroidserverView::OnCameraSet()
{
	// TODO: 在此添加命令处理程序代码
	pTD1 = new CameraSet(); //给指针分配内存  
	pTD1->chuizhi_F = Focus;
	pTD1->Create(IDD_CameraSet); //创建一个非模态对话框  
	pTD1->ShowWindow(SW_SHOWNORMAL); //显示非模态对话框
	biaoding = 1;
	
}

void CAndroidserverView::OnBnClickedCanculateDis()
{
	// TODO: 在此添加控件通知处理程序代码
	int l_pixcel = 0;	
	//获取目标与底边相差像素值
	l_pixcel = TheImage->height - (int)track_box.center.y;//现在取中心是不对的，应取与路面相接点
	m_distance = CanculateDistance(l_pixcel);
	int w_pixcel = 0;
	w_pixcel = (int)track_box.center.x - TheImage->width/2;
	m_width = CanculateWidth(abs(w_pixcel),l_pixcel);
	UpdateData(FALSE);

	///////////////////////////////////////////获取目标地图坐标,仅第一次启动时获取（1格相当于10CM，暂定）
	p_Target.y = (long)(m_distance/10 + 1);
	if(w_pixcel < 0)
		p_Target.x = (long)(m_width/10 + 10);
	else
		p_Target.x = (long)(10 - m_width/10);
	
	DrawMap();
}

double CAndroidserverView::CanculateDistance(int l_pixcel)
{
	//距离计算公式的实现

	//初始化参数
	double F,S,a,b,c,l,L,K,N1,H;
	pTD1->UpdateData(TRUE);
	F = pTD1->chuizhi_F;
	S = pTD1->chuizhi_S;
	b = pTD1->chuizhi_B;
	K = pTD1->chuizhi_K;
	H = pTD1->chuizhi_H;
	l = l_pixcel*K;

	/*c = arcsin(F/S)
	  N = sqrt(s^2 + l^2 - 2S*l*cosc)
	  a = arccos(...)
	  L = H*(tan(a+b)-tan(b))*/

	c = asin(F/S);	
	N1 = sqrt(S*S + l*l - 2*S*l*cos(c));	
	a = acos((S*S + N1*N1 - l*l)/(2*S*N1));	
	L = H*(tan(a + b) - tan(b));

	return L;
}

double CAndroidserverView::CanculateWidth(int w_pixcel,int l_pixcel)
{
	//计算实际宽度
	double W;
	
	//初始化参数
	double F,S,a,b,c,l,L,K,N1,H,Fr,Sr,lr,w;
	pTD1->UpdateData(TRUE);
	F = pTD1->chuizhi_F;
	S = pTD1->chuizhi_S;
	b = pTD1->chuizhi_B;
	K = pTD1->chuizhi_K;
	H = pTD1->chuizhi_H;
	l = l_pixcel*K;

	/*c = arcsin(F/S)
	  N = sqrt(s^2 + l^2 - 2S*l*cosc)
	  a = arccos(...)
	  L = H*(tan(a+b)-tan(b))*/

	c = asin(F/S);	
	N1 = sqrt(S*S + l*l + 2*S*l*cos(c));	
	a = acos((S*S + N1*N1 - l*l)/(2*S*N1));	
	L = H*(tan(a + b) - tan(b));

	lr = (TheImage->height/2 - l_pixcel)*K;
	w = w_pixcel*K;
	Fr = sqrt(F*F + lr*lr);
	Sr = H/cos(a+b);
	W = w*(Sr/Fr);

	

	return W;
}

int CAndroidserverView::CanculateWpixel(double W,int l_pixcel)
{
	//计算宽度像素
	int w_pixel;
	
	//初始化参数
	double F,S,a,b,c,l,L,K,N1,H,Fr,Sr,lr,w;
	pTD1->UpdateData(TRUE);
	F = pTD1->chuizhi_F;
	S = pTD1->chuizhi_S;
	b = pTD1->chuizhi_B;
	K = pTD1->chuizhi_K;
	H = pTD1->chuizhi_H;
	l = l_pixcel*K;

	/*c = arcsin(F/S)
	  N = sqrt(s^2 + l^2 - 2S*l*cosc)
	  a = arccos(...)
	  L = H*(tan(a+b)-tan(b))*/

	c = asin(F/S);	
	N1 = sqrt(S*S + l*l + 2*S*l*cos(c));	
	a = acos((S*S + N1*N1 - l*l)/(2*S*N1));	
	L = H*(tan(a + b) - tan(b));

	lr = (TheImage->height/2 - l_pixcel)*K;
	
	Fr = sqrt(F*F + lr*lr);
	Sr = H/cos(a+b);
	w = W/(Sr/Fr);
	w_pixel =(int)( w/K);

	

	return w_pixel;
}

void CAndroidserverView::OnMenushishijianshi()
{
	// TODO: 在此添加命令处理程序代码
	UpdateData(TRUE);
	if(!m_cameracheck)
	{
		RECVPARAM1 *pRecvparam1 = new RECVPARAM1;		
		pRecvparam1->psocksend=sockConn;	
		pRecvparam1->hwnd=m_hWnd;
		Flag = TRUE;
		hThread1=CreateThread(NULL,0,fun1proc,(LPVOID)pRecvparam1,0,NULL);
	}
	else
	{
		TerminateThread(hThread1,0);
	}
	m_cameracheck = !m_cameracheck;
}

void CAndroidserverView::OnBnClickedPictureprocess()
{
	// TODO: 在此添加控件通知处理程序代码
	long Rvalue,Gvalue,Bvalue;
	Rvalue = 0;
	Bvalue = 0;
	Gvalue = 0;
	int width = TheBack->width;
	int height = TheBack->height;
	int row,col;
	for( row=0;row<height;row++ )
		for( col=0;col<width;col++ )
		{
			Bvalue +=CV_IMAGE_ELEM(TheBack, unsigned char, row, col*3+0);
			Gvalue +=CV_IMAGE_ELEM(TheBack, unsigned char, row, col*3+1);
			Rvalue +=CV_IMAGE_ELEM(TheBack, unsigned char, row, col*3+2);
		}
	Rvalue = Rvalue/(width*height);
	Gvalue = Gvalue/(width*height);
	Bvalue = Bvalue/(width*height);//求取地面平均颜色

	int xstart,xend,ystart,yend,x,y;
	long Rpin=0,Gpin=0,Bpin=0;
	int i=0;
	int Hwidth = TheImage->width;
	int Hheight = TheImage->height;
	IplImage *ground = 0;
	ground = cvCreateImage( cvSize(TheImage->width, TheImage->height), IPL_DEPTH_8U, 3 );	
	image = cvCreateImage( cvSize(TheImage->width, TheImage->height), IPL_DEPTH_8U, 3 );
	cvCopy(TheImage,image);

	for(xstart=0,xend=Hwidth/3;xend<=Hwidth;xstart+=Hwidth/3,xend+=Hwidth/3)
		for(ystart=0,yend=Hheight/3;yend<=Hheight;yend+=Hheight/3,ystart+=Hheight/3)
		{
			for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					Bpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+0);
					Gpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+1);
					Rpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+2);
				}
			Bpin = Bpin/((Hwidth/3)*(Hheight/3));
			Gpin = Gpin/((Hwidth/3)*(Hheight/3));
			Rpin = Rpin/((Hwidth/3)*(Hheight/3));

			if( abs(Bpin - Bvalue) < 30 && abs(Gpin - Gvalue) < 30 && abs(Rpin - Rvalue) < 30)
			{
				for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					CV_IMAGE_ELEM(ground, unsigned char, y, x*3+0)=255;
					CV_IMAGE_ELEM(ground, unsigned char, y, x*3+1)=0;
					CV_IMAGE_ELEM(ground, unsigned char, y, x*3+2)=0;

					CV_IMAGE_ELEM(image, unsigned char, y, x*3+0)=(uchar)Bvalue;
					CV_IMAGE_ELEM(image, unsigned char, y, x*3+1)=(uchar)Gvalue;
					CV_IMAGE_ELEM(image, unsigned char, y, x*3+2)=(uchar)Rvalue;
				}
			}
			else
			{
				for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					CV_IMAGE_ELEM(ground, unsigned char, y, x*3+0)=0;
					CV_IMAGE_ELEM(ground, unsigned char, y, x*3+1)=0;
					CV_IMAGE_ELEM(ground, unsigned char, y, x*3+2)=255;
				}
			}
			i++;
		}
		//cvNamedWindow( "groundTest01", 1 );
		//cvShowImage( "groundTest01",ground );             
		cvReleaseImage( &ground);											//九方格预处理

	Rpin=0;
	Gpin=0;
	Bpin=0;
	i=0;
	IplImage *ground2 = 0;
	ground2 = cvCreateImage( cvSize(TheImage->width, TheImage->height), IPL_DEPTH_8U, 3 );	

	for(xstart=0,xend=Hwidth/9;xend<=Hwidth;xstart+=Hwidth/9,xend+=Hwidth/9)
		for(ystart=0,yend=Hheight/9;yend<=Hheight;yend+=Hheight/9,ystart+=Hheight/9)
		{
			for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					Bpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+0);
					Gpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+1);
					Rpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+2);
				}
			Bpin = Bpin/((Hwidth/9)*(Hheight/9));
			Gpin = Gpin/((Hwidth/9)*(Hheight/9));
			Rpin = Rpin/((Hwidth/9)*(Hheight/9));

			if( abs(Bpin - Bvalue) < 30 && abs(Gpin - Gvalue) < 30 && abs(Rpin - Rvalue) < 30)
			{
				for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					CV_IMAGE_ELEM(ground2, unsigned char, y, x*3+0)=255;
					CV_IMAGE_ELEM(ground2, unsigned char, y, x*3+1)=0;
					CV_IMAGE_ELEM(ground2, unsigned char, y, x*3+2)=0;

					CV_IMAGE_ELEM(image, unsigned char, y, x*3+0)=(uchar)Bvalue;
					CV_IMAGE_ELEM(image, unsigned char, y, x*3+1)=(uchar)Gvalue;
					CV_IMAGE_ELEM(image, unsigned char, y, x*3+2)=(uchar)Rvalue;
				}
			}
			else
			{
				for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					CV_IMAGE_ELEM(ground2, unsigned char, y, x*3+0)=0;
					CV_IMAGE_ELEM(ground2, unsigned char, y, x*3+1)=0;
					CV_IMAGE_ELEM(ground2, unsigned char, y, x*3+2)=255;
				}
			}
			i++;
		}
		//cvNamedWindow( "groundTest02", 1 );
		//cvShowImage( "groundTest02",ground2 );             
		cvReleaseImage( &ground2);											//八十一方格预处理

	Rpin=0;
	Gpin=0;
	Bpin=0;
	i=0;
	IplImage *ground3 = 0;
	ground3 = cvCreateImage( cvSize(TheImage->width, TheImage->height), IPL_DEPTH_8U, 3 );	

	for(xstart=0,xend=Hwidth/27;xend<=Hwidth;xstart+=Hwidth/27,xend+=Hwidth/27)
		for(ystart=0,yend=Hheight/27;yend<=Hheight;yend+=Hheight/27,ystart+=Hheight/27)
		{
			for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					Bpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+0);
					Gpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+1);
					Rpin +=CV_IMAGE_ELEM(image, unsigned char, y, x*3+2);
				}
			Bpin = Bpin/((Hwidth/27)*(Hheight/27));
			Gpin = Gpin/((Hwidth/27)*(Hheight/27));
			Rpin = Rpin/((Hwidth/27)*(Hheight/27));

			if( abs(Bpin - Bvalue) < 40 && abs(Gpin - Gvalue) < 40 && abs(Rpin - Rvalue) < 40)
			{
				for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					CV_IMAGE_ELEM(ground3, unsigned char, y, x*3+0)=255;
					CV_IMAGE_ELEM(ground3, unsigned char, y, x*3+1)=0;
					CV_IMAGE_ELEM(ground3, unsigned char, y, x*3+2)=0;

					CV_IMAGE_ELEM(image, unsigned char, y, x*3+0)=(uchar)Bvalue;
					CV_IMAGE_ELEM(image, unsigned char, y, x*3+1)=(uchar)Gvalue;
					CV_IMAGE_ELEM(image, unsigned char, y, x*3+2)=(uchar)Rvalue;
				}
			}
			else
			{
				for(x=xstart;x<xend;x++)
				for(y=ystart;y<yend;y++)
				{
					CV_IMAGE_ELEM(ground3, unsigned char, y, x*3+0)=0;
					CV_IMAGE_ELEM(ground3, unsigned char, y, x*3+1)=0;
					CV_IMAGE_ELEM(ground3, unsigned char, y, x*3+2)=255;
				}
			}
			i++;
		}
	//	cvNamedWindow( "groundTest03", 1 );
	//	cvNamedWindow( "ground00", 1 );
		             
		//cvReleaseImage( &ground3);											//九成八十一方格预处理

		/////////////////////////////////////////////////前方障碍物判断
		int lpixel;
		int wpixel;	
		int* zoubian = new int[TheImage->height*TheImage->width];
		int* youbian = new int[TheImage->height*TheImage->width];
		int yindex=0;
		int zindex=0;
		for(lpixel=TheImage->height-1;lpixel>=TheImage->height/2;lpixel--)
		{
			wpixel = CanculateWpixel(20,TheImage->height - lpixel);
			for(int i=0;i<5;i++)
			{
				x = TheImage->width/2 + wpixel/2 + i;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+0)=0;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+1)=0;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+2)=0;
				x = TheImage->width/2 - wpixel/2 - i;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+0)=0;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+1)=0;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+2)=0;
			}
			x = TheImage->width/2;
			for(int j=0;j<wpixel/2;j++)
			{
				youbian[yindex] = CV_IMAGE_ELEM(ground3, unsigned char, lpixel, (x+j)*3+2);
				yindex++;
			}
			for(int j=0;j<wpixel/2;j++)
			{
				zoubian[zindex] = CV_IMAGE_ELEM(ground3, unsigned char, lpixel, (x-j)*3+2);
				zindex++;
			}
		}
		for(lpixel=TheImage->height/2;lpixel>TheImage->height/2-5;lpixel--)
		{
			wpixel = CanculateWpixel(20,TheImage->height - lpixel);
			for(int i=0;i<wpixel/2;i++)
			{
				x = TheImage->width/2 + i;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+0)=0;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+1)=0;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+2)=0;
				x = TheImage->width/2 - i;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+0)=0;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+1)=0;
				CV_IMAGE_ELEM(ground3, unsigned char, lpixel, x*3+2)=0;
			}			
		}

		int zhuanwan;
		Rvalue = 0;
		Bvalue = 0;
		for(int i=0;i<yindex;i++)
			Rvalue += youbian[i];
		for(int i=0;i<yindex;i++)
			Bvalue += zoubian[i];
		if(Rvalue == 0 && Bvalue == 0)
			zhuanwan = 0;
		else if(Rvalue > Bvalue)
				zhuanwan = 1;//左转
		else if(Rvalue <= Bvalue)
				zhuanwan = 2;//右转


		//cvShowImage( "ground00", image );
		//cvShowImage( "groundTest03",ground3 );
		ShowImage( ground3, IDC_STC_SHADE );

		cvReleaseImage( &ground3);
		cvReleaseImage( &image);
		delete zoubian;
		delete youbian;

		ZHUANWAN = zhuanwan;
    
}

void CAndroidserverView::MeanShiftSmooth(const IplImage* src, IplImage* dst, int hs, int hr, int maxIter)
{
if(!src)return;

IplImage* srcLUV = cvCreateImage( cvGetSize( src ), src->depth, src->nChannels );
IplImage* dstLUV = cvCreateImage( cvGetSize( src ), src->depth, src->nChannels );
cvCvtColor( src, srcLUV, CV_RGB2Luv);
cvCopy( srcLUV, dstLUV );

int widthstep = srcLUV->widthStep;
int channel = srcLUV->nChannels;

for( int y = 0; y<src->height; y++ )
{
    for( int x = 0; x<src->width; x++ )
    {
        uchar L = (uchar)srcLUV->imageData[y *widthstep + x *channel];
        uchar U = (uchar)srcLUV->imageData[y *widthstep + x *channel + 1];
        uchar V = (uchar)srcLUV->imageData[y *widthstep + x *channel + 2];
        int xx = x;
        int yy = y;

        int nIter = 0;
        int count, sumL, sumu, sumv, sumx, sumy;

        while(nIter < maxIter)
        {
            count = 0;
            sumL = sumu = sumv = 0;
            sumx = sumy = 0;

            for( int m = y - hs; m <= y + hs; m++ )
            {
                for( int n = x - hs; n <= x + hs; n++ )
                {
                    if(m >= 0 && m < src->height && n >= 0 && n < src->width)
                    {
                        uchar l = (uchar)srcLUV->imageData[m *widthstep + n *channel];
                        uchar u = (uchar)srcLUV->imageData[m *widthstep + n *channel + 1];
                        uchar v = (uchar)srcLUV->imageData[m *widthstep + n *channel + 2];

                        double dist = sqrt( (double)((L - l)^2 + (U - u)^2 + (V - v)^2) );
                        if( dist < hr )
                        {
                            count++;
                            sumL += l;
                            sumu += u;
                            sumv += v;
                            sumx += n;
                            sumy += m;
                        }
                    }
                }
            }
            if(count == 0)break;
            L = sumL / count;
            U = sumu / count;
            V = sumv / count;
            xx = sumx / count;
            yy = sumy / count;

            nIter++;
        }
        dstLUV->imageData[y *widthstep + x *channel] = L;
        dstLUV->imageData[y *widthstep + x *channel + 1] = U;
        dstLUV->imageData[y *widthstep + x *channel + 2] = V;
    }
}

cvCvtColor( dstLUV, dst, CV_Luv2RGB );
cvReleaseImage(&srcLUV);
cvReleaseImage(&dstLUV);
}

void CAndroidserverView::OnBnClickedadvanceinline()
{
	// TODO: 在此添加控件通知处理程序代码
	CFont *f1 = new CFont;
	CFont *f2 = new CFont;
	f1->CreateFont(15,0,0,0,FW_THIN,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arail"));
	f2->CreateFont(15,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arail"));
	CButton* pBtn = (CButton*)GetDlgItem(IDC_advanceinline);
	if(!biaoding)
		return;
	m_advanceinline = !m_advanceinline;
	//改变按键字体
	if(m_advanceinline)
	{
		pBtn->SetFont(f2);
	}
	else
	{
		pBtn->SetFont(f1);
	}	

	delete f1;
	delete f2;
}

void CAndroidserverView::ResizeImage(IplImage* img)
{
    // 读取图片的宽和高
    int w = img->width;
    int h = img->height;
    // 找出宽和高中的较大值者
    int max = (w > h)? w: h;
    // 计算将图片缩放到TheImage区域所需的比例因子
    float scale = (float) ( (float) max / 256.0f );
    
    // 缩放后图片的宽和高
    int nw = (int)( w/scale );
    int nh = (int)( h/scale );
    // 为了将缩放后的图片存入 TheImage 的正中部位，需计算图片在 TheImage 左上角的期望坐标值
    int tlx = (nw > nh)? 0: (int)(256-nw)/2;
    int tly = (nw > nh)? (int)(256-nh)/2: 0;
    // 设置 TheImage 的 ROI 区域，用来存入图片 img
    cvSetImageROI( img, cvRect( tlx, tly, nw, nh) );
    // 对图片 img 进行缩放，并存入到 TheImage 中
    cvResize( img, TheImage );
    // 重置 TheImage 的 ROI 准备读入下一幅图片
    cvResetImageROI( img );
}
void CAndroidserverView::OnBnClickedgototarget()
{
	// TODO: 在此添加控件通知处理程序代码
/*	OnBnClickedFind();
	OnBnClickedCanculateDis();
	CPoint *proad;
	CPoint temproad;
//	proad = roadbuffer;
//	proad->x = p_Car.x;
//	proad->y = p_Car.y;
	int pdistance;
	int tempdis;
	pdistance = dis( proad,&p_Target);
	tempdis = pdistance;
	while(pdistance)
	{
		temproad.x = proad->x + 1;
		temproad.y = proad->y;
		pdistance = dis( &temproad,&p_Target);
		if(pdistance<tempdis)
		{
			proad++;
			proad->x = temproad.x;
			proad->y = temproad.y;
			tempdis = pdistance;
			continue;
		}
		temproad.x = proad->x;
		temproad.y = proad->y+ 1;
		pdistance = dis( &temproad,&p_Target);
		if(pdistance<tempdis)
		{
			proad++;
			proad->x = temproad.x;
			proad->y = temproad.y;
			tempdis = pdistance;
			continue;
		}
		temproad.x = proad->x-1;
		temproad.y = proad->y;
		pdistance = dis( &temproad,&p_Target);
		if(pdistance<tempdis)
		{
			proad++;
			proad->x = temproad.x;
			proad->y = temproad.y;
			tempdis = pdistance;
			continue;
		}
	}//写入路径信息
//	road = roadbuffer+1;
	
	DrawMap();

	CFont *f1 = new CFont;
	CFont *f2 = new CFont;
	f1->CreateFont(15,0,0,0,FW_THIN,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arail"));
	f2->CreateFont(15,0,0,0,FW_BOLD,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arail"));
	CButton* pBtn = (CButton*)GetDlgItem(IDC_gototarget);
	if(!biaoding)
		return;
	m_gototarget = !m_gototarget;
	//改变按键字体
	if(m_gototarget)
	{
		pBtn->SetFont(f2);
	}
	else
	{
		pBtn->SetFont(f1);
	}*/
	
}

int CAndroidserverView::dis(CPoint *a,CPoint *b)
{
	int dis;
	dis = abs(a->x - b->x) + abs(a->y - b->y);
	return dis;
}


bool CAndroidserverView::Rundirect()
{
	switch(ZHUANWAN)
	{
	case 0:{   //直行
			 OnBnClickedQianjin();
			 Sleep(500);
			 OnBnClickedTingzhi();
			 road++;
			 break;
			}
	case 1:{   //右转
			 OnBnClickedZuozhuan();
			 Sleep(500);
			 OnBnClickedTingzhi();
			 //direction--;
			 //if(direction<0)
				//direction = 3;	
			 break;
			}
	case 2:{	//左转
			 OnBnClickedYouzhuan();
			 Sleep(500);
			 OnBnClickedTingzhi();
			 //direction++;
			 //if(direction>3)
				// direction = 0;
			 break;
		   }
	}
	DrawMap();
	return TRUE;
}

void CAndroidserverView::OnBnClickedQianjin()
{
	// TODO: 在此添加控件通知处理程序代码
	char buf[3];
	char* ch=m_speed.GetBuffer(0);
	buf[0]='1';
	buf[1]=ch[0];
	buf[2]=ch[1];	
	if(!InputOrOutput)
		sockConn->Send(buf,3);	

	//////////////////////////////////////////////////////////////////
	ZHUANWAN = 0;
	shadingchange(2,0,0);
	/////////////////////////////////////////////////////////////////
	/*switch(direction)
	{
	case 0:{p_Car.y++;
			break;
			}
	case 1:{p_Car.x++;
			break;
		   }
	case 2:{p_Car.y--;
			break;
		   }
	case 3:{p_Car.x--;
		   break;
		   }
	}*/
	DrawMap();
}

void CAndroidserverView::OnBnClickedHoutui()
{
	// TODO: 在此添加控件通知处理程序代码
	char buf[3];
	char* ch=m_speed.GetBuffer(0);
	buf[0]='3';
	buf[1]=ch[0];
	buf[2]=ch[1];
	if(!InputOrOutput)
		sockConn->Send(buf,3);
	////////////////////////////////////////////////////////////////////
	ZHUANWAN = 0;
	shadingchange(-2,0,0);
	////////////////////////////////////////////////////////////////////
	/*switch(direction)
	{
	case 0:{p_Car.y--;
			break;
			}
	case 1:{p_Car.x++;
			break;
		   }
	case 2:{p_Car.y++;
			break;
		   }
	case 3:{p_Car.x--;
		   break;
		   }
	}*/
	DrawMap();
}

void CAndroidserverView::OnBnClickedYouzhuan()
{
	// TODO: 在此添加控件通知处理程序代码
	char buf[3];
	char* ch=m_speed.GetBuffer(0);
	buf[0]='4';
	buf[1]=ch[0];
	buf[2]=ch[1];
	if(!InputOrOutput)		
		sockConn->Send(buf,3);

	/////////////////////////////////////////////
	ZHUANWAN = 1;
	shadingchange(0,0.05,0);

	////////////////////////////////////////
	direction++;
	if(direction == 4)
		direction = 0;
}

void CAndroidserverView::OnBnClickedZuozhuan()
{
	// TODO: 在此添加控件通知处理程序代码
	char buf[3];
	char* ch=m_speed.GetBuffer(0);
	buf[0]='2';
	buf[1]=ch[0];
	buf[2]=ch[1];
	if(!InputOrOutput)
		sockConn->Send(buf,3);

	////////////////////////////////////////////
	/////////////////////////////////////////////
	ZHUANWAN = 2;
	shadingchange(0,0,0.05);
	////////////////////////////////////////////
	direction--;
	if(direction < 0)
		direction = 3;
}

void CAndroidserverView::OnBnClickedTingzhi()
{
	// TODO: 在此添加控件通知处理程序代码
	char buf[3];
	char* ch=m_speed.GetBuffer(0);
	buf[0]='0';
	buf[1]=ch[0];
	buf[2]=ch[1];
	if(!InputOrOutput)
		sockConn->Send(buf,3);
}

void CAndroidserverView::DrawMap()
{	
	//临时指定障碍物,目标地点	
	for(int i=0;i<10;i++)
	{
		p_obstacle[i].x=i;
		p_obstacle[i].y=4;
	}
	
//	for(int i=10;i<19;i++)
//	{
//		p_obstacle[i].x=i;
//		p_obstacle[i].y=12;
//	}

//	p_Target.x = 12;
//	p_Target.y = 18;


	 CWnd *pWin = GetDlgItem(IDC_MAP);
     CDC *pDC = pWin->GetDC();	
	 pDC -> Rectangle( 0,0,400,400 ) ;
	 CPen pen(PS_DOT,1,RGB(0, 0, 255));
	 pDC->SelectObject(&pen);

	 for(int i=0;i<20;i++)
	 {
		pDC->MoveTo(0,20*i);
		pDC->LineTo(400,20*i);
		
		pDC->MoveTo(20*i,0);
		pDC->LineTo(20*i,400);
	 }	 //画背景

	 CPen pen1(PS_SOLID,1,RGB(0, 0, 0));
	 pDC->SelectObject(&pen1);
		for(int i=0;p_road[i].x>=0;i++)
		{
			pDC->MoveTo(p_road[i].x*20+10,p_road[i].y*20+10);
			if(p_road[i+1].x>=0)
				pDC->LineTo(p_road[i+1].x*20+10,p_road[i+1].y*20+10);
		}
	CBrush brush3(RGB(150,150,150));
	pDC->SelectObject(&brush3);
		for(int i=0;p_road[i].x>=0;i++)
		{
			pDC->Rectangle(p_road[i].x*20,p_road[i].y*20,p_road[i].x*20+20,p_road[i].y*20+20);//路径			
		}
	

	 CBrush brush(RGB(0,0,255));
	 pDC->SelectObject(&brush);
	 pDC->Rectangle(p_Car.x*20,p_Car.y*20,p_Car.x*20+20,p_Car.y*20+20);//车坐标	 

	 CBrush brush1(RGB(0,255,0));
	 pDC->SelectObject(&brush1);
	 if(p_Target.x>=0)
	 	pDC->Rectangle(p_Target.x*20,p_Target.y*20,p_Target.x*20+20,p_Target.y*20+20);//目标坐标

	 CBrush brush2(RGB(255,0,0));
	 pDC->SelectObject(&brush2);
	 for(int i=0;p_obstacle[i].x>=0;i++)
	 	pDC->Rectangle(p_obstacle[i].x*20,p_obstacle[i].y*20,p_obstacle[i].x*20+20,p_obstacle[i].y*20+20);//障碍物坐标	 
}
void CAndroidserverView::Gototarget()
{
	int cdirection = 0;
	int steps =0;
	if(p_Car.x == road->x)
	{
		if(p_Car.y < road->y)
			{cdirection = 0;}
		else 
			{cdirection = 2;}
	}
	else if(p_Car.y == road->y)
	{
		if(p_Car.x < road->x)
			cdirection = 1;
		else 
			cdirection = 3;
	}

	int temp;
	temp = direction;
	for(steps=0;temp!=cdirection;steps++)
	{
		temp++;
		if(temp == 4)
			temp = 0;
	}

	if(direction == cdirection)
	{
		ZHUANWAN = 0;
	}
	else if(steps == 3)
		ZHUANWAN = 1;
	else
		ZHUANWAN = 2;	

	if(road->y == p_Target.y && road->x == p_Target.x)//停止gototarget
	{
		OnBnClickedgototarget();
		return;
	}

	Rundirect();
	
}

void CAndroidserverView::shading(void)
{

	CvSize size;
	size.height=TheImage->height;
	size.width=TheImage->width;
	TheImage2 = cvCreateImage(size, IPL_DEPTH_8U, IMAGE_CHANNELS);
	cvZero(TheImage2);
	//cvCopy(TheImage,TheImage2);

	 int l_pixcel,w_pixcel;
	 double chuizhi,shuiping;
	 Shade = new PP[TheImage->width*TheImage->height];
	 for(int x=0;x<TheImage->width;x++)
		 for(int y=100;y<TheImage->height;y++)
		 {
			l_pixcel = TheImage->height - y;
			w_pixcel = abs(TheImage->width/2 - x);
			chuizhi = CanculateDistance(l_pixcel);
			shuiping = CanculateWidth(w_pixcel,l_pixcel);

			Shade[y*TheImage->width+x].y=chuizhi + pTD1->D1;
			if((x - TheImage->width/2) > 0)
				Shade[y*TheImage->width+x].x=shuiping;
			else
				Shade[y*TheImage->width+x].x=-shuiping;			
		 }

		 chuizhi=(int)(CanculateDistance((int)(TheImage->height - track_box.center.y)));
		 shuiping=(int)(CanculateWidth((int)(abs(TheImage->width/2 - track_box.center.x)),(int)(TheImage->height - track_box.center.y)));
		 if((track_box.center.x - TheImage->width/2) > 0)
			 f_Target.x=-shuiping;
		 else
			 f_Target.x=shuiping;
		 f_Target.y=chuizhi + pTD1->D1; //确定目标绝对坐标


	for(int x=0;x<TheImage->width;x++)
		 for(int y=100;y<TheImage->height;y++)
		 {
			 chuizhi = Shade[y*TheImage->width+x].y;
			 shuiping = Shade[y*TheImage->width+x].x;			
			if((int)(chuizhi)%10 == 0)
			{
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+0)=0;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+1)=255;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+2)=0;				
			}
			if((int)(shuiping)%10 == 0)
			{
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+0)=0;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+1)=255;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+2)=0;				
			}
			if(abs(chuizhi-f_Target.y)<5 && abs(shuiping-f_Target.x)<5)
			{
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+0)=0;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+1)=0;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+2)=255;				
			}
		}
	ShowImage(TheImage2,IDC_STC_SHADE);
	 cvNamedWindow( "Target", 1 );
	 cvShowImage(  "Target",TheImage2 );
}

void CAndroidserverView::shadingchange(double award,double right,double left)
{
	double x1,y1;	
	switch(ZHUANWAN)
	{
	case 0:{
			 PP middle;
			 PP vector;
			 double a;
			 int width = TheImage->width;
			 int height = TheImage->height;
			 middle.x = (Shade[(height-1)*width].x + Shade[(height-1)*width + width-1].x)/2;
			 middle.y = (Shade[(height-1)*width].y + Shade[(height-1)*width + width-1].y)/2;
			 a = atan(abs(middle.y/middle.x));
			 vector.x = award*cos(a);			 
			 vector.y = award*sin(a);
			 if(middle.x>=0 && middle.y>=0)
			 {
				vector.x = vector.x;
				vector.y = vector.y;
			 }
			 else if(middle.x>=0 && middle.y < 0)
			 {
				vector.x = vector.x;
				vector.y = -vector.y;
			 }
			 else if(middle.x<0 && middle.y>=0)
			 {
				vector.x = -vector.x;
				vector.y = vector.y;
			 }
			 else if(middle.x<0 && middle.y < 0)
			 {
				vector.x = -vector.x;
				vector.y = -vector.y;
			 }
			 else
			 {
				vector.x = vector.x;
				vector.y = vector.y;
			 }


			 for(int x=0;x<TheImage->width;x++)
				for(int y=100;y<TheImage->height;y++)
				{
					Shade[y*TheImage->width+x].x += vector.x;
					Shade[y*TheImage->width+x].y += vector.y;
				}
		   }
		   break;
	case 1:{
			for(int x=0;x<TheImage->width;x++)
				for(int y=100;y<TheImage->height;y++)
				{
					x1=Shade[y*TheImage->width+x].x;
					y1=Shade[y*TheImage->width+x].y;
					Shade[y*TheImage->width+x].x = x1*cos(-right) - y1*sin(-right);
					Shade[y*TheImage->width+x].y = x1*sin(-right) + y1*cos(-right);
				}
		   }
		   break;
	case 2:{
			for(int x=0;x<TheImage->width;x++)
				for(int y=100;y<TheImage->height;y++)
				{
					x1=Shade[y*TheImage->width+x].x;
					y1=Shade[y*TheImage->width+x].y;
					Shade[y*TheImage->width+x].x = x1*cos(left) - y1*sin(left);
					Shade[y*TheImage->width+x].y = x1*sin(left) + y1*cos(left);
				}	
		   }
		   break;
	}
	CvSize size;
	size.height=TheImage->height;
	size.width=TheImage->width;
	TheImage2 = cvCreateImage(size, IPL_DEPTH_8U, IMAGE_CHANNELS);
	cvZero(TheImage2);
	//cvCopy(TheImage,TheImage2);
	double chuizhi,shuiping;
	int old=2,old1=2;
	for(int x=0;x<TheImage->width;x++)
		 for(int y=100;y<TheImage->height;y++)
		 {
			 chuizhi = Shade[y*TheImage->width+x].y;
			 shuiping = Shade[y*TheImage->width+x].x;			
			if((int)(chuizhi)%10 == 0)
			{
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+0)=0;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+1)=255;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+2)=0;				
			}		
			if((int)(shuiping)%10 == 0)
			{
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+0)=0;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+1)=255;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+2)=0;				
			}
			if(abs(chuizhi-f_Target.y)<5 && abs(shuiping-f_Target.x)<5)
			{
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+0)=0;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+1)=0;
				CV_IMAGE_ELEM(TheImage2, unsigned char, y, x*3+2)=255;				
			}
		}
	 ShowImage(TheImage2,IDC_STC_SHADE);
	 cvNamedWindow( "Target", 1 );
	 cvShowImage(  "Target",TheImage2 );
}

void CAndroidserverView::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	shading();
	ShowImage( TheImage, IDC_STC_DUMMY );            // 调用显示图片函数
}

void CAndroidserverView::OnBnClickedRuntarget()
{
	// TODO: 在此添加控件通知处理程序代码
	road++;
	while(road->x > 0)
	{
		PP mubiaodian;
		mubiaodian.x = (road->x - 10)*10;//一个栅格10cm
		mubiaodian.y = road->y * 10;

		
		double dis;
		do
		{
			//方向调整
			double k1 = (Shade[(TheImage->height/2)*TheImage->width + TheImage->width/2].x - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].x) * (mubiaodian.y - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].y);
			double k2 = (mubiaodian.x - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].x) * (Shade[(TheImage->height/2)*TheImage->width + TheImage->width/2].y - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].y);
			
			while(abs(k1-k2) > 100)
			{
				if( k1>k2 )
				{
					OnBnClickedZuozhuan();
					Sleep(1000);
				}
				else
				{
					OnBnClickedYouzhuan();
					Sleep(1000);
				}
				k1 = (Shade[(TheImage->height/2)*TheImage->width + TheImage->width/2].x - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].x) * (mubiaodian.y - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].y);
				k2 = (mubiaodian.x - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].x) * (Shade[(TheImage->height/2)*TheImage->width + TheImage->width/2].y - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].y);
			}
			//直线行驶
			OnBnClickedQianjin();
			Sleep(500);
			//距离计算
			dis = (mubiaodian.x - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].x)*(mubiaodian.x - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].x)+
					(mubiaodian.y - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].y)*(mubiaodian.y - Shade[(TheImage->height-1)*TheImage->width+TheImage->width/2].y);
		}while(dis>50);
		road++;
	}
}


void CAndroidserverView::OnBnClickedRoadcreat()
{
	// TODO: 在此添加控件通知处理程序代码
	  time_t timer,timerl;  

    time(&timer);  

    unsigned long seed = (unsigned long)timer;  

    seed %= 56000;  

    srand((unsigned int)seed); //生成随机数 

	int start,end;//起点与终点	

	//start = 1;
	start = p_Car.y*20+p_Car.x;

	//end = 4;
	end = p_Target.y*20+p_Target.x;

	//障碍物信息
	for(int i=0;i<400;i++)
	{
		if(p_obstacle[i].x<0)
			break;
		else
			C[i]=p_obstacle[i].y*20+p_obstacle[i].x;
	}
	
	calculateAllDistance();

    //蚁群系统对象  

    AntColonySystem* acs = new AntColonySystem();  

    ACSAnt* ants[M];   

    //蚂蚁分布在起点上  

    for(int k = 0; k < M; k++)  

    {  

        ants[k] = new ACSAnt(acs, start, end);  

    }    

    //随机选择一个节点计算由最近邻方法得到的一个长度  

    int node = rand() % N;  

    Lnn = CalAdjacentDistance(node);  

    //各条路径上初始化的信息素强度  

    double initInfo = 1 / (N * Lnn);  

    acs->InitParameter(initInfo);     

    //全局最优路径  

    int globalTour[N][2];  

    //全局最优长度  

    double globalBestLength = 0.0;   

	//线性化全局最优路径结果

	int resultTour[N];	

    for(int a = 0; a < NcMax; a++)  

    {  

        //局部最优路径  

        int localTour[N][2];  

        //局部最优长度  

        double localBestLength = 0.0;  

        //当前路径长度  

        double tourLength;  

        for(int j = 0; j < M; j++)  

        {  

            int* tourPath = ants[j]->Search();  

            tourLength = calculateSumOfDistance(tourPath,end);                

            //局部比较，并记录路径和长度  

            if(tourLength < localBestLength || abs(localBestLength - 0.0) < 0.000001)  

            {                 

                for(int m = 0; m< N; m++)  

                {  

                    int row = *(tourPath + 2 * m);  

                    int col = *(tourPath + 2* m + 1);  

                    localTour[m][0] = row;  

                    localTour[m][1] = col;  

                }  

                localBestLength = tourLength;             

            }  

        }  

        //全局比较，并记录路径和长度  

        if(localBestLength < globalBestLength || abs(globalBestLength - 0.0) < 0.000001)  

        {                 

            for(int m = 0; m< N; m++)  

            {  

                globalTour[m][0] = localTour[m][0];  

                globalTour[m][1] = localTour[m][1];  

            }  

            globalBestLength = localBestLength;   

        }  

        acs->UpdateGlobalPathRule(*globalTour, (int)globalBestLength,end);  

        //输出所有蚂蚁循环一次后的迭代最优路径  

        /*cout<<"第 "<<i + 1<<" 迭代最优路径:"<<localBestLength<<"."<<endl;  

		for(int m = 0; m< N; m++)  

        {  

            cout<<localTour[m][0]<<".";

			if( localTour[m][1] == end )
		
			{	
				 cout<<localTour[m][1]<<".";
				
				break;
			
			}

        }  

        cout<<endl;  */     

    }     

/*    //输出全局最优路径  

    cout<<"全局最优路径长度:"<<globalBestLength<<endl;    

    cout<<"全局最优路径:";  

    for(int m = 0; m< N; m++)  

    {  

        cout<<globalTour[m][0]<<"."; 

		if( globalTour[m][1] == end )
		
		{	
			 cout<<globalTour[m][1]<<".";
			
			break;
		
		}

    }  

    cout<<endl;*/

	//线性化全局最优路径

	resultTour[0] = globalTour[0][0];

	for(int m = 0,n = 0; m< N; m++)  

    {  

		if( obtacleBetween( resultTour[n],globalTour[m][1] ) )
			
		{			

			if(m>1)
				resultTour[++n] = globalTour[m-1][1]; 

		}

		if( globalTour[m][1] == end )

		{

			resultTour[++n] = globalTour[m][1];
			
			break;

		}

    }	

	//输出线性化结果路径     

    /*cout<<"线性化最优路径:";  

    for(int m = 0; m< N; m++)  

    {  

        cout<<resultTour[m]<<"."; 

		if( resultTour[m] == end )				 
			
			break;

    }  

    cout<<endl;*/
	

	for(int m=0;m<N;m++)
	{
		p_road[m].y = resultTour[m]/20;
		p_road[m].x = resultTour[m]%20;

		if(resultTour[m] == end)
			break;
	}

    time(&timerl);  

    int t = (int)(timerl - timer);  

	road = p_road;

	DrawMap();

	//system("pause");
     
}
