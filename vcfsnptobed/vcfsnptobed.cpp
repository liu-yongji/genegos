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

using namespace  std;


string sHelp = "Usage: ./vcfSnp2bed input_VCF_file/input_VCF_folder  output_folder \n";
string sExample = "Example file mode: ./vcfSnp2bed ./chr1.vcf ./ \nExample foldr mode:./vcfSnp2bed ./allvcf/ ./ \n";

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
** Function:  		getAllFiles(string path, vector<string>& files) 
** 获取文件夹下所有的文件名存入vector
** Create Date:  	2018.11.6
** Modify Time:  
** Author:        LYJ
** Version:     	 1.0
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


/*****************************************************************************
** Function:      n_PickInfo(string &str)
** str:  要解析的字符串行 
** Create Date:  2018.6.30
** Modify Time:  2018.7.03
** Author:        LYJ
** 返回值： -1；错误格式； 0 SNP格式  1：INDEL格式
** Version:      1.0
*******************************************************************************/
 static int n_PickInfo(string &str)
{
	bool bNormal = false;
	bool bSNP = true;
	int nposEnd = 0;
	//int nReflen = 0;
	int npos1 = 0;// 自串头位置
	int npos2 = 0;// 自串尾巴位置
	int nReturn = 0;// 返回值 0:snp  1:indel -1:error
	string sATL="";
	string strVarType = "";
	npos2 = str.find("	",0);
	string strGet;
	if (npos2!= -1)
	{
		/********************************CHORM**********************************/
		try
		{
			npos1 = npos2+1;
		}
		catch (...)
		{
			cout<<" ERROR Line : "<<  str << endl;
			cout<<" /************/ Find CHORM ERROR "<< endl;
			return -1;
		}		
		/********************************POS*************************************/
		try
		{
			npos2 = str.find("	",npos1);
			//string sPos= str.substr(npos1,npos2-npos1);
			//strGet += sPos;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
		}
		catch (...)
		{
			cout<<" ERROR Line : "<<  str << endl;
			cout<<" /************/ Find POS ERROR "<< endl;
			return -1;
		}

		/********************************ID*************************************/
		try
		{
			npos2 = str.find("	",npos1);
		//	string sID= str.substr(npos1,npos2-npos1);
			//strGet += sID;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
		}
		catch (...)
		{
			cout<<" ERROR Line : "<<  str << endl;
			cout<<" /************/ Find ID ERROR "<< endl;
			return -1;
		}

		/********************************REF***************************************/

		try
		{
			npos2 = str.find("	",npos1);
			//nReflen = npos2 - npos1;
			//string sREF= str.substr(npos1,npos2-npos1);
			//if (npos2-npos1>=2)
			//	bNormal = true;
			//strGet += sREF;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
		}
		catch (...)
		{
			cout<<" ERROR Line : "<<  str << endl;
			cout<<" /************/ Find REF ERROR "<< endl;
			return -1;
		}

		/********************************ATL***************************************/

		try
		{
			npos2 = str.find("	",npos1);
			sATL= str.substr(npos1,npos2-npos1);
			/*if (npos2-npos1>=2)
			bNormal = true;*/
			//strGet += sATL;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
			nposEnd = npos1;// 不要pass
		}
		catch (...)
		{
			cout<<" ERROR Line : "<<  str << endl;
			cout<<" /************/ Find  ERROR "<< endl;
			return -1;
		}

		/********************************QUAL***************************************/
		try
		{
			npos2 = str.find("	",npos1);
			//string sQUAL= str.substr(npos1,npos2-npos1);
			//strGet += sQUAL;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
		}
		catch (...)
		{
			cout<<" ERROR Line : "<<  str << endl;
			cout<<" /************/ Find QUAL ERROR "<< endl;
			return -1;
		}

		/********************************FILTER***************************************/
		try
		{
			npos2 = str.find("	",npos1);
			string sFILTER= str.substr(npos1,npos2-npos1);
			if (npos2-npos1 != 4)
				bNormal = true;
			//strGet += sFILTER;
			//nposEnd = npos2 +1;   // 要pass字段
			npos1= npos2+1;
			//if ( "PASS" != sFILTER  )
			//	cout << sFILTER <<endl;
				
		}
		catch (...)
		{
			cout<<" ERROR Line : "<<  str << endl;
			cout<<" /************/ Find FILTER ERROR "<< endl;
			return -1;
		}
		if (str.find("VT=SNP",npos1) == -1)//
		{
			//bMarkSpecial(pSpecial,str);
			bSNP = false;
			npos2 = str.find("VT=",npos1);
			nReturn = 1;
			//return -1;
		}
		

		if (!bSNP)
			strVarType = str.substr(npos2,npos1-npos2);
		
			string ss = str;
			ss = ss.substr(0,nposEnd);

			if (!bSNP)
				ss += strVarType + " ";
			
			ss += "\n";
		  str = ss;
		}
	else //npos2!= -1 没有找到TAB 分隔符
	{
		//cout<<"ERROR Line : "<< str << endl;
		return -1;
	}
	return nReturn;
}


 static int n_PickInfo2(string &str)
{
	bool bNormal = false;
	bool bSNP = true;
	int nposEnd = 0;
	int npos1 = 0;// 自串头位置
	int npos2 = 0;// 自串尾巴位置
	int nReturn = 0;// 返回值 0:snp  1:indel -1:error
	
	npos2 = str.find("	",0);
	string strGet;
	if (npos2!= -1)
	{
		/********************************CHORM**********************************/
			npos1 = npos2+1;
		/********************************POS*************************************/

			npos2 = str.find("	",npos1);
			string sPos= str.substr(npos1,npos2-npos1);
			int  nPos = stoi(sPos)+1;
			sPos = Int2string(nPos);
			//strGet += sPos;
			strGet = "chr" + str.substr(0,npos2+1) + sPos + "\t";
			npos1= npos2+1;
		/********************************ID*************************************/
			npos2 = str.find("	",npos1);
	  	string sID= str.substr(npos1,npos2-npos1);
			strGet += sID;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
		/********************************REF***************************************/
			npos2 = str.find("	",npos1);
			//nReflen = npos2 - npos1;
			//string sREF= str.substr(npos1,npos2-npos1);
			//if (npos2-npos1>=2)
			//	bNormal = true;
			//strGet += sREF;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
		/********************************ATL***************************************/
			npos2 = str.find("	",npos1);
			//sATL= str.substr(npos1,npos2-npos1);
			/*if (npos2-npos1>=2)
			bNormal = true;*/
			//strGet += sATL;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
			nposEnd = npos1;// 不要pass
		/********************************QUAL***************************************/
			npos2 = str.find("	",npos1);
			//string sQUAL= str.substr(npos1,npos2-npos1);
			//strGet += sQUAL;
			//strGet = str.substr(npos1,npos2-npos1);
			npos1= npos2+1;
		/********************************FILTER***************************************/
			npos2 = str.find("	",npos1);
		//	string sFILTER= str.substr(npos1,npos2-npos1);
			if (npos2-npos1 != 4)
				bNormal = true;
			//strGet += sFILTER;
			//nposEnd = npos2 +1;   // 要pass字段
			npos1= npos2+1;
			//if ( "PASS" != sFILTER  )
			//cout << sFILTER <<endl;

		if (str.find("VT=SNP",npos1) == -1)//
		{
			nReturn = 1;
			return 1;
		}
		str = strGet + "\n";
		}
	else 
	{
		return -1;
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

/*bool Get_VCF37_SNP2(string sSrcfile,string sDespath,string &sError)
{
	bool bRe = true;
	sError = "";
	string sFilename = sSrcfile.substr(sSrcfile.rfind('/')+1,sSrcfile.length() - sSrcfile.rfind('/') -1 );
	cout << "sFilename = " << sFilename <<endl;

	string snp_file;
	snp_file = sFilename.substr(0,sFilename.rfind('.'));
	snp_file += "_SNP.dat";
	snp_file = sDespath + "/" +  snp_file;
	
	cout << "snp_file = " << snp_file <<endl;

	FILE *pFile;
	if ((pFile = fopen(snp_file.c_str(),"w+"))==NULL)
	{
		sError = "创建SNP.dat 文件失败 ";
		return false;
	}

	string indel_file = sFilename.substr(0,sFilename.rfind('.'));
	indel_file += "_INDEL.dat";
	indel_file = sDespath + "/" +  indel_file;
	
	cout << "indel_file = " << indel_file <<endl;

	FILE *pFSpecial;
	if ((pFSpecial = fopen(indel_file.c_str(),"w+"))==NULL)
	{
		sError = "创建INDEL.dat 文件失败 ";
		if (fclose (pFile) != 0)
			perror("Error occurs when close file"); //报告相应错误
		return false;
	}

	string head_file = sFilename.substr(0,sFilename.rfind('.'));
	head_file += "_Head.dat";
	head_file = sDespath + "/" +  head_file;
		
	cout << "head_file = " << head_file <<endl;

	FILE *pFHead;
	if ((pFHead = fopen(head_file.c_str(),"w+"))==NULL)
	{
		sError = "创建Head.dat 文件失败 ";
		if (fclose (pFile) != 0)
			perror("Error occurs when close file"); //报告相应错误
		if (fclose (pFSpecial) != 0)
			perror("Error occurs when close file"); //报告相应错误
		return false;
	}

	int  n_CountLine = 0;
	string str_Line;
	ifstream ifs;
	
		ifs.open(sSrcfile.c_str()) ;
		if (!ifs)
		{
			cout<<"Read File Error, please check file name is right: \n"<<sSrcfile <<endl;
			cin.get(); 
			if (fclose (pFile) != 0)
				perror("Error occurs when close file"); //报告相应错误
			if (fclose (pFSpecial) != 0)
				perror("Error occurs when close file"); //报告相应错误
			if (fclose (pFHead) != 0)
				perror("Error occurs when close file"); //报告相应错误

			sError = "读取文件出错 ";
			return false;
		}

	string strFront;// 存储 head
	while(!ifs.eof())  
	{  
		getline(ifs,str_Line);
		n_CountLine++;
		if (0 ==str_Line.find("##"))// 各种文件信息提示
		{
			strFront += str_Line +"\n";
			continue;
		}
		else if (0 ==str_Line.find("#"))
		{
			n_CountLine++;
			strFront += str_Line;
			bWritResult(pFHead,strFront);
		}
		else
		{	
			n_CountLine++;
			switch(n_PickInfo(str_Line))
			{
			case 0:
				bWritResult(pFile,str_Line);
				break;
			case 1:
				bWritSpecial(pFSpecial,str_Line);
				break;
			default:
				bWritResult(pFHead,str_Line);
				break;
			}
					
		}//处理正文
	}  //end while
	
	ifs.close();  
	if (fclose (pFile) != 0)
		perror("Error occurs when close file"); //报告相应错误
	if (fclose (pFSpecial) != 0)
		perror("Error occurs when close file"); //报告相应错误
	if (fclose (pFHead) != 0)
		perror("Error occurs when close file"); //报告相应错误

	return bRe;
}*/


bool getVcfBedFile(string sSrcfile,string sDespath,string &sError)
{
	bool bRe = true;
	sError = "";
	string sFilename = sSrcfile.substr(sSrcfile.rfind('/')+1,sSrcfile.length() - sSrcfile.rfind('/') -1 );
	//cout << "sFilename = " << sFilename <<endl;

	string snp_file;
	snp_file = sFilename.substr(0,sFilename.rfind('.'));
	snp_file += "_SNP.bed";
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
		if (0 ==str_Line.find("#")){continue;}
		else{	
			n_CountLine++;
			if(0 == n_PickInfo2(str_Line))
				bWritResult(pFile,str_Line);					
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
	bool bFolder = false;
  
  
   if(arg <2)
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
			 	if(sGet.find(".VCF")== -1 && sGet.find(".vcf")== -1)
			 		{
			 			cout << "Source file format error < expectd format is : \".vcf\"> "<< endl;
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
	}
	
	if(strDespath == ""){
			strDespath = "./";
	}
  
  Repalce_char(strSrc,"//","/");
  Repalce_char(strDespath,"//","/");
	
	cout << "Source file : " << strSrc << endl;
	cout << "Destination path :" << strDespath << endl;
  
  if(bFolder == false)
  	{
  		cout << strSrc << "\n";
  		getVcfBedFile(strSrc,strDespath,serr);
  	}
  else // folder mode
  	{
  		 vector<string> aLLFiles;
       string strFileName = "";
	     int nCount = getAllFiles(strSrc,aLLFiles);

			for(int i = 0;i<nCount;i++) // creat thread
			{
				strFileName = aLLFiles.at(i);
				Repalce_char(strFileName,"//","/");
				if(-1 == strFileName.find("vcf"))
					 continue;
				cout << strFileName << "\n";
				getVcfBedFile(strFileName,strDespath,serr);
		    
			}//end for
  		
  	}
  
  
	return 0;
}

