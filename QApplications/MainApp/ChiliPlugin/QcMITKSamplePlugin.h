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

    static QcPlugin* GetPluginInstance()
    {
      return s_PluginInstance;
    }

public slots :
       
    void selectSerie (QcLightbox*);
    virtual void 	lightboxFilled (QcLightbox* lightbox);
    virtual void 	lightboxTiles (QcLightboxManager *lbm, int tiles);
    void CreateNewSampleApp();
private:

    QcTask *task;
    ToolBar *toolbar ;
    bool activated;

    static QcPlugin* s_PluginInstance;
};



#endif
