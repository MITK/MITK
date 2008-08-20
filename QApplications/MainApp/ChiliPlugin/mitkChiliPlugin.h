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

// class QIDToolButton inherit from
#include <qtoolbutton.h>

#include <mitkIpPicUnmangle.h>
// class mitkChiliPlugin inherit from
#include <chili/plugin.h>

//MITK
#include <mitkIpPic.h>
#include <mitkPACSPlugin.h>
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

  /**Documentation
  @brief interface between Chili and MITK
  @ingroup Process
  @ingroup Chili
  */

class ChiliPlugin : protected QcPlugin, public PACSPlugin
{
  Q_OBJECT

  public:

    QString name();
    const char** xpm();

    virtual void handleMessage( ipInt4_t type, ipMsgParaList_t *list );

    /*!
    \brief Return the ConferenceID.
    @returns m_QmitkChiliPluginConferenceID
    */
    virtual int GetConferenceID();

    /*!
    \brief This function return the Plugin-Capabilities like canSave, canWrite or isPlugin.
    @returns Return the current CHILI-Plugin-Capabilities.
    */
    virtual PACSPluginCapability GetPluginCapabilities();

    /*!
    \brief Return all parent-child-saved-volumes to a series.
    @param seriesOID   This parameter have to be set. All volume-entries to this seriesOID get searched.
    @returns All parent-child-saved-volumes with the overgiven seriesOID.
    Return an empty list if no entrie found.
    IMPORTANT: This function dont set the attributes "childLabel" and "parentLabel".
    */
    virtual ParentChildRelationInformationList GetSeriesRelationInformation( const std::string& seriesOID );

    /*!
    \brief Return all parent-child-saved-volumen to a study.
    @param studyOID   The XML-File of the study get loaded and all volume-entries returned. If no OID set, the current selected study get used.
    @returns All parent-child-saved-volumes.
    Return an empty list if no entrie found.
    */
    virtual ParentChildRelationInformationList GetStudyRelationInformation( const std::string& studyOID = "" );

    /*!
    \brief Return the studyinformation to a series.
    @param seriesOID   If no OID set, the current selected study get used.
    @returns The current or specific studyinformation.
    Return StudyInformation.OID == "" if no entry found.
    */
    virtual StudyInformation GetStudyInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return the patientinformation to a series.
    @param seriesOID   If no OID set, the current selected series get used.
    @returns The current or specific patientinformation.
    Return PatientInformation.OID == "" if no entry found.
    */
    virtual PatientInformation GetPatientInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return the seriesinformation to a series.
    @param seriesOID   If no OID set, the current selected series get used.
    @returns The current or specific seriesinformation.
    Return SeriesInformation.OID == "" if no entry found.
    */
    virtual SeriesInformation GetSeriesInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return a list of all seriesinformation to a study.
    @param studyOID   If no OID set, the current selected study get used.
    @returns The current or specific seriesinformationlist.
    Return an empty list if no entry found.
    */
    virtual SeriesInformationList GetSeriesInformationList( const std::string& studyOID = "" );

    /*!
    \brief Return the textinformation to the overgiven textOID.
    @param textOID   This parameter have to be set.
    @returns The textinformation.
    Return TextInformation.OID == "" if no entry found.
    */
    virtual TextInformation GetTextInformation( const std::string& textOID );

    /*!
    \brief Return a list of all textinformation to a series.
    @param seriesOID   Set the seriesOID to get the textList.
    @returns A list of textinformation from one series.
    If no texts could found, this function returns an empty list.
    The parameter have to be set.
    This function dont return the text which used to save and load the parent-child-relationship.
    */
    virtual TextInformationList GetTextInformationList( const std::string& seriesOID );

    /*!
    \brief Return the number of current shown lightboxes.
    @returns The visible lightboxcount.
    */
    virtual unsigned int GetLightboxCount();

    /*!
    \brief Return the first empty Lightbox.
    @returns An empty lightbox.
    There are only four available lightboxes in chili, if all of them not empty the function return NULL and show a message.
    */
    virtual QcLightbox* GetNewLightbox();

    /*!
    \brief Return the current selected Lightbox.
    @returns The current lightbox.
    If there is no current lightbox the function return NULL and show a message.
    */
    virtual QcLightbox* GetCurrentLightbox();

    /*!
    \brief Chili save all images as 2D images. While loading, they have to combined to volumes. Therefor actually three different readers available. The first one use the image number and the spacing between two slices.The second one use the most common spacing. The third one use the slice location and all possible spacing-combinations between the slices. Use this function to set the readers.
    @param readerType   Actually 0 used for the ImageNumberFilter, 1 for the SingleSpacingFilter and 2 used for SpacingSetFilter.
    If no parameter set, the ImageNumberFilter get used.
    */
    virtual void SetReaderType( unsigned int readerType = 0 );

    virtual void SendAbortFilterEvent();

    /*!
    \brief This function load a single parent-child-volume.
    @param seriesOID   This parameter specify the parent-child-volume.
    @param label   This parameter specify the parent-child-volume.
    @returns The searched volume as DataTreeNode.
    Return NULL if no entry found.
    */
    virtual DataTreeNode::Pointer LoadParentChildElement( const std::string& seriesOID, const std::string& label );

