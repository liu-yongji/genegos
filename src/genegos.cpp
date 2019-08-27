#include "sys/sysinfo.h"
#include"sys/time.h"
#include<dirent.h>
#include <sys/mman.h> 
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h> 
#include<memory.h> 
#include<iostream> 
#include<string.h>
#include <fcntl.h>
#include<fstream>
#include <map>
#include <pthread.h>
#include <unistd.h>
#include <error.h>
#define _MAX_INT_DIG 32
#define CHR_NUM 24

using namespace  std;
struct SNPBag // 存储没有对齐的包
{
	int nPos ; //对应位置
	string sInfo; //全部信息
};
struct InitBag
{
	string strFile;
	int *memeStart;
	InitBag()
	{
		strFile = "";
		memeStart = NULL;
	}
};

struct ChainBag
{
	bool bReverse;
	bool bHasSun;
	int  nSrcStart;
	int  nSrcEnd;
	int  nDesStart;
	int  nDesEnd;
	int  nQuality;
	ChainBag()
	{
		bReverse = false;
		bHasSun = false;
		nSrcStart = -1;
		nSrcEnd = -1;
		nDesStart =-1;
		nDesEnd = -1;
		nQuality =0;
	}
};

struct  CmySection
{
	std::map<int,int> map_Chain_1V1;
	std::map<int,ChainBag> map_Chain;
	CmySection()
	{
		map_Chain_1V1.clear();
		map_Chain.clear();
	}
	~CmySection()
	{
		map_Chain_1V1.clear();
		map_Chain.clear();
	}
};

class THREAD_FILEMMAP_ARG
{
public:
	FILE *p_snp;
	FILE *p_indel;
	char *pStart;
	long nlen;
	string sResult; // 记录有效转换
	string sFail;   // 记录未能转换的记录
	CmySection *makeUp;
	
	THREAD_FILEMMAP_ARG()
	{
		nlen = -1;
	  sResult = "";
    sFail  = "";  
    pStart = NULL;
	  makeUp = NULL;
	}
	~THREAD_FILEMMAP_ARG()
	{
		sResult = "";
    sFail  = ""; 
    pStart = NULL;
    makeUp = NULL;
	}
};



typedef std::map <int,int>::iterator Map_IntInt_iterator;
typedef std::map <int,ChainBag>::iterator Map_Chain_iterator;
typedef std::map <string,SNPBag>::iterator Map_Rs38_iterator;
typedef  pair<map<int,ChainBag>::iterator,bool> MapRet;
typedef  pair<map<string,SNPBag>::iterator,bool> Map_RS_Ret;



/*****************************************************************************
** Function:     to_string(int _Val)
**  扩展string <int> // 原因是<string> 库里面没有
** Create Date:  2018.7.17
** Modify Time:  2018.7.17
** Author:        LYJ
** Version:      1.0
*******************************************************************************/

string to_string(int _Val)
{	
	char _Buf[2 * _MAX_INT_DIG];
        snprintf(_Buf, sizeof(_Buf), "%d", _Val);
	return (string(_Buf));
}

int stoi(string str)
{
	int num = atoi( str.c_str() );
	return num;
}



/*****************************************************************************
** Function:    is_xxx_exist(string sFilename)
** 判断文件或者路径是否存在
** Create Date:  2018.7.17
** Modify Time:  2018.7.17
** Author:        LYJ
** Version:      1.0
*******************************************************************************/

 bool is_file_exist(string sFilename)
 {
 	if( 0 != access(sFilename.c_str(),F_OK))
 		return false;
  return true;
 }
 
 
bool is_dir_exist(string sDir)
 {
 	
 	  DIR *dirptr=opendir(sDir.c_str());
 	  
 	  if( NULL == dirptr)
 	  	return false;
 	  
 	  closedir(dirptr);
 	  return true;
 }
 
