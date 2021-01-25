#include "sys/sysinfo.h"
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h> 
#include<memory.h> 
#include<iostream> 
#include<string>
#include<fstream>
#include <map>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#define _MAX_INT_DIG 32

#define THREADNUM 40
#define BIN_FILE_NUM 24

using namespace  std;

typedef  unsigned	int  Uint;

struct THREADARG 
{
	int nNum ;
	map<Uint,Uint> *Map_PosBag;
	string sFileName;
	THREADARG()
	{
		nNum = 0;
		sFileName = "";
		Map_PosBag =NULL;
	}
};


string sHelp = "Usage: ./checkbedError input_VCF_file/input_VCF_folder  output_folder  reference_rs_folder \n";
string sExample = "Example: ./checkbedError ./chr1.vcf ./ ./all_GRCh38_vcfs/ \n";

/*****************************************************************************
** Function:     Int2string(int _Val)
**  扩展string <int> // 原因是<string> 库里面没有
** Create Date:  2018.7.17
** Modify Time:  2018.7.17
** Author:        LYJ
** Version:      1.0
*******************************************************************************/

string Int2string(int _Val)
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


Uint stoUint(string str)
{
	unsigned int result(0);  //最大可表示值为4294967296（=2‘32-1）
	if(str.length() >10)
		return 0;
	for (int i = str.size()-1;i >= 0;i--)
	{
		unsigned int temp(0),k = str.size() - i - 1;
		if (isdigit(str[i]))
		{
			temp = str[i] - '0';
			while (k--)
				temp *= 10;
			result += temp;
		}
		else
			break;
	}
	return result;
}


string Uint2string(Uint _Val)
{	
	char _Buf[2 * _MAX_INT_DIG];
  snprintf(_Buf, sizeof(_Buf), "%u", _Val);
	return (string(_Buf));
}

/*****************************************************************************
** Function:      Replace_str(string& str,const string&old_value,const string& new_value)
** str:  替换字符串中所有的指定字符  返回替换的个数
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:      1.0
*******************************************************************************/

int Replace_str(string &str,const string &old_value,const string& new_value)
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

/*****************************************************************************
** Function:  getAllFiles(string path, vector<string>& files) 
** 获取文件夹下所有的文件名存入vector
** Create Date:  2018.11.6
** Modify Time:  
** Author:        LYJ
** Version:      1.0
*******************************************************************************/
int  getAllFiles(string path, vector<string>& files) 
{
	// 文件句柄
	int nRe = 0;
	DIR *dirp;
  struct dirent *dp;
  dirp = opendir(path.c_str());
  while ((dp = readdir(dirp)) != NULL) 
    {
    	  if(strcmp(dp->d_name,".")==0 || strcmp(dp->d_name,"..")==0)    ///current dir OR parrent dir
    	  	  continue;
        
        string strName(dp->d_name);
        strName = path + "/" + strName;
        files.push_back(strName);
        nRe++ ;
    }
    closedir(dirp);
	return nRe;
}

int  getAllFiles(string path, vector<string>& files,unsigned long& ulSize) 
{
	// 文件句柄
	  int nRe = 0;
    DIR *dirp;
    struct dirent *dp;
    dirp = opendir(path.c_str());
    struct stat statbuf;
    while ((dp = readdir(dirp)) != NULL) 
    {
    	 if(strcmp(dp->d_name,".")== 0 || strcmp(dp->d_name,"..") == 0 )    ///current dir OR parrent dir
    	  	  continue;
       string strName(dp->d_name);
       
       strName = path + "/" + strName;
       stat(strName.c_str(), &statbuf);
       ulSize += (statbuf.st_size);
       files.push_back(strName);
       nRe++ ;
    }
    closedir(dirp);
	  return nRe;
}



