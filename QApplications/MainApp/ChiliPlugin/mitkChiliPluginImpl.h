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

// class mitkChiliPluginImpl inherit from
#include <mitkChiliPlugin.h>
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

class ChiliPluginImpl : protected QcPlugin, public ChiliPlugin
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
    \brief Return true if the application run as plugin and false if its standalone.
    @returns True for Chili-Plugin, false for Standalone.
    */
    virtual bool IsPlugin();

    /*!
    \brief Return the studyinformation.
    @param seriesOID   If you dont set the input, you get the current selected study. If you want a specific study, set the input.
    @returns The current or specific studyinformation.
    If no study could found, this function return StudyInformation.OID == "".
    If no parameter set, the current selected study returned.
    */
    virtual StudyInformation GetStudyInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return the patientinformation.
    @param seriesOID   If you dont set the input, you get the current selected patient. If you want a specific patient, set the input.
    @returns The current or specific patientinformation.
    If no patient could found, this function return PatientInformation.OID == "".
    If no parameter set, the current selected patient returned.
    */
    virtual PatientInformation GetPatientInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return the seriesinformation.
    @param seriesOID   If you dont set the input, you get the current selected series. If you want a specific series, set the input.
    @returns The current or specific seriesinformation.
    If no series could found, this function return SeriesInformation.OID == "".
    If no parameter set, the current selected series returned.
    */
    virtual SeriesInformation GetSeriesInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return a list of all seriesinformation.
    @param studyOID   If you dont set the input, you get the seriesList of the current selected study. If you want a list of a specific study, set the input.
    @returns The current or specific seriesinformationlist.
    If no series could found, this function returns an empty list.
    If no parameter set, the seriesList of the current selected study returned.
    */
    virtual SeriesInformationList GetSeriesInformationList( const std::string& studyOID = "" );

    /*!
    \brief Return the textinformation.
    @param textOID   Set the textOID to get the information.
    @returns The textinformation.
    If no text could found, this function return TextInformation.OID == "".
    The parameter have to be set.
    */
    virtual TextInformation GetTextInformation( const std::string& textOID );

    /*!
    \brief Return a list of all textinformation.
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
    \brief Chili save all images as 2D images. While loading, they have to combined to volumes. Therefor actually three different readers available. The first one use the image number and the spacing between two slices. The second one use the slice location and all possible spacing-combinations between the slices. The third one use the most common spacing.
    @param readerType   Actually 0 used for the PicDescriptorToNode(ImageNumber), 1 for the PicDescriptorToNodeSecond(SpacingCombination) and 2 used for PicDescriptorToNodeThird(most common spacing).
    If no parameter set, the PicDescriptorToNode get used.
    */
    virtual void SetReaderType( unsigned int readerType = 0 );

    /*!
    \brief Load all images from the given lightbox.
    @param inputLightbox   The lightbox to read. If no lightbox set, the current lightbox get used.
    @returns Multiple mitk::DataTreeNodes as vector.
    If no parameter set, the current selected Lightbox get used. Chili dont support text-access via lightbox. If you want to load them, use LoadAllTextsFromSeries(...). The slices get combined with the internal set ReaderType.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromLightbox( QcLightbox* inputLightbox = NULL );

    /*!
    \brief Load all Image- and Text-Files from the series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    The parameter have to be set. This function use LoadAllImagesFromSeries(...) and LoadAllTextsFromSeries(...).
    */
    virtual std::vector<DataTreeNode::Pointer> LoadCompleteSeries( const std::string& seriesOID );

    /*!
    \brief Load all Images from the series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    The parameter have to be set. This function load all files via FileDownload from chili. Chili save the files in the same file-format, like they saved on the server. That mean that *.pic or *.dcm are possible. Dicomfiles get transformed to pic. The slices get combined with the internal set ReaderType. Should other file-formats should save, they get load from harddisk with the DataTreeNodeFactory.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadAllImagesFromSeries( const std::string& seriesOID );

    /*!
    \brief Load all Text-files from the series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    Important: Chili combine the filename, the OID, MimeType, ... to create the databasedirectory, so different files can be saved with the same filename. The filename from database is used to save the files. So we have to work sequently, otherwise we override the files ( twice filenames ).
    The function GlobalIterateTextTwoCallback(...) return a list of all textOID's and textPath's from all included text-files in this series.
    With this information LoadOneText( seriesOID, textOID, textPath ) is used step by step.
    The parameter have to be set.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadAllTextsFromSeries( const std::string& seriesOID );

    /*!
    \brief Load one Text-files.
    @param textOID   Set the single text.
    @returns one mitk::DataTreeNode
    To load a single text-file, you need more than the textOID, this function search for the missing attributes with qTextQuery(...) and use LoadOneText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath ).
    The parameter have to be set.
    */
    virtual DataTreeNode::Pointer LoadOneText( const std::string& textOID );

    /*!
    \brief Load one Text-files.
    @param seriesOID   Set the series to load from.
    @param textOID   Set the single text.
    @param textPath   The chili-database-path from the file which should read.
    @returns one mitk::DataTreeNode
    This function load from database. All needed parameter set as input. The file get saved to harddisk, get readed via factory ( current: mitkImageWriterFactory, mitkPointSetWriterFactory, mitkSurfaceVtkWriterFactory ) to mitk and deleted from harddisk.
    The parameter have to be set.
    */
    virtual DataTreeNode::Pointer LoadOneText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath );

    /*!
    \brief Save Images- and Texts-Files to Chili via Fileupload.
    @param inputNodes   Thats the nodes to save.
    This function provides a dialog where the user can decide if he want to create a new series, save to series, override, ... .
    Then SaveAsNewSeries(...) or SaveToSeries(...) get used.
    */
    virtual void SaveToChili( DataStorage::SetOfObjects::ConstPointer inputNodes );

    /*!
    \brief Save Images- and Texts-Files to Chili via Fileupload.
    @param inputNodes   Thats the nodes to save.
    @param studyOID   In which study the new series should created?
    @param seriesNumber   With wich seriesNumber the series should created?
    @param seriesDescription   With wich seriesDescription the series should created?
    @param overrideExistingSeries   Should existing data override ( only possible if the data saved by MBI )?
    This function create a new series and use the function SaveToSeries() . No dialog is used.
    */
    virtual void SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, std::string studyOID, int seriesNumber, std::string seriesDescription, bool overrideExistingSeries );

    /*!
    \brief Save Images- and Texts-Files to Chili via Fileupload.
    @param inputNodes   Thats the nodes to save.
    @param StudyOID   In which study should saved?
    @param SeriesOID   In which series should saved?
    @param overrideExistingSeries   If nodes alway exist in this study, do you want to override them or not ( only possible if the data saved by MBI )?
    This function save the nodes to via FileUpload to chili.
    */
    virtual void SaveToSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, std::string studyOID, std::string seriesOID, bool overrideExistingSeries );

    mitkClassMacro(ChiliPluginImpl,ChiliPlugin);
    itkNewMacro(ChiliPluginImpl);
    virtual ~ChiliPluginImpl();

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
    ChiliPluginImpl();

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

    /** This is a list of TextInformation. This list get filled from GlobalIterateTextOneCallback() and provide all text of one series. */
    TextInformationList m_TextInformationList;

