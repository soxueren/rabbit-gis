#ifndef DESKTOP_H
#define DESKTOP_H

#include <QtGui/QMainWindow>
#include "ui_desktop.h"

class Desktop : public QMainWindow
{
	Q_OBJECT

public:
	Desktop(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Desktop();

private:
	Ui::DesktopClass ui;
};

#endif // DESKTOP_H
