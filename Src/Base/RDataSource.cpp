
#include "Base/RDataSource.h"
#include <math.h>
#include <process.h>
#include "Base/FileType.h"

//////////////////////////////////////////////////////////////////////////
#include "curl.h"
//#include "types.h"
#include "easy.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "ogr_core.h"
#include <Base/RFileRaster.h>

struct MyHttpFile {
	char *filename;
	FILE *stream;
	long nfilesize;
};

int my_fwrite(void *buffer, int isize, int imemb, void *stream){
	struct MyHttpFile *out = (struct MyHttpFile* )stream;
	if(out){
		if(out->nfilesize>1000000){ // 限制大小，不能无限制传输
			return 0; 
		}
	}

	if(out && !out->stream){
		out->stream =  fopen(out->filename, "wb");
		if(!out->stream)
			return -1;
	}

	if(out){
		out->nfilesize += isize * imemb;
		return (int)fwrite(buffer, isize, imemb, out->stream);
	}
	else{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

RTileDownload::RTileDownload()
{
	m_iThreadCount=2;
	m_iDownloadCount=0;
	m_pWndHandler=0;
	m_pRRenderDataPreparedFun=NULL;
}



void RTileDownload::SendFinishedMsg()
{
	if(m_pRRenderDataPreparedFun != NULL)
	{
		m_pRRenderDataPreparedFun(m_pWndHandler);
	}
}

void RTileDownload::SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd)
{
	m_pRRenderDataPreparedFun = pRRenderDataPreparedFun;
	m_pWndHandler = pWnd;
}



int DownLoadFile(const string& strURL, const string& strLocalFile)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	CURLcode res; 
	CURL* curl = curl_easy_init();

	struct MyHttpFile httpFile = {	NULL,	NULL, 0};
	httpFile.filename = (char*) strLocalFile.c_str();

	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_easy_setopt(curl, CURLOPT_URL, strURL.c_str());

	// 可以无数级重定向
	curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1);
	curl_easy_setopt(curl,CURLOPT_MAXREDIRS,-1);

	// 链接服务端等待时间，若超时则放弃
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);	// 与CURLOPT_TIMEOUT不能同时开启，否则在release版本下连接服务端时会出现无法下载的问题
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	// 设置最低限速防止被挂起，10s内1 byte的数据都没有传输到
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 10L);

	/* Set a pointer to our struct to pass to the callback */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&httpFile);
	/* Define our callback to get called when there's data to be written */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);

	// begin download !
	res = curl_easy_perform(curl);

	if (res == CURLE_OK)
	{
		if(httpFile.stream)
			fclose(httpFile.stream); /* close the local file */

		double dt;
		res = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &dt);
	}
	else
	{
		//CString strTmp= "文件下载失败.";
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return (int)res;
}

unsigned int  __stdcall DownLoadThread(void *pParams)
{
	if(pParams!=NULL)
	{
		RTileDownload* pTileDownload = (RTileDownload*)pParams;
		if(pTileDownload == NULL)
			return 0;

		vector<string>* pArrFiles = pTileDownload->GetDownLoadFiles();
		vector<string>* pArrURLs = pTileDownload->GetDownLoadURLs();
		if(pArrFiles==NULL || pArrURLs==NULL)
			return 0;

		for(;;)
		{
			int iCur = pTileDownload->GetCurrDownloadID();
			if( iCur == pArrFiles->size())
			{
				pTileDownload->SendFinishedMsg();
				pTileDownload->AddDownloadID();
				return 0;
			}

			if(iCur>pArrFiles->size())
				return 0;

			string strURL, strLocalFile;
			strURL = pArrURLs->at(iCur);
			strLocalFile = pArrFiles->at(iCur);
			pTileDownload->AddDownloadID();

			DownLoadFile(strURL, strLocalFile);
		}
	}
}

void RTileDownload::Start()
{
	m_iDownloadCount=0;
	for(int i=0; i<m_iThreadCount; i++)
	{
		_beginthreadex(NULL, 0, &DownLoadThread, (void *)this, 0, NULL);
	}
}

