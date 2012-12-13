

#include "Base/RFileRaster.h"
#include "gdal_priv.h"


RFileRaster::RFileRaster(): m_iband_count(0),
m_iheight(0),
m_iwidth(0),
m_ibits_per_pixel(0),
m_dataset(NULL)

{
	GDALAllRegister();


}

//RFileRaster::RFileRaster(const std::string& filename)
//{
//	if(filename.empty())
//		m_filename = filename;
//	
//	GDALAllRegister();
//
//}

RFileRaster::~RFileRaster()
{
	Close();
}

bool RFileRaster::Open(const string& filename)
{
	if(!filename.empty())
		m_filename = filename;


	m_dataset = (GDALDataset *)GDALOpen(m_filename.c_str(), GA_ReadOnly);
	if(m_dataset==NULL)
		return false;

	m_iband_count = m_dataset->GetRasterCount(); // 波段数目
	//if(iband_count>3) // 屏幕只能显示三个波段 
	//	iband_count=3;

	m_iwidth=m_dataset->GetRasterXSize();
	m_iheight=m_dataset->GetRasterYSize();

	GDALRasterBand* pband = m_dataset->GetRasterBand(1);
	m_ibits_per_pixel =(int) pband->GetRasterDataType();
}

const char* RFileRaster::GetDataTypeName()
{
	if(m_dataset==NULL)
		return "";

	GDALRasterBand* hband = m_dataset->GetRasterBand(1);
	return GDALGetDataTypeName(GDALGetRasterDataType(hband));
}

bool RFileRaster::get_all_data(int iband, unsigned char*& buf)
{
	if(iband>m_iband_count || iband<0)
		return false;

	GDALRasterBand* band = m_dataset->GetRasterBand(iband);

	if(band->GetRasterDataType() == GDT_Byte){	
	int ibytes_per_line=m_iwidth;
	if(ibytes_per_line%4)
		ibytes_per_line += 4-ibytes_per_line%4;

	if(buf == NULL){
		buf = new unsigned char [m_iheight* ibytes_per_line];
		memset(buf, 255, m_iheight*ibytes_per_line);	
	}

	//band->ReadBlock()
// 	int iblock_sizex, iblock_sizey;
// 	band->GetBlockSize(&iblock_sizex, &iblock_sizey);
// 
// 	int xblocks = (m_iwidth + iblock_sizex -1 )/ iblock_sizex;
// 	int yblocks = (m_iheight + iblock_sizey -1) / iblock_sizey;

	band->RasterIO(GF_Read, 0, 0, m_iwidth, m_iheight, buf, m_iwidth, m_iheight, 
		band->GetRasterDataType(), 0, 0);
	}

}


void RFileRaster::GetBlockSize(int& bx, int& by)
{
	if(m_dataset){
		m_dataset->GetRasterBand(1)->GetBlockSize(&bx, &by);
	}
}

void RFileRaster::GetBlockCount(int& xblocks, int& yblocks)
{
	if(m_dataset){
		int bx, by;
		GetBlockSize(bx, by);
		xblocks = (m_iwidth+ bx - 1) / bx;
		yblocks = (m_iheight + by - 1) / by;
	}
}

int RFileRaster::ReadBlock(int iband, int xoff, int yoff, unsigned char*& buf)
{
	if(m_dataset==NULL)
		return -1;

	GDALRasterBand* band = m_dataset->GetRasterBand(iband);
	if(band){
		 return (int)band->ReadBlock(xoff, yoff, buf);
	}
}

void RFileRaster::GetBound	(double* dleft, double* dtop, double* dright, double* dbottom)
{
	if(m_dataset==NULL)
		return;

	// 图像坐标
	int ileft=0, itop=0, iright=m_dataset->GetRasterXSize(), ibottom = m_dataset->GetRasterYSize();

	double	adfGeoTransform[6];
	if(m_dataset->GetGeoTransform(adfGeoTransform) == CE_None){
		*dleft = adfGeoTransform[0] + adfGeoTransform[1] * ileft	+ adfGeoTransform[2] * itop;
		*dtop = adfGeoTransform[3] + adfGeoTransform[4] * ileft + adfGeoTransform[5] * itop;
		*dright = adfGeoTransform[0] + adfGeoTransform[1] * iright	+ adfGeoTransform[2] * ibottom;
		*dbottom = adfGeoTransform[3] + adfGeoTransform[4] * iright	+ adfGeoTransform[5] * ibottom;
	}
	else{
		*dleft = ileft;
		*dright = iright;
		*dtop = ibottom;
		*dbottom = itop;
	}
}

double RFileRaster::GetNoData()
{
	if(m_dataset!=NULL){
		return m_dataset->GetRasterBand(1)->GetNoDataValue();
	}
	return -9999;
}






std::string RFileRaster::get_srs()
{
	std::string srs;
	if(m_dataset)
		srs = m_dataset->GetProjectionRef();

	return srs;

}


void RFileRaster::Close()
{
	if(m_dataset){
		GDALClose(m_dataset);
		m_dataset=NULL;
	}

}