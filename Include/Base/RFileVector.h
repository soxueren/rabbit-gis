
#ifndef RFILE_VECTOR_H
#define RFILE_VECTOR_H
#include "Base/RDefine.h"
#include <string>
using namespace std;

class OGRDataSource;
class OGRLayer;

class BASE_API RFileVector
{
public:

	RFileVector();
	~RFileVector();

	bool Open(const string& filename);

	int GetLayerCount() const;
	OGRLayer* GetLayer(int ilay);

	void GetBound(double *dleft, double *dtop, double *dright, double *dbottom);

	void Close();


private:
	 OGRDataSource* m_poDS;

	 string m_strfilename;
};

#endif //RFILE_VECTOR_H