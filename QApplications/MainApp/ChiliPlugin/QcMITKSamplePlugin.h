#ifndef QCMITKSAMPLEPLUGIN_H
#define QCMITKSAMPLEPLUGIN_H

#include <mitkChiliPluginImpl.h>
#include <chili/plugin.h>
#include <chili/ipUtil.h>
#include <ipMsg/ipMsg.h>
#include <ipMsg/ipMsgTypes.h>
#include <ipPic/ipTypes.h>

class QcMITKTask;
class SampleApp;

class QcEXPORT QcMITKSamplePlugin: public QcPlugin
{
  Q_OBJECT

  public:

    QcMITKSamplePlugin(QWidget* parent);
    ~QcMITKSamplePlugin();

    QString name();
    const char** xpm();

    static QcPlugin* GetPluginInstance()
    {
      return s_QmitkPluginInstance;
    }

    virtual void handleMessage( ipInt4_t type, ipMsgParaList_t *list );

    static ipBool_t GlobalIterateSeriesCallback( int rows, int row, series_t* series, void* user_data );

  public slots:

    //Slot from QcPlugin, when study is selected
    virtual void studySelected( study_t* );

    // image selection methods
    void selectSerie (QcLightbox*);

    // still undocumented slot of QcPlugin
    virtual void lightboxFilled (QcLightbox* lightbox);

    // still undocumented slot of QcPlugin
    virtual void lightboxTiles (QcLightboxManager *lbm, int tiles);

  protected:

    // teleconference methods
    virtual void connectPartner();
    virtual void disconnectPartner();

  private:

    SampleApp* app;

    // the Chili task object
    QcMITKTask* task;

    // single instance of this plugin
    static QcPlugin* s_QmitkPluginInstance;

    mitk::ChiliPluginImpl* m_MITKPluginInstance;
};

#endif
