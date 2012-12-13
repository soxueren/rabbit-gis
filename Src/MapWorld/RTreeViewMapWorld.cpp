#include "MapWorld/RTreeViewMapWorld.h"
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
 #include <QPushButton>
#include <QFileDialog>
#include <QSpacerItem>
#include "Base/RDataSource.h"

RTreeViewMapWorld::RTreeViewMapWorld(QWidget *parent /*= 0*/)
{
	setHeaderHidden(true);
	//QTreeWidgetItem* pItemHeader = headerItem();
	//pItemHeader->setText(0, QString::fromLocal8Bit("���ߵ�ͼ"));
	this->setColumnCount(1);

	QTreeWidgetItem* pItemMapWorld = new QTreeWidgetItem(this);
	pItemMapWorld->setText(0, QString::fromLocal8Bit("���ͼ"));

	addTopLevelItem(pItemMapWorld);

	//QList<QTreeWidgetItem *> items;
	//for (int i = 0; i < 10; ++i)
	//	items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i))));
	//insertTopLevelItems(0, items);

}

RTreeViewMapWorld::~RTreeViewMapWorld()
{

}


//////////////////////////////////////////////////////////////////////////
RTreeViewDownload::RTreeViewDownload(QWidget *parent/* = 0*/)
{
	setHeaderHidden(true);
	this->setColumnCount(1);

	//QTreeWidgetItem* pItemMapWorld = new QTreeWidgetItem(this);
	//pItemMapWorld->setText(0, QString::fromLocal8Bit("����1"));

	QList<QTreeWidgetItem *> items;
	items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString::fromLocal8Bit("ȫ������"))));
	items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString::fromLocal8Bit("��������"))));
	items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString::fromLocal8Bit("�����"))));
	insertTopLevelItems(0, items);
	//addTopLevelItem(pItemMapWorld);

}

RTreeViewDownload::~RTreeViewDownload()
{

}

//////////////////////////////////////////////////////////////////////////
RTreeViewDownloadTask::RTreeViewDownloadTask(QWidget *parent/* = 0*/)
{
	//setHeaderHidden(true);
	this->setColumnCount(4);

	QStringList labels;
	labels.append(QString::fromLocal8Bit("����"));
	labels.append(QString::fromLocal8Bit("״̬"));
	labels.append(QString::fromLocal8Bit("����"));
	labels.append(QString::fromLocal8Bit("����"));
	labels.append(QString::fromLocal8Bit("��Χ"));

	this->	setHeaderLabels(labels);

	/*QList<QTreeWidgetItem *> items;
	items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString::fromLocal8Bit("ȫ������"))));
	items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString::fromLocal8Bit("��������"))));
	items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString::fromLocal8Bit("�����"))));
	insertTopLevelItems(0, items);*/
	//addTopLevelItem(pItemMapWorld);

}

RTreeViewDownloadTask::~RTreeViewDownloadTask()
{

}

