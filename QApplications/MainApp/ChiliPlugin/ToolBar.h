#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <qwidget.h>
#include <qlayout.h>

class QPushButton;
class QWidget;
class QGridLayout;
class QButtonGroup;

class ToolBar: public QWidget
{
    //Q_OBJECT

public:
    ToolBar(QWidget* parent);
    ~ToolBar();
    void SetWidget(QWidget* ap);
    QButtonGroup* GetToolBar();
    void ConnectButton(int number);
        
private:
    QWidget* widget;
    QGridLayout* layout;
    QButtonGroup* toolbar;

/*private slots:
    void ClickButton(bool );
 */   

};

#endif