/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITKCHILIPLUGIN_H_HEADER_INCLUDED
#define MITKCHILIPLUGIN_H_HEADER_INCLUDED

#include <qtoolbutton.h>

#include "mitkIpPicUnmangle.h"
#include <chili/plugin.h>

#include "mitkIpPic.h"
#include "mitkPACSPlugin.h"
#include "mitkCHILIInformation.h"
#include "mitkLoadFromCHILI.h"
#include "mitkSaveToCHILI.h"


class QcMITKTask;
class QHBoxLayout;
class QIDToolButton;
class QPopupMenu;

class Chili3Conference;
class SampleApp;

namespace mitk {

const int m_QmitkChiliPluginConferenceID = 5001;

typedef enum
{
  MITKc = 0,
  QTc,
  LAUNCHc,
  ARRANGEc,
  TOKENREQUESTc,
  TOKENSETc,
  TEXTc,
  MOUSEMOVEc ,
} ConfMsgType;


/**Documentation
 * \brief Plugin class for the CHILI PACS Workstation
 *
 * This class implements QcPlugin as expected by the CHILI PACS Workstation
 * AND it implements mitk::PACSPlugin, defining how this interface is implemented for CHILI.
 *
 * \ingroup IO
 * \ingroup Chili
 */
class ChiliPlugin : protected QcPlugin, public PACSPlugin
{
  Q_OBJECT

  public:

    QString name();
    const char** xpm();

    virtual void handleMessage( ipInt4_t type, ipMsgParaList_t *list );

    /*!
     * \brief Return the ConferenceID.
     * \returns m_QmitkChiliPluginConferenceID
     */
    virtual int GetConferenceID();

    virtual PACSPluginCapability GetPluginCapabilities();
    
    virtual PatientInformationList GetPatientInformationList();
    
    virtual StudyInformationList GetStudyInformationList( const PatientInformation& patient );

    virtual SeriesInformationList GetSeriesInformationList( const std::string& studyInstanceUID = "" );
    
    virtual DocumentInformationList GetDocumentInformationList( const std::string& seriesInstanceUID = "" );

    virtual PatientInformation GetPatientInformation( const std::string& seriesInstanceUID = "" );
    
    virtual StudyInformation GetStudyInformation( const std::string& seriesInstanceUID = "" );

    virtual SeriesInformation GetSeriesInformation( const std::string& seriesInstanceUID = "" );

    virtual DocumentInformation GetDocumentInformation( const std::string& seriesInstanceUID, 
                                                        unsigned int instanceNumber );

    virtual unsigned int GetLightboxCount();

    virtual unsigned int GetActiveLightbox();

    virtual void SetReaderType( unsigned int readerType = 0 );

    virtual void AbortPACSImport();

    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromLightbox( unsigned int lightboxIndex = 0 );
    
    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromSeries( const std::string& seriesInstanceUID );

    virtual std::vector<mitk::DataTreeNode::Pointer> LoadImagesFromSeries( std::vector<std::string> seriesInstanceUIDs );

    virtual std::vector<DataTreeNode::Pointer> LoadTextsFromSeries( const std::string& seriesInstanceUID );
    
    virtual DataTreeNode::Pointer LoadSingleText( const std::string& seriesInstanceUID, unsigned int instanceNumber );
    virtual void DownloadSingleFile( const std::string& seriesInstanceUID, 
                                     unsigned int instanceNumber, 
                                     const std::string& filename );

    virtual void SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, 
                                  const std::string& studyInstanceUID, 
                                  int seriesNumber, 
                                  const std::string& seriesDescription );

