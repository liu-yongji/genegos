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
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <error.h>
#define _MAX_INT_DIG 32
#define CHR_NUM 24

using namespace  std;


struct ChainBag
{
	bool bReverse;
	int  nSrcStart;
	int  nSrcEnd;
	int  nDesStart;
	int  nDesEnd;
	int  nSpanChr; 
	string strDesChr;
	ChainBag()
	{
		bReverse = false;
		nSrcStart = -1;
		nSrcEnd = -1;
		nDesStart =-1;
		nDesEnd = -1;
		nSpanChr = 0;
		strDesChr = "";
	}
	~ChainBag()
	{
		bReverse = false;
		nSrcStart = -1;
		nSrcEnd = -1;
		nDesStart =-1;
		nDesEnd = -1;
		nSpanChr = 0;
		strDesChr = "";
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
	string sResult; 
	string sFail;
	CmySection *makeUp;
	map<string,CmySection> *map_Section; // convert via stand chain
	
	THREAD_FILEMMAP_ARG()
	{
		nlen = -1;
	  sResult = "";
    sFail  = "";  
    pStart = NULL;
	  makeUp = NULL;
	  map_Section = NULL;
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
typedef  pair<map<int,ChainBag>::iterator,bool> MapRet;
typedef std::map<string,CmySection>::iterator mapChr_Chains_iterator;
typedef std::map<string,std::map<int,ChainBag> >::iterator ChrChain_iterator;




/*****************************************************************************
** Function:     to_string(int _Val)
** Create Date:  2018.7.17
** Modify Time:  2018.7.17
** Author:        LYJ
** Version:       1.0
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
** Function:     is_file_exist(string sFilename)
** Create Date:  2018.7.17
** Modify Time:  2018.7.17
** Author:        LYJ
** Version:       1.0
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
** Function:      bWritResult
** write str to file
** Create Date:  2018.7.04
** Modify Time:  2018.7.04
** Author:       LYJ
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
** Function:      Replace_char(string& str,const string&old_value,const string& new_value)
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:       1.0
*******************************************************************************/
int Replace_char(string &str,const char &old_value,const string& new_value)
{
	int nRe = 0;
	for(int pos=0;pos!=-1;pos+=new_value.length())
	{
		if((pos=str.find(old_value,pos))!=-1)
		{
			str.replace(pos,1,new_value);
			nRe++;
		}
		else 
			break;
	}
	return nRe;
}

int Replace_char(string &str,const string &old_value,const string& new_value)
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
** Function:     getCurrentPath(string &sPath )
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:       1.0
*******************************************************************************/
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

/*****************************************************************************
** Function:     GetFileSize(string sfname)
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:       1.0
*******************************************************************************/
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


/*****************************************************************************
** Function:     GetChr(string strch)
** return int  1~22 x:23 y=24
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:       1.0
*******************************************************************************/

 static int GetChr(string strch)
  {
  	Replace_char(strch,"chr","");
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
  
/*****************************************************************************
** Function:     GetChr(int nChr)
** return string  chr1~22  23:chrX   24:chrY
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:       1.0
*******************************************************************************/
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
  
/*****************************************************************************
** Function:     GetPos(string strPos)
** return int  filter wrong format
** Create Date:  2018.7.05
** Modify Time:  2018.7.05
** Author:        LYJ
** Version:       1.0
*******************************************************************************/
  static int GetPos(string strPos)
  {
  	if(strPos == "" || strPos.find_first_not_of("0123456789")!= -1)
  		return -1;
		return stoi(strPos);
  }
 
 
  
 
/*****************************************************************************
** Function:    GetPosSectionMakeUp(int nSpos,CmySection makeUp)
** return get mapping pos from target chain
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
		else if (nSpos <it->first && nSpos< it->second.nSrcEnd)
		{
			break;
		}

	}
	return nRe;

}

static inline int GetPosSectionMakeUp(int nSpos,CmySection *pmakeUp,string &sDesChr)
{
	int nRe = -1;
	if(nSpos<0)
		return nRe;
		
	Map_IntInt_iterator itInt = pmakeUp->map_Chain_1V1.find(nSpos);
	if (itInt != pmakeUp->map_Chain_1V1.end())
	{
		nRe = itInt->second;
		return nRe;
	}
	//std::map<int,ChainBag> &map_Chain = pmakeUp->map_Chain;
	Map_Chain_iterator it = pmakeUp->map_Chain.begin();
	for (;it != pmakeUp->map_Chain.end();it++)
	{
		if (nSpos >=it->first && nSpos<= it->second.nSrcEnd)
		{
			if(""!= it->second.strDesChr)
			  sDesChr = it->second.strDesChr;
			
			if (!it->second.bReverse)//"+"
				nRe = it->second.nDesStart + nSpos - it->first;
			else
				nRe = it->second.nDesStart - nSpos + it->first; 
			break;
		}
		else if (nSpos <it->first && nSpos< it->second.nSrcEnd)
		{
			break;
		}

	}
	return nRe;

}

///////////////////////modify by lyj 11.22 to make sure stand chains 
static inline int GetPosSectionMakeUp(int nSpos,CmySection *pmakeUp,string &sDesChr,bool &bRevs)
{
	int nRe = -1;
	if(nSpos<0)
		return nRe;
		
	Map_IntInt_iterator itInt = pmakeUp->map_Chain_1V1.find(nSpos);
	if (itInt != pmakeUp->map_Chain_1V1.end())
	{
		nRe = itInt->second;
		return nRe;
	}
	//std::map<int,ChainBag> &map_Chain = pmakeUp->map_Chain;
	Map_Chain_iterator it = pmakeUp->map_Chain.begin();
	for (;it != pmakeUp->map_Chain.end();it++)
	{
		if (nSpos >it->first && nSpos<= it->second.nSrcEnd)
		{
			if(""!= it->second.strDesChr)
			  sDesChr = it->second.strDesChr;
			
			bRevs = it->second.bReverse;
			if (!bRevs)//"+"
				nRe = it->second.nDesStart + nSpos - it->first;
			else
				nRe = it->second.nDesStart - nSpos + it->first; 
			break;
		}
		else if (nSpos <it->first && nSpos< it->second.nSrcEnd)
		{
			break;
		}

	}
	return nRe;

}

static inline int GetPosSection(int nSpos,CmySection *pmakeUp)
{
	int nRe = -1;
	if(nSpos<0)
		return nRe;
	
	Map_Chain_iterator it = pmakeUp->map_Chain.begin();
	for (;it != pmakeUp->map_Chain.end();it++)
	{
		if (nSpos >it->first && nSpos<= it->second.nSrcEnd)
		{ 
			nRe  = nSpos - it->first;
			break;
		}
		else if (nSpos <it->first && nSpos< it->second.nSrcEnd)
			break;

	}
	return nRe;
}


/*****************************************************************************
** Function:   GetMakeupChain_Reverse(string chianFile,std::map<int,ChainBag> &map_Chain)
** 38 -> 37    only ues in cnyx mode
** Create Date:  2018.8.20
** Modify Time:  
** Author:        LYJ
** Version:      1.0
*******************************************************************************/
int GetMakeupChain_Reverse(string chianFile,std::map<int,int> &map_Chain_1v1,std::map<int,ChainBag> &map_Chain)
{
	ifstream ifs;
	ifs.open(chianFile.c_str(),ios_base::in);
	if (!ifs)
	{
		cout<<"Read File Error, please check file name is right: "<<chianFile<<endl;
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
		if (str_Line.find("<1v1>") != -1) 
		{
			np1 = str_Line.find("<1v1>");
			string src37 = str_Line.substr(0,np1);
			int n37 = stoi(src37);

			np1 = str_Line.find(" ",np1+1);
			string src38 =str_Line.substr(np1);
			int n38 = stoi(src38);
			map_Chain_1v1.insert(make_pair(n38,n37));
			continue;
		}
		else
		{
		   string srcStart37 = str_Line.substr(np1,np2-np1);
		   np1 = np2 +1;
		   int nStart37 = stoi(srcStart37);
		   ///////////////////////////////////////////////
		   np2 = str_Line.find(" ",np1);
		   string srcEnd37 = str_Line.substr(np1,np2-np1);
		   np1 = np2 +1;
		   int nSEND37 = stoi(srcEnd37);

			///////////////////////////////////////////////
			np2 = str_Line.find(" ",np1);
		  string sstrand = str_Line.substr(np1,np2-np1);
	  	np1 = np2 +1;
		  //////////////////////////////////////////////
		  np2 = str_Line.find(" ",np1);
		  string sDesStart38 = str_Line.substr(np1,np2-np1);
		  np1 = np2 +1;
		  int nES38 = stoi(sDesStart38);

		  np2 = str_Line.find(" ",np1);
		  string sDesEnd38 = str_Line.substr(np1,np2-np1);
		  np1 = np2 +1;
		  int nED38 = stoi(sDesEnd38);
		
		  if (sstrand == "-")
		  	{
		  		c_bag.bReverse = true;
		  		c_bag.nSrcStart = nED38;
		      c_bag.nSrcEnd =  nES38;
		      c_bag.nDesStart = nSEND37;
		      c_bag.nDesEnd = nStart37;
		  	}
		  else
		  	{		      
		      c_bag.nSrcStart = nES38;
		      c_bag.nSrcEnd =  nED38;
		      c_bag.nDesStart = nStart37;
		      c_bag.nDesEnd = nSEND37;
		  		c_bag.bReverse = false;
		  	}
		  
		  if (c_bag.nSrcStart != -1)
		   {
			   map_Chain.insert(make_pair(c_bag.nSrcStart,c_bag));
		   }
		 }
		
	}  //end while  

	ifs.close();
	return nCount;
}


/*****************************************************************************
** Function:   GetMakeupChain(string chianFile,std::map<int,ChainBag> &map_Chain)
** 37 -> 38
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
		return -1;

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
		if (str_Line.find("<1v1>") != -1) //
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


		/////////////////////////////////////////////
		np2 = str_Line.find(" ",np1);
		string srcEnd = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;
		int nSEND = stoi(srcEnd);

		////////////////////////////////////////////
		np2 = str_Line.find(" ",np1);
		string sstrand = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;

		////////////////////////////////////////////
		np2 = str_Line.find(" ",np1);
		string sDesStart = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;
		int nES = stoi(sDesStart);
		//c_bag.nDesStart = nES;

		////////////////////////////////////////////
		np2 = str_Line.find(" ",np1);
		string sDesEnd = str_Line.substr(np1,np2-np1);
		np1 = np2 +1;
		int nED = stoi(sDesEnd);
		//c_bag.nDesEnd = nED;
		if ((nStart == c_bag.nSrcEnd +1)&&((!c_bag.bReverse && nES == c_bag.nDesEnd +1)||(c_bag.bReverse && nES == c_bag.nDesEnd -1) ))// 
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

		}
		
	}  //end while

	MapRet ret = map_Chain.insert(std::make_pair(c_bag.nSrcStart,c_bag)); 
	if (!ret.second)
	{
		cout<<"insert error"<<endl;
	}

	ifs.close();
	return nCount;
}

/*****************************************************************************
** Function:   bCoordinate(string &sChr,string &sStart,CmySection *pmakeUp)
** convert file via stand chains file 
** Create Date:  2019.12.15
** Modify Time:  
** Author:        LYJ
** Version:       1.0
*******************************************************************************/

  //ordinary VCF mode
 bool inline bCoordinate(string &sChr,string &sStart,CmySection *pmakeUp)
 {
 	   int nStart = GetPos(sStart);
	   if(nStart == -1) {return false;}
	   	
     string sDesChr = "";
     bool bRe_E = false;
     int nDesPos= GetPosSectionMakeUp(nStart,pmakeUp,sDesChr,bRe_E);
     if(nDesPos == -1)
     	return false;
     
     if("" != sDesChr)
     	sChr = sDesChr;
     
     sStart = to_string(nDesPos);
    return true;
 	
 }
 
 //ordinary BED mode
 bool inline bCoordinate(string &sChr,string &sStart,string &sDend,CmySection *pmakeUp)
 {
 	   int nt_start = GetPos(sStart);
	   int nt_end = GetPos(sDend);
	   if(nt_start == -1 || nt_end == -1 ) {return false;}
	   	
	   int nGap = nt_end - nt_start;
     bool bRe_S = false;
     bool bRe_E = false;
     string DesChr1 = "";
     string DesChr2 = "";
     
     int nPosEnd = GetPosSectionMakeUp(nt_end,pmakeUp,DesChr2,bRe_E);
     if(nPosEnd == -1)
     	return false;
     	
     int nPosStart = GetPosSectionMakeUp(nt_start,pmakeUp,DesChr1,bRe_S);
    
     if(nPosStart == -1 || DesChr1 != DesChr2)
     	{
     		if(DesChr2 != "") {sChr = DesChr2;}
     		if(bRe_E)
     			{
     				nPosStart = nPosEnd;
     				nPosEnd += nGap;
     			}
     		else
     			nPosStart = nPosEnd - nGap;
     	}
     	else //nPosStart != -1
     	{
     		 if(DesChr2 != "") {sChr = DesChr2;}
     		 //////////////////////////////////////////////////
     		 if(nGap != abs(nPosStart-nPosEnd)) 
     		 	{
     		 		 int nGapReal = GetPosSection(nt_start,pmakeUp) +1;
     		 		 if( nGapReal>=nGap )
     		 		 	{
     		 		 		if(bRe_E)
     		 		 			nPosStart = nPosEnd + nGap;
     		 		 		else
     		 		 			nPosStart = nPosEnd - nGap;
     		 		 	}
     		 	}
     		 	//////////////////////////////////////////////////
     		 
     		 if(nPosStart > nPosEnd ) //
     		 {
     		 			nt_start = nPosStart;
     				  nPosStart = nPosEnd;
     				  nPosEnd = nt_start;
     		 }
     	}
     	sStart = to_string(nPosStart);
     	sDend  = to_string(nPosEnd);
    
    return true;
 	
 }
 
 static inline int nSplitStr2List(string &str,vector<string> &sL,const string splt)
{
	int npS = 0;
	int npE = 0;
	int nRe = 0;
	string strGet = "";
	sL.clear();
	for (;npE != -1;)
	{
		npE = str.find(splt,npS);
		if (npE == -1) 
		{
			strGet = str.substr(npS);
			sL.push_back(strGet);
			nRe++ ;
			continue;
		}
		strGet = str.substr(npS,npE-npS);
		npS = npE +1;
		sL.push_back(strGet);
		nRe ++;
	}
	return nRe;
}
 
 bool bIintChains(string sChainFile,map<string,CmySection> *mapChr_Chains)
{
	ifstream ifs;
	ifs.open(sChainFile.c_str(),ios_base::in);
	if (!ifs)
	{
		cout<<"Read File Error, please check file name is right: "<<sChainFile<<endl;
		return false;
	}

  int ntStart,ntEnd,nqStart,nqEed,nSrcStart,nDesStart,nSrcEnd,nDesEnd,nqSize,nSpan;
  ntStart=ntEnd=nqStart=nqEed=nSrcStart=nDesStart=nSrcEnd=nDesEnd=nqSize=nSpan=0;
	
	string str_Line,sDesChrName,sLastChr,sStand;
	str_Line=sDesChrName=sLastChr=sStand="";
	ChainBag c_bag;
	CmySection sC;
	mapChr_Chains_iterator ChainsIter;
	vector<string> subDis;

	while(!ifs.eof())  
	{  
		getline(ifs,str_Line);
		if (str_Line == "\r" ||str_Line == "")
			continue;
		
		if (str_Line.find("chain") != -1) // head
		 {
			if( 0 == nSplitStr2List(str_Line,subDis," ")) //splited by 'space'
				{
					ifs.close();
	        return false;
				}
			
			if(sLastChr == "")
				{
					sLastChr = subDis[2];
					mapChr_Chains->insert(std::make_pair(subDis[2],sC));
					ChainsIter = mapChr_Chains->find(subDis[2]);
				} 
			else if(sLastChr != subDis[2]) // change chain
				{
					ChainsIter = mapChr_Chains->find(subDis[2]);
					if(ChainsIter == mapChr_Chains->end())
					 {
					  	mapChr_Chains->insert(std::make_pair(subDis[2],sC));
					  	ChainsIter = mapChr_Chains->find(subDis[2]);
					  	if(ChainsIter == mapChr_Chains->end())
					  	{
					  		 ifs.close();
	               return false;
					  	}
					 }
				}

			nSrcStart = stoi(subDis[5]);
			nDesStart = stoi(subDis[10]);
			nqSize = stoi(subDis[8]);				
			sStand = subDis[9];
			c_bag.strDesChr = (subDis[2] == subDis[7])?(""):(subDis[7]);
			c_bag.bReverse = (sStand == "-")?(true):(false);
		 }
		else // normal three or one    // "167417 50000 80249" or "40302"
			{
				Replace_char(str_Line," ","\t");// ensemble is space ; ucsc is \t
				int nCell = nSplitStr2List(str_Line,subDis,"\t");
				nSpan = stoi(subDis[0]);
			  c_bag.nSrcStart = nSrcStart;
			  c_bag.nSrcEnd = nSrcStart + nSpan;
			  nDesEnd = nDesStart + nSpan;
			  if (c_bag.bReverse) //"-"
			   {
				    c_bag.nDesStart = nqSize - (nDesStart);
				    c_bag.nDesEnd = nqSize - nDesEnd ;
			   }
			 else
			   	{
				    c_bag.nDesStart = nDesStart;
				    c_bag.nDesEnd = nDesEnd;
			    }
			  if( 3 == nCell )
			  	{ 
			        nSrcStart = c_bag.nSrcEnd + stoi(subDis[1]);
			        nDesStart = nDesEnd + stoi(subDis[2]);
			    }
			    MapRet ret =  ChainsIter->second.map_Chain.insert(std::make_pair(c_bag.nSrcStart,c_bag));
				  if (!ret.second)
				   {
					    ifs.close();
	            return false;
				   }
			}
	}
	
	ifs.close();
	return true;
}


  
  void * TCnvtBed_cnyx(void* pT)
	{
		  THREAD_FILEMMAP_ARG *pBag = (THREAD_FILEMMAP_ARG *)pT;
			char *pChar = pBag->pStart;
	    long nlen = pBag->nlen;
	    int nChr = -1;
	    int nPos = -1;
	    int nTCout = 0;
	    int  nslen =0;
	    int  nLine_len =0;
	    int  nreflen =0;         //chr + pos £»
	    string sChr = "";
	    string sPosStart = "";   // posend
	    string sPos = "";
	    string sPosEnd = "";      // posend
	    string sLine = "";
	    bool  bConvert = false;
	    
	    pBag->sResult.reserve(nlen*1.1); 
	    
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
	    		        if((*pStart_Line) == '#')
	    		        	pBag->sResult += sLine;
	    		        else
	    		        	pBag->sFail += sLine;
	    				 	}
	    				
	    				nTCout =  nslen = nreflen = nLine_len = 0;
	    			  sChr = sPos = "";
	            pS = pStart_Line = pChar;
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
	    
	    ////////////////////////last line without///////////////////////////// 
	   if(nLine_len >0)
	   	{
	   	  if(bConvert)
	   	  	{
	    	   sLine.assign(pStart_REF,nLine_len-nreflen-1);
	    	   sLine = sChr + "\t" + sPosStart + "\t"  + sPosEnd + "\t" + sLine;
	    	   pBag->sResult += sLine;
	        }
	      else
	      	{
	    	    sLine.assign(pStart_Line, nLine_len-1);
	         if(sLine.find("#")== 0)
	    		   pBag->sResult += sLine;
	    	   else
	    		   pBag->sFail += sLine;
	        }
	   	}
	   //////////////////////////////////////////////////////// 

	    
	    
	   
	   string strRe = " finish \n";
	   char *cSt =(char *) strRe.data();
	  return (void *)cSt;
  }
  
  void * TCnvtVCF_cnyx(void* pT)
	{
		  THREAD_FILEMMAP_ARG *pBag = (THREAD_FILEMMAP_ARG *)pT;
			char *pChar = pBag->pStart;
	    long nlen = pBag->nlen;
	    int nChr = -1;
	    int nPos = -1;
	    int nTCout = 0;
	    int  nslen =0;
	    int  nLine_len =0;
	    int  nreflen =0;  // chr + pos len£»
	    string sPass = "";
	    string sChr = "";
	    string sPos = "";
	    string sPosEnd = "";   // posend
	    string sLine = "";
	    bool  bConvert = false;
	    pBag->sResult.reserve(nlen); // init result len
	    
	    char *pS = pChar;
	    char *pStart_Line = pChar;
	    char *pStart_REF = pChar;
	    
	    for(;nlen >=0 ;nlen--){
	    	switch(*pChar++){
	    		case'\n':
	    		  {
	    				if(bConvert)
	    					{
	    						sLine.assign(pStart_REF, nLine_len-nreflen+1);
	    						sLine = sChr + "\t" + sPos + "\t" + sLine;
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
	    
	   ////////////////////////last line without '\n'////////////////////////////// 
	   if(nLine_len >0)
	   	{
	   	  if(bConvert)
	    		{
	    			sLine.assign(pStart_REF, nLine_len-nreflen+1);
	    			sLine = sChr + "\t" + sPos + "\t" + sLine;
	    			pBag->sResult += sLine;
	    		}
	      else
	      	{
	    	    sLine.assign(pStart_Line, nLine_len-1);
	         if(sLine.find("#")== 0)
	    		   pBag->sResult += sLine;
	    	   else
	    		   pBag->sFail += sLine;
	        }
	   	}
	   //////////////////////////////////////////////////////// 
	    
	   
	  string strRe = " finish \n";
	  char *cSt =(char *) strRe.data();
	  return (void *)cSt;
  }
  
  void * TCnvtBed_stand(void* pT)
	{
		  THREAD_FILEMMAP_ARG *pBag = (THREAD_FILEMMAP_ARG *)pT;
			char *pChar = pBag->pStart;
	    long nlen = pBag->nlen;
	    map<string,CmySection>* pChr_Sec = pBag->map_Section;
	    mapChr_Chains_iterator it;
	    int nChr = -1;
	    int nspanChr = -1;
	    int nPos = -1;
	    int nTCout = 0;
	    int  nslen =0;
	    int  nLine_len =0;
	    int  nreflen =0;  // chr + pos len£»
	    string sChr = "";
	    string sPos = "";
	    string sPosStart = ""; // posend
	    string sPosEnd = ""; // posend
	    string sLine = "";
	    string sLastChar = "";
	    bool  bConvert = false;
	    int nPosStart = -1;
	    int nPosEnd = -1;
	    string DesChr1 = "";
	    string DesChr2 = "";
	    
	    
	    pBag->sResult.reserve(nlen*1.1); // str mem
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
	    						sLine = sChr + "\t" + sPosStart + "\t"  + sPosEnd + "\t" + sLine;
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
	            nTCout = nslen = nreflen = nLine_len = 0;
	            sChr = sPos = "";
	            pS = pStart_Line = pChar;
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
	    							sPosStart.assign(pS,nslen);
	    							if( sChr != sLastChar )
	    								{
	    									sLastChar = sChr;
	    									it = pChr_Sec->find(sChr);
	    							    if(it == pChr_Sec->end())// chrx //x
	    								     it = pChr_Sec->find("chr"+sChr);
	    								  if(it == pChr_Sec->end())// x
	    								  	{
	    								  		string s = sChr;
	    								  		Replace_char(s,"chr","");
	    								  		it = pChr_Sec->find(s);
	    								  	}    
	    								}	
	    						  break;
	    						}
	    					case 3:
	    						{
	    							sPosEnd.assign(pS,nslen);
	    							if(bCoordinate(sChr,sPosStart,sPosEnd,&it->second))
	    							{
			                pStart_REF = pChar;
			                nreflen = nLine_len;
			                bConvert = true;
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
	    
	   ////////////////////////////////////////////////////// 
	   if(nLine_len >0)
	   	{
	   	  if(bConvert)
	   	  	{
	    	   sLine.assign(pStart_REF,nLine_len-nreflen-1);
	    	   sLine = sChr + "\t" + sPosStart + "\t"  + sPosEnd + "\t" + sLine;
	    	   pBag->sResult += sLine;
	        }
	      else
	      	{
	    	    sLine.assign(pStart_Line, nLine_len-1);
	         if(sLine.find("#")== 0)
	    		   pBag->sResult += sLine;
	    	   else
	    		   pBag->sFail += sLine;
	        }
	   	}
	   //////////////////////////////////////////////////////// 
	    
	    
	    
	   
	   string strRe = " finish \n";
	   char *cSt =(char *) strRe.data();
	  return (void *)cSt;
  }
  
  void * TCnvtVCF_stand(void* pT)
	{
		  THREAD_FILEMMAP_ARG *pBag = (THREAD_FILEMMAP_ARG *)pT;
			char *pChar = pBag->pStart;
	    long nlen = pBag->nlen;
	    map<string,CmySection>* pChr_Sec = pBag->map_Section;
	    mapChr_Chains_iterator it;
	    
	    int nChr = -1;
	    int nspanChr = -1;
	    int nPos = -1;
	    int nTCout = 0;
	    int  nslen =0;
	    int  nLine_len =0;
	    int  nreflen =0;  // chr + pos
	    
	    string sChr = "";
	    string sPos = "";
	    string sLastChar = "";
	    string sLine = "";
	    bool  bConvert = false;
	    pBag->sResult.reserve(nlen); //
	    
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
	    						sLine = sChr + "\t" + sPos + "\t" + sLine;
	    						pBag->sResult += sLine;
	    					}
	    				 else
	    				 	{
	    				 		sLine.assign(pStart_Line, nLine_len+1);
	    		        //if(sLine.find("#")== 0)
	    		        if(sLine[0]=='#')
	    		        	pBag->sResult += sLine;
	    		        else
	    		        	pBag->sFail += sLine;
	    				 	}
	    				
	    				nTCout = 0;
	    				nslen =0;
	    				nreflen =0;
	    				nLine_len =0;
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
	    							sPos.assign(pS,nslen);
	    							if( sChr != sLastChar )
	    								{
	    									sLastChar = sChr;
	    									it = pChr_Sec->find(sChr);
	    							    if(it == pChr_Sec->end())// chrx //x
	    								     it = pChr_Sec->find("chr"+sChr);
	    								  if(it == pChr_Sec->end())// x
	    								  	{
	    								  		string s = sChr;
	    								  		Replace_char(s,"chr","");
	    								  		it = pChr_Sec->find(s);
	    								  	}    
	    								}	
	    								
	    							if(bCoordinate(sChr,sPos,&it->second))
	    							{
			                pStart_REF = pChar;
			                nreflen = nLine_len;
			                bConvert = true;
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
	    
	  //////////////////////////////////////////////////// 
	   if(nLine_len >0)
	   	{
	   	  if(bConvert)
	    		{	    			    			
	    			sLine.assign(pStart_REF, nLine_len-nreflen+1);
	    			sLine = sChr + "\t" + sPos + "\t" + sLine;
	    			pBag->sResult += sLine;
	    		}
	      else
	      	{
	    	    sLine.assign(pStart_Line, nLine_len-1);
	         if(sLine.find("#")== 0)
	    		   pBag->sResult += sLine;
	    	   else
	    		   pBag->sFail += sLine;
	        }
	   	}
	   //////////////////////////////////////////////////////// 
	   
	  string strRe = " finish \n";
	  char *cSt =(char *) strRe.data();
	  return (void *)cSt;
  }

bool Cnvt_File(string sSrcfile,string sSectionDir,string sDespath,string &sError,bool bSrcVersion)
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
	     
	 if ((fd = open(sSrcfile.c_str(), O_RDONLY)) < 0){  
	      perror("open"); 
	      return false;  
	 } 
	  
	  nFilesize = GetFileSize(fd);
	  if ((mapped = (char*)mmap(NULL, nFilesize, PROT_READ, MAP_SHARED, fd, 0)) == (void *)-1) {  
	   	    close(fd);
	        perror("mmap"); 
	        return false; 
	   }
	  close(fd);
	///////////////////////////////////////////intital convert section/////////////////////////////////////
	
	for (int i= 0;i<24 ;i++){
		strSection = "chr"+ to_string(i+1) + "_convert.dat";
		if (i == 22)
			strSection =  "chrX_convert.dat";
		else if (i == 23)
			strSection =  "chrY_convert.dat";
		strSection = sSectionDir + "/" + strSection;
		
		Replace_char(sSectionDir,"//","/");
		if(bSrcVersion == true){ //38--¡·37
				if(GetMakeupChain_Reverse(strSection,mysection[i].map_Chain_1V1,mysection[i].map_Chain) <= 0){
					sError = "Intial Genegos-chains error";
			    return false;
				}
		}
		else{ // 37-->38
			if(GetMakeupChain(strSection,mysection[i].map_Chain_1V1,mysection[i].map_Chain) <= 0){
					sError = "Intial Genegos-chains error";
			    return false;
				}	
		}
	}
	
	//////////////////////////////////////////////MAKE DES FILES/////////////////////////////////////////////////////////

	string sFilename = sSrcfile.substr(sSrcfile.rfind('/')+1,sSrcfile.length() - sSrcfile.rfind('/') -1 );
	string mapFile,postfix;
	mapFile = sFilename.substr(0,sFilename.rfind('.'));
	postfix = sFilename.substr(sFilename.rfind('.'));
	mapFile += "_genegos" + postfix;
	mapFile = sDespath + "/" +  mapFile;
	Replace_char(mapFile,"//","/");

	
	if ((pFile = fopen(mapFile.c_str(),"w+"))==NULL){
		sError = "Creat file error\n";
		return false;
	}
	
	string unmapFile = sFilename.substr(0,sFilename.rfind('.'));
	unmapFile += "_genegos.unmap";
	unmapFile = sDespath + "/" +  unmapFile;
	Replace_char(unmapFile,"//","/");
	
	if ((pFSpecial = fopen(unmapFile.c_str(),"w+")) == NULL){
		sError = "Creat file error\n";
		if (fclose (pFile) != 0)
			perror("Error occurs when close file"); 
		
		return false;
	}


	 long nPerThread = (nFilesize >4096)? (nFilesize/ncpuNum):nFilesize; 
	 long nLeft = nFilesize;
	 char *pStart = mapped;
	 long  nlen = 0;
	 int nRealThread = 0;
	 
	  for(int i= 0;i< ncpuNum && nLeft >0;i++)
	  {
      nRealThread ++;
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
		  for(;(*pStart) != '\n'&&nLeft>0;pStart++){
				nLeft --;
			  nlen ++;
			}
			 pStart++;
		 	 nLeft --;
		 	 nlen++;
			Thread_Check[i].nlen = nlen;
	  }
	  
	 
	 
	 for (int i=0;i<nRealThread;i++){
			int nerror = 0;
			if(bVcf == false)
				nerror = pthread_create(&Init_thread[i],NULL,TCnvtBed_cnyx,(void *)&Thread_Check[i]);
			else
				nerror = pthread_create(&Init_thread[i],NULL,TCnvtVCF_cnyx,(void *)&Thread_Check[i]);
			if (0!=nerror){
				sError = "Creat thread error \n";
	      if (fclose (pFile) != 0)
	      	perror("Error occurs when close file"); 
			  return false;
			}
	 } 
		 
	for (int i = 0; i <nRealThread ;i++){
				char *ps = NULL;
				int nerror = pthread_join(Init_thread[i],(void **)&ps);
				if (0!=nerror){
					sError = "Wait thread error \n";
					printf("pthread_join  %s \n", strerror(nerror));
					return false;
				}
				else {
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

////////////////////convert file via stand chain//////////////////////////////////////////////////////////////////
bool Cnvt_File_Stand(string sSrcfile,string sChainFile,string sDespath,string &sError)
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
	///////////////////////////////////////////intital convert section via stand chains/////////////////////////////////////
	 map<string,CmySection> mapChr_Chains;
	 if( false == bIintChains(sChainFile,&mapChr_Chains))
	 	{
	 		    close(fd);
	        sError = "initialize chains error\n"; 
	        return false; 
	 	}
	//////////////////////////////////////////////MAKE DES FILES////////////////////////////////////////////////////////////
	
	string sFilename = sSrcfile.substr(sSrcfile.rfind('/')+1,sSrcfile.length() - sSrcfile.rfind('/') -1 );
	string mapFile,postfix;
	mapFile = sFilename.substr(0,sFilename.rfind('.'));
	postfix = sFilename.substr(sFilename.rfind('.'));
	mapFile += "_genegos" + postfix;
	mapFile = sDespath + "/" +  mapFile;
	Replace_char(mapFile,"//","/");

	
	if ((pFile = fopen(mapFile.c_str(),"w+"))==NULL)
	{
		sError = "Creat file error\n";
		return false;
	}
	
	string smark_file = sFilename.substr(0,sFilename.rfind('.'));
	smark_file += "_genegos.unmap";
	smark_file = sDespath + "/" +  smark_file;
	Replace_char(smark_file,"//","/");
	
	if ((pFSpecial = fopen(smark_file.c_str(),"w+"))==NULL)
	{
		sError = "Creat file error\n";
		if (fclose (pFile) != 0)
			perror("Error occurs when close file"); 
		
		return false;
	}
	
	 
	 long nPerThread = (nFilesize >4096)? (nFilesize/ncpuNum):nFilesize; //split mem
	 long nLeft = nFilesize;
	 char *pStart = mapped;
	 long  nlen = 0;
	 int nRealThread = 0;
	 
	  for(int i= 0;i< ncpuNum && nLeft >0;i++)
	  {
      nRealThread ++;
			Thread_Check[i].pStart = pStart;
			Thread_Check[i].makeUp  = NULL ;
			Thread_Check[i].map_Section = &mapChr_Chains;
			if(nLeft <= nPerThread)
			{
				Thread_Check[i].nlen = nLeft;
				nLeft = 0;
				break; 
			}
				  
			nLeft -= nPerThread;
		  pStart += nPerThread;
		  nlen = nPerThread;
			for(;(*pStart) != '\n'&&nLeft>0;pStart++)
			{
				nLeft --;
				nlen ++;
			}
			
			 pStart++;
		 	 nLeft --;
		 	 nlen++;
			 Thread_Check[i].nlen = nlen;
	  }
	  
	  	 
	   for (int i=0;i< nRealThread;i++)
		 {
			int nerror = 0;
			if(bVcf == false)
				nerror = pthread_create(&Init_thread[i],NULL,TCnvtBed_stand,(void *)&Thread_Check[i]);
			else
				nerror = pthread_create(&Init_thread[i],NULL,TCnvtVCF_stand,(void *)&Thread_Check[i]);
			if (0!=nerror)
			{
				sError = "Creat thread error \n";
	      if (fclose (pFile) != 0)
	      	perror("Error occurs when close file");
			  return false;
			}
		 } 
		 
		for (int i = 0; i <nRealThread ;i++)
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
	 if(arg <3){
	 		cout << "Input error! \n";
	 		return 0;
	 	}

	string sSectionDir ="";
	string strSrc = "";
	string strDespath = "";
	string serr = "";
	bool bSrcVersion = false; // false = GRCh38   true = GRCh37
	for(int i=0; i<arg; i++)
	{
   	string sGet = args[i];
   	switch (i)
   	{
   		case 1:  
   			     if(true ==  is_file_exist(sGet)){
			 	       if(sGet.find(".VCF")== -1 && sGet.find(".vcf")== -1 && sGet.find(".bed")== -1&& sGet.find(".BED")== -1){
			 			       cout << "Source file format error < expectd format is : \".vcf\" or \".bed\"> "<< endl;
			 		         return -1;
			 		     }
				        strSrc = sGet;
			        }
			       else{ cout << "Source file not exist " << sGet << endl; return -1;} 
   		break;
   		case 2:
   			     if(true == is_dir_exist(sGet)) strDespath = sGet;
			       else{ cout << "DesPath not exist " << sGet << endl; return -1;}
   		break;
   		case 3:
   			     if(true == is_dir_exist(sGet) || true ==  is_file_exist(sGet) )  sSectionDir = sGet;
			       else { cout << "chains file not exist " << sGet << endl; return -1;}
   		break;
   		case 4:
   			     if(sGet == "-R" || sGet == "-r" ) bSrcVersion = true;
   		break;
   		default:
   			break;	
   	}
	}
	
	if(strDespath == ""){
		 if(!getCurrentPath(strDespath)){
					cout << "DesPath is required " << endl;
				 	return -1;
		}	 	
	}
  
	cout << "Source file : " << strSrc << endl;
	cout << "Despath :" << strDespath << endl;
	
	time_t rawtime; 
  struct tm * timeinfo; 
  time ( &rawtime ); 
  timeinfo = localtime ( &rawtime ); 
  printf ( "Start time is: %d:%d:%d  \n", timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec );
  
  if(sSectionDir.find(".chain") == -1) // cnyx mode
  	{
  		 if( false == Cnvt_File(strSrc,sSectionDir,strDespath,serr,bSrcVersion)){
   	      cout << serr << endl;
   	   }
       else
   	     cout << "genegos convert done\n";
  	}
  	else  // stand mode
  		{
  			if( false == Cnvt_File_Stand(strSrc,sSectionDir,strDespath,serr)){
   	       cout << serr << endl;
   	    }
  		}
 
  time ( &rawtime ); 
  timeinfo = localtime ( &rawtime ); 
  printf ( "Finished time is: %d:%d:%d    \n", timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec );
	return 0;
}
