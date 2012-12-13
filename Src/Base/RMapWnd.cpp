
#include "Base/RMapWnd.h"
#include "gdal.h"
#include <gdal_priv.h>

#include "ogrsf_frmts.h"

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgb.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_ellipse.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_conv_contour.h"
#include "agg_conv_stroke.h"
#include "agg_pixfmt_gray.h"
#include "agg_scanline_p.h"
#include "agg_span_allocator.h"
#include "agg_span_image_filter_rgb.h"
#include "agg_image_accessors.h"
#include "agg_conv_transform.h"
#include "agg_path_storage.h"
#include "agg_span_image_filter_gray.h"

//#include "Desktop/RMapInit.h"
#include "Base/RFileVector.h"
#include "Base/FileType.h"
#include "Base/RDataSource.h"
#include "Base/RLog.h"
#include <process.h>

//////////////////////////////////////////////////////////////////////////

RLayer::RLayer() 
{
	m_pDataSource=NULL;
	m_pDrawParams=NULL;
	m_pMap=NULL;

}

RLayer::RLayer(RDataSource* pDataSource)
{
	if(pDataSource!=NULL)
		m_pDataSource=pDataSource;
	m_pDrawParams=NULL;
	m_pMap=NULL;
}

RLayer::~RLayer()
{
}

void RLayer::AddStyle(const string& strKey, const string& strValue)
{
	m_layStyles[strKey] = strValue;
}


int calc_bytes_per_line(int iw, int ibpp){
	int ibytes = iw * ibpp / 8;
	if(ibytes%4)
		ibytes += 4-ibytes%4;

	return ibytes;
}

void RLayer::Draw(int iwidth, int iheight, unsigned char *& buf)
{
//	if(GetType() == int(RLyrVector))
	{

		int ilayCount;// = m_pfileVector->GetLayerCount();
		for(int ilay=0; ilay<ilayCount; ++ilay){
			if(m_pDrawParams->GetCancel()) // 有取消消息，当然不继续绘制了
				break;

			OGRLayer* player;// = m_pfileVector->GetLayer(ilay);
			OGRFeature *pfeature=NULL;
			player->ResetReading();

			RStyle layStyle;
			map<string, string>::const_iterator iterstyle = m_layStyles.find("color");
			if(iterstyle != m_layStyles.end()){
				string strColor = iterstyle->second;
				layStyle.m_iColor = atoi(strColor.c_str());
			}			

			RDrawFactory drawer;
			drawer.Init(iwidth, iheight, buf);

			vector<OGRGeometry*> arrGeometry;
			vector<OGRFeature*> arrFeature;

			OGREnvelope extView;// 地图可显示范围
			OGREnvelope geometryView; // 对象范围
			m_pDrawParams->GetViewBound(&extView.MinX, &extView.MaxY, &extView.MaxX, &extView.MinY);
			player->SetSpatialFilterRect(extView.MinX, extView.MinY, extView.MaxX, extView.MaxY); // 按范围过滤查询

			while ( (pfeature=player->GetNextFeature()) != NULL)		{
				if(m_pDrawParams->GetCancel()) {// 有取消消息，当然不继续绘制了
					OGRFeature::DestroyFeature( pfeature );
					break;
				}

				OGRGeometry *poGeometry = pfeature->GetGeometryRef();
				//drawer.DrawGeometry(m_pDrawParams, poGeometry, &layStyle);		 
				//OGRFeature::DestroyFeature( pfeature );
				poGeometry->getEnvelope(&geometryView);
				if(extView.Intersects(geometryView)){
					arrGeometry.push_back(poGeometry);
					arrFeature.push_back(pfeature);
				}
				else{
					OGRFeature::DestroyFeature( pfeature );
				}
			}

			int iCount = arrGeometry.size();
			drawer.DrawGeometry(m_pDrawParams, &arrGeometry[0], arrGeometry.size(), &layStyle);
			
			for(int i=0; i<arrFeature.size(); ++i){ // 释放内存
				OGRFeature* pFeature = arrFeature[i];
				OGRFeature::DestroyFeature(pFeature);
			}
			arrGeometry.clear();
			arrFeature.clear();
		}

		if(m_pDrawParams->GetCancel()) // 本次取消绘制后，别影响下次绘制
			m_pDrawParams->SetCancel(false);

	}
	//else if(GetType() == int(RLyrRaster))
	{
		

		GDALDataset* pDataset;// = m_pfileRaster->GetDataset();
		if(pDataset==NULL)
			return;

		OGREnvelope viewBound;
		m_pDrawParams->GetViewBound(&viewBound.MinX, &viewBound.MaxY, &viewBound.MaxX, &viewBound.MinY);

		OGREnvelope imgBound;
	//	m_pfileRaster->GetBound(&imgBound.MinX, &imgBound.MaxY, &imgBound.MaxX, &imgBound.MinY);

		if(!viewBound.Intersects(imgBound)){ // 地图范围与图层范围无相交不显示
			return;
		}

		int iCount = pDataset->GetRasterCount();
		if(iCount >= 3){
			OGREnvelope interBound;
			interBound.Merge(viewBound);
			interBound.Intersect(imgBound); // 地图范围与图层范围相交的范围

			//根据相交范围，计算图像的可显示范围（图像坐标）
			int iWidth = pDataset->GetRasterXSize();
			int iHeight = pDataset->GetRasterYSize();
			double dRatiox = (imgBound.MaxX-imgBound.MinX)/iWidth;
			double dRatioy = (imgBound.MaxY-imgBound.MinY)/iHeight;

			int ileft = interBound.MinX / dRatiox; // 可显示的地理范围，对应的图像坐标范围
			int iright = interBound.MaxX / dRatiox;
			int itop = interBound.MaxY / dRatioy;
			int ibottom = interBound.MinY / dRatioy;

			double dleft, dtop, dright, dbottom; // 可显示范围对应的 设备坐标范围
			m_pDrawParams->MPtoDP(interBound.MinX, interBound.MaxY, &dleft, &dtop);
			m_pDrawParams->MPtoDP(interBound.MaxX, interBound.MinY, &dright, &dbottom);

			// 图层可显示的宽度与高度
			int iViewWidth = dright-dleft;
			int iViewHeight = fabs(dtop-dbottom);

			//////////////////////////////////////////////////////////////////////////
			// 进度信息
			int iBegin=0;
			int iEnd = iViewHeight;
			int ibandList[3] = {1,2,3};
			unsigned char* pData = new unsigned char[iViewWidth*iViewHeight*3];
			if(pDataset->RasterIO(GF_Read, ileft, iHeight-itop, iright-ileft, itop-ibottom, 
				(void*)pData, iViewWidth, iViewHeight, GDT_Byte, 
				3, ibandList, 0, 0, 0) == CE_None){

					int iBytesPerLine = calc_bytes_per_line(iwidth, 24); // 显示窗口一行字节数
					int iOffX = dleft; //(interBound.MinX-viewBound.MinX)/(viewBound.MaxX-viewBound.MinX)*iwidth;
					int iOffy = dtop;//(viewBound.MaxY-interBound.MaxY)/(viewBound.MaxY-viewBound.MinY)*iheight;
					//iOffX = calc_bytes_per_line(iOffX, 24);
					iOffX *= 3; // 三个波段,因此水平方向位置乘3
					iOffy = iOffy*iBytesPerLine;

					for(int i=0; i<iViewHeight; i++){
						for(int j=0; j<iViewWidth; j++){
							buf[iOffy+iBytesPerLine*i+iOffX+3*j] = pData[iViewWidth*i+j];//第一波段
							buf[iOffy+iBytesPerLine*i+iOffX+3*j+1] = pData[iViewHeight*iViewWidth+iViewWidth*i+j];//第二波段
							buf[iOffy+iBytesPerLine*i+iOffX+3*j+2] = pData[iViewHeight*iViewWidth*2+iViewWidth*i+j];// 第三波段
						}					
					}
			}
			delete [] pData;
		}
		else if(iCount==1){
			OGREnvelope interBound;
			interBound.Merge(viewBound);
			interBound.Intersect(imgBound); // 地图范围与图层范围相交的范围

			//根据相交范围，计算图像的可显示范围（图像坐标）
			int iWidth = pDataset->GetRasterXSize();
			int iHeight = pDataset->GetRasterYSize();
			double dRatiox = (imgBound.MaxX-imgBound.MinX)/iWidth;
			double dRatioy = (imgBound.MaxY-imgBound.MinY)/iHeight;

			int ileft = interBound.MinX / dRatiox; // 可显示的地理范围，对应的图像坐标范围
			int iright = interBound.MaxX / dRatiox;
			int itop = interBound.MaxY / dRatioy;
			int ibottom = interBound.MinY / dRatioy;
			if(ibottom<0){
				itop = itop-ibottom;
				ibottom = 0;
			}

			double dleft, dtop, dright, dbottom; // 可显示范围对应的 设备坐标范围
			m_pDrawParams->MPtoDP(interBound.MinX, interBound.MaxY, &dleft, &dtop);
			m_pDrawParams->MPtoDP(interBound.MaxX, interBound.MinY, &dright, &dbottom);

			// 图层可显示的宽度与高度
			int iViewWidth = dright-dleft;
			int iViewHeight = fabs(dtop-dbottom);

			int ibandList[1] = {1};
			unsigned char* pData = new unsigned char[iViewWidth*iViewHeight];
			if(pDataset->RasterIO(GF_Read, ileft, iHeight-itop, iright-ileft, itop-ibottom, 
				(void*)pData, iViewWidth, iViewHeight, GDT_Byte, 
				1, ibandList, 0, 0, 0) == CE_None){

					int iBytesPerLine = calc_bytes_per_line(iwidth, 24); // 显示窗口一行字节数
					int iOffX = dleft; //(interBound.MinX-viewBound.MinX)/(viewBound.MaxX-viewBound.MinX)*iwidth;
					int iOffy = dtop;//(viewBound.MaxY-interBound.MaxY)/(viewBound.MaxY-viewBound.MinY)*iheight;
					//iOffX = calc_bytes_per_line(iOffX, 24);
					iOffX *= 3; // 三个波段,因此水平方向位置乘3
					iOffy = iOffy*iBytesPerLine;

					// 调色板处理
					GDALRasterBand * pRasterBand = pDataset->GetRasterBand(1);
					GDALColorTable* pColorTables = pRasterBand->GetColorTable();
					if(pColorTables!=NULL){ // 索引色
						int iColorCount = pColorTables->GetColorEntryCount();

						for(int i=0; i<iViewHeight; i++){
							for(int j=0; j<iViewWidth; j++){
								int index = pData[iViewWidth*i+j];
								if(index<iColorCount){
									const GDALColorEntry* colorEntry = pColorTables->GetColorEntry(index);

									buf[iOffy+iBytesPerLine*i+iOffX+3*j] = colorEntry->c1;
									buf[iOffy+iBytesPerLine*i+iOffX+3*j+1] = colorEntry->c2;
									buf[iOffy+iBytesPerLine*i+iOffX+3*j+2] = colorEntry->c3;

								}
							}
						}
					}
					else{ // 灰度图
						for(int i=0; i<iViewHeight; i++){
							for(int j=0; j<iViewWidth; j++){
								buf[iOffy+iBytesPerLine*i+iOffX+3*j] = pData[iViewWidth*i+j];//第一波段
								buf[iOffy+iBytesPerLine*i+iOffX+3*j+1] = pData[iViewWidth*i+j];//第一波段
								buf[iOffy+iBytesPerLine*i+iOffX+3*j+2] = pData[iViewWidth*i+j];//第一波段
							}
						}
					}
					delete [] pData;
			}
		}
	}
}

