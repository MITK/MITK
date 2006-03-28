#ifndef QCMITKSAMPLEPLUGIN_H
#define QCMITKSAMPLEPLUGIN_H

#include <chili/plugin.h>
#include "ToolBar.h"
#include <chili/ipUtil.h>
#include <ipMsg/ipMsg.h>
#include <ipMsg/ipMsgTypes.h>
#include <ipPic/ipTypes.h>

#include <qevent.h>
#include "QmitkEventCollector.h"

class QcMITKTask;
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

    virtual void handleMessage( ipInt4_t type, ipMsgParaList_t *list );

public slots :

    void selectSerie (QcLightbox*);
    virtual void        lightboxFilled (QcLightbox* lightbox);
    virtual void        lightboxTiles (QcLightboxManager *lbm, int tiles);
    void CreateNewSampleApp();

protected:
    virtual void connectPartner();
    virtual void disconnectPartner();

private:

    QcMITKTask *task;
    ToolBar *toolbar ;
    bool activated;

    static QcPlugin* s_PluginInstance;
};



#endif