//////////////////////////////////////////////////////////////////////////
RDialogDownload::RDialogDownload(QWidget *parent /*= 0*/)
{
	QGroupBox * pGroupBoxDownload = new QGroupBox;

	QGridLayout *pGridLayoutGroupBox = new QGridLayout;

	QLabel* pLabelLeft = new QLabel(QString::fromLocal8Bit("��:"));
	QLabel* pLabelTop = new QLabel(QString::fromLocal8Bit("��:"));
	QLabel* pLabelRight = new QLabel(QString::fromLocal8Bit("��:"));
	QLabel* pLabelBottom = new QLabel(QString::fromLocal8Bit("��:"));
	pGridLayoutGroupBox->addWidget(pLabelLeft, 1, 0, Qt::AlignRight);
	pGridLayoutGroupBox->addWidget(pLabelTop, 2, 0, Qt::AlignRight);
	pGridLayoutGroupBox->addWidget(pLabelRight, 3, 0, Qt::AlignRight);
	pGridLayoutGroupBox->addWidget(pLabelBottom, 4, 0, Qt::AlignRight);

	m_pLineEditLeft = new QLineEdit;
	m_pLineEditTop = new QLineEdit;
	m_pLineEditRight = new QLineEdit;
	m_pLineEditBottom = new QLineEdit;
	connect(m_pLineEditLeft, SIGNAL(editingFinished()), this, SLOT(LineEditingFinished()));
	connect(m_pLineEditTop, SIGNAL(editingFinished()), this, SLOT(LineEditingFinished()));
	connect(m_pLineEditRight, SIGNAL(editingFinished()), this, SLOT(LineEditingFinished()));
	connect(m_pLineEditBottom, SIGNAL(editingFinished()), this, SLOT(LineEditingFinished()));
	
	pGridLayoutGroupBox->addWidget(m_pLineEditLeft, 1, 1);
	pGridLayoutGroupBox->addWidget(m_pLineEditTop, 2, 1);
	pGridLayoutGroupBox->addWidget(m_pLineEditRight, 3, 1);
	pGridLayoutGroupBox->addWidget(m_pLineEditBottom, 4, 1);

	QLabel* pLabelOutPath = new QLabel(QString::fromLocal8Bit("���·��:"));
	pGridLayoutGroupBox->addWidget(pLabelOutPath, 5, 0);

	m_pLineEditOutPath = new QLineEdit("d:/Downloads");
	pGridLayoutGroupBox->addWidget(m_pLineEditOutPath, 5, 1);

	m_pPushBtnOutPath = new QPushButton("...");
	connect(m_pPushBtnOutPath, SIGNAL(clicked()), this, SLOT(OnPushBtnOutPathClicked()));
	pGridLayoutGroupBox->addWidget(m_pPushBtnOutPath, 5, 2);

	QLabel* pLabelTaskName = new QLabel(QString::fromLocal8Bit("��������:"));
	pGridLayoutGroupBox->addWidget(pLabelTaskName, 6, 0);
	QLineEdit* pLineEditTaskName = new QLineEdit;
	pLineEditTaskName->setText("NewTask");
	pLineEditTaskName->setMinimumWidth(400);
	pGridLayoutGroupBox->addWidget(pLineEditTaskName, 6, 1);



	//layoutGroupBox->addWidget(pGridLayoutGroupBox);
	pGroupBoxDownload->setLayout(pGridLayoutGroupBox);


	//////////////////////////////////////////////////////////////////////////
	// ���ؼ����б�
	m_pTreeWidgetLeves = new QTreeWidget();
	m_pTreeWidgetLeves->setColumnCount(5);

	QStringList labels;
	labels.append(QString::fromLocal8Bit("����"));
	labels.append(QString::fromLocal8Bit("��ʼ���к�"));
	labels.append(QString::fromLocal8Bit("��ֹ���к�"));
	labels.append(QString::fromLocal8Bit("��Ƭ��Ŀ"));
	labels.append(QString::fromLocal8Bit("��ע"));

	m_pTreeWidgetLeves->setHeaderLabels(labels);

	QList<QTreeWidgetItem *> items;
	for(int i=2; i<=18; i++)
	{
		QString strLevel = QString::fromLocal8Bit("%1 ��").arg(i);
		QStringList listContents;
		listContents<<strLevel<<""<<""<<""<<"";
		QTreeWidgetItem* pItem = new QTreeWidgetItem((QTreeWidget*)0, listContents);
		pItem->setCheckState(0, Qt::Unchecked);
		items.append(pItem);
	}
	m_pTreeWidgetLeves->insertTopLevelItems(0, items);
	//////////////////////////////////////////////////////////////////////////

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(pGroupBoxDownload);
	mainLayout->addWidget(m_pTreeWidgetLeves);
	
	//////////////////////////////////////////////////////////////////////////
	// ȷ��
	QPushButton* pPushBtnYes = new QPushButton(QString::fromLocal8Bit("ȷ��"));
	QPushButton* pPushBtnNo = new QPushButton(QString::fromLocal8Bit("	ȡ��"));
	connect(pPushBtnYes, SIGNAL(clicked()), this, SLOT(accept()));
	connect(pPushBtnNo, SIGNAL(clicked()), this, SLOT(reject()));

	QPushButton* pPushBtnSelectAll = new QPushButton(QString::fromLocal8Bit("ȫѡ"));
	QPushButton* pPushBtnSelectNone = new QPushButton(QString::fromLocal8Bit("��ѡ"));
	QPushButton* pPushBtnSelectClear = new QPushButton(QString::fromLocal8Bit("���"));
	connect(pPushBtnSelectAll, SIGNAL(clicked()), this, SLOT(OnPushBtnSelectAllClicked()));
	connect(pPushBtnSelectNone, SIGNAL(clicked()), this, SLOT(OnPushBtnSelectNoneClicked()));
	connect(pPushBtnSelectClear, SIGNAL(clicked()), this, SLOT(OnPushBtnSelectClearClicked()));

	QHBoxLayout *pHBoxLayoutOK = new QHBoxLayout;

	pHBoxLayoutOK->addWidget(pPushBtnSelectAll);//, 0, Qt::AlignLeft);
	pHBoxLayoutOK->addWidget(pPushBtnSelectNone);//, 0, Qt::AlignLeft);
	pHBoxLayoutOK->addWidget(pPushBtnSelectClear);//, 0, Qt::AlignLeft);

	QSpacerItem* pSpacerItem = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
	pHBoxLayoutOK->addSpacerItem(pSpacerItem);

	pHBoxLayoutOK->addWidget(pPushBtnYes);//, 0, Qt::AlignRight);
	pHBoxLayoutOK->addWidget(pPushBtnNo);//, 0, Qt::AlignRight);
	pHBoxLayoutOK->setContentsMargins(0,0,0,0);
	mainLayout->addLayout(pHBoxLayoutOK);

	setLayout(mainLayout);
	setWindowTitle(QString::fromLocal8Bit("�½�����"));

	m_dLeft=0;
	m_dTop=0;
	m_dRight=0;
	m_dBottom=0;

}


