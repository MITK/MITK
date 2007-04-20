#ifndef QCMITKSAMPLEPLUGIN_H
#define QCMITKSAMPLEPLUGIN_H

#include <chili/plugin.h>
#include "ToolBar.h"
#include <chili/ipUtil.h>
#include <ipMsg/ipMsg.h>
#include <ipMsg/ipMsgTypes.h>
#include <ipPic/ipTypes.h>


class QcMITKTask;
class SampleApp;
class ToolBar;

class QcEXPORT QcMITKSamplePlugin: public QcPlugin
{

    Q_OBJECT

  public:

    QcMITKSamplePlugin(QWidget* parent);
    ~QcMITKSamplePlugin();

    QString name();
    const char** xpm();

    SampleApp* app; // TODO warum? protected machen

    static QcPlugin* GetPluginInstance()
    {
      return s_PluginInstance;
    }

    virtual void handleMessage( ipInt4_t type, ipMsgParaList_t *list );

  public slots:
    
    virtual void studySelected( study_t* );

    // image selection methods
    void selectSerie (QcLightbox*);

    // still undocumented slot of QcPlugin
    virtual void lightboxFilled (QcLightbox* lightbox);
    
    // still undocumented slot of QcPlugin
    virtual void lightboxTiles (QcLightboxManager *lbm, int tiles);

    void CreateNewSampleApp(bool force=false);

  protected:

    // teleconference methods
    virtual void connectPartner();
    virtual void disconnectPartner();

  private:

    // the Chili task object
    QcMITKTask* task;

    // the MITK toolbar for selecting a lightbox (to indicate it should be loaded)
    ToolBar* toolbar;

    bool m_Activated;
    int m_PicCounter;
    bool m_IsFilledDataTree;

    // single instance of this plugin
    static QcPlugin* s_PluginInstance;
};

#endif