void RTileDownload::SetDownLoadURLs(const vector<string>& arrURLs)
{
	m_arrDownLoadURLs.clear();
	for(int i=0; i<arrURLs.size(); i++)
	{
		m_arrDownLoadURLs.push_back(arrURLs[i]);
	}
}

void RTileDownload::SetDownLoadFiles(const vector<string>& arrFiles)
{
	m_arrDownLoadFiles.clear();
	for(int i=0; i<arrFiles.size(); i++)
	{
		m_arrDownLoadFiles.push_back(arrFiles[i]);
	}
}



//////////////////////////////////////////////////////////////////////////

RDataset::RDataset():
m_dLeft(-180.0),
m_dTop(90.0),
m_dRight(180.0),
m_dBottom(-90.0)
{
}

RDataset::~RDataset()
{
}

string GetServerName(const string& strType, int iLevel)
{
	string strServer = "A0512_EMap";
	if(2<=iLevel && iLevel<=10)
	{
		if(strcmp(strType.c_str(), TDT_ANNO) == 0)
		{
			strServer = "AB0512_Anno";
		}
		else if(strcmp(strType.c_str(), TDT_ANNOE) == 0)
		{
			strServer = "AB0106_AnnoE";
		}
	}
	else if(iLevel==11 || iLevel==12)
	{
		strServer = "B0627_EMap1112";	
	}
	else if(13<=iLevel && iLevel<=18)
	{
		strServer = "siwei0608";
	}
	return strServer;
}


string RDataset::GetFileName(int iLevel, const string& strType, int iRow, int iCol)
{
	string  strTmp = "/MapWorld/%s/L%d/X%dY%d.png";
	string strServer = GetServerName(strType, iLevel);

	char chTmp[256];
	memset(chTmp, '0', 256);
	sprintf(chTmp, strTmp.c_str(),  strServer.c_str(), iLevel, iCol, iRow);
	string strFileName = chTmp;
	return strFileName;
}

string RDataset::GetURLName(int iLevel, const string& strType, int iRow, int iCol)
{
	string strServer = GetServerName(strType, iLevel);
	string  strTmp = "http://tile%d.tianditu.com/DataServer?T=%s&X=%d&Y=%d&L=%d";

	//产生0～7之间的随机数
	srand((unsigned) time(0)); // 系统流逝时间作为初始种子
	int iServer = rand()%7;

	char chURL[256];
	memset(chURL, '0', 256);
	sprintf(chURL, strTmp.c_str(), iServer, strServer.c_str(), iCol, iRow, iLevel);
	string strURL = chURL;
	return strURL;
}


void RDataset::GetRowAndCol(double dLeft, double dTop, double dRight, double dBottom, int iLevel,
				  int& iRowStart, int& iRowEnd, int& iColStart, int& iColEnd, double& dTileWidth)
{
	if(iLevel<2)
		iLevel=2;

	if (iLevel>18)
	{
		iLevel=18;
	}

	int iPicNumsX = 1<<iLevel; // 当前级别水平方向的图片张数
	int iPicNumsY = iPicNumsX/2; // 当前级别垂直方向图片张数
	double dResolution = (m_dRight-m_dLeft) / (256*iPicNumsX);
	dTileWidth = dResolution*256; // 一张图片宽度

	int iColLeft = (int)floor((dLeft+180.0) / dTileWidth);
	int iColRight = (int)ceil((dRight +180.0)/ dTileWidth);
	int iRowBottom = (int)floor((dBottom-90.0) / dTileWidth);
	int iRowTop = (int)ceil((dTop-90.0) / dTileWidth);
	if(iColLeft>=iPicNumsX || iColRight<0 || iRowTop<=-iPicNumsY || iRowBottom>0)
		return ;

	if(iColLeft<0)
		iColLeft=0;
	if(iColRight>iPicNumsX)
		iColRight=iPicNumsX;
	if(iRowBottom<-iPicNumsY)
		iRowBottom=-iPicNumsY;
	if(iRowTop>0)
		iRowTop=0;

	iRowBottom = abs(iRowBottom);
	iRowTop = abs(iRowTop);

	iRowStart=iRowTop;
	iRowEnd=iRowBottom;
	iColStart=iColLeft;
	iColEnd=iColRight;
}


