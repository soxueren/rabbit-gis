
#include "Base/RFileVector.h"
#include "ogrsf_frmts.h"

RFileVector::RFileVector(): m_poDS(NULL)
{
	OGRRegisterAll();
}

RFileVector::~RFileVector()
{
	Close();
}


bool RFileVector::Open(const string& filename)
{
	if(!filename.empty())
		m_strfilename = filename;

	m_poDS = OGRSFDriverRegistrar::Open(m_strfilename.c_str(), FALSE );

	if( m_poDS == NULL )	{
		printf( "Open failed.\n" );
		return false;
	}

	return true;
}

int RFileVector::GetLayerCount() const
{
	if(m_poDS){
		return m_poDS->GetLayerCount();
	}
	
	return 0;
}

OGRLayer* RFileVector::GetLayer(int ilay)
{
	if(m_poDS){
		int icount = m_poDS->GetLayerCount();
		if(ilay>icount-1)
			return NULL;

		return m_poDS->GetLayer(ilay);
	}

	return NULL;
}

void RFileVector::GetBound(double *dleft, double *dtop, double *dright, double *dbottom)
{
	if(m_poDS==NULL)
		return;

	OGREnvelope allBound;

	int iCount = GetLayerCount();
	for(int i=0; i<iCount; ++i){
		OGRLayer* layer = m_poDS->GetLayer(i);
		if(layer != NULL){
			OGREnvelope ext;
			layer->GetExtent(&ext, true);
			allBound.Merge(ext);
		}
	}

	*dleft = allBound.MinX;
	*dtop = allBound.MaxY;
	*dright = allBound.MaxX;
	*dbottom = allBound.MinY;
}




void RFileVector::Close()
{
	if(m_poDS!=NULL){
		OGRDataSource::DestroyDataSource( m_poDS );
		m_poDS=NULL;
	}
}
