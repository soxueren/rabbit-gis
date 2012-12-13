/********************************************************************************
** Form generated from reading UI file 'desktop.ui'
**
** Created: Thu Dec 13 14:57:16 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DESKTOP_H
#define UI_DESKTOP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DesktopClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *DesktopClass)
    {
        if (DesktopClass->objectName().isEmpty())
            DesktopClass->setObjectName(QString::fromUtf8("DesktopClass"));
        DesktopClass->resize(600, 400);
        menuBar = new QMenuBar(DesktopClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        DesktopClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(DesktopClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        DesktopClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(DesktopClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        DesktopClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(DesktopClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        DesktopClass->setStatusBar(statusBar);

        retranslateUi(DesktopClass);

        QMetaObject::connectSlotsByName(DesktopClass);
    } // setupUi

    void retranslateUi(QMainWindow *DesktopClass)
    {
        DesktopClass->setWindowTitle(QApplication::translate("DesktopClass", "Desktop", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DesktopClass: public Ui_DesktopClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DESKTOP_H