void RDataset::RequestTileData(double dLeft, double dTop, double dRight, double dBottom, int iLevel)
{
	int iRowTop(0), iRowBottom(0), iColLeft(0), iColRight(0);
	double dTileWidth;
	GetRowAndCol(dLeft, dTop, dRight, dBottom, iLevel, iRowTop, iRowBottom, iColLeft, iColRight, dTileWidth);
	double dResolution = dTileWidth/256;

	string strPath;// = "E:\\code\\RabbitMap\\Builds\\BinD\\Cache";
	char chCurrentDirectory[256];
	memset(chCurrentDirectory, 0, 256);
	GetCurrentDirectory(256, chCurrentDirectory);
	GetModuleFileName(NULL, chCurrentDirectory, 256);
	strPath = CRFile::GetDirName(chCurrentDirectory);;

	vector<string> arrFiles;
	vector<string> arrURLs;
	
	// 行列起始点在左上
	for(int iRow=iRowTop; iRow<iRowBottom; iRow++)
	{
		for(int iCol=iColLeft; iCol<iColRight; iCol++)
		{
			string strType = GetName();
			string strFileName = strPath + GetFileName(iLevel, strType, iRow, iCol);
			if(!CRFile::IsExist(strFileName)){
				string strURL = GetURLName(iLevel, GetName(), iRow, iCol);
				string strDir = CRFile::GetDirName(strFileName);
				//mkdir(strDir.c_str());
				CRFile::MkDir(strDir);
				arrFiles.push_back(strFileName);
				arrURLs.push_back(strURL);
				//DownLoadOneFile(strURL, strFileName);
			}
		}
	}

	if(arrURLs.size()>0 && arrFiles.size()>0 && arrFiles.size()==arrURLs.size())
	{
		m_TileDownLoader.SetDownLoadURLs(arrURLs);
		m_TileDownLoader.SetDownLoadFiles(arrFiles);
		m_TileDownLoader.Start();
	}

	// 搜集要显示的数据

}