/*****************************************************************************
** Function:    GetPosSectionMakeUp(int nSpos,CmySection makeUp)
**   // GRCh37 版本 pos 转 38 版本位置  CmySection 是自己组装的数据结构
** Create Date:  2018.7.17
** Modify Time:  2018.7.17
** Author:        LYJ
** Version:      1.0
*******************************************************************************/
static inline int GetPosSectionMakeUp(int nSpos,CmySection *pmakeUp)
{
	int nRe = -1;
	Map_IntInt_iterator itInt = pmakeUp->map_Chain_1V1.find(nSpos);
	if (itInt != pmakeUp->map_Chain_1V1.end())
	{
		nRe = itInt->second;
		return nRe;
	}
	
	Map_Chain_iterator it = pmakeUp->map_Chain.begin();
	for (;it != pmakeUp->map_Chain.end();it++)
	{
		if (nSpos >=it->first && nSpos<= it->second.nSrcEnd)
		{
			bool bre= it->second.bReverse;
			int nDif = nSpos - it->first;
			if (!bre)//"+"
				nRe = it->second.nDesStart + nDif;
			else
				nRe = it->second.nDesStart - nDif; 
			break;
		}
		else if (nSpos <it->first && nSpos< it->second.nSrcEnd)  //超出范围 说明没有找到区间 也就是说 两个版本位置错开了
		{
			break;
		}

	}
	return nRe;

}


/*****************************************************************************
** Function:   GetMakeupChain(string chianFile,std::map<int,ChainBag> &map_Chain)
** chianFile ：参考对照关系文件
** Create Date:  2018.8.20
** Modify Time:  
** Author:        LYJ
** Version:      1.0
*******************************************************************************/

int GetMakeupChain(string chianFile,std::map<int,int> &map_Chain_1v1,std::map<int,ChainBag> &map_Chain)
{
	ifstream ifs;
	ifs.open(chianFile.c_str(),ios_base::in);
	if (!ifs)
	{
		//cout<<"Read File Error, please check file name is right: "<<chianFile<<endl;
		return -1;
	}

	int np1 = 0;
	int np2 = 0;
	ChainBag c_bag;
	int nCount  = 0 ;
	string str_Line = "";

	while(!ifs.eof())  
	{  
		np1 = 0;
		np2 = 0;
		getline(ifs,str_Line);
		np2 = str_Line.find(" ",np1);
		if (np2 == -1)
			continue;
		nCount ++;
		if (str_Line.find("<1v1>") != -1) // 一对一的关系
		{
			np1 = str_Line.find("<1v1>");
			string srcStart = str_Line.substr(0,np1);
			int nStart = stoi(srcStart);

			np1 = str_Line.find(" ",np1+1);
			string srcEnd =str_Line.substr(np1);
			int nSEND = stoi(srcEnd);
			map_Chain_1v1.insert(make_pair(nStart,nSEND));
			continue;
		}
		else
		{
		string srcStart = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;
		int nStart = stoi(srcStart);


		/////////////////////chain score//////////////////////////
		np2 = str_Line.find(" ",np1);
		string srcEnd = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;
		int nSEND = stoi(srcEnd);
		//c_bag.nSrcEnd = nSEND;

		/////////////////////chrName//////////////////////////
		np2 = str_Line.find(" ",np1);
		string sstrand = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;

		/////////////////////chrsize//////////////////////////
		np2 = str_Line.find(" ",np1);
		string sDesStart = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;
		int nES = stoi(sDesStart);
		//c_bag.nDesStart = nES;

		//////////////////////strand/////////////////////////
		np2 = str_Line.find(" ",np1);
		string sDesEnd = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;
		int nED = stoi(sDesEnd);
		//c_bag.nDesEnd = nED;
		if ((nStart == c_bag.nSrcEnd +1)&&((!c_bag.bReverse && nES == c_bag.nDesEnd +1)||(c_bag.bReverse && nES == c_bag.nDesEnd -1) ))// 说明是连起来的
		{
			c_bag.nSrcEnd = nSEND;
			c_bag.nDesEnd = nED;
		}
		else
		{
			if (c_bag.nSrcStart != -1)
			{
				MapRet ret = map_Chain.insert(std::make_pair(c_bag.nSrcStart,c_bag));
				if (!ret.second)
				{
					cout<<"insert error"<<endl;
					//system("pause");
				}
			}
			
			c_bag.nSrcStart = nStart;
			c_bag.nSrcEnd = nSEND;
			if (sstrand == "-")
				c_bag.bReverse = true;
			else
				c_bag.bReverse = false;

			c_bag.nDesStart = nES;
			c_bag.nDesEnd = nED;
		}

		}// 区间关系
		
	}  //end while  挑出所有的对应关系 并做了排序

	MapRet ret = map_Chain.insert(std::make_pair(c_bag.nSrcStart,c_bag)); // 最后一条
	if (!ret.second)
	{
		cout<<"insert error"<<endl;
		//system("pause");
	}

	ifs.close();
	return nCount;
}