    /*!
    \brief Load all images from the given lightbox.
    @param inputLightbox   The lightbox to read. If no lightbox set, the current lightbox get used.
    @returns Multiple mitk::DataTreeNodes as vector.
    If no parameter set, the current selected Lightbox get used. Chili dont support text-access via lightbox. If you want to load them, use LoadAllTextsFromSeries(...). The slices get combined with the internal set ReaderType.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromLightbox( QcLightbox* inputLightbox = NULL );

    /*!
    \brief Load all Image- and Text-Files to a series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    The seriesOID have to be set. This function use LoadAllImagesFromSeries(...) and LoadAllTextsFromSeries(...).
    */
    virtual std::vector<DataTreeNode::Pointer> LoadFromSeries( const std::string& seriesOID );

    /*!
    \brief Load all Images to a series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    The parameter have to be set. This function load all files via FileDownload from chili. Chili save the files in the same file-format, like they saved on the server. That mean that *.pic or *.dcm are possible. Dicomfiles get transformed to pic. The slices get combined with the internal set ReaderType. Should other file-formats should save, they get load from harddisk with the DataTreeNodeFactory.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromSeries( const std::string& seriesOID );

    /*!
    \brief Load all Text-entries to a series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    Important: Chili combine the filename, the OID, MimeType, ... to create the databasedirectory, so different files can be saved with the same filename. The filename from database is used to save the files. So we have to work sequently, otherwise we override the files ( twice filenames ).
    The seriesOID have to be set.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadTextsFromSeries( const std::string& seriesOID );

    /*!
    \brief Load one Text-files.
    @param textOID   Set the single text.
    @returns one mitk::DataTreeNode
    To load a single text-file, you need more than the textOID, this function search for the missing attributes with qTextQuery(...) and use LoadOneText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath ).
    The textOID have to be set.
    */
    virtual DataTreeNode::Pointer LoadSingleText( const std::string& textOID );

    /*!
    \brief Load one Text-files.
    @param seriesOID   Set the series to load from.
    @param textOID   Set the single text.
    @param textPath   The chili-database-path from the file which should read.
    @returns one mitk::DataTreeNode
    This function load from database. The file get saved to harddisk and readed via factory. The file get deleted if it could read.
    All parameter have to be set.
    */
    virtual DataTreeNode::Pointer LoadSingleText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath );

    /*!
    \brief The result of all Load-Functions are DataTreeNodes. If you want to add them to the DataStorage with relations, use this function.
    @param inputNodes   The DataTreeNodes to save.
    */
    virtual void SetRelationsToDataStorage( std::vector<DataTreeNode::Pointer> inputNodes );

    /*!
    \brief Save Images- and Texts-Files with User-Dialog to Chili via Fileupload.
    @param inputNodes   The nodes to save.
    This function provides a dialog where the user can select if he want to create a new series, save to series, override, ...
    The nodes have to be set.
    */
    virtual void SaveToChili( DataStorage::SetOfObjects::ConstPointer inputNodes );

    /*!
    \brief Save Images- and Texts-Files without User-Dialog as new series to Chili.
    @param inputNodes   The nodes to save.
    @param studyOID   The aim-study where the new series should created.
    @param seriesNumber   The seriesNumber for the new series.
    @param seriesDescription   The seriesDescription for the new series.
    This function create a new series and save all nodes. No dialog is used.
    */
    virtual void SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& studyOID, int seriesNumber, const std::string& seriesDescription );

    /*!
    \brief Save Images- and Texts-Files without User-Dialog to a series.
    @param inputNodes   The nodes to save.
    @param StudyOID   The aim-study where the nodes should saved.
    @param SeriesOID   The aim-series where the nodes should saved.
    @param overrideExistingSeries   Its possible that the nodes to save always exist. Set true if you want to override existing data.
    Only data which are saved by MBI can be override.
    */
    virtual void SaveToSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& seriesOID, bool overrideExistingSeries );

    mitkClassMacro(ChiliPlugin,PACSPlugin);
    itkNewMacro(ChiliPlugin);
    virtual ~ChiliPlugin();

  public slots:

    /** Called when a lightbox import button is clicked. */
    void lightBoxImportButtonClicked(int row);

    /** Called when Study is selected. */
    virtual void studySelected( study_t* );

    /** Called when a new lightbox get visible in chili. */
    virtual void lightboxTiles (QcLightboxManager *lbm, int tiles);

  protected slots:

    /** Slot to reinitialize the ChiliPlugin. */
    void OnApproachReinitializationOfWholeApplication();

    /** Slot to create the SampleApp. */
    void CreateSampleApp();

    void OnMenuSeparateByAcquisitionNumberClicked();
    void OnMenuImportFilterSelected(int id);
   
  protected:

    /** REMEMBER: QcPlugin inheritanced protected */

    /** This Class use the QcPlugin-Function "void sendMessage( ipMsgType_t type, ipMsgParaList_t *list )". Its protected because not everybody should send Messages. */
    friend class ::Chili3Conference;

    /** Need a QcPluginInstance for "create". */
    friend QObject* ::create( QWidget *parent );

    /** Needed to instantiate QcPlugin. */
    static QWidget* s_Parent;

    /** Set s_Parent; set by QcEXPORT QObject* create( QWidget *parent ). */
    static void SetQtParent( QWidget* parent );

    /** Return the QcPluginInstance; used by QcEXPORT QObject* create( QWidget *parent ). */
    static QcPlugin* GetQcPluginInstance();

  private:

    CHILIInformation::Pointer m_ChiliInformation;
    LoadFromCHILI::Pointer m_LoadFromCHILI;
    SaveToCHILI::Pointer m_SaveToCHILI;

    /** the application */
    SampleApp* app;

    /** the task */
    QcMITKTask* task;

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

    QHBoxLayout* horzlayout;

    /** Constuctor */
    ChiliPlugin();

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

    QPopupMenu* m_PopupMenu;
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