void RDataset::GetViewIMGData(double dLeft, double dTop, double dRight, double dBottom, int iLevel,
					int iWidth, int iHeight, unsigned char*& pBuf)
{

	int iRowTop(0), iRowBottom(0), iColLeft(0), iColRight(0);
	double dTileWidth;
	GetRowAndCol(dLeft, dTop, dRight, dBottom, iLevel, iRowTop, iRowBottom, iColLeft, iColRight, dTileWidth);
	double dResolution = dTileWidth/256;

	string strPath;// = "E:\\code\\RabbitMap\\Builds\\BinD\\Cache";
	char chCurrentDirectory[256];
	memset(chCurrentDirectory, 0, 256);
	GetCurrentDirectory(256, chCurrentDirectory);
	GetModuleFileName(NULL, chCurrentDirectory, 256);
	strPath = CRFile::GetDirName(chCurrentDirectory);;

	OGREnvelope viewBound;
	viewBound.MinX = dLeft;
	viewBound.MaxX = dRight;
	viewBound.MinY = dBottom;
	viewBound.MaxY = dTop;


	vector<string> arrFiles;

	// 行列起始点在左上
	for(int iRow=iRowTop; iRow<iRowBottom; iRow++)
	{
		for(int iCol=iColLeft; iCol<iColRight; iCol++)
		{
			string strFileName = strPath + GetFileName(iLevel, GetName(), iRow, iCol);
			arrFiles.push_back(strFileName);
			if(CRFile::IsExist(strFileName))
			{
				// 从文件中拿到图片数据
				GDALDataset* pFileRaster = (GDALDataset *)GDALOpen(strFileName.c_str(), GA_ReadOnly);
				if(pFileRaster != NULL)
				{
					int iXSize = pFileRaster->GetRasterXSize();
					int iYSize = pFileRaster->GetRasterYSize();
					int iRasterCount = pFileRaster->GetRasterCount();

					GDALRasterBand* pRasterBand = pFileRaster->GetRasterBand(1);
					GDALDataType iDataType = pRasterBand->GetRasterDataType();

					OGREnvelope tileBound;
					tileBound.MinX = iCol*dTileWidth-180.0;
					tileBound.MaxY = -iRow*dTileWidth + 90.0;
					tileBound.MaxX = tileBound.MinX+dTileWidth;
					tileBound.MinY = tileBound.MaxY - dTileWidth;

					OGREnvelope copyBound;
					copyBound.Merge(tileBound);

					copyBound.Intersect(viewBound); // 瓦片范围与可显示地理范围相交叠的部分 

					// 根据相交范围，拼接影像数据
					int iOffsetXInView = (copyBound.MinX - dLeft) / dResolution; // 屏幕图像中位置X
					int iOffsetYInView = (dTop - copyBound.MaxY) / dResolution;

					int iOffsetXInFile = (copyBound.MinX - tileBound.MinX) / dResolution; // 瓦片中位置X
					int iOffsetYInFile = (tileBound.MaxY - copyBound.MaxY) / dResolution;
					int iCopyLines = (copyBound.MaxY-copyBound.MinY) / dResolution;
					int iCopyWidth = (copyBound.MaxX-copyBound.MinX) / dResolution;

					if(iRasterCount == 1)
					{
						if(iDataType == GDT_Byte)
						{
							GDALColorTable* pColorTable = pRasterBand->GetColorTable();
							bool bHasAlpha = false; // 是否有透明通道 
							int iAlphaIndex =-1; // 透明色在调色板中的索引
							for(int ia=0; ia<pColorTable->GetColorEntryCount(); ia++)
							{
								const GDALColorEntry* pColorEntry = pColorTable->GetColorEntry(ia);
								if(pColorEntry->c4 ==0)
								{
									bHasAlpha=true;
									iAlphaIndex = ia;
									break;
								}
							}

							unsigned char* pFileData = new unsigned char[iXSize*iYSize];
							pFileRaster->RasterIO(GF_Read, 0, 0, iXSize, iYSize, pFileData, iXSize, iYSize, GDT_Byte, 1, NULL, 0, 0, 0);

							int iBytesPerLineView = iWidth*3;
							if(iBytesPerLineView%4)
								iBytesPerLineView += 4-iBytesPerLineView%4;

							int iBytesPerLineTile = iXSize;

							for(int i=0; i<iCopyLines; i++)
							{
								for(int j=0; j<iCopyWidth; j++)
								{
									int iColorEntry = pFileData[(i+iOffsetYInFile)*iBytesPerLineTile+iOffsetXInFile+j];
									if(bHasAlpha && iColorEntry == iAlphaIndex) // 透明的颜色
										continue;
									const GDALColorEntry* pColorEntry = pColorTable->GetColorEntry(iColorEntry);
									int iOffsetView = (iOffsetYInView+i)*iBytesPerLineView+iOffsetXInView*3+3*j; // 屏幕像素偏移位置
									pBuf[iOffsetView] =pColorEntry->c1;
									pBuf[iOffsetView+1] = pColorEntry->c2;
									pBuf[iOffsetView+2] = pColorEntry->c3;
								}
							}

							delete [] pFileData;
						}

					}
					else if(iRasterCount == 3)
					{
						if(iDataType == GDT_Byte)
						{
							unsigned char* pFileDataR = new unsigned char[iXSize*iYSize];
							unsigned char* pFileDataG = new unsigned char[iXSize*iYSize];
							unsigned char* pFileDataB = new unsigned char[iXSize*iYSize];
							//int iBandMap[3] = {1, 2, 3};
							int iBytesPerLineView = iWidth*3;
							int iBytesPerLineTile = iXSize;

							//pFileRaster->RasterIO(GF_Read, 0, 0, iXSize, iYSize, pFileData, iXSize, iYSize, GDT_Byte, 3, iBandMap, 8, iBytesPerLineTile, 0);
							
							GDALRasterBand* pRasterBandR = pFileRaster->GetRasterBand(1);
							GDALRasterBand* pRasterBandG = pFileRaster->GetRasterBand(2);
							GDALRasterBand* pRasterBandB = pFileRaster->GetRasterBand(3);

							pRasterBandR->RasterIO(GF_Read, 0, 0, iXSize, iYSize, pFileDataR, iXSize, iYSize, GDT_Byte, 0, 0);
							pRasterBandG->RasterIO(GF_Read, 0, 0, iXSize, iYSize, pFileDataG, iXSize, iYSize, GDT_Byte, 0, 0);
							pRasterBandB->RasterIO(GF_Read, 0, 0, iXSize, iYSize, pFileDataB, iXSize, iYSize, GDT_Byte, 0, 0);


							for(int i=0; i<iCopyLines; i++)
							{
								for(int j=0; j<iCopyWidth; j++)
								{
									int iOffsetView = (iOffsetYInView+i)*iBytesPerLineView+iOffsetXInView*3+3*j; // 屏幕像素偏移位置
									int iOffsetFile = (i+iOffsetYInFile)*iBytesPerLineTile+iOffsetXInFile+j;
									pBuf[iOffsetView] = pFileDataR[iOffsetFile];
									pBuf[iOffsetView+1] = pFileDataG[iOffsetFile];
									pBuf[iOffsetView+2] = pFileDataB[iOffsetFile];
								}
							}

							delete [] pFileDataR;
							delete [] pFileDataG;
							delete [] pFileDataB;
						}

					}

					GDALClose(pFileRaster);
				}		
			}
		}
	}
}