/*****************************************************************************
** Function:      bWritResult
** 将CHR VCF文件 提取到的信息重新写入
** Create Date:  2018.7.04
** Modify Time:  2018.7.04
** Author:        LYJ
** Version:      1.0
*******************************************************************************/


bool bWritResult(FILE * &pFile,string *pstrInfo)
{
	int nLen = pstrInfo->length();
	int nWriten= 0;
	nWriten = fwrite (pstrInfo->c_str(), 1, nLen, pFile);

	if (nLen != nWriten)
	{
		printf("write file length error src_len: % d  written_len: %d \n",nLen ,nWriten);
		return false;
	}
	return true;
}

/*****************************************************************************
** Function:      Repalce_char(string& str,const string&old_value,const string& new_value)
** str:  替换字符串中所有的指定字符  返回替换的个数
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:       1.0
*******************************************************************************/
int Repalce_char(string &str,const char &old_value,const string& new_value)
{
	int nRe = 0;
	for(int pos=0;pos!=-1;pos+=new_value.length())
	{
		if((pos=str.find(old_value,pos))!=-1)
		{
			//str.replace(pos,old_value.length(),new_value);
			str.replace(pos,1,new_value);
			nRe++;
		}
		else 
			break;
	}
	return nRe;
}

int Repalce_char(string &str,const string &old_value,const string& new_value)
{
	int nRe = 0;
	for(int pos=0;pos!=-1;pos+=new_value.length())
	{
		if((pos=str.find(old_value,pos))!=-1)
		{
			str.replace(pos,old_value.length(),new_value);
			nRe++;
		}
		else 
			break;
	}
	return nRe;
}

bool  getCurrentPath(string &sPath )
{
	char current_path[1024]; 
  int cnt = readlink("/proc/self/exe",current_path, 1024); 
  if (cnt < 0 || cnt >= 1024) 
  { 
    printf("***Error***\n"); 
     return false;
  } 
  int i; 
  for (i = cnt; i >=0; --i) 
  { 
    if (current_path[i] == '/') 
    { 
        current_path[i] = '\0'; 
        break; 
    }
  } 
  sPath = current_path ;
  return true;
}


long GetFileSize(string sfname)
{
	 int fd;  
	 struct stat sb;  
	
	 if ((fd = open(sfname.c_str(), O_RDONLY)) < 0)
	 	return -1;
	  
	 if ((fstat(fd, &sb)) == -1)
	  	return -2;
	  	
	 long  nFileSize  = sb.st_size;
	 close(fd);
	  
	return nFileSize;
}

