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

#ifndef MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD
#define MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD

#include <itkObject.h>

#include <mitkDataTree.h>
#include <mitkDataStorage.h>

#define NUMBER_OF_THINKABLE_LIGHTBOXES 4

class QcLightbox;

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
class MITK_CORE_EXPORT PACSPlugin : public itk::Object
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

    struct PSRelationInformation
     {
       std::string label;
       std::string id;
       std::string oid;
       std::list<std::string> parentLabel;
       std::list<std::string> childLabel;
       bool image;
     };

     typedef std::list<PSRelationInformation> PSRelationInformationList;

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

    mitkClassMacro( PACSPlugin,itk::Object );
    itkNewMacro( PACSPlugin );
    virtual ~PACSPlugin();

  protected:

    PACSPlugin();
};

}

#endif
