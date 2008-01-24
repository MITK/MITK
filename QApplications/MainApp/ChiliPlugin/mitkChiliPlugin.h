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

// class mitkChiliPlugin inherit from
#include <mitkPACSPlugin.h>
#include <chili/plugin.h>
// class QIDToolButton inherit from
#include <qtoolbutton.h>
#include <tinyxml.h>  // xml used to save the parent-child-relationship
#include "mitkImageToPicDescriptor.h"  // define the "TagInformationList"

extern "C" char *dbGetNewOID( void );  //TODO delete if its integrated into the plugin.h --> needed to get the textOID for the parentChildRelationship

class QcMITKTask;
class SampleApp;
class QIDToolButton;
class Chili3Conference;
class QHBoxLayout;

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
    \brief Return true if the application run as plugin and false if the application run as standalone.
    @returns True for Chili-Plugin, false for Standalone.
    */
    virtual bool IsPlugin();

    /*!
    \brief Return all parent-child-saved-volumes to a series.
    @param seriesOID   This parameter have to be set. All volume-entries to this seriesOID get searched.
    @returns All parent-child-saved-volumes with the overgiven seriesOID.
    Return an empty list if no entrie found.
    IMPORTANT: This function dont set the attributes "childLabel" and "parentLabel".
    */
    virtual PSRelationInformationList GetSeriesRelationInformation( const std::string& seriesOID );

    /*!
    \brief Return all parent-child-saved-volumen to a study.
    @param studyOID   The XML-File of the study get loaded and all volume-entries returned. If no OID set, the current selected study get used.
    @returns All parent-child-saved-volumes.
    Return an empty list if no entrie found.
    */
    virtual PSRelationInformationList GetStudyRelationInformation( const std::string& studyOID = "" );

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
    virtual std::vector<DataTreeNode::Pointer> LoadCompleteSeries( const std::string& seriesOID );

    /*!
    \brief Load all Images to a series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    The parameter have to be set. This function load all files via FileDownload from chili. Chili save the files in the same file-format, like they saved on the server. That mean that *.pic or *.dcm are possible. Dicomfiles get transformed to pic. The slices get combined with the internal set ReaderType. Should other file-formats should save, they get load from harddisk with the DataTreeNodeFactory.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadAllImagesFromSeries( const std::string& seriesOID );

    /*!
    \brief Load all Text-entries to a series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    Important: Chili combine the filename, the OID, MimeType, ... to create the databasedirectory, so different files can be saved with the same filename. The filename from database is used to save the files. So we have to work sequently, otherwise we override the files ( twice filenames ).
    The seriesOID have to be set.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadAllTextsFromSeries( const std::string& seriesOID );

    /*!
    \brief Load one Text-files.
    @param textOID   Set the single text.
    @returns one mitk::DataTreeNode
    To load a single text-file, you need more than the textOID, this function search for the missing attributes with qTextQuery(...) and use LoadOneText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath ).
    The textOID have to be set.
    */
    virtual DataTreeNode::Pointer LoadOneText( const std::string& textOID );

    /*!
    \brief Load one Text-files.
    @param seriesOID   Set the series to load from.
    @param textOID   Set the single text.
    @param textPath   The chili-database-path from the file which should read.
    @returns one mitk::DataTreeNode
    This function load from database. The file get saved to harddisk and readed via factory. The file get deleted if it could read.
    All parameter have to be set.
    */
    virtual DataTreeNode::Pointer LoadOneText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath );

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

    /** the application */
    SampleApp* app;

    /** the task */
    QcMITKTask* task;

    /** Constuctor */
    ChiliPlugin();

    /** MITK provides different reader to combine slices to a volume. This variable use to set the reader and use the right one. */
    int m_UseReader;

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

    /** This function return a temporary directory. It is a new directory in the system-specific temp-Directory. Use m_tempDirectory. */
    std::string GetTempDirectory();

    /** This is a list of TextInformation. This list get filled from GlobalIterateTextForCompleteInformation() and provide all text of one series. */
    TextInformationList m_TextInformationList;

#ifdef CHILI_PLUGIN_VERSION_CODE

    struct TextFilePathAndOIDStruct
    {
      std::string TextFilePath;
      std::string TextFileOID;
    };
    /** Thats a list of text-file-path and oid. This list used to load text-Files. Its not possible to save all text-files to harddisk and load them. The filename can be the same, then the file on harddisk get override. So we create a list and load one after another. */
    std::list<TextFilePathAndOIDStruct> m_TextFileList;

    /** This function save the text-path and text-oid to the m_TextFileList. This list get used to load textFiles. */
    static ipBool_t GlobalIterateToLoadAllText( int rows, int row, text_t *text, void *user_data );

    /** Get used to load all text to one series ( GetTextInformationList() ). This function save all found series into m_TextInformationList.*/
    static ipBool_t GlobalIterateTextForCompleteInformation( int rows, int row, text_t *text, void *user_data );

    /** Iterate over all text and search for "ParentChild.xml", the function GetTextInformationList() dont return this one. */
    static ipBool_t GlobalIterateForRelation( int rows, int row, text_t *text, void *user_data );

    /** This function iterate over all text and search the parent-child-xml-file. */
    static ipBool_t GlobalIterateForText( int rows, int row, text_t *text, void *user_data );

