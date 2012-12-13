#ifndef __FILETYPE_H__
#define __FILETYPE_H__

#pragma once

#include "Base/RDefine.h"

#include <string>
using namespace std;

//////////////////////////////////////////////////////////////////////////
#define REP 1e-10
#define RNEP -1e-10

static bool RIS0(double dx) 
{
	if(RNEP<dx && dx<REP)
		return true;
	return false;
}
//////////////////////////////////////////////////////////////////////////

class BASE_API CRFile{
public:
	enum Type{
		None=0,
		SHP=1,  // "ESRI Shapefile";		"shp";
		TAB=2, // "MapInfo File";		"tab";
		MIF=3, // "MapInfo File";		"mif";		
		GML=4, // "GML";					"gml";
		KML=5, // "KML";					"kml";
		CSV=9, // "CSV";				"csv";

		
		
		TXT=10,

		TIFF = 101,
		IMG = 102,
		JPG = 103,
		BMP=104,
		PNG=105,
		};

	//! \brief 文件类别
	enum FClass{
		fcVector=1, // 矢量文件
		fcRaster, // 栅格文件
	};

	static int GetTypeByExtName(const string& strExt);
	static string GetExtNameByType(int iType);
	
	static string GetDriverNameByType(int iType);

	static string GetExtNameFromPath(const string& strPath);
	static string GetTitleNameFromPath(const string& strPath);

	static CRFile::FClass GetClass(int iType);

	static CRFile::FClass GetClassByName(const string& strPath);

	static bool IsExist(const string& strFileName);
	static string GetDirName(const string& strFileName);
	static void MkDir(const string& strDir);


};


typedef struct CRfileDetail{
	CRFile::Type iType;
	char* chExt; // 扩展名
	char* chDri; // 驱动插件名
};

static CRfileDetail gfileDetail[]={
	{CRFile::SHP, "shp", "ESRI Shapefile"},
	{CRFile::TAB, "tab", "MapInfo File"},
	{CRFile::MIF, "mif", "MapInfo File"},
	{CRFile::GML, "gml", "GML"},
	{CRFile::KML, "kml", "KML"},
	{CRFile::CSV, "csv", "CSV"},
	{CRFile::TXT, "txt", "Text File"},

	{CRFile::TIFF, "tif", "TIFF/GeoTIFF"},
	{CRFile::IMG, "img", "Erdas Imagine "},
	{CRFile::JPG, "jpg", "JPEG File"},
	{CRFile::BMP, "bmp", "BMP File"},
	{CRFile::PNG, "png", "PNG File"},
	
	{CRFile::None, NULL, NULL}

};

#endif //__FILETYPE_H__