/*****************************************************************************
** Function:      bWriteLog
** 写追加方式写long文件   
** Create Date:  2018.7.04
** Modify Time:  2018.7.04
** Author:        LYJ
** Version:      1.0
*******************************************************************************/
bool bWriteLog(string & sFilemame,string &strInfo)
{
	time_t tNow;
	tNow = time(&tNow);

	char tmpBuf[255];   
	strftime(tmpBuf, 255, "====================================== %Y%m%d_%H:%M:%S  ", localtime(&tNow));
	string sTime(tmpBuf);
	strInfo  = sTime + strInfo + "\n";


	FILE *pFile;
	if ((pFile = fopen(sFilemame.c_str(),"a+"))==NULL)
	{
		printf("无法打开Log文件!\n");
		return false;
	}

	int nLen = strInfo.length();
	int nWriten = fwrite (strInfo.c_str(), 1, strInfo.length(), pFile);
	if (nLen != nWriten)
	{
		printf("写入Log文件长度错误! 字符串长度实际长度 % d     实际写入长度 ： %d \n",nLen ,nWriten);
		if (fclose (pFile) != 0)
			perror("Error occurs when write file"); //报告相应错误

		return false;
	}
	if (fclose (pFile) != 0)
	{
		perror("Error occurs when close file"); //报告相应错误
		return false;
	}
	return true;

}


string sGetfileDir(string & sFilemame)
{
	string strDir = "";
	int nPos = sFilemame.rfind('/');
	strDir = sFilemame.substr(0,nPos);
	if (strDir == "" || strDir.find('/') == -1)
		strDir = "";
	return strDir;
}

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
** Function:      bWritResult
** 将CHR VCF文件 提取到的信息重新写入
** Create Date:  2018.7.04
** Modify Time:  2018.7.04
** Author:        LYJ
** Version:      1.0
*******************************************************************************/
bool bWritResult(FILE * &pFile,string &strInfo)
{
	int nLen = strInfo.length();
	int nWriten= 0;
	/*if (g_Mutiple)
	{
	EnterCriticalSection(&g_Write_Cs);
	nWriten = fwrite (strInfo.c_str(), 1, strInfo.length(), pFile);
	LeaveCriticalSection(&g_Write_Cs);
	}
	else*/
		nWriten = fwrite (strInfo.c_str(), 1, strInfo.length(), pFile);

		if (nLen != nWriten)
		{
		   printf("写入Log文件长度错误! 字符串长度实际长度 % d     实际写入长度 ： %d \n",nLen ,nWriten);
		   return false;
		}
	return true;

}


/*****************************************************************************
** Function:      nCountChar(string &str,const char &c)
** str:  查找指定字符的个数
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:      1.0
*******************************************************************************/
int nCountChar(string &str,const char &c)
{
	int nRe = 0;
	for(int pos=0;pos!=-1;pos++)
	{
		if((pos=str.find(c,pos))!=-1)
			nRe++;
		else 
			break;
	}
	return nRe;
}

/*****************************************************************************
** Function:   nCountstr(string &str,const string &sCount)
** str:  查找指定字串的个数
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:      1.0
*******************************************************************************/
int nCountstr(string &str,string &c)
{
	int nRe = 0;
	for(int pos=0;pos!=-1;pos++)
	{
		if((pos=str.find(c,pos))!=-1)
			nRe++;
		else 
			break;
	}
	return nRe;
}


int nCountstr(string &str,const string sCount)
{
	int nRe = 0;
	for(int pos=0;pos!=-1;pos += sCount.length())
	{
		if((pos=str.find(sCount,pos))!=-1)
			nRe++;
		else 
			break;
	}
	return nRe;
}