long GetFileSize(int fd)
{ 
	 struct stat sb;  
	 if ((fstat(fd, &sb)) == -1)
	  	return -1;
	return sb.st_size;
}

 static int GetChr(string strch)
  {
  	Repalce_char(strch,"chr","");
  	if(strch == "" || strch.find_first_not_of("0123456789xyXY")!= -1)
  		return -1;
  		
		int nNum = 0;
		if (strch == "X"||strch == "x")
			return 23;
		else if (strch == "Y"||strch == "y")
			return 24;
		else
			nNum = stoi(strch);
	  
	  return nNum;
  }
  static string GetChr(int nChr)
  {
  	string strch = "chr";
  	
  	if(nChr == 23)
  		strch = "chrX";
  	else if(nChr == 24)
  			strch = "chrY";
  	else
  		  strch += to_string(nChr);
	  return strch;
  }
  

  static int GetPos(string strPos)
  {
  	if(strPos == "" || strPos.find_first_not_of("0123456789")!= -1)
  		return -1;
		return stoi(strPos);
  }
  
  void * TCnvtBed(void* pT)
	{
		  THREAD_FILEMMAP_ARG *pBag = (THREAD_FILEMMAP_ARG *)pT;
			char *pChar = pBag->pStart;
	    long nlen = pBag->nlen;
	    int nChr = -1;
	    int nPos = -1;
	    int nTCout = 0;
	    int  nslen =0;
	    int  nLine_len =0;
	    int  nreflen =0;  // 标记chr + pos 长度；
	    string sPass = "";
	    string sChr = "";
	    string sPos = "";
	    string sPosEnd = ""; // posend
	    string sLine = "";
	    bool  bConvert = false;
	    
	    pBag->sResult.reserve(nlen*1.1); // 设置好文件内存使用大小
	    
	    char *pS = pChar;
	    char *pStart_Line = pChar;
	    char *pStart_REF = pChar;
	    
	    for(;nlen >=0 ;nlen--)
	    {
	    	switch(*pChar++)
	    	{
	    		case'\n':
	    			{
	    				if(bConvert)
	    					{
	    						sLine.assign(pStart_REF, nLine_len-nreflen+1);
	    						sLine = sChr + "\t" + sPos + "\t"  + sPosEnd + "\t" + sLine;
	    						pBag->sResult += sLine;
	    					}
	    				 else
	    				 	{
	    				 		sLine.assign(pStart_Line, nLine_len+1);
	    		        if(sLine.find("#")== 0)
	    		        	pBag->sResult += sLine;
	    		        else
	    		        	pBag->sFail += sLine;
	    				 	}
	    				
	    				nTCout = 0;
	    				nslen =0;
	    				nreflen =0;
	    				nLine_len =0;
	    				sPass = "";
	            sChr = "";
	            sPos = "";
	            pS = pChar;
	            pStart_Line = pChar;
	            bConvert = false;
	    				break;
	    			}
	    		case '\t':
	    			{
	    				nTCout ++;
	    				nLine_len++;
	    				switch(nTCout)
	    				{
	    					case 1:
	    						sChr.assign(pS,nslen);
	    					  break;
	    					case 2:
	    						{
	    							nChr = GetChr(sChr);
	    							if(nChr != -1)
	    								{
	    									sPos.assign(pS,nslen);
	    									nPos = GetPos(sPos);
	    									if(nPos != 1)
	    										{
			                     int nPos38 = GetPosSectionMakeUp(nPos,&pBag->makeUp[nChr-1]);
			                      if(nPos38 != -1)
			                      	{
			                      		sChr = GetChr(nChr);
			                      		sPos = to_string(nPos38);
			                          bConvert = true;
			                      		//pStart_REF = pChar;
			                      		//nreflen = nLine_len;
			                      	}
	    										}
	    								}
	    						  break;
	    						}
	    					case 3:
	    						{
	    							if(nChr != -1)
	    								{
	    									sPosEnd.assign(pS,nslen);
	    									nPos = GetPos(sPosEnd);
	    									if(nPos != 1)
	    										{
			                      int nPos38 = GetPosSectionMakeUp(nPos,&pBag->makeUp[nChr-1]);
			                      if(nPos38 != -1)
			                      	{
			                      		sPosEnd = to_string(nPos38);
			                      		pStart_REF = pChar;
			                      		//bConvert = true;
			                      		nreflen = nLine_len;
			                      	}
			                      else
			                      	{
			                      		sPosEnd = "null\t";
			                      		pStart_REF = pChar;
			                      		nreflen = nLine_len;
			                      	}
			                      	 
			                      	
	    										}
	    								}
	    						  break;
	    						}
	    					default:
	    						break;
	    				}
	    				pS = pChar;
	    				nslen =0;
	    				break;
	    			}
	    	  default:
	    	  	{
	    	  		nslen ++;
	    	  		nLine_len++;
	    	  		break;
	    	  	}
	    	  	
	    	} ///////////end switch 
	    }////////////end for
	   
	   string strRe = " finish \n";
	   char *cSt =(char *) strRe.data();
	  return (void *)cSt;
  }
  
  
   void * TCnvtVCF(void* pT)
	{
		  THREAD_FILEMMAP_ARG *pBag = (THREAD_FILEMMAP_ARG *)pT;
			char *pChar = pBag->pStart;
	    long nlen = pBag->nlen;
	    int nChr = -1;
	    int nPos = -1;
	    int nTCout = 0;
	    int  nslen =0;
	    int  nLine_len =0;
	    int  nreflen =0;  // 标记chr + pos 长度；
	    string sPass = "";
	    string sChr = "";
	    string sPos = "";
	    string sPosEnd = ""; // posend
	    string sLine = "";
	    bool  bConvert = false;
	    pBag->sResult.reserve(nlen); // 设置好文件内存使用大小
	    
	    char *pS = pChar;
	    char *pStart_Line = pChar;
	    char *pStart_REF = pChar;
	    
	    for(;nlen >=0 ;nlen--)
	    {
	    	switch(*pChar++)
	    	{
	    		case'\n':
	    			{
	    				if(bConvert)
	    					{
	    						sLine.assign(pStart_REF, nLine_len-nreflen+1);
	    						sLine = sChr + "\t" + sPos + "\t"  + sPosEnd + "\t" + sLine;
	    						pBag->sResult += sLine;
	    					}
	    				 else
	    				 	{
	    				 		sLine.assign(pStart_Line, nLine_len+1);
	    		        if(sLine.find("#")== 0)
	    		        	pBag->sResult += sLine;
	    		        else
	    		        	pBag->sFail += sLine;
	    				 	}
	    				
	    				nTCout = 0;
	    				nslen =0;
	    				nreflen =0;
	    				nLine_len =0;
	    				sPass = "";
	            sChr = "";
	            sPos = "";
	            pS = pChar;
	            pStart_Line = pChar;
	            bConvert = false;
	    				break;
	    			}
	    		case '\t':
	    			{
	    				nTCout ++;
	    				nLine_len++;
	    				switch(nTCout)
	    				{
	    					case 1:
	    						sChr.assign(pS,nslen);
	    					  break;
	    					case 2:
	    						{
	    							nChr = GetChr(sChr);
	    							if(nChr != -1)
	    								{
	    									sPos.assign(pS,nslen);
	    									nPos = GetPos(sPos);
	    									if(nPos != 1)
	    										{
			                     int nPos38 = GetPosSectionMakeUp(nPos,& pBag->makeUp[nChr-1]);
			                      if(nPos38 != -1)
			                      	{
			                      		sChr = GetChr(nChr);
			                      		sPos = to_string(nPos38);
			                      		bConvert = true;
			                      		pStart_REF = pChar;
			                      		nreflen = nLine_len;
			                      	}
	    										}
	    								}
	    						  break;
	    						}
	    					default:
	    						break;
	    				}
	    				pS = pChar;
	    				nslen =0;
	    				break;
	    			}
	    	  default:
	    	  	{
	    	  		nslen ++;
	    	  		nLine_len++;
	    	  		break;
	    	  	}
	    	  	
	    	} ///////////end switch 
	    }////////////end for
	   
	  string strRe = " finish \n";
	  char *cSt =(char *) strRe.data();
	  return (void *)cSt;
  }
  

