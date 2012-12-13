
#include "Desktop/RFileInfo.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTextEdit>

#include "Base/RFileRaster.h"

RFileInfo::RFileInfo(QWidget *parent, Qt::WFlags flags)
{
	m_centralWidget = new QTabWidget;
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_centralWidget);
	setLayout(mainLayout);

	QWidget* pwidimginfo = new QWidget;
	QTableView* tableview = create_imginfo_tableview();
	QVBoxLayout *imgLayout = new QVBoxLayout;
	imgLayout->addWidget(tableview);
	pwidimginfo->setLayout(imgLayout);
	m_centralWidget->addTab(pwidimginfo, "Image Info");

	QWidget* pwidgeoinfo = new QWidget;
	m_pgeoInfo = new QTextEdit;
	m_pgeoInfo->setReadOnly(true);
	QVBoxLayout* geoLayout = new QVBoxLayout;
	geoLayout->addWidget(m_pgeoInfo);
	pwidgeoinfo->setLayout(geoLayout);
	m_centralWidget->addTab(pwidgeoinfo, "Geographic Info");


	QPushButton* bntok = new QPushButton("OK");
	connect(bntok, SIGNAL(clicked()), this, SLOT(close()));
	mainLayout->addWidget(bntok);


}

RFileInfo::~RFileInfo()
{

}

QTableView* RFileInfo::create_imginfo_tableview()
{
	m_pItemModel = new QStandardItemModel(7, 4, this);
	m_pItemModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
	m_pItemModel->setHeaderData(1, Qt::Horizontal, tr("Value"));
	m_pItemModel->setHeaderData(2, Qt::Horizontal, tr("Name"));
	m_pItemModel->setHeaderData(3, Qt::Horizontal, tr("Value"));
	QModelIndex i = m_pItemModel->index(0,0, QModelIndex());
	m_pItemModel->setData(i, "Width:");

	i = m_pItemModel->index(1,0, QModelIndex());
	m_pItemModel->setData(i, "Height:");

	i = m_pItemModel->index(2,0, QModelIndex());
	m_pItemModel->setData(i, "Block Width:");

	i = m_pItemModel->index(3,0, QModelIndex());
	m_pItemModel->setData(i, "Block Height:");

	i = m_pItemModel->index(4,0, QModelIndex());
	m_pItemModel->setData(i, "Band Count:");

	i = m_pItemModel->index(5,0, QModelIndex());
	m_pItemModel->setData(i, "Data Type:");

	i = m_pItemModel->index(6,0, QModelIndex());
	m_pItemModel->setData(i, "NoData Value:");

	i = m_pItemModel->index(0, 2, QModelIndex());
	m_pItemModel->setData(i, "Left:");
	i = m_pItemModel->index(1, 2, QModelIndex());
	m_pItemModel->setData(i, "Upper:");
	i = m_pItemModel->index(2, 2, QModelIndex());
	m_pItemModel->setData(i, "Right:");
	i = m_pItemModel->index(3, 2, QModelIndex());
	m_pItemModel->setData(i, "Lower:");

	i = m_pItemModel->index(4, 2, QModelIndex());
	m_pItemModel->setData(i, "X Resolution:");
	i = m_pItemModel->index(5, 2, QModelIndex());
	m_pItemModel->setData(i, "Y Resolution:");

	QTableView* tv = new QTableView;
	tv->setSelectionBehavior(QAbstractItemView::SelectRows);
	//tv->horizontalHeader()->setStretchLastSection(true);
	tv->verticalHeader()->hide();
	tv->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tv->setSelectionMode(QAbstractItemView::SingleSelection);
	tv->setModel(m_pItemModel);

	return tv;
}

void RFileInfo::set_filename(const std::string& filename)
{
	if(filename.empty())
		return;

	RFileRaster filer;
	filer.Open(filename);

	int iw=filer.GetWidth();
	int ih=filer.GetHeight();
	int ibpp=filer.GetDataType();
	std::string strbpp=filer.GetDataTypeName();
	int ibc = filer.GetBandCount();
	int iblkw, iblkh;
	filer.GetBlockSize(iblkw, iblkh);

	QModelIndex i = m_pItemModel->index(0,1, QModelIndex());
	m_pItemModel->setData(i, iw);

	i = m_pItemModel->index(1,1, QModelIndex());
	m_pItemModel->setData(i, ih);

	i = m_pItemModel->index(2,1, QModelIndex());
	m_pItemModel->setData(i, iblkw);

	i = m_pItemModel->index(3,1, QModelIndex());
	m_pItemModel->setData(i, iblkh);

	i = m_pItemModel->index(4,1, QModelIndex());
	m_pItemModel->setData(i, ibc);

	i = m_pItemModel->index(5,1, QModelIndex());
	m_pItemModel->setData(i, strbpp.c_str());

	double dnodata = filer.GetNoData();
	QString strnodata = QString("%1").arg(dnodata, 0,  'f');
	i = m_pItemModel->index(6, 1, QModelIndex());
	m_pItemModel->setData(i, strnodata);

	double dleft, dtop, dright, dbottom;
	filer.GetBound(&dleft, &dtop, &dright, &dbottom);

	QString strval = QString("%1").arg(dleft, 0, 'f');
	i = m_pItemModel->index(0, 3, QModelIndex());
	m_pItemModel->setData(i, strval);
 
	strval = QString("%1").arg(dtop, 0, 'f');
	i = m_pItemModel->index(1, 3, QModelIndex());
	m_pItemModel->setData(i, strval);

	strval = QString("%1").arg(dright, 0, 'f');
	i = m_pItemModel->index(2, 3, QModelIndex());
	m_pItemModel->setData(i, strval);

	strval = QString("%1").arg(dbottom, 0, 'f');
	i = m_pItemModel->index(3, 3, QModelIndex());
	m_pItemModel->setData(i, strval);

	strval = QString("%1").arg(double((dright-dleft) / iw), 0, 'f');
	i = m_pItemModel->index(4, 3, QModelIndex());
	m_pItemModel->setData(i, strval);

	strval = QString("%1").arg(double((dtop-dbottom) / ih), 0, 'f');
	i = m_pItemModel->index(5, 3, QModelIndex());
	m_pItemModel->setData(i, strval);

	std::string srs = filer.get_srs();
	if (srs.empty()){
		srs = "No Projection information.";
	}

	QString qsrs = srs.c_str();

	qsrs.replace("GEOGCS", "\t\t\nGEOGCS");
	qsrs.replace("DATUM", "\t\nDATUM");
	qsrs.replace("PROJECTION", "\t\nPROJECTION");
	qsrs.replace("SPHEROID", "\t\nSPHEROID");
	qsrs.replace("PARAMETER", "\t\nPARAMETER");
	qsrs.replace("UNIT", "\t\nUNIT");

	m_pgeoInfo->setPlainText(qsrs);

	/************************************************************************/
	/* 

	// 图像信息在此
	QLabel* img_info = new QLabel(strimg);
	img_info->setAlignment(Qt::AlignLeft|Qt::AlignLeft);
	img_info->setTextInteractionFlags(Qt::TextSelectableByMouse);



	// 投影信息在此
	QString qsrs("No Geographic Info.");
	
	*/
	/************************************************************************/


}