/*****************************************************************************
** Function:      Repalce_char(string& str,const string&old_value,const string& new_value)
** str:  替换字符串中所有的指定字符  返回替换的个数
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:      1.0
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

bool RepalecPos(int nPos,string &str)
{
	int npos1 = 0;// 自串头位置
	int npos2 = 0;// 自串尾巴位置
	npos2 = str.find("	",0);
	if (npos2!= -1)
	{
		npos1= npos2+1;	
		string strHead = str.substr(0,npos1);// chr
		strHead += Int2string(nPos);
		npos2 = str.find("	",npos1);
		str = str.substr(npos2,str.length()-npos2);
		str = strHead + str;
		return true;
	}
	return false;
}

//ALL.chr11.phase3_shapeit2_mvncall_integrated_v5a.20130502.genotypes.vcf
//ALL.chr10_GRCh38.genotypes.20170504.vcf

 static int n_PickInfo382(string &str,Uint &nPos,Uint &urs,int &nChr)
{
	int npos1 = 0;// 自串头位置
	int npos2 = 0;// 自串尾巴位置
	int nReturn = 0;// 返回值 0:snp  1:indel -1:error
	
	npos2 = str.find("\t",0);
	if (npos2!= -1)
	{
		/********************************CHORM**********************************/
		  string sChr= str.substr(0,npos2);
		  Repalce_char(sChr,"chr","");
		  nChr = stoi(sChr);
			npos1 = npos2+1;
		/********************************POS*************************************/
			npos2 = str.find("\t",npos1);
		  string sPos= str.substr(npos1,npos2-npos1);
		  nPos = stoUint(sPos);
			npos1= npos2+1;
		/********************************ID*************************************/
			npos2 = str.find("\t",npos1);
	  	string sID= str.substr(npos1,npos2-npos1);
	  	if(sID.find_first_not_of("rs0123456789") != -1 )
	  		return -1;
	  		
	  	if(Replace_str(sID,"rs","")!= 1)
	  		return -1;
	  	
	  	urs = stoUint(sID);
      /* if (str.find("VT=SNP",npos1) == -1) { return -1;}*/
		}
		else 
			nReturn = -1;
			
	return nReturn;
}


void *  thread_fun_GRCh38BED(void* pT)  // 线程函数
{
	THREADARG  *pbg = (THREADARG  *)pT;
	//printf("thread created success ID %lu \n ",pthread_self());
	//////////////////////////////////////////////////
	string sSrcfile = pbg->sFileName;
	string str_Line;
	string strRe;
	char *cSt =(char *) strRe.data();
	
	ifstream ifs;
	ifs.open(sSrcfile.c_str());
	if (!ifs)
	{
		cout<<"Read File Error, please check file name is right: \n"<<sSrcfile <<endl;
		return (void *)cSt;
	}

	int nLinenum =0;
	Uint nPos =0;
	Uint nRs =0;
	int nChr = -1;
	while(!ifs.eof())  
	{ 
		str_Line = "";
		getline(ifs,str_Line);
		if(str_Line.length()<2)
			continue;
		if(str_Line[0] == '#')
			continue;
			
		if( 0 != n_PickInfo382(str_Line,nPos,nRs,nChr))
			continue;
		
		nLinenum++;

		if(nRs != 0 && nPos!=0)
			pbg->Map_PosBag->insert(make_pair(nRs,nPos));
	}//end for
	
  ifs.close();
	
	return (void *)cSt;
	/////////////////////////////////////////////////
}