#endif

    /** This function iterate over all series and search the parent-child-xml-file. */
    static ipBool_t GlobalIterateForRelation( int /*rows*/, int /*row*/, series_t* series, void* user_data );

    /** This function return the ipPicDescriptors for the image_instance_uids saved at the m_SavedImageInstanceUIDs-list. */
    static ipBool_t GlobalIterateLoadOnlySingleImages( int /*rows*/, int row, image_t* image, void* user_data );

    /** This is a list of SeriesInformation. This list get filled from GlobalIterateSeriesForCompleteInformation() and provide all series from one study. */
    SeriesInformationList m_SeriesInformationList;

    /** Get used to load all series to one study ( GetSeriesInformationList() ). This function save all found series into m_SeriesInformationList.*/
    static ipBool_t GlobalIterateSeriesForCompleteInformation( int rows, int row, series_t* series, void* user_data );

    /** This function get used from LoadAllImagesFromSeries() and save Image-Files to harddisk and save the filenames to m_FileList. */
    static ipBool_t GlobalIterateLoadImage( int rows, int row, image_t* image, void* user_data );

    /** This function compare the imageNumbers and set the higher one to m_MaximumImageNumber. */
    static ipBool_t GlobalIterateImagesForMaximalImageNumber( int rows, int row, image_t* image, void* user_data );

    /** This list get used to load image-files. The Plugin handle *.pic and *.dcm. All other image-formats get saved to this list and try to load via DataTreeNodeFactory. */
    std::list<std::string> unknownImageFormatPath;

    /** This list get used to load image-files. The Plugin handle *.pic and *.dcm. *.dcm get converted to *.pic. */
    std::list<ipPicDescriptor*> m_ImageList;

    /** This list get used to save image_instance_uids. With this list the fitting ipPicDescriptors get load. */
    std::list<std::string> m_SavedImageInstanceUIDs;

    /** This struct get used to check if the saved relations create circles. */
    struct CircleCheckStruct
    {
      std::string VolumeLabel;
      int ParentCount;
      std::list<std::string> ChildList;
    };
    std::list<CircleCheckStruct> m_CircleCheckStructure;

    /** The XML-File */
    TiXmlDocument* m_currentXmlDoc;

    /** This variable save the seriesoid of the parent-child-text. */
    std::string m_ParentSeriesOID;

    /** This variable save the textoid of the parent-child-text. */
    std::string m_ParentTextOID;

    /** This function return the volume-label. Therefore the saved volumes get checked with the overgiven image_instance_UIDs. */
    std::string CheckForVolumeLabel( std::list< std::string > ImageInstanceUIDs );

    /** This function init the xml-file for the overgiven study. */
    bool InitParentChild( const std::string& studyOID );

    /** This function add a new volume-entry for the overgiven nodes. */
    void AddNewEntryToXML( DataTreeNode::Pointer node, std::list< std::string > CurrentImageInstanceUIDs, const std::string& seriesOID );

    /** This function initialize the parent-child-struct to test if new relations create circle. */
    void InitCircleCheckStructure();

    /** This function save the relations between the nodes. */
    void SaveRelationShip( std::list<DataTreeNode::Pointer> inputList, const std::string& seriesOID );

    /** This function save a single relation to the xml-file. Therefore all existing relations get check for circles. */
    bool SaveSingleRelation( const std::string& childVolumeLabel, const std::string& parentVolumeLabel );

    /** This function search all images ( using GlobalIterateImagesForMaximalImageNumber() ) of one series and search the maximum imagenumber. The maximal imagenumber is used to save. We want no double imageNumber. */
    int GetMaximumImageNumber( const std::string& seriesOID );

    /** The maximum imageNumber from a series. */
    int m_MaximumImageNumber;

    /** Generate the TagList from the given study, series and patient. Therefore only specific tags get used. This taglist used as input for class mitk::ImageToPicDescriptor. The taglist is needed to create new series or add to an existing one. */
    ImageToPicDescriptor::TagInformationList GetNeededTagList( study_t* study, patient_t* patient, series_t* series );

    /** teleconference methods */
    virtual void connectPartner();
    virtual void disconnectPartner();

    /** Invoke event: if a new study selected */
    void SendStudySelectedEvent();

    /** Invoke event: if the Lightbox count changed */
    void SendLightBoxCountChangedEvent();

    QObject* findProgressBar(QObject* object);
    bool ChiliFillingLightbox();

    QHBoxLayout* horzlayout;
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