void RLayer::Draw(OGRLayer* pLayer, int iWidth, int iHeight, unsigned char *& pBuf)
{
	if(false) // 多线程方式
	{
		m_layerOnDrawBackground.SetOGRLayer(pLayer);
		m_layerOnDrawBackground.SetDeviceContext(iWidth, iHeight, pBuf);
		m_layerOnDrawBackground.SetDrawParams(m_pDrawParams);
		m_layerOnDrawBackground.StartDrawBackground();
		unsigned char* pDrawedData = m_layerOnDrawBackground.GetDrawData() ;
		if(pDrawedData != NULL)
		{
			int iBytesPerLine = iWidth*3; // RGB一行字节数
			if (iBytesPerLine%4)
				iBytesPerLine += 4-iBytesPerLine%4;
			memcpy(pBuf, pDrawedData, iBytesPerLine*iHeight);
		}
	}
	else
	{
		clock_t t1=clock();
		pLayer->ResetReading();
		RStyle layStyle;

		/*map<string, string>::const_iterator iterstyle = m_layStyles.find("color");
		if(iterstyle != m_layStyles.end()){
		string strColor = iterstyle->second;
		layStyle.m_iColor = atoi(strColor.c_str());
		}			
		*/

		OGREnvelope extView;// 地图可显示范围
		OGREnvelope geometryView; // 对象范围
		
		m_pDrawParams->GetViewBound(&extView.MinX, &extView.MaxY, &extView.MaxX, &extView.MinY);
		pLayer->SetSpatialFilterRect(extView.MinX, extView.MinY, extView.MaxX, extView.MaxY); // 按范围过滤查询

		RDrawFactory drawFactory;
		drawFactory.Init(iWidth, iHeight, pBuf);

		// 进度信息 
		int iNum=0;

		OGRFeature* pFeature=NULL;
		while ( (pFeature = pLayer->GetNextFeature()) != NULL)		
		{
			if(m_pDrawParams->GetCancel()) 
			{// 有取消消息，当然不继续绘制了
				OGRFeature::DestroyFeature( pFeature );
				break;
			}

			OGRGeometry *poGeometry = pFeature->GetGeometryRef();
			poGeometry->getEnvelope(&geometryView);
			if(extView.Intersects(geometryView))
			{
				drawFactory.DrawGeometry(m_pDrawParams, &poGeometry, 1, &layStyle);
			}

			OGRFeature::DestroyFeature( pFeature );

			if(iNum%50 == 0)
			{
				RMap* pMap = GetMap();
				if(pMap!=NULL)
				{
					MapStatusCallBackFun pFunc = pMap->GetMapStatusCallBackFun();
					unsigned long pHandle = pMap->GetMapStatusHandle();
					if(pFunc!=NULL && pHandle!=NULL)
					{
						char chTmp[256];
						chTmp[255] = '\0';
						sprintf_s(chTmp, "已处理[%d]个对象.", iNum);
						string strMsg = chTmp;
						pFunc(pHandle, strMsg);
					}
				}				
			}
			iNum++;

		}

		clock_t t2=clock();
		double dCostTime = (double)(t2-t1)/CLOCKS_PER_SEC; // 秒
		char chTmp[256];
		chTmp[255]='\0';

		sprintf_s(chTmp, "绘制图层[%s]耗时[%2.1f]秒.", pLayer->GetName(), dCostTime);

		RLog::Print(RLog::Info, chTmp);

	}
}

void RLayer::Draw(RFileRaster* pFileRaster, int iWidth, int iHeight, unsigned char*& pBuf)
{
	OGREnvelope viewBound;
	m_pDrawParams->GetViewBound(&viewBound.MinX, &viewBound.MaxY, &viewBound.MaxX, &viewBound.MinY);

	OGREnvelope imgBound;
	pFileRaster->GetBound(&imgBound.MinX, &imgBound.MaxY, &imgBound.MaxX, &imgBound.MinY);
	if(!viewBound.Intersects(imgBound)){ // 地图范围与图层范围无相交不显示
		return;
	}

	GDALDataset* pDataset = pFileRaster->GetDataset();
	int iCount = pDataset->GetRasterCount();
	if(iCount >= 3){
		OGREnvelope interBound;
		interBound.Merge(viewBound);
		interBound.Intersect(imgBound); // 地图范围与图层范围相交的范围

		//根据相交范围，计算图像的可显示范围（图像坐标）
		int iIMGWidth = pDataset->GetRasterXSize();
		int iIMGHeight = pDataset->GetRasterYSize();
		double dRatiox = (imgBound.MaxX-imgBound.MinX)/iIMGWidth;
		double dRatioy = (imgBound.MaxY-imgBound.MinY)/iIMGHeight;

		int ileft = (interBound.MinX-imgBound.MinX) / dRatiox; // 可显示的地理范围，对应的图像坐标范围
		int iright = (interBound.MaxX -imgBound.MinX) / dRatiox;
		int itop = (imgBound.MaxY-interBound.MaxY) / dRatioy;
		int ibottom = (imgBound.MaxY-interBound.MinY) / dRatioy;

		double dleft, dtop, dright, dbottom; // 可显示范围对应的 设备坐标范围
		m_pDrawParams->MPtoDP(interBound.MinX, interBound.MaxY, &dleft, &dtop);
		m_pDrawParams->MPtoDP(interBound.MaxX, interBound.MinY, &dright, &dbottom);

		// 图层可显示的宽度与高度
		int iViewWidth = dright-dleft;
		int iViewHeight = fabs(dtop-dbottom);

		int ibandList[3] = {1,2,3};
		unsigned char* pData = new unsigned char[iViewWidth*iViewHeight*3];
		if(pDataset->RasterIO(GF_Read, ileft, itop, iright-ileft, ibottom-itop, 
			(void*)pData, iViewWidth, iViewHeight, GDT_Byte, 
			3, ibandList, 0, 0, 0) == CE_None)
		{
			int iBytesPerLine = calc_bytes_per_line(iWidth, 24); // 显示窗口一行字节数
			int iOffX = dleft; //(interBound.MinX-viewBound.MinX)/(viewBound.MaxX-viewBound.MinX)*iwidth;
			int iOffy = dtop;//(viewBound.MaxY-interBound.MaxY)/(viewBound.MaxY-viewBound.MinY)*iheight;
			//iOffX = calc_bytes_per_line(iOffX, 24);
			iOffX *= 3; // 三个波段,因此水平方向位置乘3
			iOffy = iOffy*iBytesPerLine;

			for(int i=0; i<iViewHeight; i++){
				for(int j=0; j<iViewWidth; j++){
					pBuf[iOffy+iBytesPerLine*i+iOffX+3*j] = pData[iViewWidth*i+j];//第一波段
					pBuf[iOffy+iBytesPerLine*i+iOffX+3*j+1] = pData[iViewHeight*iViewWidth+iViewWidth*i+j];//第二波段
					pBuf[iOffy+iBytesPerLine*i+iOffX+3*j+2] = pData[iViewHeight*iViewWidth*2+iViewWidth*i+j];// 第三波段
				}					
			}
		}
		delete [] pData;
	}
	else if(iCount==1)
	{
		OGREnvelope interBound;
		interBound.Merge(viewBound);
		interBound.Intersect(imgBound); // 地图范围与图层范围相交的范围

		//根据相交范围，计算图像的可显示范围（图像坐标）
		int iIMGWidth = pDataset->GetRasterXSize();
		int iIMGHeight = pDataset->GetRasterYSize();
		double dRatiox = (imgBound.MaxX-imgBound.MinX)/iIMGWidth;
		double dRatioy = (imgBound.MaxY-imgBound.MinY)/iIMGHeight;

		int ileft = interBound.MinX / dRatiox; // 可显示的地理范围，对应的图像坐标范围
		int iright = interBound.MaxX / dRatiox;
		int itop = interBound.MaxY / dRatioy;
		int ibottom = interBound.MinY / dRatioy;
		if(ibottom<0){
			itop = itop-ibottom;
			ibottom = 0;
		}

		double dleft, dtop, dright, dbottom; // 可显示范围对应的 设备坐标范围
		m_pDrawParams->MPtoDP(interBound.MinX, interBound.MaxY, &dleft, &dtop);
		m_pDrawParams->MPtoDP(interBound.MaxX, interBound.MinY, &dright, &dbottom);

		// 图层可显示的宽度与高度
		int iViewWidth = dright-dleft;
		int iViewHeight = fabs(dtop-dbottom);

		int ibandList[1] = {1};
		unsigned char* pData = new unsigned char[iViewWidth*iViewHeight];
		if(pDataset->RasterIO(GF_Read, ileft, iIMGHeight-itop, iright-ileft, itop-ibottom, 
			(void*)pData, iViewWidth, iViewHeight, GDT_Byte, 
			1, ibandList, 0, 0, 0) == CE_None)
		{

				int iBytesPerLine = calc_bytes_per_line(iWidth, 24); // 显示窗口一行字节数
				int iOffX = dleft; //(interBound.MinX-viewBound.MinX)/(viewBound.MaxX-viewBound.MinX)*iwidth;
				int iOffy = dtop;//(viewBound.MaxY-interBound.MaxY)/(viewBound.MaxY-viewBound.MinY)*iheight;
				//iOffX = calc_bytes_per_line(iOffX, 24);
				iOffX *= 3; // 三个波段,因此水平方向位置乘3
				iOffy = iOffy*iBytesPerLine;

				// 调色板处理
				GDALRasterBand * pRasterBand = pDataset->GetRasterBand(1);
				GDALColorTable* pColorTables = pRasterBand->GetColorTable();
				if(pColorTables!=NULL)
				{ // 索引色
					int iColorCount = pColorTables->GetColorEntryCount();

					for(int i=0; i<iViewHeight; i++){
						for(int j=0; j<iViewWidth; j++){
							int index = pData[iViewWidth*i+j];
							if(index<iColorCount){
								const GDALColorEntry* colorEntry = pColorTables->GetColorEntry(index);

								pBuf[iOffy+iBytesPerLine*i+iOffX+3*j] = colorEntry->c1;
								pBuf[iOffy+iBytesPerLine*i+iOffX+3*j+1] = colorEntry->c2;
								pBuf[iOffy+iBytesPerLine*i+iOffX+3*j+2] = colorEntry->c3;

							}
						}
					}
				}
				else
				{ // 灰度图
					for(int i=0; i<iViewHeight; i++){
						for(int j=0; j<iViewWidth; j++){
							pBuf[iOffy+iBytesPerLine*i+iOffX+3*j] = pData[iViewWidth*i+j];//第一波段
							pBuf[iOffy+iBytesPerLine*i+iOffX+3*j+1] = pData[iViewWidth*i+j];//第一波段
							pBuf[iOffy+iBytesPerLine*i+iOffX+3*j+2] = pData[iViewWidth*i+j];//第一波段
						}
					}
				}
				delete [] pData;
		}
	}


}