int  mt_InitMap24_GRCH38BED(string strFolder,map<Uint,Uint> *mapS_Chain_RS)
{
	///////////////////////////////////分成多个map//////////////////////////////////////////////
	  pthread_t Init_thread[BIN_FILE_NUM];
	  THREADARG  maprsPos[BIN_FILE_NUM];
	  vector<string> aLLFiles;
    string strFileName = "";
    
	  int nCount = getAllFiles(strFolder,aLLFiles);
    if(nCount < BIN_FILE_NUM )
    	{
    		cout<< "BIN_FILE_NUM != FILE NUM";
    		return -1;
    	}
			for(int i = 0;i<nCount;i++) // creat thread
			{
				
				//ALL.chr10_GRCh38.genotypes.20170504.vcf

				strFileName = aLLFiles.at(i);
				
		    int np1 = strFileName.rfind('/')+1;
		    int np2 = strFileName.rfind('.');
		    string sfiname = strFileName.substr(np1);
		    sfiname = sfiname.substr(0,sfiname.find("_"));
		    //Replace_str(sfiname,"chr","");
		    Replace_str(sfiname,"ALL.chr","");
		    Replace_str(sfiname,"X","23");
		    Replace_str(sfiname,"Y","24");
		    if(sfiname.find_first_not_of("0123456789") != -1)
		      continue;
		    int nChr = stoi(sfiname) -1;
		    
		    cout<< "get rs info from :" <<strFileName<< endl;
		    //cout<<nChr<< endl;
		    
		    maprsPos[nChr].nNum = nChr;
		    maprsPos[nChr].Map_PosBag = &mapS_Chain_RS[nChr];
		    maprsPos[nChr].sFileName = strFileName;
		   
			int nerror = pthread_create(&Init_thread[nChr],NULL,thread_fun_GRCh38BED,(void *)&maprsPos[nChr]);
			 if (0!=nerror)
			  	{
			  		 printf("creat thread error :    %d  %s \r\n",i,strerror(nerror));
			  		 return -1;
			  	}	
			  	
			 //sleep(10000);
			}
			
			for(int i = 0;i<BIN_FILE_NUM;i++) //wait end
			{
				char *ps = NULL;
				int nerror = pthread_join(Init_thread[i],(void **)&ps);
				if (0!=nerror)
				{
					printf("pthread_join  %s \r\n", strerror(nerror));
					return -1;
				}
				//printf("pthread_join  %s \r\n", ps);	 
			}
			//end for
	////////////////////////////////////////////////////////////////////////////////////////////
	
	cout<< "start check" << endl;
return 0;
}


string sGetPos(string strRs,map<Uint,Uint>*RS_POSS)
{
	  string sPos = "NULL";
	  Replace_str(strRs,"rs","");
    Replace_str(strRs,"Rs","");
    Replace_str(strRs,"RS","");
    Replace_str(strRs,"=","");
    Replace_str(strRs,"\t","");
    if( strRs == "" || strRs.find_first_not_of("0123456789 ") != -1 || strRs.length() >10)// “-” 是为了识别-1 推
    	{
    		sPos = "error rs";
    		return sPos;
    	}
    
        Uint  unRS = stoUint(strRs);
	  
	      bool bFind = false;
	      int nChr = 0;
	      int nPos = 0;
	      map<Uint,Uint>::iterator it;
	      for(int i = 0;i<BIN_FILE_NUM;i++)
	       {
		        nChr = i+1;
		        it = RS_POSS[i].find(unRS);
		        if(it != RS_POSS[i].end() )
	          {
	          	nPos =  it->second;
	  		    	bFind = true;
	  			    break;
	          }
	       }// end for
	 
	 if(bFind)
	 	{
	 		    if(nChr == 23)
	 		    	sPos = "X\t";
	  	    else if(nChr == 24)
	  	       sPos = "Y\t";
	  	    else
	  	       sPos = Int2string(nChr)  + "\t" ;
	  	     
	  	    sPos += Int2string(nPos);
	  	    sPos = "chr" + sPos;
	 	}
	  return sPos;
	
}


// 用Urs 返回位置和chr
bool nGetPos1kgGRCh38(Uint unRS,map<Uint,Uint>*RS_POSS,int &nPos,int &nChr)
{
	bool bFind = false;
  map<Uint,Uint>::iterator it;
	for(int i = 0;i<BIN_FILE_NUM;i++){
		nChr = i+1;
		it = RS_POSS[i].find(unRS);
		if(it != RS_POSS[i].end() )
	  {
	    nPos =  it->second;
	  	bFind = true;
	    break;
     }
  }// end for
	 
	return bFind;
	
}


/*****************************************************************************
** Function:      n_PickInfo(string &str)
** str:  要解析的字符串行 
** Create Date:  2018.6.30
** Modify Time:  2018.7.03
** Author:        LYJ
** 返回值： -1；错误格式； 0 SNP格式  1：INDEL格式
** Version:      1.0
*******************************************************************************/