void RDataset::GetRequestURL(double dLeft, double dTop, double dRight, double dBottom, int iLevel)
{
	if(iLevel<2)
		iLevel=2;

	if (iLevel>18)
	{
		iLevel=18;
	}

	int iPicNumsX = 1<<iLevel; // 当前层水平方向的图片张数
	int iPicNumsY = iPicNumsX/2; // 当前级别垂直方向图片张数
	double dResolution = (m_dRight-m_dLeft) / (256*iPicNumsX);
	double dTileWidth = dResolution*256; // 一张图片宽度
	int iColLeft = int((dLeft+180.0) / dTileWidth);
	int iColRight = int((dRight +180.0)/ dTileWidth);
	int iRowBottom = int((dBottom-90.0) / dTileWidth);
	int iRowTop = int((dTop-90.0) / dTileWidth);
	if(iColLeft>=iPicNumsX || iColRight<0 || iRowTop<=-iPicNumsY || iRowBottom>0)
		return;

	if(iColLeft<0)
		iColLeft=0;
	if(iColRight>iPicNumsX)
		iColRight=iPicNumsX;
	if(iRowBottom<-iPicNumsY)
		iRowBottom=-iPicNumsY;
	if(iRowTop>0)
		iRowTop=0;

	iRowBottom = abs(iRowBottom);
	iRowTop = abs(iRowTop);

	vector<string> arrURLs;
	// 行列起始点在左上
	for(int iRow=iRowTop; iRow<iRowBottom; iRow++)
	{
		for(int iCol=iColLeft; iCol<iColRight; iCol++)
		{
			string  strTmp = "http://tile1.tianditu.com/DataServer?T=A0512_EMap&X=%d&Y=%d&L=%d";
			char chURL[256];
			memset(chURL, '0', 256);
			sprintf(chURL, strTmp.c_str(), iCol, iRow, iLevel);
			string strURL = chURL;
			arrURLs.push_back(strURL);
		}
	}

	DownLoadTiles(arrURLs, "E:\\code\\RabbitMap\\Builds\\BinD");
}