void RLayer::Draw(RDataset* pDataset, int iWidth, int iHeight, unsigned char*& pBuf)
{
	double dLeft, dTop, dRight, dBottom;
	m_pDrawParams->GetViewBound(&dLeft, &dTop, &dRight, &dBottom);


	double d2xPIxR = 20037508.342789244*2; // 地球赤道周长
	double dRatio = m_pDrawParams->GetRatio(); // 坐标转换比率： 设备坐标/地理坐标（逻辑坐标）
	double dResolution = (256*4) / 360.0; // 像素坐标/地理坐标
	int iLevel  = dRatio / dResolution;

	// 求出2的N次方中的N值
	double x = log(double(2));
	double y = log(double(iLevel));
	int n=int(y/x);

	iLevel = 2+n;
	pDataset->RequestTileData(dLeft, dTop, dRight, dBottom, iLevel);
	pDataset->GetViewIMGData(dLeft, dTop, dRight, dBottom, iLevel, iWidth, iHeight, pBuf);
	

	/***********************************************************************

	地球周长 2*PI*R = 20037508.342789244*2 (km)

	***********************************************************************/


}






void load_image_rgb24(RFileRaster* pfile, unsigned char*& buf)
{
	int iw=pfile->GetWidth();
	int ih=pfile->GetHeight();
	int iBytesPerLine = calc_bytes_per_line(iw, 24);

	unsigned char* pdata = buf;//new unsigned char[iBytesPerLine * ih];

	int ibandcount = 3;//pfile->GetBandCount();
	unsigned char** pband_bits = new unsigned char*[ibandcount];
	for(int ib=0; ib<ibandcount;++ib)
		pband_bits[ib] = new unsigned char[iw * ih];

	for (int ib=0; ib<ibandcount; ++ib){
		pfile->get_all_data(ib+1, pband_bits[ib]);
	}

	for(int ib=0; ib<ibandcount; ++ib){
		for(int irow=0; irow<ih; ++irow)
			for(int icol=0; icol<iw; ++icol){
				pdata[irow*iBytesPerLine + icol*ibandcount +ib] = pband_bits[ib][irow*iw + icol];
			}
	}

	for(int ib=0; ib<ibandcount; ++ib){
		delete [] pband_bits[ib];
	}
}

int RLayer::DrawRGB24(int iw, int ih, unsigned char*& buf)
{

	int ibytes_per_line = calc_bytes_per_line(iw, 24); // RGB一行字节数
	int iimg_width;// = m_pfileRaster->GetWidth();
	int iimg_height;// = m_pfileRaster->GetHeight();
	int iimg_bytes_per_line = calc_bytes_per_line(iimg_width, 24);

	// 屏幕位图是底图
	unsigned char* pdst = buf;
	unsigned char* pimgbuf = new unsigned char[iimg_bytes_per_line*iimg_height];
//	load_image_rgb24(m_pfileRaster, pimgbuf);

	agg::rendering_buffer rbuf_img(pimgbuf, iimg_width, iimg_height,  iimg_bytes_per_line); // 图像内存
	agg::rendering_buffer rbuf_win(pdst, iw, ih,  ibytes_per_line); // 屏幕位图内存

	typedef agg::pixfmt_bgr24 pixfmt;
	pixfmt pixf(rbuf_win);
	pixfmt pixf_img(rbuf_img);
	agg::renderer_base<pixfmt> renb(pixf);
	//renb.clear(agg::rgba(1,1,1)); // 设置背景白色

	agg::trans_affine mtx = m_pDrawParams->get_mtx(iimg_width, iimg_height); // 构建图像的变换矩阵	
	agg::trans_affine img_mtx;
	img_mtx *= mtx;
	img_mtx.invert(); // 此矩阵需要倒转一下

	agg::span_allocator< agg::rgba8> sa;

	// 线性插值器，处理图像缩放
	typedef agg::span_interpolator_linear<> interpolator_type;
	interpolator_type interpolator(img_mtx);

	//typedef agg::image_accessor_clip<pixfmt> img_source_type;
	//img_source_type img_src(pixf_img, agg::rgba8(0, 255, 255));

	typedef agg::span_image_filter_rgb_bilinear_clip<pixfmt,	interpolator_type> span_gen_type;
	span_gen_type sg(pixf_img, agg::rgba_pre(1, 1, 1,1), interpolator);

	agg::rasterizer_scanline_aa<> ras;
	ras.clip_box(0, 0, iw, ih); // 按照屏幕大小设置裁剪矩形，避免缩放过程消耗大量内存

	agg::scanline_u8 sl;

	agg::path_storage path_img_box; // 按照图像尺寸设置图像边界矩形
	path_img_box.move_to(0,0);
	path_img_box.line_to(iimg_width, 0);
	path_img_box.line_to(iimg_width, iimg_height);
	path_img_box.line_to(0, iimg_height);
	//path_img_box.line_to(0,0);

	//agg::conv_stroke<agg::path_storage> stroke(path_test);
	agg::trans_affine img_box_mtx;
	img_box_mtx *= mtx; // 内存图像变换后，图像的边框需要跟着变换

	agg::conv_transform<agg::path_storage> ct(path_img_box, img_box_mtx);	
	ras.add_path(ct);
	agg::render_scanlines_aa(ras, sl, renb, sa, sg);
	//agg::render_scanlines_bin(ras, sl, renb, sa, sg);

	delete [] pimgbuf;
	pimgbuf=NULL;

	return 1;
}


//////////////////////////////////////////////////////////////////////////

RLayers::~RLayers()
{
	for(int i=0; i<size(); ++i){
		RLayer* layer = (*this)[i];
		delete layer;
	}

	clear();
}

void RLayers::GetBound(double* dleft, double* dtop, double* dright, double* dbottom) const
{
	OGREnvelope bound;
	for(int i=0; i<size(); ++i){
		OGREnvelope exttmp;
		RLayer* layer = (*this)[i];
// 		if(layer)
// 			layer->GetBound(&exttmp.MinX, &exttmp.MaxY, &exttmp.MaxX, &exttmp.MinY);

		bound.Merge(exttmp);
	}

	*dleft = bound.MinX;
	*dtop = bound.MaxY;
	*dright = bound.MaxX;
	*dbottom = bound.MinY;
}

RLayer* RLayers::GetLayerByName(const string& strName)
{
	for(int i=0; i<size(); ++i){
		RLayer* layer = (*this)[i];
		if(layer !=NULL && layer->GetName()==strName){
			return layer;
		}
	}
}


//////////////////////////////////////////////////////////////////////////


