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

#ifndef MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD
#define MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD

#include <itkObject.h>

#include <mitkDataTree.h>
#include <mitkDataStorage.h>

#define NUMBER_OF_THINKABLE_LIGHTBOXES 4

class QcPlugin;
class QcLightbox;
class interSliceGeometry_t;

namespace mitk {

//TODO get used like this "mitk::m_QmitkChiliPluginConferenceID" in QmitkChili3ConferenceKit, not like this "GetConferenceID()"
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

  /** Documentation
  @brief interface between Chili and MITK
  @ingroup IO
  @ingroup Chili

  This is the default-implementation, for the real implementation look at mitkChiliPluginImpl.
  */
class PACSPlugin : public itk::Object
{
  public:

    /** This struct contain all possible informations about the studies. */
    struct StudyInformation
    {
      std::string OID;
      std::string InstanceUID;
      std::string ID;
      std::string Date;
      std::string Time;
      std::string Modality;
      std::string Manufacturer;
      std::string ReferingPhysician;
      std::string Description;
      std::string ManufacturersModelName;
      std::string AccessionNumber;
      std::string InstitutionName;
      std::string PerformingPhysician;
      std::string ReportingPhysician;
      std::string LastAccess;
      int ImageCount;
    };

    /** This struct contain all possible informations about the patient. */
    struct PatientInformation
    {
      std::string OID;
      std::string Name;
      std::string ID;
      std::string BirthDate;
      std::string BirthTime;
      std::string Sex;
      std::string MedicalRecordLocator;
      std::string Comment;
    };

    /** This struct contain all possible informations about the series. */
    struct SeriesInformation
    {
      std::string OID;
      std::string InstanceUID;
      int Number;
      int Acquisition;
      int EchoNumber;
      int TemporalPosition;
      std::string Date;
      std::string Time;
      std::string Description;
      std::string Contrast;
      std::string BodyPartExamined;
      std::string ScanningSequence;
      std::string FrameOfReferenceUID;
      int ImageCount;
    };

    /** There can be lots of series to one study, so we need a list. */
    typedef std::list<SeriesInformation> SeriesInformationList;

    /** This struct contain all possible informations about the TextFile (all other then PIC-Files). */
    struct TextInformation
    {
      std::string OID;
      std::string MimeType;
      std::string ChiliText;
      std::string Status;
      std::string FrameOfReferenceUID;
      std::string TextDate;
      std::string Description;
    };

    /** There can be lots of texts to one series, so we need a list. */
    typedef std::list<TextInformation> TextInformationList;

    /*!
    \brief Return a mitk::PACSPlugin-Instance as singleton.
    @param destroyInstance   Needed because of double inheritance of mitk::ChiliPlugin, both want to destroy the instance.
    @returns The singleton PACSPlugin* - Instance.
    */
    static PACSPlugin* GetInstance( bool destroyInstance = false );

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
    \brief Chili save all images as 2D images. While loading, they have to combined to volumes. Therefor actually three different readers available. The first one use the image number and the spacing between two slices.The second one use the most common spacing. The third one use the slice location and all possible spacing-combinations between the slices. 
    @param readerType   Actually 0 used for the ImageNumberFilter, 1 for the SingleSpacingFilter and 2 used for SpacingSetFilter.
    If no parameter set, the ImageNumberFilter get used.
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
    The function GlobalIterateToLoadAllText(...) return a list of all textOID's and textPath's from all included text-files in this series.
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
    \brief This function load the saved nodes and the relation between.
    @param seriesOID   Set the series to load from.
    This function load the saved nodes and the relation between them. Therefore the seriesOID is needed and have to be set. The function return no nodes, the function add the nodes automatically to the datastorage.
    */
    virtual void LoadParentChildRelation( const std::string& seriesOID );

    /*!
    \brief Save Images- and Texts-Files with User-Dialog to Chili via Fileupload.
    @param inputNodes   Thats the nodes to save.
    This function provides a dialog where the user can select if he want to create a new series, save to series, override, ... and the series to save.
    SaveAsNewSeries(...) or SaveToSeries(...) get used.
    */
    virtual void SaveToChili( DataStorage::SetOfObjects::ConstPointer inputNodes );

    /*!
    \brief Save Images- and Texts-Files without User-Dialog as new Series to Chili via Fileupload.
    @param inputNodes   Thats the nodes to save.
    @param studyOID   In which study the new series should created?
    @param seriesNumber   With wich seriesNumber the series should created?
    @param seriesDescription   With wich seriesDescription the series should created?
    This function create a new series and use the function SaveToSeries() . No dialog is used.
    */
    virtual void SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, std::string studyOID, int seriesNumber, std::string seriesDescription );

    /*!
    \brief Save Images- and Texts-Files without User-Dialog to given series to Chili via Fileupload.
    @param inputNodes   Thats the nodes to save.
    @param StudyOID   In which study should saved?
    @param SeriesOID   In which series should saved?
    @param overrideExistingSeries   If nodes alway exist in this study, do you want to override them or not ( only possible if the data saved by MBI )?
    This function save the nodes via FileUpload to chili. No dialog get used. The parent-child-relation saved automatically.
    USE THIS FUNCTION ONLY, IF YOU KNOW THE DESTINATION-SERIES.
    */
    virtual void SaveToSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, std::string studyOID, std::string seriesOID, bool overrideExistingSeries );

    mitkClassMacro( PACSPlugin,itk::Object );
    itkNewMacro( PACSPlugin );
    virtual ~PACSPlugin();

  protected:

    PACSPlugin();
};

}

#endif
