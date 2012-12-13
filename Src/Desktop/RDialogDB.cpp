
#include "Desktop/RDialogDB.h"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

RDialogDB::RDialogDB(QWidget *parent /*= 0*/)
{
	m_lineEditServer = new QLineEdit();
	QLabel* labServer = new QLabel;
	labServer->setText("Server:");

	m_lineEditUser = new QLineEdit();
	m_lineEditPwd = new QLineEdit();

	QFormLayout *layout = new QFormLayout;
	layout->addRow(labServer, m_lineEditServer);
	layout->addRow("User:", m_lineEditUser);
	layout->addRow("Password", m_lineEditPwd);

	QPushButton* btOK = new QPushButton("&OK");
	QPushButton* btCancel = new QPushButton("&Cancel");
	connect(btOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(btCancel, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout* layOK = new QHBoxLayout;
	layOK->addWidget(btOK);
	layOK->addWidget(btCancel);

	layout->addRow(layOK);

	setLayout(layout);

	 connect(this, SIGNAL(accepted()), this, SLOT(GetValue()));
}

RDialogDB::~RDialogDB()
{

}

void RDialogDB::GetValue()
{
	QString strServer = m_lineEditServer->text();
	QString strUser = m_lineEditUser->text();
	QString strPwd = m_lineEditPwd->text();

	m_strServer = strServer.toLocal8Bit().data();// .local8Bit().data();
	m_strUser = strUser.toLocal8Bit().data();//.local8Bit().data();
	m_strPwd = strPwd.toLocal8Bit().data();//.local8Bit().data();

}

void RDialogDB::closeEvent ( QCloseEvent * e )
{
	

	
}