unsigned int  __stdcall OGRLayerOnDrawThread(void *pParams)
{
	if(pParams==NULL)
		return 0;
	ROGRLayerOnDrawBackground* pOnDrawBackground = (ROGRLayerOnDrawBackground*)pParams;

	OGRFeature *pfeature=NULL;
	OGRLayer* pLayer = pOnDrawBackground->GetOGRLayer();
	pLayer->ResetReading();

	RStyle layStyle;
	/*map<string, string>::const_iterator iterstyle = m_layStyles.find("color");
	if(iterstyle != m_layStyles.end()){
		string strColor = iterstyle->second;
		layStyle.m_iColor = atoi(strColor.c_str());
	}			*/

	int iWidth(0);
	int iHeight(0);
	unsigned char* pBuf=NULL;
	pOnDrawBackground->GetDeviceContext(iWidth, iHeight, pBuf);
	pOnDrawBackground->GetDrawFactory()->Init(iWidth, iHeight, pBuf);

	vector<OGRGeometry*> arrGeometry;
	vector<OGRFeature*> arrFeature;

	OGREnvelope extView;// 地图可显示范围
	OGREnvelope geometryView; // 对象范围
	RDrawParms* pDrawParams=pOnDrawBackground->GetDrawParams();
	pDrawParams->GetViewBound(&extView.MinX, &extView.MaxY, &extView.MaxX, &extView.MinY);
	pLayer->SetSpatialFilterRect(extView.MinX, extView.MinY, extView.MaxX, extView.MaxY); // 按范围过滤查询

	while ( (pfeature = pLayer->GetNextFeature()) != NULL)		{

		if(pDrawParams->GetCancel()) {// 有取消消息，当然不继续绘制了
			OGRFeature::DestroyFeature( pfeature );
			break;
		}

		OGRGeometry *poGeometry = pfeature->GetGeometryRef();
		//drawer.DrawGeometry(m_pDrawParams, poGeometry, &layStyle);		 
		//OGRFeature::DestroyFeature( pfeature );
		poGeometry->getEnvelope(&geometryView);
		if(extView.Intersects(geometryView)){
			arrGeometry.push_back(poGeometry);
			arrFeature.push_back(pfeature);
		}
		else{
			OGRFeature::DestroyFeature( pfeature );
		}
	}

	int iCount = arrGeometry.size();
	pOnDrawBackground->GetDrawFactory()->DrawGeometry(pDrawParams, &arrGeometry[0], arrGeometry.size(), &layStyle);

	for(int i=0; i<arrFeature.size(); ++i){ // 释放内存
		OGRFeature* pFeature = arrFeature[i];
		OGRFeature::DestroyFeature(pFeature);
	}
	arrGeometry.clear();
	arrFeature.clear();

	pOnDrawBackground->SendFinishedMsg();
}

//////////////////////////////////////////////////////////////////////////

ROGRLayerOnDrawBackground::ROGRLayerOnDrawBackground()
{
	m_iWinWidth=m_iWinHeight=0;
	m_pLayer=NULL;
	m_pBuf=NULL;
	m_pDrawParams=NULL;
	m_pRenderDataPreparedFun=NULL;
	m_pWndHander=0;
	m_dLeftPer=m_dTopPer= m_dRightPer= m_dBottomPer=0;
}
ROGRLayerOnDrawBackground::~ROGRLayerOnDrawBackground()
{
	if(m_pBuf != NULL)
	{
		delete [] m_pBuf;
		m_pBuf=NULL;
	}

}

void ROGRLayerOnDrawBackground::SetOGRLayer(OGRLayer* pLayer)
{
	m_pLayer=pLayer;
}
void ROGRLayerOnDrawBackground::SetDrawParams(RDrawParms* pDrawParams)
{
	m_pDrawParams=pDrawParams;
}
void ROGRLayerOnDrawBackground::SetDeviceContext(int idcWidth, int idcHeight, unsigned char* pBuf)
{
	m_iWinHeight=idcHeight;
	m_iWinWidth=idcWidth;
	int iBytesPerLine = m_iWinWidth*3;
	if (iBytesPerLine%4)
		iBytesPerLine += 4-iBytesPerLine%4;

	if(m_pBuf!=NULL)
	{
		delete [] m_pBuf;
		m_pBuf=NULL;
	}

	m_pBuf=new unsigned char[iBytesPerLine * m_iWinHeight];
	memcpy(m_pBuf, pBuf, iBytesPerLine*m_iWinHeight);
}

void ROGRLayerOnDrawBackground::GetDeviceContext(int& iWidth, int& iHeight, unsigned char*& pBuf)
{
	iWidth=m_iWinWidth;
	iHeight = m_iWinHeight;

	pBuf=m_pBuf;
}

void ROGRLayerOnDrawBackground::StartDrawBackground()
{
	double dLeft, dTop, dRight, dBottom;
	m_pDrawParams->GetViewBound(&dLeft, &dTop, &dRight, &dBottom);

	if(!RIS0(m_dLeftPer-dLeft) || !RIS0(m_dTopPer-dTop) || !RIS0(m_dRightPer-dRight) || !RIS0(m_dBottomPer-dBottom))
		_beginthreadex(NULL, 0, &OGRLayerOnDrawThread, (void *)this, 0, NULL);
}

void ROGRLayerOnDrawBackground::SendFinishedMsg()
{
	double dLeft, dTop, dRight, dBottom;
	m_pDrawParams->GetViewBound(&dLeft, &dTop, &dRight, &dBottom);

	m_dLeftPer=dLeft;
	m_dTopPer=dTop;
	m_dRightPer=dRight;
	m_dBottomPer=dBottom;

	if(m_pRenderDataPreparedFun!=NULL)
		m_pRenderDataPreparedFun(m_pWndHander);

}
void ROGRLayerOnDrawBackground::SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd)
{
	m_pRenderDataPreparedFun=pRRenderDataPreparedFun;
	m_pWndHander=pWnd;
}

//////////////////////////////////////////////////////////////////////////


RTrackingLayer::RTrackingLayer()
{

}

RTrackingLayer::~RTrackingLayer()
{
	RemoveAll();
}

void RTrackingLayer::AddGeometry(OGRGeometry* pGeometry, const RStyle& geomStyle)
{
	if(pGeometry!=NULL)
	{
		OGRGeometry* pGeom = pGeometry->clone();
		m_arrGeometrys.push_back(pGeom);
		m_arrStyles.push_back(geomStyle);
	}

}

void RTrackingLayer::RemoveAll()
{
	int i=0;
	for(i=0; i<m_arrGeometrys.size(); i++)
	{
		OGRGeometry* pGeom = m_arrGeometrys[i];
		if(pGeom)
		{
			delete pGeom;
			pGeom = NULL;
		}
	}

	m_arrGeometrys.clear();
	m_arrStyles.clear();

}

//////////////////////////////////////////////////////////////////////////

RMap::RMap() : m_iwidth(0),
m_iheight(0),
m_bRedraw(false),
m_pBuf(NULL),
m_pBufCache2(NULL)
{
	m_pMainWnd=NULL;
	m_bFirstOnDraw = true;
	m_pRRenderDataPreparedFun=NULL;
	m_pWndHandler=0;
	m_pDataSource=NULL;

	m_pMapStatusCallBackFun=NULL;
	m_pMapStatusHandle=NULL;

}

RMap::~RMap()
{
	if(m_pBuf!=NULL){
		delete [] m_pBuf;
		m_pBuf=NULL;
	}

	if(m_pBufCache2!=NULL){
		delete [] m_pBufCache2;
		m_pBufCache2=NULL;
	}
}


void RMap::Pan(int idx, int idy)
{
	m_drawPrams.Pan(idx, idy);
	SetRedrawFlag();
}

void RMap::ZoomIn(int x, int y)
{
	m_drawPrams.Zoom(x, y, 2);
	SetRedrawFlag();
}

void RMap::ZoomOut(int x, int y)
{
	m_drawPrams.Zoom(x, y, 0.5);
	SetRedrawFlag();
}

void RMap::ZoomEntire()
{
	m_drawPrams.ZoomEntire();
	SetRedrawFlag();
}


void RMap::ZoomRect(int ileft, int itop, int iright, int ibottom, bool bin/*=true*/)
{
	m_drawPrams.ZoomRect(ileft, itop, iright, ibottom, bin);
	SetRedrawFlag();
}

// 移动屏幕过程，把之前的二级缓存贴出来就中了
void RMap::MouseMove(int idx, int idy)//; // 参数是偏移量
{
	if(m_pBuf==NULL || m_pBufCache2==NULL)
		return;

	//if(abs(idx)<4 || abs(idy)<4){ // 如果连4个像素都不够，就不要移屏操作
	//	return;
	//}

	int ibytesPerLine = calc_bytes_per_line(m_iwidth, 24);
	int ioffsetInLine = abs(idx)*3;//  calc_bytes_per_line(abs(idx), 24); //解决闪屏问题
	int icopyed = ibytesPerLine-ioffsetInLine;
	memset(m_pBuf, 205, ibytesPerLine*m_iheight);

	if(idx>0){
		if(idy>0){ // 向左、向下移动
			for(int i=0; (i+idy)<m_iheight; ++i)
				memcpy(m_pBuf+(i+idy)*ibytesPerLine+ioffsetInLine, m_pBufCache2+i*ibytesPerLine, icopyed );
		}
		else{ // 向左、向上移动
			idy = abs(idy);
			for(int i=0; (i+idy)<m_iheight; ++i)
				memcpy(m_pBuf+i*ibytesPerLine+ioffsetInLine, m_pBufCache2+(idy+i)*ibytesPerLine, icopyed );
		}
	}
	else{
		if(idy>0){ // 向右、向下移动
			for(int i=0; (i+idy)<m_iheight; ++i)
				memcpy(m_pBuf+(i+idy)*ibytesPerLine, m_pBufCache2+i*ibytesPerLine+ioffsetInLine, icopyed );
		}
		else{ // 向右、向上移动
			idy = abs(idy);
			for(int i=0; (i+idy)<m_iheight; ++i)
				memcpy(m_pBuf+i*ibytesPerLine, m_pBufCache2+(idy+i)*ibytesPerLine+ioffsetInLine, icopyed );
		}
	}		
}


void RMap::AddLayer(RLayer* pLayer)
{
	string strName=pLayer->GetName();
	bool bFind=false;
	for(int i=0; i<m_arrLayers.size(); i++)
	{
		RLayer* pLay = m_arrLayers.at(i);
		string strLayer = pLay->GetName();
		if(strcmp(strName.c_str(), strLayer.c_str())==0)
		{
			bFind=true;
			break;	
		}
	}

	if(!bFind)
	{
		pLayer->SetMap(this); // 关联地图 
		m_arrLayers.push_back(pLayer);		
	}

}


