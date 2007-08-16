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

#define NUMBER_OF_THINKABLE_LIGHTBOXES 4

class QcMITKTask;
class SampleApp;
class QIDToolButton;
class Chili3Conference;

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
    \brief return the ConferenceID
    @returns m_QmitkChiliPluginConferenceID
    */
    virtual int GetConferenceID();

    /*!
    \brief Return true if the application run as plugin and false if its standalone.
    @returns True for ChiliPlugin, false for Standalone.
    */
    virtual bool IsPlugin();

    /*!
    \brief Return the studyinformation.
    @param studyOID   If you dont set the input, you get the current selected study. If you want a specific study, set the input.
    @returns The current or specific studyinformation.
    If no study could found, this function return StudyInformation.OID == "".
    */
    virtual StudyInformation GetStudyInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return the patientinformation.
    @param patientOID   If you dont set the input, you get the current selected patient. If you want a specific patient, set the input.
    @returns The current or specific patientinformation.
    If no patient could found, this function return PatientInformation.OID == "".
    */
    virtual PatientInformation GetPatientInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return the seriesinformation.
    @param seriesOID   If you dont set the input, you get the current selected series. If you want a specific series, set the input.
    @returns The current or specific seriesinformation.
    If no series could found, this function return SeriesInformation.OID == "".
    */
    virtual SeriesInformation GetSeriesInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return a list of all seriesinformation from one study.
    @param studyOID   Set the study from wich one you want to know all series.
    @returns The current or specific seriesinformation.
    If no series could found, this function returns an empty list.
    */
    virtual SeriesInformationList GetSeriesInformationList( const std::string& studyOID = "" );

    /*!
    \brief Return the textinformation.
    @param textOID   Set the text from which one you want the information?
    @returns The textinformation.
    If no text could found, this function return TextInformation.OID == "".
    */
    virtual TextInformation GetTextInformation( const std::string& textOID );

    /*!
    \brief Return a list of all textinformation from one series.
    @param seriesOID   Set the series from which one you want to know all texts.
    @returns A list of textinformation from one series.
    If no texts could found, this function returns an empty list.
    */
    virtual TextInformationList GetTextInformationList( const std::string& seriesOID );

    /*!
    \brief Return the number of current shown lightboxes in chili.
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
    \brief Return the current active Lightbox.
    @returns The current lightbox.
    If there is no current lightbox the function return NULL and show a message.
    */
    virtual QcLightbox* GetCurrentLightbox();

    /*!
    \brief Load all images from the given lightbox.
    @param inputLightbox   The lightbox to read. If no lightbox set, the current lightbox get used.
    @returns Multiple mitk::DataTreeNodes as vector.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromLightbox( QcLightbox* inputLightbox = NULL );

    /*!
    \brief Save image into the given Lightbox.
    @param sourceImage   The image to save.
    @param levelWindow   The levelWindow for the image.
    @param seriesOID   The seriesOID from the series where the image should saved. For example use "GetCurrentSelectedSeries()" to get the information. If the image was loaded from chili, the mitk::DataTreeNode have a Property named "SeriesOID", use them here!
    @param nameProperty   The "name"-Property needed if different studies selected. So you can see which Node came from which study.
    @param lightbox   This function use the lightbox to save the single slices. Therefore you have to set a lightbox to show and save the slices. Use "GetCurrentLightbox()" or "GetNewLightbox()".
    */
    virtual void SaveImageToLightbox( Image* sourceImage, const PropertyList::Pointer propertyList, QcLightbox* lightbox );

    /*!
    \brief Load all Image- and Text-Files from the series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    */
    virtual std::vector<DataTreeNode::Pointer> LoadCompleteSeries( const std::string& seriesOID );

    /*!
    \brief Load all Images from the series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    This function save all image-files ( current known: *.pic, *.dcm, *.jpg ) from database to harddisk.
    The *dcm-files get converted to ipPicDescriptor. These and the *pic-files get readed via mitk::PicDescriptorsToNode ( this class returns multiple mitk::images).
    The other files ( current *.jpg ) get readed via mitk::DataTreeNodeFactory (this class return one mitk::image). */
    virtual std::vector<DataTreeNode::Pointer> LoadAllImagesFromSeries( const std::string& seriesOID );

    /*!
    \brief Load all Text-files from the series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
    This function works sequently: it save one text-file to harddisk, read them via factories ( current: mitkImageWriterFactory, mitkPointSetWriterFactory, mitkSurfaceVtkWriterFactory ) to mitk and delete the saved file.
    Important: This function use the filename from database to save the files. In the database it is possible, that a filename is twice in a series ( thats realy two different entries ). So we have to work sequently, otherwise we override the files ( twice filenames ).
    */
    virtual std::vector<DataTreeNode::Pointer> LoadAllTextsFromSeries( const std::string& seriesOID );

    /*!
    \brief Load one Text-files.
    @param seriesOID   Set the series to load from.
    @param textOID   Set the single text.
    @returns one mitk::DataTreeNode
    */
    virtual DataTreeNode::Pointer LoadOneTextFromSeries( const std::string& seriesOID, const std::string& textOID );

    /*!
    \brief Save Images- and Texts-Files to Chili via Fileupload.
    @param inputNodes   Thats the nodes to save.
    */
    virtual void SaveToChili( DataStorage::SetOfObjects::ConstPointer inputNodes );

    mitkClassMacro(ChiliPluginImpl,ChiliPlugin);
    itkNewMacro(ChiliPluginImpl);
    virtual ~ChiliPluginImpl();

  public slots:

    /** called when a lightbox import button is clicked */
    void lightBoxImportButtonClicked(int row);
    /** called when Study is selected */
    virtual void studySelected( study_t* );
    /** called when a new lightbox get visible in chili */
    virtual void lightboxTiles (QcLightboxManager *lbm, int tiles);

  protected slots:

    /** Slot to reinitialize the ChiliPlugin */
    void OnApproachReinitializationOfWholeApplication();

  protected:

    /** REMEMBER: QcPlugin inheritanced protected */

    /** This Class use the QcPlugin-Function "void sendMessage( ipMsgType_t type, ipMsgParaList_t *list )". Its protected because not everybody should send Messages. */
    friend class ::Chili3Conference;

    /** need a QcPluginInstance for "create" */
    friend QObject* ::create( QWidget *parent );

    /** needed to instantiate QcPlugin */
    static QWidget* s_Parent;

    /** set s_Parent; set by QcEXPORT QObject* create( QWidget *parent ) */
    static void SetQtParent( QWidget* parent );

    /** return the QcPluginInstance; used by QcEXPORT QObject* create( QWidget *parent ) */
    static QcPlugin* GetQcPluginInstance();

  private:

    /** the application */
    SampleApp* app;
    /** the task */
    QcMITKTask* task;

    SeriesInformationList m_SeriesInformationList;

    static ipBool_t GlobalIterateSeriesCallback( int rows, int row, series_t* series, void* user_data );

    TextInformationList m_TextInformationList;

