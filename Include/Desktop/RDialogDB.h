
#ifndef _RDIAGLOGDB_INCLUDE_H__
#define _RDIAGLOGDB_INCLUDE_H__

#include <QDialog>

#include <string>
using namespace std;

class QLineEdit;

class RDialogDB : public QDialog { 
	Q_OBJECT

public:
	explicit RDialogDB(QWidget *parent = 0);
	~RDialogDB();

	void closeEvent ( QCloseEvent * e );

	string GetServer() const {return m_strServer;};
	string GetUser() const { return m_strUser;}
	string GetPwd() const {return m_strPwd;}

	public slots:
		void GetValue();

protected:
	
	string m_strServer;
	string m_strUser;
	string m_strPwd;

private:
	QLineEdit *m_lineEditUser;
	QLineEdit *m_lineEditPwd;
	QLineEdit *m_lineEditServer;

};

#endif