bool isbig_image(RFileRaster* pfile){

	int imgw=pfile->GetWidth();
	int imgh=pfile->GetHeight();
	int ibc = pfile->GetBandCount();
	__int64 size =0;
	if(pfile->GetDataType()==GDT_Byte){
		size = __int64(imgw) * __int64(imgh) * ibc ;
	}
	else if(pfile->GetDataType()==GDT_UInt16 || pfile->GetDataType()==GDT_Int16){
		size = __int64(imgw) * __int64(imgh) * ibc * 16 / 8;
	}
	else{

	}

	if (size< 200*1024*1024){
		return false;
	}

	return true;
}

// 是否使用分块显示模式，开放给用户配置
bool use_cell_model(){
	//RMapInit init;
	//return init.get_drawbyblock();
	return false;
}

void RMap::Resize(int iw, int ih)
{
	m_iwidth=iw; 
	m_iheight=ih;
	m_drawPrams.ChangeDP(iw, ih);

}

void RMap::DrawOnFirstTime(int idcWidth, int idcHeight)
{
	RLayers* arrLayers = GetLayers();
	int iCount = arrLayers->size();

	OGREnvelope mapBound;
	for(int ilay=0; ilay<iCount; ++ilay)
	{
		RLayer* layer = (*arrLayers)[ilay];
		if(layer != NULL)
		{
			if(m_pDataSource->GetType() == RDataSource::RDsFileVector)
			{
				RDataSourceVector* pDsVector=(RDataSourceVector *)m_pDataSource;
				OGRDataSource* poDs = pDsVector->GetOGRDataSource();
				int ilayCount = poDs->GetLayerCount();
				string strLayerName = layer->GetName();
				for(int ilayer=0; ilayer<ilayCount; ++ilayer)
				{
					if(strcmp(strLayerName.c_str(), poDs->GetLayer(ilayer)->GetName())==0)
					{
						OGREnvelope layBound;
						poDs->GetLayer(ilayer)->GetExtent(&layBound);
						mapBound.Merge(layBound);
						m_drawPrams.Init(0, 0, idcWidth, idcHeight, mapBound.MinX, mapBound.MaxY, mapBound.MaxX, mapBound.MinY);
						break;;
					}
				}
			}

			/*
			string strLayerName = layer->GetName();
			string strDsName = layer->GetDataSource()->GetName();
//			int iType = layer->GetType();
//			if(iType == RLayer::RLyrWeb)
			{
				RDataSource* pDataSource = (RDataSource* )layer->GetDataSource();
				if(strcmp(pDataSource->GetName().c_str(), strDsName.c_str()) ==0)
				{
					OGREnvelope webDsBound;
					webDsBound.MinX=-180;
					webDsBound.MaxY=90;
					webDsBound.MaxX=180;
					webDsBound.MinY=-90;
					mapBound.Merge(webDsBound);
					double dRatio = (256*4) / (mapBound.MaxX-mapBound.MinX) ;
					m_drawPrams.SetCustomRatio(dRatio);
					m_drawPrams.Init(0, 0, idcWidth, idcHeight, mapBound.MinX, mapBound.MaxY, mapBound.MaxX, mapBound.MinY);
					break;
				}
			}
//			else if(iType == RLayer::RLyrRaster)
			{
				RFileRaster* pFileRaster = (RFileRaster*)layer->GetDataSource();
				string strName = pFileRaster->GetName();
				if(strcmp(strName.c_str(), strDsName.c_str()) == 0)
				{
					OGREnvelope layBound;
					pFileRaster->GetBound(&layBound.MinX, &layBound.MaxY, &layBound.MaxX, &layBound.MinY);
					mapBound.Merge(layBound);
					m_drawPrams.Init(0, 0, idcWidth, idcHeight, mapBound.MinX, mapBound.MaxY, mapBound.MaxX, mapBound.MinY);
					m_drawPrams.ZoomResolution(pFileRaster->GetWidth(), pFileRaster->GetHeight());
					break;
				}
			}*/
		
		}
	}
}


void RMap::Draw(int iw, int ih, unsigned char*& buf)
{
	if(m_iwidth!=iw || m_iheight!=ih)
	{
		Resize(iw, ih);
		SetRedrawFlag(true);
	}

	int ibytes_per_line=calc_bytes_per_line(iw, 24);
	if(!GetRedrawFlag())
	{
		memcpy(buf, m_pBuf, ibytes_per_line*ih);
		return;
	}

	RLayers* arrLayers = GetLayers();
	int iCount = arrLayers->size();

	if(m_bFirstOnDraw)
	{ // 初次显示地图，需初始化地图范围
		DrawOnFirstTime(iw, ih);
		m_bFirstOnDraw=false;
	}
	
	for(int ilay=0; ilay<iCount; ++ilay)
	{
		RLayer* layer = (*arrLayers)[ilay];
		if(layer != NULL)
		{
			layer->SetDrawParams(&m_drawPrams);
			RDataSource* pDs = layer->GetDataSource();
			if(pDs->GetType() == RDataSource::RDsFileVector)
			{
				RDataSourceVector* pDsVector = (RDataSourceVector*)pDs;
				OGRDataSource* poDs = pDsVector->GetOGRDataSource();
				int ilayCount = poDs->GetLayerCount();
				string strLayerName = layer->GetName();
				for(int ilay=0; ilay<ilayCount; ++ilay)
				{
					OGRLayer* pLayer = poDs->GetLayer(ilay);
					if(strcmp(strLayerName.c_str(), pLayer->GetName()) == 0)
					{
						layer->Draw(pLayer, iw, ih, buf);
					}
				}
			}


			/*
			layer->SetDrawParams(&m_drawPrams);
			//if(layer->GetType() == RLayer::RLyrVector)
			{
				OGRLayer* pLayer = GetLayerVector(layer->GetDataSource()->GetName(), layer->GetName());
				layer->SetRRenderDataPreparedFun(m_pRRenderDataPreparedFun, m_pWndHandler);
				layer->Draw(pLayer, iw, ih, buf);
			}
			//else if(layer->GetType() == RLayer::RLyrRaster)
			{
				RFileRaster* pFileRaster = GetLayerRaster(layer->GetDataSource()->GetName(), layer->GetName());
				layer->Draw(pFileRaster, iw, ih, buf);
			}
//			else if(layer->GetType() == RLayer::RLyrWeb)
			{
				RDataSource *pDataSource = (RDataSource *)layer->GetDataSource();
				RDataset* pDataset = pDataSource->GetDataset(layer->GetName());
				pDataset->SetRRenderDataPreparedFun(m_pRRenderDataPreparedFun, m_pWndHandler);
				layer->Draw(pDataset, iw, ih, buf);
			}
		*/
		}
	}

	if(m_pBuf!=NULL)
	{
		delete [] m_pBuf;
	}
	
	if(m_pBufCache2!=NULL)
	{
		delete [] m_pBufCache2;
	}

	m_pBuf = new unsigned char[ibytes_per_line*ih];
	m_pBufCache2 = new unsigned char[ibytes_per_line*ih];
	memcpy(m_pBuf, buf, ibytes_per_line*ih);
	memcpy(m_pBufCache2, buf, ibytes_per_line*ih);

	// 跟踪层对象绘制
	//DrawTrackingLayer(iw, ih);

	SetRedrawFlag(false);
}

OGRLayer* RMap::GetLayerVector(const string& strDsName, const string& strLayerName)
{
	if(m_pMainWnd==NULL)
		return NULL;

// 	int iDsCount = m_pMainWnd->m_arrDataSources.size();
// 	for(int i=0; i<iDsCount; ++i){
// 		OGRDataSource* pDs = m_pMainWnd->m_arrDataSources[i];
// 		if(strcmp(strDsName.c_str(), pDs->GetName()) == 0)
// 		{
// 			
// 		}
// 	}

	return NULL;
}

RFileRaster* RMap::GetLayerRaster(const string& strDsName, const string& strLayerName)
{
	if(m_pMainWnd==NULL)
		return NULL;

// 	int iDsCount = m_pMainWnd->m_arrFileRasters.size();
// 	for(int i=0; i<iDsCount; ++i){
// 		RFileRaster* pDs = m_pMainWnd->m_arrFileRasters[i];
// 		if(strcmp(strDsName.c_str(), pDs->GetName().c_str()) == 0)
// 		{
// 			return pDs;
// 		}
// 	}

	return NULL;
}

RDataset* RMap::GetLayerWeb(const string& strDsName, const string& strLayerName)
{
	if(m_pMainWnd==NULL)
		return NULL;

// 	int iDsCount = m_pMainWnd->m_arrDataSourceWebs.size();
// 	for(int i=0; i<iDsCount; ++i){
// 		RDataSource* pDs = m_pMainWnd->m_arrDataSourceWebs[i];
// 		if(strcmp(strDsName.c_str(), pDs->GetName().c_str()) == 0)
// 		{
// 			for(int idt=0; idt<pDs->GetDatasetCount(); idt++)
// 			{
// 				RDataset* pDtWeb = pDs->GetDataset(idt);
// 				if(strcmp(pDtWeb->GetName().c_str(), strLayerName.c_str()) == 0)
// 					return pDtWeb;
// 			}
// 		}
// 	}

	return NULL;
}