void RDataset::DownLoadTiles(const vector<string>& arrURLs, const string& strPath)
{
	string strSavePath = strPath;
	

	curl_global_init(CURL_GLOBAL_DEFAULT);

	CURLcode res; 
	CURL* curl = curl_easy_init();

	for(int i=0; i<arrURLs.size(); i++)
	{
		struct MyHttpFile httpFile = {	NULL,	NULL, 0};

		string strURL = arrURLs[i];
		char chFileName[9];// = "%d.png";
		memset(chFileName, 0, 9);
		sprintf(chFileName, "\\%d.png", i);
		string strFileName = chFileName;
		string strLocalFile = strSavePath + strFileName;


		httpFile.filename = (char*) strLocalFile.c_str();

		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		curl_easy_setopt(curl, CURLOPT_URL, strURL.c_str());

		// 可以无数级重定向
		curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1);
		curl_easy_setopt(curl,CURLOPT_MAXREDIRS,-1);

		// 链接服务端等待时间，若超时则放弃
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);	// 与CURLOPT_TIMEOUT不能同时开启，否则在release版本下连接服务端时会出现无法下载的问题
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		// 设置最低限速防止被挂起，10s内1 byte的数据都没有传输到
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 10L);

		/* Set a pointer to our struct to pass to the callback */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&httpFile);
		/* Define our callback to get called when there's data to be written */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);

		// begin download !
		res = curl_easy_perform(curl);

		if (res == CURLE_OK)
		{
			if(httpFile.stream)
				fclose(httpFile.stream); /* close the local file */

			double dt;
			res = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &dt);
		}
		else
		{
			//CString strTmp= "文件下载失败.";
		}
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

void RDataset::SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd)
{
	m_TileDownLoader.SetRRenderDataPreparedFun(pRRenderDataPreparedFun, pWnd);
}

int RDataset::DownLoadOneFile(const string& strURL, const string& strLocalFile)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	CURLcode res; 
	CURL* curl = curl_easy_init();

	struct MyHttpFile httpFile = {	NULL,	NULL, 0};
	httpFile.filename = (char*) strLocalFile.c_str();

	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_easy_setopt(curl, CURLOPT_URL, strURL.c_str());

	// 可以无数级重定向
	curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1);
	curl_easy_setopt(curl,CURLOPT_MAXREDIRS,-1);

	// 链接服务端等待时间，若超时则放弃
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);	// 与CURLOPT_TIMEOUT不能同时开启，否则在release版本下连接服务端时会出现无法下载的问题
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	// 设置最低限速防止被挂起，10s内1 byte的数据都没有传输到
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 10L);

	/* Set a pointer to our struct to pass to the callback */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&httpFile);
	/* Define our callback to get called when there's data to be written */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);

	// begin download !
	res = curl_easy_perform(curl);

	if (res == CURLE_OK)
	{
		if(httpFile.stream)
			fclose(httpFile.stream); /* close the local file */

		double dt;
		res = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &dt);
	}
	else
	{
		//CString strTmp= "文件下载失败.";
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return (int)res;
}



/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

RDataSource::RDataSource()
{
	m_Type=RDataSource::RDsNone;
	GDALAllRegister();
	OGRRegisterAll();	
}

RDataSource::~RDataSource()
{
	for(int i=0; i<m_arrDatasets.size(); ++i)
	{
		RDataset* pDataset = m_arrDatasets[i];
		if(pDataset)
		{
			delete pDataset;
			pDataset=NULL;
		}
	}

	m_arrDatasets.clear();
}

int RDataSource::GetDatasetCount() const
{
	return m_arrDatasets.size();
}

RDataset* RDataSource::GetDataset(int iDt) const 
{
	int iCount = GetDatasetCount();
	if(iDt<iCount){
		return m_arrDatasets[iDt];
	}
	return NULL;
}

RDataset* RDataSource::GetDataset(const string& strName) const 
{
	for(int i=0; i<m_arrDatasets.size(); i++)
	{
		RDataset* pDataset = m_arrDatasets[i];
		if(pDataset!=NULL && strcmp(strName.c_str(), pDataset->GetName().c_str())==0)
		{
			return pDataset;
		}
	}

	return NULL;

}


void RDataSource::AddDataset(RDataset* pDataset)
{
	if(pDataset != NULL)
	{
		m_arrDatasets.push_back(pDataset);
	}

}


//////////////////////////////////////////////////////////////////////////

