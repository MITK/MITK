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
class ChiliPlugin : public itk::Object
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
      std::string Number;
      std::string Acquisition;
      std::string EchoNumber;
      std::string TemporalPosition;
      std::string Date;
      std::string Time;
      std::string Description;
      std::string Contrast;
      std::string BodyPartExamined;
      std::string ScanningSequence;
      std::string FrameOfReferenceUID;
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

    /** Struct to transport the PicTags (with Description and Content ). */
    struct TagInformationStruct
    {
      std::string PicTagDescription;
      std::string PicTagContent;
    };

    /** We need more than one PicTag. */
    typedef std::list<TagInformationStruct> TagInformationList;

    /*!
    \brief Return a mitk::ChiliPlugin-Instance as singleton.
    @param destroyInstance   Needed because of double inheritance of mitk::ChiliPluginImpl, both want to destroy the instance.
    @returns The singleton ChiliPlugin* - Instance.
    */
    static ChiliPlugin* GetInstance( bool destroyInstance = false );

    /*!
    \brief Return the ConferenceID.
    @returns The internal variable m_QmitkChiliPluginConferenceID.
    */
    virtual int GetConferenceID();

    /*!
    \brief Return true if the application run as plugin and false if its standalone.
    @returns True for ChiliPlugin, false for Standalone.
    */
    virtual bool IsPlugin();

    /*!
    \brief Return the studyinformation.
    @param seriesOID   If you dont set the input, you get the current selected study. If you want a specific study, set the input.
    @returns The current or specific studyinformation.
    If no study could found, this function return StudyInformation.OID == "".
    */
    virtual StudyInformation GetStudyInformation( const std::string& seriesOID = "" );

    /*!
    \brief Return the patientinformation.
    @param seriesOID   If you dont set the input, you get the current selected patient. If you want a specific patient, set the input.
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
    virtual void SaveImageToLightbox( Image* sourceImage, const mitk::PropertyList::Pointer propertyList, QcLightbox* lightbox );

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
    */
    virtual std::vector<DataTreeNode::Pointer> LoadAllImagesFromSeries( const std::string& seriesOID );

    /*!
    \brief Load all Text-files from the series.
    @param seriesOID   Set the series to load from.
    @returns Multiple mitk::DataTreeNodes as vector.
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

    mitkClassMacro( ChiliPlugin,itk::Object );
    itkNewMacro( ChiliPlugin );
    virtual ~ChiliPlugin();

  protected:

    ChiliPlugin();
};

}

#endif