void RMap::DrawWhenZoom(int iw, int ih, int iPosx, int iPosy, bool bZoomIn/*=true*/)
{
	if(m_pBuf==NULL || m_pBufCache2==NULL)
		return;

	if(iPosx<0 || iPosx>iw || iPosy<0 || iPosy>ih)
		return;

	int iBytesPerLine = calc_bytes_per_line(iw, 24); // RGB一行字节数
	unsigned char* pdst = m_pBuf;
	memset(pdst, 205, iBytesPerLine*ih);
	agg::rendering_buffer rbuf_img(m_pBufCache2, iw, ih,  iBytesPerLine); // 缓存位图
	agg::rendering_buffer rbuf_win(pdst, iw, ih,  iBytesPerLine); // 屏幕位图内存

	typedef agg::pixfmt_bgr24 pixfmt;
	pixfmt pixf(rbuf_win);
	pixfmt pixf_img(rbuf_img);
	agg::renderer_base<pixfmt> renb(pixf);
	//renb.clear(agg::rgba(1,1,1)); // 设置背景白色

	agg::trans_affine src_mtx; // 缩放操作
	src_mtx *= agg::trans_affine_translation(-iPosx, -iPosy);
	if(bZoomIn)
		src_mtx *= agg::trans_affine_scaling(2.0);
	else
		src_mtx *= agg::trans_affine_scaling(0.5);

	src_mtx *= agg::trans_affine_translation(iPosx, iPosy);
	agg::trans_affine img_mtx =	src_mtx;
	img_mtx.invert(); // 此矩阵需要倒转一下

	agg::span_allocator< agg::rgba8> sa;

	// 线性插值器，处理图像缩放
	typedef agg::span_interpolator_linear<> interpolator_type;
	interpolator_type interpolator(img_mtx);

	//typedef agg::image_accessor_clip<pixfmt> img_source_type;
	//img_source_type img_src(pixf_img, agg::rgba8(0, 255, 255));

	typedef agg::span_image_filter_rgb_bilinear_clip<pixfmt,	interpolator_type> span_gen_type;
	span_gen_type sg(pixf_img, agg::rgba_pre(1, 1, 1,1), interpolator);

	agg::rasterizer_scanline_aa<> ras;
	ras.clip_box(0, 0, iw, ih); // 按照屏幕大小设置裁剪矩形，避免缩放过程消耗大量内存

	agg::scanline_u8 sl;

	agg::path_storage path_img_box; // 按照图像尺寸设置图像边界矩形
	path_img_box.move_to(0,0);
	path_img_box.line_to(iw, 0);
	path_img_box.line_to(iw, ih);
	path_img_box.line_to(0, ih);
	path_img_box.line_to(0,0);

	//agg::conv_stroke<agg::path_storage> stroke(path_test);
	agg::conv_transform<agg::path_storage> ct(path_img_box, src_mtx);	
	ras.add_path(ct);
	agg::render_scanlines_aa(ras, sl, renb, sa, sg);
}

void RMap::DrawWhenZoomRect(int iw, int ih, int ileft, int itop, int iright, int ibottom)
{
	if(m_pBuf==NULL || m_pBufCache2==NULL)
		return;

	int iBytesPerLine = calc_bytes_per_line(iw, 24); // RGB一行字节数
	unsigned char* pdst = m_pBuf;
	agg::rendering_buffer rbuf_img(m_pBufCache2, iw, ih,  iBytesPerLine); // 缓存位图
	agg::rendering_buffer rbuf_win(pdst, iw, ih,  iBytesPerLine); // 屏幕位图内存

	typedef agg::pixfmt_bgr24 pixfmt;
	pixfmt pixf(rbuf_win);
	pixfmt pixf_img(rbuf_img);
	agg::renderer_base<pixfmt> renb(pixf);
	//renb.clear(agg::rgba(1,1,1)); // 设置背景白色

	int iPosx = ileft + (iright-ileft)*0.5;
	int iPosy = ibottom + (itop-ibottom)*0.5;
	double dScaleX = (iright-ileft)/iw;
	double dScaleY = (itop-ibottom)/ih;
	double dScale = dScaleX>dScaleY ? dScaleX : dScaleY;

	agg::trans_affine src_mtx; // 缩放操作
	src_mtx *= agg::trans_affine_translation(-iPosx, -iPosy);
	src_mtx *= agg::trans_affine_scaling(dScale);

	src_mtx *= agg::trans_affine_translation(iPosx, iPosy);
	agg::trans_affine img_mtx =	src_mtx;
	img_mtx.invert(); // 此矩阵需要倒转一下

	agg::span_allocator< agg::rgba8> sa;

	// 线性插值器，处理图像缩放
	typedef agg::span_interpolator_linear<> interpolator_type;
	interpolator_type interpolator(img_mtx);

	//typedef agg::image_accessor_clip<pixfmt> img_source_type;
	//img_source_type img_src(pixf_img, agg::rgba8(0, 255, 255));

	typedef agg::span_image_filter_rgb_bilinear_clip<pixfmt,	interpolator_type> span_gen_type;
	span_gen_type sg(pixf_img, agg::rgba_pre(1, 1, 1,1), interpolator);

	agg::rasterizer_scanline_aa<> ras;
	ras.clip_box(0, 0, iw, ih); // 按照屏幕大小设置裁剪矩形，避免缩放过程消耗大量内存

	agg::scanline_u8 sl;

	agg::path_storage path_img_box; // 按照图像尺寸设置图像边界矩形
	path_img_box.move_to(0,0);
	path_img_box.line_to(iw, 0);
	path_img_box.line_to(iw, ih);
	path_img_box.line_to(0, ih);
	path_img_box.line_to(0,0);

	//agg::conv_stroke<agg::path_storage> stroke(path_test);
	agg::conv_transform<agg::path_storage> ct(path_img_box, src_mtx);	
	ras.add_path(ct);
	agg::render_scanlines_aa(ras, sl, renb, sa, sg);
}

void RMap::DrawTrackingLayer(int iWinWidth, int iWinHeight)
{
	if(m_pBuf==NULL || m_pBufCache2==NULL)
		return;

	RTrackingLayer* pLayer = GetTrackingLayer();
	RDrawFactory drawer;
	drawer.Init(iWinWidth, iWinHeight, m_pBuf);
	int iBytesPerLine = calc_bytes_per_line(iWinWidth, 24);
	memcpy(m_pBuf, m_pBufCache2, iBytesPerLine*iWinHeight); // 先用缓存，这样可清楚上次跟踪层上显示的对象

	for (int i=0; i<pLayer->GetGeometryCount(); i++)
	{
		OGRGeometry* pGeometry = pLayer->m_arrGeometrys[i];
		drawer.DrawGeometry(&m_drawPrams, &pGeometry, 1, &pLayer->m_arrStyles[i]);
	}
}

void RMap::SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd)
{
	m_pWndHandler = pWnd ; // 窗口指针
	m_pRRenderDataPreparedFun = pRRenderDataPreparedFun;
}


int RMap::draw_rgb24(int iw, int ih, unsigned char*& buf)
{
	int ibytes_per_line = calc_bytes_per_line(iw, 24); // RGB一行字节数
	int iimg_width;// = m_pfiler->GetWidth();
	int iimg_height;// = m_pfiler->GetHeight();
	int iimg_bytes_per_line = calc_bytes_per_line(iimg_width, 24);

	// 屏幕位图是底图
	unsigned char* pdst = buf;
	unsigned char* pimgbuf = new unsigned char[iimg_bytes_per_line*iimg_height];
	//load_image_rgb24(m_pfiler, pimgbuf);

	agg::rendering_buffer rbuf_img(pimgbuf, iimg_width, iimg_height,  iimg_bytes_per_line); // 图像内存
	agg::rendering_buffer rbuf_win(pdst, iw, ih,  ibytes_per_line); // 屏幕位图内存

	typedef agg::pixfmt_bgr24 pixfmt;
	pixfmt pixf(rbuf_win);
	pixfmt pixf_img(rbuf_img);
	agg::renderer_base<pixfmt> renb(pixf);
	//renb.clear(agg::rgba(1,1,1)); // 设置背景白色

	agg::trans_affine mtx = m_drawPrams.get_mtx(iimg_width, iimg_height); // 构建图像的变换矩阵	
	agg::trans_affine img_mtx;
	img_mtx *= mtx;
	img_mtx.invert(); // 此矩阵需要倒转一下

	agg::span_allocator< agg::rgba8> sa;

	// 线性插值器，处理图像缩放
	typedef agg::span_interpolator_linear<> interpolator_type;
	interpolator_type interpolator(img_mtx);

	//typedef agg::image_accessor_clip<pixfmt> img_source_type;
	//img_source_type img_src(pixf_img, agg::rgba8(0, 255, 255));

	typedef agg::span_image_filter_rgb_bilinear_clip<pixfmt,	interpolator_type> span_gen_type;
	span_gen_type sg(pixf_img, agg::rgba_pre(1, 1, 1,1), interpolator);

	agg::rasterizer_scanline_aa<> ras;
	ras.clip_box(0, 0, iw, ih); // 按照屏幕大小设置裁剪矩形，避免缩放过程消耗大量内存

	agg::scanline_u8 sl;

	agg::path_storage path_img_box; // 按照图像尺寸设置图像边界矩形
	path_img_box.move_to(0,0);
	path_img_box.line_to(iimg_width, 0);
	path_img_box.line_to(iimg_width, iimg_height);
	path_img_box.line_to(0, iimg_height);
	//path_img_box.line_to(0,0);

	//agg::conv_stroke<agg::path_storage> stroke(path_test);
	agg::trans_affine img_box_mtx;
	img_box_mtx *= mtx; // 内存图像变换后，图像的边框需要跟着变换

	agg::conv_transform<agg::path_storage> ct(path_img_box, img_box_mtx);	
	ras.add_path(ct);
	agg::render_scanlines_aa(ras, sl, renb, sa, sg);
	//agg::render_scanlines_bin(ras, sl, renb, sa, sg);
	
	delete [] pimgbuf;
	pimgbuf=NULL;

	return 1;
}



// 下面的算法的前提是： 基于图像不动，窗口移动
// 根据图像的尺寸、可视范围，计算出可视范围内块的位置
void calc_block_cells(int img_width, int img_height, int view_left, int view_top, int view_right, int view_bottom,
					  int block_width, int block_height, int* block_left, int* block_right, int* block_top, int *block_bottom){

						  if(view_left<=img_width){
							  if(view_left>0)
								  *block_left = view_left / block_width;
							  else
								  *block_left=0;
						  }

						  if(view_right>=0){
							  if(view_right<img_width)
								  *block_right = (view_right  + block_width -1) / block_width;
							  else
								  *block_right = (img_width  + block_width -1) / block_width;
						  }

						  if( view_top<=img_height){
							  if(view_top>0)
								  *block_top = view_top  / block_height;
							  else
								  *block_top=0;
						  }

						  if(view_bottom>=0){
							  if(view_bottom<img_height)
								  *block_bottom = (view_bottom  + block_height -1) / block_height;
							  else
								  *block_bottom = (img_height  + block_height -1) / block_height;
						  }
}