RDataSourceVector::RDataSourceVector()
{
	SetType(RDsFileVector);
	m_poDs=NULL;
}

RDataSourceVector::~RDataSourceVector()
{

}

bool RDataSourceVector::Open(const string& strName)
{
	m_poDs = OGRSFDriverRegistrar::Open(strName.c_str());
	if(m_poDs!=NULL)
	{
		SetName(strName);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

RDataSourceRaster::RDataSourceRaster()
{
	SetType(RDsFileRaster);
	m_pFileRaster=NULL;
}

RDataSourceRaster::~RDataSourceRaster()
{

}

bool RDataSourceRaster::Open(const string& strName)
{
	m_pFileRaster = new RFileRaster;		
	if(m_pFileRaster->Open(strName))
	{
		string strDtName = CRFile::GetTitleNameFromPath(strName);
		SetName(strName);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// 天地图
RDataSourceMapWorld::RDataSourceMapWorld()
{
	SetType(RDataSource::RDsWeb);
	SetName("天地图");
	RDataset* pDataset = new RDataset;
	pDataset->SetName(TDT_EMAP);
	AddDataset(pDataset);


	pDataset = new RDataset;
	pDataset->SetName(TDT_ANNO);
	AddDataset(pDataset);

	pDataset = new RDataset;
	pDataset->SetName(TDT_ANNOE);
	AddDataset(pDataset);
}

RDataSourceMapWorld::~RDataSourceMapWorld()
{

}

//////////////////////////////////////////////////////////////////////////

RWorkspace::RWorkspace()
{

}

RWorkspace::~RWorkspace()
{
	Close();
}

void RWorkspace::AddDataSource(RDataSource* pDataSource)
{
	bool bFind=false;
	for(int i=0; i<m_pDataSources.size(); i++)
	{
		RDataSource* pDs = m_pDataSources[i];
		string strName1 = pDataSource->GetName();
		string strName2 = pDs->GetName();
		if(strcmp(strName1.c_str(), strName2.c_str()) == 0)
		{
			bFind=true;
			break;
		}
	}

	if(!bFind)
	{
		m_pDataSources.push_back(pDataSource);
	}
}


// 工作空间中新增数据源
RDataSource* RWorkspace::AddDataSource(const string& strFileName)
{
	CRFile::FClass fcType = CRFile::GetClassByName(strFileName);
	if(fcType == CRFile::fcRaster){

		RDataSourceRaster *pDs = new RDataSourceRaster;
		if(pDs->Open(strFileName))
		{
			AddDataSource(pDs);
			return pDs;
		}
	}
	else if (fcType == CRFile::fcVector)	
	{
		RDataSourceVector *pDs = new RDataSourceVector;
		if(pDs->Open(strFileName))
		{
			AddDataSource(pDs);
			return pDs;
		}
	}
	return NULL;
}

void RWorkspace::RemoveDataSource(const string& strDsName)
{
	for(vector<RDataSource*>::iterator itertmp=m_pDataSources.begin(); itertmp!=m_pDataSources.end(); itertmp++)
	{
		RDataSource* pDs = *itertmp;

		if(strcmp(strDsName.c_str(), pDs->GetName().c_str())==0)
		{
			m_pDataSources.erase(itertmp);
			break;
		}
	}

}


// 通过名称查找数据源
RDataSource* RWorkspace::GetDataSource(const string& strDsName)
{
	for(int i=0; i<m_pDataSources.size(); i++)
	{
		RDataSource* pDs = m_pDataSources[i];
		if(strcmp(strDsName.c_str(), pDs->GetName().c_str())==0)
		{
			return pDs;
		}
	}

	return NULL;
}

// 关闭所有数据源
void RWorkspace::Close()
{
	for(int i=0; i<m_pDataSources.size(); i++)
	{
		RDataSource* pDs = m_pDataSources[i];
		if(pDs!=NULL)
		{
			delete pDs;
			pDs=NULL;
		}
	}

	m_pDataSources.clear();
}

//////////////////////////////////////////////////////////////////////////