#ifdef CHILI_PLUGIN_VERSION_CODE
    static ipBool_t GlobalIterateTextOneCallback( int rows, int row, text_t *text, void *user_data );

    /** function to iterate over all textfiles from a series;
    the db-path and the text-oid get saved;
    dont download the files here, because of double entries;
    its possible to save different text-files under the same filename, if you save here, you override the files on harddisk
    IMPORTANT: works for chili 3.9 or higher only */
    static ipBool_t GlobalIterateTextTwoCallback( int rows, int row, text_t *text, void *user_data );
#endif

    TagInformationList GetAllAvailableStudyAndPatientPicTags( const std::string& seriesOID );

    /** the count of the shown Lightboxes in chili */
    unsigned int m_LightBoxCount;

    /** while importing the lightbox to MITK, it should not be possible to click the importbutton several times (FIX BUG 483) */
    bool m_InImporting;

    /** the current tempDirectory which get used to load and save files for the down - and up - load */
    std::string m_tempDirectory;

    /** its a list of all Filenames, needed to download picdescriptors --> need to know which to load */
    std::list<std::string> m_FileList;

    struct TextFilePathAndOIDStruct
    {
      std::string TextFilePath;
      std::string TextFileOID;
    };
    /** its a list of all TextFiles with Path and OID */
    std::list<TextFilePathAndOIDStruct> m_TextFileList;

    /** a vector of all Tool-/LightboximportButtons */
    std::vector<QIDToolButton*> m_LightBoxImportButtons;

    /** secret button to reinit application */
    QToolButton* m_LightBoxImportToggleButton;

    /** Constuctor */
    ChiliPluginImpl();

    /** return the temporary directory, it is a new directory in the system-specific temp-Directory */
    std::string GetTempDirectory();

    /** function to iterate over all images from a series
    works for chili 3.9 or higher only */
    static ipBool_t GlobalIterateImagesCallback( int rows, int row, image_t* image, void* user_data );

    /** teleconference methods */
    virtual void connectPartner();
    virtual void disconnectPartner();

    /** Invoke event: if a new study selected */
    //void SendStudySelectedEvent();
    /** Invoke event: if the Lightbox count changed */
    void SendLightBoxCountChangedEvent();
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