int RMap::draw_rgb24_byblock(int iw, int ih, unsigned char*& buf)
{
	int ibytes_per_line = calc_bytes_per_line(iw, 24);//iw*3; // RGB一行字节数

	// 屏幕位图是底图
	unsigned char* pdst =buf;

	int imgw;//=m_pfiler->GetWidth();
	int imgh;//=m_pfiler->GetHeight();

	agg::trans_affine mtx = m_drawPrams.get_mtx(imgw, imgh);
	agg::trans_affine img_view_mtx;
	img_view_mtx *= mtx; 
	img_view_mtx.invert();

	// 窗口大小
	double dview_left=0;
	double dview_top=0;
	double dview_right=iw;
	double dview_bottom=ih;

	// 窗口变换
	img_view_mtx.transform((double*)&dview_left, (double*)&dview_top);
	img_view_mtx.transform((double*)&dview_right, (double*)&dview_bottom);

	if(dview_left > imgw || dview_top>imgh
		|| dview_right<0 || dview_bottom<0){
			SetRedrawFlag(false);
			return -1; // 不在显示范围内
	}

	// 计算有多少个影像块位于可视范围内
	int iblock_width=0, iblock_height=0;
	 //m_pfiler->GetBlockSize(iblock_width, iblock_height);
	int block_left = -1;
	int block_right=-1;
	int block_top=-1;
	int block_bottom =-1;

	calc_block_cells(imgw, imgh, dview_left, dview_top,
		dview_right, dview_bottom, iblock_width, iblock_height, 
		&block_left, &block_right, &block_top, &block_bottom);

	int ibytes_per_line_blk = calc_bytes_per_line(iblock_width, 24);

	unsigned char* temblock = new unsigned char[ibytes_per_line_blk*iblock_height*3];
	memset(temblock, 255, ibytes_per_line_blk*iblock_height*3);

	int ibytes_per_line_band = iblock_width; // 这个不需要对齐了 import!!!

	unsigned char* temband = new unsigned char[ibytes_per_line_band*iblock_height];

	for(int iblky=block_top; iblky<block_bottom; ++iblky){
		if(m_drawPrams.GetCancel())
			break;

		for(int iblkx=block_left; iblkx<block_right; ++iblkx){
			if(m_drawPrams.GetCancel())
				break;

			// 第一个条带
			for(int i=0; i<3; ++i){
				if(/*m_pfiler->ReadBlock(i+1, iblkx, iblky, temband)==*/0){
				for(int ir=0; ir<iblock_height; ++ir){
					for(int ic=0; ic<iblock_width; ++ic){
						temblock[ir*ibytes_per_line_blk+ic*3+i] = temband[ir*ibytes_per_line_band+ic];
					}
				}
				}
			} // end 	for(int i=0; i<3; ++i)		

			agg::rendering_buffer rbuf_img(temblock, iblock_width, iblock_height,  ibytes_per_line_blk); // 图像内存
			agg::rendering_buffer rbuf_win(pdst, iw, ih,  ibytes_per_line); // 屏幕位图内存

			typedef agg::pixfmt_bgr24 pixfmt;
			pixfmt pixf(rbuf_win);
			pixfmt pixf_img(rbuf_img);
			agg::renderer_base<pixfmt> renb(pixf);
			//renb.clear(agg::rgba(1,1,1)); // 设置背景白色

			agg::trans_affine img_mtx;
			img_mtx *= mtx;

			// 还需算算单元块的偏移(水平、垂直）两个方向
			double dcell_left = 0;
			double dcell_top = 0;
			double dcell_right = iblock_width;
			double dcell_bottom = iblock_height;
			img_mtx.transform(&dcell_left, &dcell_top);
			img_mtx.transform(&dcell_right, &dcell_bottom);

			double offsetx = iblkx*(dcell_right-dcell_left);
			double offsety = iblky*(dcell_bottom-dcell_top);

			img_mtx *= agg::trans_affine_translation(offsetx, offsety); // import!!!!
			img_mtx *= agg::trans_affine_translation(0.5, 0.5);
			img_mtx.invert(); // 此矩阵需要倒转一下
			
			agg::span_allocator< agg::rgba8> sa;

			// 线性插值器，处理图像缩放
			typedef agg::span_interpolator_linear<> interpolator_type;
			interpolator_type interpolator(img_mtx);

			typedef agg::span_image_filter_rgb_bilinear_clip<pixfmt,	interpolator_type> span_gen_type;
			span_gen_type sg(pixf_img, agg::rgba_pre(1, 1, 1,1), interpolator);

			agg::rasterizer_scanline_aa<> ras;

			ras.clip_box(0, 0, iw, ih); // 按照屏幕大小设置裁剪矩形，避免缩放过程消耗大量内存

			agg::scanline_u8 sl;

			dcell_left = 0;
			dcell_top = 0;
			dcell_right = iblock_width;
			dcell_bottom = iblock_height;
			agg::path_storage path_img_box; // 按照图像尺寸设置图像边界矩形
			path_img_box.move_to(dcell_left, dcell_top);
			path_img_box.line_to(dcell_right, dcell_top);
			path_img_box.line_to(dcell_right, dcell_bottom);
			path_img_box.line_to(dcell_left, dcell_bottom);
			//path_img_box.line_to(0,0);
			
			//agg::conv_stroke<agg::path_storage> stroke(path_img_box);
			agg::trans_affine img_box_mtx;
			img_box_mtx *= mtx; // 内存图像变换后，图像的边框需要跟着变换			
			img_box_mtx *= agg::trans_affine_translation(offsetx, offsety ); // import!!!!
			img_box_mtx *= agg::trans_affine_translation(0.5, 0.5);			

			agg::conv_transform<agg::path_storage> ct(path_img_box, img_box_mtx);	
			//agg::conv_contour<agg::conv_transform<agg::path_storage>> contour(ct);
			ras.add_path(ct);
			agg::render_scanlines_aa(ras, sl, renb, sa, sg);			
		}
	}

	delete [] temband;
	temband = NULL;

	delete [] temblock;
	temblock=NULL;

	SetRedrawFlag(false);

	return 1;
}

int RMap::draw_gray8(int iw, int ih, unsigned char*& buf)
{
	int ibytes_per_line24 = calc_bytes_per_line(iw, 24); // RGB一行字节数
	
	int iimg_width;// = m_pfiler->GetWidth();
	int iimg_height;// = m_pfiler->GetHeight();

	int ibytes_per_line8 = calc_bytes_per_line(iw, 8);

	unsigned char* mem_img8 = new unsigned char[ih*ibytes_per_line8];
	//memset(mem_img8, 255, ih*ibytes_per_line8);

	unsigned char* pimgbuf = new unsigned char[iimg_width*iimg_height];
	//m_pfiler->get_all_data(1, pimgbuf);

	agg::rendering_buffer rbuf_img(pimgbuf, iimg_width, iimg_height,  iimg_width); // 图像内存
	agg::rendering_buffer rbuf_win(mem_img8, iw, ih,  ibytes_per_line8); // 屏幕位图内存

	typedef agg::pixfmt_gray8 pixfmt;
	pixfmt pixf(rbuf_win);
	pixfmt pixf_img(rbuf_img);
	agg::renderer_base<pixfmt> renb(pixf);
	//renb.clear(agg::rgba(1,1,1)); // 设置背景白色

	agg::trans_affine mtx = m_drawPrams.get_mtx(iimg_width, iimg_height);
	agg::trans_affine img_mtx=mtx;
	img_mtx.invert(); // 此矩阵需要倒转一下

	agg::span_allocator< agg::gray8> sa;

	// 线性插值器，处理图像缩放
	typedef agg::span_interpolator_linear<> interpolator_type;
	interpolator_type interpolator(img_mtx);

	typedef agg::span_image_filter_gray_bilinear_clip<pixfmt,	interpolator_type> span_gen_type;
	span_gen_type sg(pixf_img, agg::rgba_pre(1, 1, 1,1), interpolator);

	agg::rasterizer_scanline_aa<> ras;
	ras.clip_box(0, 0, iw, ih); // 按照屏幕大小设置裁剪矩形，避免缩放过程消耗大量内存

	agg::scanline_u8 sl;

	agg::path_storage path_img_box; // 按照图像尺寸设置图像边界矩形
	path_img_box.move_to(0,0);
	path_img_box.line_to(iimg_width, 0);
	path_img_box.line_to(iimg_width, iimg_height);
	path_img_box.line_to(0, iimg_height);
	//path_img_box.line_to(0,0);

	//agg::conv_stroke<agg::path_storage> stroke(path_test);
	agg::trans_affine img_box_mtx;
	img_box_mtx *= mtx; // 内存图像变换后，图像的边框需要跟着变换

	agg::conv_transform<agg::path_storage> ct(path_img_box, img_box_mtx);	
	ras.add_path(ct);
	agg::render_scanlines_aa(ras, sl, renb, sa, sg);

	unsigned char* pdst = buf;
	for(int ir=0; ir<ih; ++ir){
		for(int ic=0; ic<iw; ++ic){
			pdst[ir*ibytes_per_line24+ic*3] = 
			pdst[ir*ibytes_per_line24+ic*3+1] = 
			pdst[ir*ibytes_per_line24+ic*3+2] = mem_img8[ir*ibytes_per_line8+ic];
		}
	}

	delete [] mem_img8;
	mem_img8 = NULL;

	delete [] pimgbuf;
	pimgbuf=NULL;

	SetRedrawFlag(false);
	return 1;
}

