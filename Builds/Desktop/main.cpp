#include "Desktop/Rabbit.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Rabbit w;
	w.show();
	return a.exec();
}