#ifdef CHILI_PLUGIN_VERSION_CODE
    /** Get used to load all text to one series ( GetTextInformationList() ). This function save all found series into m_TextInformationList.*/
    static ipBool_t GlobalIterateTextOneCallback( int rows, int row, text_t *text, void *user_data );

    struct TextFilePathAndOIDStruct
    {
      std::string TextFilePath;
      std::string TextFileOID;
    };
    /** Thats a list of text-file-path and oid. This list used to load text-Files. Its not possible to save all text-files to harddisk and load them. The filename can be the same, then the file on harddisk get override. So we create a list and load one after another. */
    std::list<TextFilePathAndOIDStruct> m_TextFileList;
    /** This function save the text-path and text-oid to the m_TextFileList. This list get used to load textFiles. */
    static ipBool_t GlobalIterateTextTwoCallback( int rows, int row, text_t *text, void *user_data );
#endif

    /** This is a list of SeriesInformation. This list get filled from GlobalIterateSeriesCallback() and provide all series from one study. */
    SeriesInformationList m_SeriesInformationList;
    /** Get used to load all series to one study ( GetSeriesInformationList() ). This function save all found series into m_SeriesInformationList.*/
    static ipBool_t GlobalIterateSeriesCallback( int rows, int row, series_t* series, void* user_data );

    /** Thats a list of all filenames. This list get filled from GlobalIterateImagesCallback(). The list get used to load all image-files from harddisk. */
    std::list<std::string> m_FileList;
    /** This function get used from LoadAllImagesFromSeries() and save Image-Files to harddisk and save the filenames to m_FileList. */
    static ipBool_t GlobalIterateImagesCallbackOne( int rows, int row, image_t* image, void* user_data );

    /** Check the series if a ParentChild-TextFile exist and set m_currentXmlDoc. */
    void CheckCurrentSeriesForRelation( const std::string& seriesOID );
    /** This function add a volume to the xml-file, therefore it check the included one and add only new one. */
    void AddVolumeToParentChild( std::list< std::string > newVolume, DataTreeNode::Pointer node );
    /** This function save the relations between the nodes. */
    void SaveRelationShip( DataStorage::SetOfObjects::ConstPointer inputNodes );

#ifdef CHILI_PLUGIN_VERSION_CODE
static ipBool_t mitk::ChiliPluginImpl::GlobalIterateTextThirdCallback( int rows, int row, text_t *text, void *user_data );
#endif

    /** This variable save the OID of the parent-child-text. */
    std::string m_ParentOID;
    /** Thats the xml-file where the parent-child-realtionship saved. */
    TiXmlDocument* m_currentXmlDoc;

    struct NodeDescriptionStruct
    {
      DataTreeNode::Pointer Node;
      std::string VolumeDescription;
    };
    /** The different nodes get saved to the xml-file under automatic generate indices. For the relationship we need the generated indices. */
    std::list<NodeDescriptionStruct> m_RelationShipHelpList;

//    TextInformationList m_TextList;
//    std::string GetAddedTextOID( const std::string& seriesOID );

    /** This function search all images ( using GlobalIterateImagesCallbackTwo() ) of one series and search the maximum imagenumber. The maximal imagenumber is used to save. We want no double imageNumber. */
    int GetMaximumImageNumber( std::string seriesOID );
    /** This function compare the imageNumbers and set the higher one to m_MaximumImageNumber. */
    static ipBool_t GlobalIterateImagesCallbackTwo( int rows, int row, image_t* image, void* user_data );
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
