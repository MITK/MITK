#ifndef QCMITKSAMPLEPLUGIN_H
#define QCMITKSAMPLEPLUGIN_H

#include <chili/plugin.h>
class QcTask;

class QcEXPORT QcMITKSamplePlugin: public QcPlugin {

    Q_OBJECT

public:

    QcMITKSamplePlugin (QWidget *parent);
    ~QcMITKSamplePlugin ();

    QString name(); 
    const char ** xpm();

private:

    QcTask *task;

};



#endif