int RMap::draw_gray8_byblock(int iw, int ih, unsigned char*& buf)
{
	int ibytes_per_line = calc_bytes_per_line(iw, 24);//iw*3; // RGB一行字节数

	// 屏幕位图是底图
	unsigned char* pdst =buf;

	int imgw;//=m_pfiler->GetWidth();
	int imgh;//=m_pfiler->GetHeight();

	agg::trans_affine mtx = m_drawPrams.get_mtx(imgw, imgh);
	agg::trans_affine img_view_mtx;
	img_view_mtx *= mtx; 
	img_view_mtx.invert();

	// 窗口大小
	double dview_left=0;
	double dview_top=0;
	double dview_right=iw;
	double dview_bottom=ih;

	// 窗口变换
	img_view_mtx.transform((double*)&dview_left, (double*)&dview_top);
	img_view_mtx.transform((double*)&dview_right, (double*)&dview_bottom);

	if(dview_left > imgw || dview_top>imgh
		|| dview_right<0 || dview_bottom<0){
			SetRedrawFlag(false);
			return -1; // 不在显示范围内
	}

	// 计算有多少个影像块位于可视范围内
	int iblock_width=0, iblock_height=0;
	//m_pfiler->GetBlockSize(iblock_width, iblock_height);
	int block_left = -1;
	int block_right=-1;
	int block_top=-1;
	int block_bottom =-1;

	calc_block_cells(imgw, imgh, dview_left, dview_top,
		dview_right, dview_bottom, iblock_width, iblock_height, 
		&block_left, &block_right, &block_top, &block_bottom);

	int ibytes_per_line_blk = calc_bytes_per_line(iblock_width, 24);

	unsigned char* temblock = new unsigned char[ibytes_per_line_blk*iblock_height*3];
	memset(temblock, 255, ibytes_per_line_blk*iblock_height*3);

	int ibytes_per_line_band = iblock_width; // 这个不需要对齐了 import!!!

	unsigned char* temband = new unsigned char[ibytes_per_line_band*iblock_height];

	for(int iblky=block_top; iblky<block_bottom; ++iblky){
		for(int iblkx=block_left; iblkx<block_right; ++iblkx){
			// 第一个条带
			//for(int i=0; i<3; ++i){
			int i=0;
				if(/*m_pfiler->ReadBlock(i+1, iblkx, iblky, temband)==*/0){
					for(int ir=0; ir<iblock_height; ++ir){
						for(int ic=0; ic<iblock_width; ++ic){
							temblock[ir*ibytes_per_line_blk+ic*3+i] = 
								temblock[ir*ibytes_per_line_blk+ic*3+i+1] = 
								temblock[ir*ibytes_per_line_blk+ic*3+i+2] = temband[ir*ibytes_per_line_band+ic];
						}
					}
				}
			//} // end 	for(int i=0; i<3; ++i)		

			agg::rendering_buffer rbuf_img(temblock, iblock_width, iblock_height,  ibytes_per_line_blk); // 图像内存
			agg::rendering_buffer rbuf_win(pdst, iw, ih,  ibytes_per_line); // 屏幕位图内存

			typedef agg::pixfmt_bgr24 pixfmt;
			pixfmt pixf(rbuf_win);
			pixfmt pixf_img(rbuf_img);
			agg::renderer_base<pixfmt> renb(pixf);
			//renb.clear(agg::rgba(1,1,1)); // 设置背景白色

			agg::trans_affine img_mtx;
			img_mtx *= mtx;

			// 还需算算单元块的偏移(水平、垂直）两个方向
			double dcell_left = 0;
			double dcell_top = 0;
			double dcell_right = iblock_width;
			double dcell_bottom = iblock_height;
			img_mtx.transform(&dcell_left, &dcell_top);
			img_mtx.transform(&dcell_right, &dcell_bottom);

			double offsetx = iblkx*(dcell_right-dcell_left);
			double offsety = iblky*(dcell_bottom-dcell_top);

			img_mtx *= agg::trans_affine_translation(offsetx, offsety); // import!!!!
			//img_mtx *= agg::trans_affine_translation(0.5, 0.5);
			img_mtx.invert(); // 此矩阵需要倒转一下

			agg::span_allocator< agg::rgba8> sa;

			// 线性插值器，处理图像缩放
			typedef agg::span_interpolator_linear<> interpolator_type;
			interpolator_type interpolator(img_mtx);

			typedef agg::span_image_filter_rgb_bilinear_clip<pixfmt,	interpolator_type> span_gen_type;
			span_gen_type sg(pixf_img, agg::rgba_pre(1, 1, 1,1), interpolator);

			agg::rasterizer_scanline_aa<> ras;

			ras.clip_box(0, 0, iw, ih); // 按照屏幕大小设置裁剪矩形，避免缩放过程消耗大量内存

			agg::scanline_u8 sl;

			dcell_left = 0;
			dcell_top = 0;
			dcell_right = iblock_width;
			dcell_bottom = iblock_height;
			agg::path_storage path_img_box; // 按照图像尺寸设置图像边界矩形
			path_img_box.move_to(dcell_left, dcell_top);
			path_img_box.line_to(dcell_right, dcell_top);
			path_img_box.line_to(dcell_right, dcell_bottom);
			path_img_box.line_to(dcell_left, dcell_bottom);
			//path_img_box.line_to(0,0);

			//agg::conv_stroke<agg::path_storage> stroke(path_img_box);
			agg::trans_affine img_box_mtx;
			img_box_mtx *= mtx; // 内存图像变换后，图像的边框需要跟着变换			
			img_box_mtx *= agg::trans_affine_translation(offsetx, offsety ); // import!!!!
			//img_box_mtx *= agg::trans_affine_translation(0.5, 0.5);			

			agg::conv_transform<agg::path_storage> ct(path_img_box, img_box_mtx);	
			//agg::conv_contour<agg::conv_transform<agg::path_storage>> contour(ct);

			img_box_mtx.transform(&dcell_left, &dcell_top);
			img_box_mtx.transform(&dcell_right, &dcell_bottom);
			//ras.add_path(ct);
			ras.move_to_d(dcell_left, dcell_top);
			ras.line_to_d(dcell_right, dcell_top);
			ras.line_to_d(dcell_right, dcell_bottom);
			ras.line_to_d(dcell_left, dcell_bottom);
			agg::render_scanlines_aa(ras, sl, renb, sa, sg);			
		}
	}

	delete [] temband;
	temband = NULL;

	delete [] temblock;
	temblock=NULL;

	SetRedrawFlag(false);

	return 1;
}

int RMap::draw_int16(int iw, int ih, unsigned char*& buf)
{
	int ibytes_per_line24 = calc_bytes_per_line(iw, 24); // RGB一行字节数

	int iimg_width;// = m_pfiler->GetWidth();
	int iimg_height;// = m_pfiler->GetHeight();

	int ibytes_per_line16 = calc_bytes_per_line(iw, 16);
/*	short* mem_img16 = new unsigned char[ih*ibytes_per_line16];
	//memset(mem_img8, 255, ih*ibytes_per_line8);

	unsigned char* pimgbuf = new unsigned char[iimg_width*iimg_height];
	m_pfiler->get_all_data(1, pimgbuf);

	agg::rendering_buffer rbuf_img(pimgbuf, iimg_width, iimg_height,  iimg_width); // 图像内存
	agg::rendering_buffer rbuf_win(mem_img16, iw, ih,  ibytes_per_line16); // 屏幕位图内存

	typedef agg::pixfmt_gray8 pixfmt;
	pixfmt pixf(rbuf_win);
	pixfmt pixf_img(rbuf_img);
	agg::renderer_base<pixfmt> renb(pixf);
	//renb.clear(agg::rgba(1,1,1)); // 设置背景白色

	agg::trans_affine mtx = m_drawPrams.get_mtx(iimg_width, iimg_height);
	agg::trans_affine img_mtx=mtx;
	img_mtx.invert(); // 此矩阵需要倒转一下

	agg::span_allocator< agg::gray8> sa;

	// 线性插值器，处理图像缩放
	typedef agg::span_interpolator_linear<> interpolator_type;
	interpolator_type interpolator(img_mtx);

	typedef agg::span_image_filter_gray_bilinear_clip<pixfmt,	interpolator_type> span_gen_type;
	span_gen_type sg(pixf_img, agg::rgba_pre(1, 1, 1,1), interpolator);

	agg::rasterizer_scanline_aa<> ras;
	ras.clip_box(0, 0, iw, ih); // 按照屏幕大小设置裁剪矩形，避免缩放过程消耗大量内存

	agg::scanline_u8 sl;

	agg::path_storage path_img_box; // 按照图像尺寸设置图像边界矩形
	path_img_box.move_to(0,0);
	path_img_box.line_to(iimg_width, 0);
	path_img_box.line_to(iimg_width, iimg_height);
	path_img_box.line_to(0, iimg_height);
	//path_img_box.line_to(0,0);

	//agg::conv_stroke<agg::path_storage> stroke(path_test);
	agg::trans_affine img_box_mtx;
	img_box_mtx *= mtx; // 内存图像变换后，图像的边框需要跟着变换

	agg::conv_transform<agg::path_storage> ct(path_img_box, img_box_mtx);	
	ras.add_path(ct);
	agg::render_scanlines_aa(ras, sl, renb, sa, sg);

	unsigned char* pdst = buf;
	for(int ir=0; ir<ih; ++ir){
		for(int ic=0; ic<iw; ++ic){
			pdst[ir*ibytes_per_line24+ic*3] = 
				pdst[ir*ibytes_per_line24+ic*3+1] = 
				pdst[ir*ibytes_per_line24+ic*3+2] = mem_img16[ir*ibytes_per_line16+ic];
		}
	}

	delete [] mem_img16;
	mem_img16 = NULL;

	delete [] pimgbuf;
	pimgbuf=NULL;

	SetRedrawFlag(false); */
	return 1;
}



int RMap::draw_layer(OGRLayer* player, int iw, int ih, unsigned char*& buf)
{
	if(player==NULL)
		return -1;

	OGRFeature *pfeature=NULL;
	player->ResetReading();
	
	RDrawFactory drawer;
	drawer.Init(iw, ih, buf);

	while ( (pfeature=player->GetNextFeature()) != NULL)
	{
		OGRGeometry *poGeometry = pfeature->GetGeometryRef();
		//drawer.DrawGeometry(&m_drawPrams, poGeometry);		 
		OGRFeature::DestroyFeature( pfeature );
	}

	SetRedrawFlag(false);

	return 1;
}







