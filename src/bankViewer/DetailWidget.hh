/****************************************************************************
** Form interface generated from reading ui file 'DetailWidget.ui'
**
** Created: Thu Apr 27 23:37:25 2006
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QTextEdit;

class DetailWidget : public QWidget
{
    Q_OBJECT

public:
    DetailWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~DetailWidget();

    QGroupBox* detailBox;
    QTextEdit* detailText;

protected:
    QGridLayout* DetailWidgetLayout;
    QVBoxLayout* detailBoxLayout;

protected slots:
    virtual void languageChange();

};

#endif // DETAILWIDGET_H
