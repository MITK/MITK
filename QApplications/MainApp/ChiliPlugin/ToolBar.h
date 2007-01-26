#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <qwidget.h>
#include <qlayout.h>
#include <qclightbox.h>
#include <chili/plugin.h>

class QWidget;
class QGridLayout;
class QButtonGroup;

class ToolBar: public QWidget
{
    Q_OBJECT

public:
    ToolBar(QWidget* parent,QcPlugin* qcplugin);
    ~ToolBar();
    void SetWidget(QWidget* ap);
    QButtonGroup* GetToolBar();
    void ConnectButton(int number);
    bool KeepDataTreeNodes();
    
public slots:
    void Reinitialize(bool );
    void ButtonToggled(bool );
    void ToolbarMode(bool );

signals:
    void LightboxSelected(QcLightbox*);
    void ChangeWidget(bool=false);
        
private:
    QWidget* widget;
    QWidget* task;
    QcPlugin* plugin;
    QGridLayout* layout;
    QButtonGroup* toolbar;    
    int idLightbox;
    void SelectLightbox(int id);
    bool m_KeepDataTreeNodes;
};

#endif
