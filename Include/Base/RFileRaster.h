
#ifndef FILE_RASTER_H
#define FILE_RASTER_H

#pragma once

#include "Base/RDefine.h"
#include <string>
using namespace std;

class GDALDataset;

class BASE_API RFileRaster
{
public:
	RFileRaster();
	//RFileRaster(const std::string& filename);
	virtual ~RFileRaster();

	bool Open(const string& filename);
	string GetName() const { return m_filename;}

	GDALDataset* GetDataset() const {return m_dataset;}

	int GetWidth() const { return m_iwidth;};
	int GetHeight() const {return m_iheight;}

	int GetBandCount(){ return m_iband_count;}
	int GetDataType() {return m_ibits_per_pixel;} // 注意返回值类型是 GDALDataType
	const char* GetDataTypeName();
	void GetBlockSize(int& bx, int& by);
	void GetBlockCount(int& xblocks, int& yblocks);
	int ReadBlock(int iband, int xoff, int yoff, unsigned char*& buf);

	void GetBound(double* dleft, double* dtop, double* dright, double* dbottom);

	double GetNoData();

	/************************************************************************/
	/* 得到指定波段的全部数据，数据已经对齐, 用户管理内存*/
	/************************************************************************/
	bool get_all_data(int iband, unsigned char*& buf);

	/************************************************************************/
	/* 获取坐标信息描述                                                                     */
	/************************************************************************/
	std::string get_srs();

	void Close();



private:

	string m_filename;
	int m_iband_count;
	int m_iwidth;
	int m_iheight;
	int m_ibits_per_pixel;

	GDALDataset* m_dataset;

};



#endif