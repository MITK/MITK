#ifndef QCMITKSAMPLEPLUGIN_H
#define QCMITKSAMPLEPLUGIN_H

#include <chili/plugin.h>
class QcTask;
class SampleApp;

class QcEXPORT QcMITKSamplePlugin: public QcPlugin {

    Q_OBJECT

public:

    QcMITKSamplePlugin (QWidget *parent);
    ~QcMITKSamplePlugin ();

    QString name(); 
    const char ** xpm();

    virtual void 	lightboxFilled (QcLightbox* lightbox);

    SampleApp *ap;

public slots :
    
    void selectSerie (QcLightbox* lightbox);

private:

    QcTask *task;

};



#endif