    virtual void SaveToSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, 
                               const std::string& seriesInstanceUID, 
                               bool ovewriteExistingSeries );

    virtual void UploadFileAsNewSeries( const std::string& filename,
                                        const std::string& mimeType,
                                        const std::string& studyInstanceUID, 
                                        int seriesNumber, 
                                        const std::string& seriesDescription );

    virtual void UploadFileToSeries( const std::string& filename,
                                     const std::string& filebasename, 
                                     const std::string& mimeType,
                                     const std::string& seriesInstanceUID, 
                                     bool overwriteExistingSeries );


    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromLightbox( QcLightbox* inputLightbox = NULL );

    mitkClassMacro(ChiliPlugin,PACSPlugin);
    itkNewMacro(ChiliPlugin);

  protected slots:

    /** Called when a lightbox import button is clicked. */
    void lightBoxImportButtonClicked(int row);

    /** Called when Study is selected. */
    virtual void studySelected( study_t* );

    /** Called when a new lightbox get visible in chili. */
    virtual void lightboxTiles (QcLightboxManager *lbm, int tiles);
    
    /** Called when a new lightbox in chili is filled */
    virtual void lightboxFilled( QcLightbox * );

  protected slots:

    /** Slot to reinitialize the ChiliPlugin. */
    void ReinitMITKApplication();

    /** Slot to create the SampleApp. */
    void CreateSampleApp();

    void OnMenuSeparateByAcquisitionNumberClicked();
    void OnMenuImportFilterSelected(int id);
   
  protected:

    ChiliPlugin();
    virtual ~ChiliPlugin();
    
    virtual void SetRelationsToDataStorage( std::vector<DataTreeNode::Pointer> inputNodes );

    /** REMEMBER: QcPlugin inheritanced protected */

    /** This class uses the QcPlugin-Function 
     *  "void sendMessage( ipMsgType_t type, ipMsgParaList_t *list )". 
     *  It is protected because not everybody should send messages. 
     */
    friend class ::Chili3Conference;

    /** Need a QcPluginInstance for "create". */
    friend QObject* ::create( QWidget *parent );

    /** Needed to instantiate QcPlugin. */
    static QWidget* s_Parent;

    /** Set s_Parent; set by QcEXPORT QObject* create( QWidget *parent ). */
    static void SetQtParent( QWidget* parent );

    /** Return the QcPluginInstance; used by QcEXPORT QObject* create( QWidget *parent ). */
    static QcPlugin* GetQcPluginInstance();
    
    static ChiliPlugin* GetChiliPluginInstance();

    typedef std::map<std::string, std::string> UID2OIDMap;
    UID2OIDMap m_StudyOIDForInstanceUID;
    UID2OIDMap m_SeriesOIDForInstanceUID;

    typedef std::map<std::pair<std::string, unsigned int>, std::string> UIDAndInstanceNumber2OIDMap;
    UIDAndInstanceNumber2OIDMap m_TextOIDForSeriesInstanceUIDAndInstanceNumber;
    
    const std::string GetStudyOIDFromInstanceUID( const std::string& studyInstanceUID );
    void UpdateStudyOIDForInstanceUID( const std::string& studyOID, const std::string& studyInstanceUID );

    const std::string GetSeriesOIDFromInstanceUID( const std::string& seriesInstanceUID );
    void UpdateSeriesOIDForInstanceUID( const std::string& seriesOID, const std::string& seriesInstanceUID );

    const std::string GetTextOIDFromSeriesInstanceUIDAndInstanceNumber( const std::string& seriesInstanceUID, 
                                                                        unsigned int textInstanceNumber );
    void UpdateTextOIDFromSeriesInstanceUIDAndInstanceNumber( const std::string& textOID,
                                                              const std::string& seriesInstanceUID, 
                                                              unsigned int textInstanceNumber );

    friend class CHILIInformation;
    friend class LoadFromCHILI;
    friend class SaveToCHILI;

  private:

    CHILIInformation::Pointer m_ChiliInformation;
    LoadFromCHILI::Pointer m_LoadFromCHILI;
    SaveToCHILI::Pointer m_SaveToCHILI;

    /** the application */
    SampleApp* m_MITKMainApp;

    /** the task */
    QcMITKTask* m_Task;

    /** A vector of all Tool-/LightboximportButtons. */
    std::vector<QIDToolButton*> m_LightBoxImportButtons;

    /** Secret button to reinit application. */
    QToolButton* m_LightBoxImportToggleButton;

    /** The count of the shown Lightboxes in Chili. */
    unsigned int m_LightBoxCount;

    /** While importing the lightbox to MITK, it should not be possible to click the importbutton several times (FIX BUG 483). */
    bool m_InImporting;

    /** The current tempDirectory which get used to load and save files for the Down - and Upload. Get set by GetTempDirectory(). */
    std::string m_tempDirectory;

    QHBoxLayout* m_ImportButtonLayout;

    QPopupMenu* m_PopupMenu;

    std::vector<QObject*> m_ObjectsConnectedTo;

    /** This function return a temporary directory. It is a new directory in the system-specific temp-Directory. Use m_tempDirectory. */
    std::string GetTempDirectory();

    /** teleconference methods */
    virtual void connectPartner();
    virtual void disconnectPartner();

    /** Invoke event: if a new study selected */
    void SendStudySelectedEvent();

    /** Invoke event: if the Lightbox count changed */
    void SendLightBoxCountChangedEvent();

    QObject* findProgressBar(QObject* object);
    bool ChiliFillingLightbox();
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

#endif

