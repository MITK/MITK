/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKCHILIPLUGINIMPL_H_HEADER_INCLUDED
#define MITKCHILIPLUGINIMPL_H_HEADER_INCLUDED

#include <mitkChiliPlugin.h>

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

namespace mitk {

  /**Documentation
  @brief interface between Chili and MITK
  @ingroup Process
  @ingroup Chili
  */

class ChiliPluginImpl : public ::QcPlugin, public ChiliPlugin
{
  Q_OBJECT

  public:

    QString name();
    const char** xpm();

    virtual void handleMessage( ipInt4_t type, ipMsgParaList_t *list );

    static ipBool_t GlobalIterateSeriesCallback( int rows, int row, series_t* series, void* user_data );

    /** return the StudyInformation of the current selected study */
    virtual StudyInformation GetCurrentSelectedStudy();
    /** return the list of the series to the current selected study */
    virtual SeriesList GetCurrentSelectedSeries();
    /** return the PatientInformation of the current selected study */
    virtual unsigned int GetLightBoxCount();

    /** return if the application run as standalone or as chiliplugin */
    virtual bool IsPlugin();
    /** return the conferenceid */
    virtual int GetConferenceID();

    /** Set the properties from the list to the datatreenode. The description of the properties get the prefix "Chili: ". */
    virtual void AddPropertyListToNode( const mitk::PropertyList::Pointer property, mitk::DataTreeNode* );

    /** Set the QtParent, which is needed to instantiate QcPlugin */
    static void SetQtParent( QWidget* parent );

    mitkClassMacro(ChiliPluginImpl,ChiliPlugin);
    itkNewMacro(ChiliPluginImpl);
    virtual ~ChiliPluginImpl();

  public slots:

    // image selection methods
    //void selectSerie( QcLightbox* );

    /** called when a lightbox import button is clicked */
    void lightBoxImportButtonClicked(int row);

    /** Slot from QcPlugin: */
    /** When Study is selected */
    virtual void studySelected( study_t* );

    /**  still undocumented */
    virtual void lightboxFilled( QcLightbox* lightbox );

    /** still undocumented */
    virtual void lightboxTiles (QcLightboxManager *lbm, int tiles);

  protected:

    ChiliPluginImpl::ChiliPluginImpl();

    /** Invoke events: */
    /** if a new study selected */
    void SendStudySelectedEvent();
    /** if the Lightbox count changed */
    void SendLightBoxCountChangedEvent();

    /** teleconference methods */
    virtual void connectPartner();
    virtual void disconnectPartner();

    StudyInformation m_CurrentStudy;
    SeriesList m_CurrentSeries;
    unsigned int m_LightBoxCount;

//    bool m_currentStudyChanged;
//    /** a list of all series to the current selected Study */
//    SeriesList m_CurrentSeries;
//    /** the count of the shown Lightboxes in chili */
//    unsigned int m_LightBoxCount;

  private:

    static QWidget* s_Parent;

    SampleApp* app;

    QcMITKTask* task;

    std::vector<QIDToolButton*> m_LightBoxImportButtons;
    QToolButton* m_LightBoxImportToggleButton;
};

} // namespace mitk

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

#endif /* MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD */
