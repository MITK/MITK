#ifndef QCMITKSAMPLEPLUGIN_H
#define QCMITKSAMPLEPLUGIN_H

#include <chili/plugin.h>
#include "ToolBar.h"

class QcTask;
class SampleApp;
class ToolBar;

class QcEXPORT QcMITKSamplePlugin: public QcPlugin {

    Q_OBJECT

public:

    QcMITKSamplePlugin (QWidget *parent);
    ~QcMITKSamplePlugin ();

    QString name(); 
    const char ** xpm();
    
    SampleApp *ap;

public slots :
       
    void selectSerie (QcLightbox*);
    void ConnectWidget(bool );
    virtual void 	lightboxFilled (QcLightbox* lightbox);
    virtual void 	lightboxTiles (QcLightboxManager *lbm, int tiles);
private:

    QcTask *task;
    ToolBar *toolbar ;
    int idLightbox;
    void selectLightbox(int id);
};



#endif