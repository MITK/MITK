#ifndef QCMITKSAMPLEPLUGIN_H
#define QCMITKSAMPLEPLUGIN_H

#include <mitkChiliPluginImpl.h>
#include <chili/plugin.h>
#include <chili/ipUtil.h>
#include <ipMsg/ipMsg.h>
#include <ipMsg/ipMsgTypes.h>
#include <ipPic/ipTypes.h>

#include <qtoolbutton.h>

#include <vector>

class QcMITKTask;
class SampleApp;
class QIDToolButton;

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
    
    // called when a lightbox import button is clicked
    void lightBoxImportButtonClicked(int row);
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

    std::vector<QIDToolButton*> m_LightBoxImportButtons;
    QToolButton* m_LightBoxImportToggleButton;
};

class QIDToolButton : public QToolButton
{

  Q_OBJECT

  public:

    QIDToolButton( int id, QWidget* parent = NULL, const char* name = NULL )
    : QToolButton(parent, name),
      m_ID(id)
    {
    }

    virtual ~QIDToolButton()
    {
    }

  signals:
    void clicked(int);

  protected:

    virtual void mouseReleaseEvent ( QMouseEvent* e )
    {
      QToolButton::mouseReleaseEvent(e);
      emit clicked(m_ID);
    }

    int m_ID;
};


#endif
