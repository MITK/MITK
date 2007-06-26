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

    /** return the StudyInformation of the current selected study */
    virtual StudyInformation GetCurrentSelectedStudy();
    /** return the list of the series to the current selected study */
    virtual SeriesList GetCurrentSelectedSeries();

    #if ( CHILI_VERSION >= 38 )
    /** return the PatientInformation of the current selected study */
    virtual PatientInformation GetCurrentSelectedPatient();
    /** return a list of all TextFiles
    * - of the current selected series (if the user specify no series_OID)
      - of a specified series (if the user set the parameter) */
    virtual TextFileList GetTextFileInformation( std::string seriesOID = "" );
    #endif

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

    #if ( CHILI_VERSION >= 38 )
    /** with this function you can save a file to the chilidatabase
    * If you want to save the file to the current selected study and series, then set the datatreenode only.
    * If you want to save the file to a specific study and series, then you have to set all parameter. */
    virtual void UploadViaFile( DataTreeNode* node, std::string studyInstanceUID = "", std::string patientOID = "", std::string studyOID = "", std::string seriesOID = "" );

    /** with this function you can load a file from the chilidatabase
    * For the chiliText and MimeType use GetTextFileInformation(). */
    virtual void DownloadViaFile( std::string chiliText, std::string MimeType, DataTreeIteratorBase* parentIterator );

    //UnderConstruction
    virtual void UploadViaBuffer( DataTreeNode* );
    virtual DataTreeNode* DownloadViaBuffer();
    #endif

  public slots:

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

    static ipBool_t GlobalIterateSeriesCallback( int rows, int row, series_t* series, void* user_data );
    static ipBool_t GlobalIterateImagesCallback( int rows, int row, image_t* image, void* user_data );
    #if ( CHILI_VERSION >= 38 )
    static ipBool_t GlobalIterateTextCallback( int rows, int row, text_t *text, void *user_data );
    #endif

    /** teleconference methods */
    virtual void connectPartner();
    virtual void disconnectPartner();

    #if ( CHILI_VERSION >= 38 )
    /** if the study changed, we have to iterate over the series for a new current-selected-study-list */
    bool m_currentStudyChanged;
    /** a list of all series to the current selected Study */
    SeriesList m_CurrentSeries;
    /** a list of all TextFiles from a series */
    TextFileList m_TextFileList;
    /** the count of the shown Lightboxes in chili */
    unsigned int m_LightBoxCount;
    #else
    /** the current selected study */
    StudyInformation m_CurrentStudy;
    /** a list of all series to the current selected Study */
    SeriesList m_CurrentSeries;
    /** the count of the shown Lightboxes in chili */
    unsigned int m_LightBoxCount;
    #endif

  private:

    static QWidget* s_Parent;

    bool m_InImporting;

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