static int n_PickInfo2(string &str,int &nPosS,int &nPosE,Uint &urs)
{
	bool bNormal = false;
	bool bSNP = true;
	int nposEnd = 0;
	int npos1 = 0;// 自串头位置
	int npos2 = 0;// 自串尾巴位置
	int nReturn = -1;// 返回值 0:snp  1:indel -1:error
	
	
	npos2 = str.find("\t",0);
	if (npos2!= -1)
	{
		/********************************CHORM**********************************/
		  string sChr = str.substr(0,npos2);
/*		  Replace_str(sChr,"chr","");
		  if(sChr.find_first_not_of("0123456789") != -1 )
	  		return -1;
		  if(sChr == "X")
		  	nChr = 23;
		  else if(sChr == "X")
		  	nChr = 24;
		  else
		  	nChr = stoi(sChr);*/
			npos1 = npos2+1;
		/********************************POS start*************************************/
			npos2 = str.find("\t",npos1);
			string sPos= str.substr(npos1,npos2-npos1);
			int  nPos = stoi(sPos);
			nPosS = stoi(sPos);
			npos1 = npos2+1;
			nposEnd = npos1;
			/********************************POS END*************************************/
			npos2 = str.find("\t",npos1);
			string sPosE= str.substr(npos1,npos2-npos1);
			nPosE = stoi(sPosE);
			npos1= npos2+1;
		/********************************rsID*************************************/
		  string sID= "";
			npos2 = str.find("\t",npos1);
			
			sID = (npos2 == -1)?(str.substr(npos1)):(str.substr(npos1,npos2-npos1));
			
	  	if(Replace_str(sID,"rs","")!= 1)
	  		return -1;
	  	if(sID.find_first_not_of("0123456789") != -1 )
	  		return -1;
	  	urs = stoUint(sID);
	  	str = str.substr(0,nposEnd);
	  	nReturn = 0;
	  	//cout << sChr << "\t" << sPos << "\t" <<sPosE << "\t" << sID << endl;
		}
		

	return nReturn;
}


/*****************************************************************************
** Function:    bGet_VCF37_SNP(string sVCFfile,string sDesDir)
**  sSrcfile // 37版本的VCF 文件,  sSectionDir 转换区间的文件夹, sDespath 目标保存路径
**   sError 转换失败原因
** Create Date:  2018.9.06 
** Modify Time:  2018.9.06
** Author:        LYJ
** Version:      1.0
*******************************************************************************/

bool bCheckBed1KG38(string sSrcfile,string sDespath,map<Uint,Uint>*RS_POSS,string &sError)
{
	bool bRe = true;
	sError = "";
	string sFilename = sSrcfile.substr(sSrcfile.rfind('/')+1,sSrcfile.length() - sSrcfile.rfind('/') -1 );
	cout << "sFilename = " << sFilename <<endl;

	string snp_file;
	snp_file = sFilename.substr(0,sFilename.rfind('.'));
	snp_file += "_error.dat";
	snp_file = sDespath + "/" +  snp_file;
	
	Repalce_char(snp_file,"//","/");
	
	cout << "Out_put file: " << snp_file <<endl;
	int  n_CountLine = 0;
	string str_Line;
	ifstream ifs;
	
	ifs.open(sSrcfile.c_str()) ;
	if (!ifs){
			cout<<"Read File Error, please check file name is right: \n"<<sSrcfile <<endl;
			sError = "读取文件出错 ";
			return false;
	}
		
	FILE *pFile;
	if ((pFile = fopen(snp_file.c_str(),"w+"))==NULL)
	{
		sError = "创建SNP.dat 文件失败 ";
		return false;
	}

	while(!ifs.eof())  
	{  
		getline(ifs,str_Line);
		//cout << str_Line << endl;
		if (0 ==str_Line.find("#")){continue;}
		//if (str_Line.find("VT=SNP") == -1) {continue;}

		n_CountLine++;
		Uint urs = 0;
		int nPosS = 0;
		int nPosE = 0;
		int nChr = -1;
		 		
 		if(0 == n_PickInfo2(str_Line,nPosS,nPosE,urs)) // 抽离出来 抽离出来位置
		{ 
			///////获取GRCh38中Rs号的位置////////////////////////////////////////////////////////
			  int nChr38 = -1;
			  int nPos38 = -1;
		    
		    if(true == nGetPos1kgGRCh38(urs,RS_POSS,nPos38,nChr38) )
		    {
		      if(nPosS != nPos38 )
		    	{
		    		string sError = str_Line + "find in 1kg GRCh38: chr" + Int2string(nChr38) + "\t" + Int2string(nPos38) + " rs";
		    		sError +=  Uint2string(urs) + "\n";
		    		bWritResult(pFile,sError);
	 	          
		    		//cout << str_Line << "find in 1kg GRCh38: chr" << nChr38 << "\t" << nPos38 << " rs:" ;
		    		//printf("%u \n",urs);
		    	}
		    }
		   ////////////////////////////////////////////////////////////////////////////////////////
		}					
		
	}  //end while
	
	ifs.close();  
	if (fclose (pFile) != 0)
		perror("Error occurs when close file"); //报告相应错误

	return bRe;
}