bool Cnvt_37to38_File(string sSrcfile,string sSectionDir,string sDespath,string &sError)
{
	bool bRe = true;
	int fd;  
	char *mapped;
	bool bVcf = true;
	long nFilesize =0;
	FILE *pFile;
	FILE *pFSpecial;
	string strSection = "";
	sError = "";
	CmySection mysection[24] ;
	
	int ncpuNum = get_nprocs();
	ncpuNum = (ncpuNum>1)?(ncpuNum-1):1;
		
	pthread_t Init_thread[ncpuNum];
	THREAD_FILEMMAP_ARG  Thread_Check[ncpuNum];
	 
	 if (sSrcfile.find(".vcf") == -1 && sSrcfile.find(".VCF") == -1)
	 	 bVcf = false;
	     
	 if ((fd = open(sSrcfile.c_str(), O_RDONLY)) < 0)
	  {  
	      perror("open"); 
	      return false;  
	  } 
	  
	  nFilesize = GetFileSize(fd);
	  if ((mapped = (char*)mmap(NULL, nFilesize, PROT_READ, MAP_SHARED, fd, 0)) == (void *)-1) 
	   {  
	   	    close(fd);
	        perror("mmap"); 
	        return false; 
	   }
	  close(fd);
	///////////////////////////////////////////intital convert section/////////////////////////////////////
	
	for (int i= 0;i<24 ;i++)// 1到22号
	{
		strSection = "chr"+ to_string(i+1) + "_convert.dat";
		if (i == 22)
			strSection =  "chrX_convert.dat";
		else if (i == 23)
			strSection =  "chrY_convert.dat";
		strSection = sSectionDir + "/" + strSection;
		Repalce_char(sSectionDir,"//","/");
		if (GetMakeupChain(strSection,mysection[i].map_Chain_1V1,mysection[i].map_Chain) <= 0)
		{
			sError = "Intial Genegos-chains error";
			//cout <<  "intial convert error \n";
			return false;
		}
	}
	//////////////////////////////////////////////MAKE DES FILES/////////////////////////////////////////////////////////

	string sFilename = sSrcfile.substr(sSrcfile.rfind('/')+1,sSrcfile.length() - sSrcfile.rfind('/') -1 );
	string mapFile,postfix;
	mapFile = sFilename.substr(0,sFilename.rfind('.'));
	postfix = sFilename.substr(sFilename.rfind('.'));
	mapFile += "_genegos" + postfix;
	mapFile = sDespath + "/" +  mapFile;
	Repalce_char(mapFile,"//","/");

	
	if ((pFile = fopen(mapFile.c_str(),"w+"))==NULL)
	{
		sError = "Creat file error\n";
		return false;
	}
	
	string unmapFile = sFilename.substr(0,sFilename.rfind('.'));
	unmapFile += "_genegos.unmap";
	unmapFile = sDespath + "/" +  unmapFile;
	Repalce_char(unmapFile,"//","/");
	
	if ((pFSpecial = fopen(unmapFile.c_str(),"w+")) == NULL)
	{
		sError = "Creat file error\n";
		if (fclose (pFile) != 0)
			perror("Error occurs when close file"); //报告相应错误
		
		return false;
	}
	//////////////////////////////////////////////////初始化临界区///////////////////////////////////////////////////////////////
	 long nPerThread = nFilesize/ ncpuNum; // 每个线程分一块内存
	 long nLeft = nFilesize;
	 char *pStart = mapped;
	 long  nlen = 0;
	 
	 for(int i= 0;i< ncpuNum;i++)
	 {
			Thread_Check[i].pStart = pStart;
			Thread_Check[i].makeUp = mysection ;
			if(nLeft <= nPerThread)
			{
				Thread_Check[i].nlen = nLeft;
				nLeft = 0;
				break; 
			} 
			nLeft -= nPerThread;
		  pStart += nPerThread;
		  nlen = nPerThread;
			for(;(*pStart) != '\n';pStart++)
			{
				nLeft --;
				nlen ++;
			}
			 pStart++;
		 	 nLeft --;
		 	 nlen++;
			 Thread_Check[i].nlen = nlen;
	 }
	 
	 for (int i=0;i< ncpuNum;i++)
	 {
			int nerror = 0;
			if(bVcf == false)
				nerror = pthread_create(&Init_thread[i],NULL,TCnvtBed,(void *)&Thread_Check[i]);
			else
				nerror = pthread_create(&Init_thread[i],NULL,TCnvtVCF,(void *)&Thread_Check[i]);
			if (0!=nerror)
			{
				sError = "Creat thread error \n";
	      if (fclose (pFile) != 0)
	      	perror("Error occurs when close file"); 
			  return false;
			}
	 } 
		 

	for (int i = 0; i <  ncpuNum ;i++)
	{
				char *ps = NULL;
				int nerror = pthread_join(Init_thread[i],(void **)&ps);
				if (0!=nerror)
				{
					sError = "Wait thread error \n";
					printf("pthread_join  %s \n", strerror(nerror));
					return false;
				}
				else 
				{
					bWritResult(pFile,&Thread_Check[i].sResult);
					bWritResult(pFSpecial,&Thread_Check[i].sFail);
				}
	}
		
	if ((munmap((void *)mapped, nFilesize)) == -1)
		perror("munmap");  

	if (fclose (pFile) != 0)
		perror("Error occurs when close file"); 
	if (fclose (pFSpecial) != 0)
		perror("Error occurs when close file"); 

	return bRe;
}