void RDialogDownload::OnPushBtnOutPathClicked()
{
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::DirectoryOnly);
	if(int iResult = dlg.exec() == QDialog::Accepted)
	{
		QDir thisDir = dlg.directory();

		QString strPath = thisDir.absolutePath();
		m_pLineEditOutPath->setText(strPath);
	}
	else{
		QDir thisDir = dlg.directory();
		int i=0;
	}

}

void RDialogDownload::OnPushBtnSelectAllClicked()
{
	int iCount = m_pTreeWidgetLeves->topLevelItemCount();
	for(int i=0; i<iCount; i++)
	{
		QTreeWidgetItem *	topLevelItem = m_pTreeWidgetLeves->topLevelItem(i);
		topLevelItem->setCheckState(0, Qt::Checked);		
	}

}


void RDialogDownload::OnPushBtnSelectNoneClicked() // ��ѡ
{
	int iCount = m_pTreeWidgetLeves->topLevelItemCount();
	for(int i=0; i<iCount; i++)
	{
		QTreeWidgetItem *	topLevelItem = m_pTreeWidgetLeves->topLevelItem(i);
		if(topLevelItem->checkState(0) == Qt::Checked)
		{
			topLevelItem->setCheckState(0, Qt::Unchecked);		
		}
		else
		{
			topLevelItem->setCheckState(0, Qt::Checked);		
		}
	}
}

void RDialogDownload::OnPushBtnSelectClearClicked()
{
	int iCount = m_pTreeWidgetLeves->topLevelItemCount();
	for(int i=0; i<iCount; i++)
	{
		QTreeWidgetItem *	topLevelItem = m_pTreeWidgetLeves->topLevelItem(i);
		topLevelItem->setCheckState(0, Qt::Unchecked);		
	}
}

void RDialogDownload::LineEditingFinished()// �������ҷ�Χ�༭���¼�
{
	QString strLeft = m_pLineEditLeft->text();
	QString strTop = m_pLineEditTop->text();
	QString strRight = m_pLineEditRight->text();
	QString strBottom = m_pLineEditBottom->text();

	if(strLeft.isEmpty() || strTop.isEmpty() || strRight.isEmpty() || strBottom.isEmpty())
	{
		return;
	}

	bool okLeft=false;
	bool okTop=false;
	bool okRight=false;
	bool okBottom=false;
	m_dLeft = strLeft.toDouble(&okLeft);
	m_dTop = strTop.toDouble(&okTop);
	m_dRight = strRight.toDouble(&okRight);
	m_dBottom = strBottom.toDouble(&okBottom);

	if(okLeft && okTop && okRight && okBottom)
	{
		RDataSourceMapWorld dataSource;
		RDataset* pDataset = dataSource.GetDataset(TDT_EMAP);
		if(pDataset==NULL)
			return;

		for(int i=2; i<=18; i++)
		{
			QTreeWidgetItem *pItem = m_pTreeWidgetLeves->topLevelItem (i-2);
			if(pItem!=NULL)
			{
				int iRowLeft, iRowRight, iColTop, iColBottom;
				double dTileWidth;
				pDataset->GetRowAndCol(m_dLeft, m_dTop, m_dRight, m_dBottom, i, iRowLeft, iRowRight, iColTop, iColBottom, dTileWidth);
				QString strText;
				strText = QString("(%1,%2)").arg(iRowLeft).arg(iColTop);
				pItem->setText(1, strText);
				strText = QString("(%1,%2)").arg(iRowRight).arg(iColBottom);
				pItem->setText(2, strText);
				__int64 iTileNums = __int64(iRowRight-iRowLeft) * __int64(iColBottom-iColTop);
				strText = QString("%1").arg(iTileNums);
				pItem->setText(3, strText);
			}
		}
	}
}