int main(int arg,char *args[])
{

	//string strSrc = "/home/liuyongji/liuyj/grch37_rawdata/chr22.vcf";
	string strSrc = "";
	string strDespath = "";
	string serr = "";
	string sMapBinFolder = "";
	bool bFolder = false;
  

   if(arg <4)
	 	{
	 		 cout << sHelp << sExample;
	 		 return 0;
	 	}
	 	
	 	

	///////////////////////////获取输入参数////////////////////////////////////////////////////
	for(int i=0; i<arg; i++)
	{
   	string sGet = args[i];
		if(i == 1)
		{  
			if(true ==  is_file_exist(sGet))
			 {
			 	if( true == is_dir_exist(sGet))
			 	{
			 		strSrc = sGet;
			 		bFolder = true;
			 		continue;
			 	}
			 	
			 	
			 	if(sGet.find(".bed")== -1 && sGet.find(".vcf")== -1)
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
			if(true == is_dir_exist(sGet) )
			    sMapBinFolder = sGet;
			else
			{
			 	cout << "Reference rs folder not exist " << sGet << endl;
			 	return -1;
			}
		}
		
	}
	
	if(strDespath == ""){
			strDespath = "./";
	}
  
   Repalce_char(strSrc,"//","/");
   Repalce_char(strDespath,"//","/");
   Repalce_char(sMapBinFolder,"//","/");
	
	cout << "Source file : " << strSrc << endl;
	cout << "Destination path :" << strDespath << endl;
	cout << "reference  path :" << sMapBinFolder << endl;

	//string sMapBinFolder = "/home/liuyongji/liuyj/grch38_rawdta/rawvcf"  ; 
	map<Uint,Uint> RS_POSS[BIN_FILE_NUM];

	string strRs = "";//
	if(-1 == mt_InitMap24_GRCH38BED(sMapBinFolder,RS_POSS))
	{
			cout << "Init error " << endl;
			return 0;
	}
	
	
	if(bFolder)
		{
			vector<string> aLLFiles;
       string strFileName = "";
	     int nCount = getAllFiles(strSrc,aLLFiles);

			for(int i = 0;i<nCount;i++) 
			{
				strFileName = aLLFiles.at(i);
				Repalce_char(strFileName,"//","/");
				if(-1 == strFileName.find("bed"))
					 continue;
				cout << "check "<<strFileName << "\n";
				bCheckBed1KG38(strFileName,strDespath,RS_POSS,serr);
		    
			}//end for
		}
	  else
	  	{
	  		bCheckBed1KG38(strSrc,strDespath,RS_POSS,serr);
	  	}
	     
	
  
	return 0;
}