int main(int arg,char *args[])
{
	 if(arg <1)
	 	{
	 		cout << cout << "Source file not exist \n";
	 		return 0;
	 	}

	//string sSectionDir ="./public/liuyj/grch37_to_grch38_chain/newest_range_add_indel";
	//string sSectionDir ="./Genegos-chains";
	string sSectionDir ="";
	string strSrc = "";
	string strDespath = "";
	string serr = "";
	///////////////////////////获取输入参数////////////////////////////////////////////////////
	for(int i=0; i<arg; i++)
	{
   	string sGet = args[i];
		if(i == 1)
		{  
			if(true ==  is_file_exist(sGet))
			 {
			 	if(sGet.find(".VCF")== -1 && sGet.find(".vcf")== -1 && sGet.find(".bed")== -1&& sGet.find(".BED")== -1)
			 		{
			 			cout << "Source file format error < expectd format is : \".vcf\" or \".bed\"> "<< endl;
			 		  return -1;
			 		}
				strSrc = sGet;
			 }
			 else
			 	{
			 		cout << "Source file not exist " << sGet << endl;
			 		return -1;
			 	}
				
		}
		if(i == 2)
		{
			if(true == is_dir_exist(sGet))
			    strDespath = sGet;
			else
			{
			 	cout << "DesPath not exist " << sGet << endl;
			 	return -1;
			}
		}
		if(i == 3)
		{
			if(true == is_dir_exist(sGet))
			    sSectionDir = sGet;
			else
			{
			 	cout << "Genegos-chains dir not exist " << sGet << endl;
			 	return -1;
			}
		}
	}
	
	if(strDespath == "")
		{
				if(!getCurrentPath(strDespath))
				{
						cout << "DesPath is required " << endl;
				 	 return -1;
				}	 	
		}
  
		
	time_t rawtime; 
  struct tm * timeinfo; 
  time ( &rawtime ); 
  timeinfo = localtime ( &rawtime ); 
  printf ( "Start time is: %d:%d:%d  \n", timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec );
  
  cout << "Source file : " << strSrc << endl;
	cout << "Despath :" << strDespath << endl;
  
   if( false == Cnvt_37to38_File(strSrc,sSectionDir,strDespath,serr))
   	 cout << serr << endl;
   else
   		cout << "genegos convert done\n";
  time ( &rawtime ); 
  timeinfo = localtime ( &rawtime ); 
  printf ( "Finished time is: %d:%d:%d    \n", timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec );
	return 0;
}
