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

#include "mitkDataStorage.h"

namespace mitk {

/** Documentation
  \brief Interface for minimal PACS communication
  \ingroup IO
  \ingroup Chili

  Defines some basic function for communication with a PACS.
  Currently only really implemented for the CHILI Workstation (see CHILIPlugin),
  but should be basic enough to work with differnt systems, too.

  \TODO The PatientInformation, StudyInformation, SeriesInformation structures should be classes able to contain any kind of tags
*/
class MITK_CORE_EXPORT PACSPlugin : public itk::Object
{
  public:

    /** This struct contain the plugin capabilities. */
    struct PACSPluginCapability
    {
      bool IsPACSFunctional;  // is there actually a real PACS connectivity implemented, configured and working right now?
      bool HasLoadCapability; // can the current implementation load data from a PACS?
      bool HasSaveCapability; // can the current implementation save data into the PACS?
    };

    /** Information about a patient in the PACS. Fields should correspond to DICOM PS 3.4-2008 Annex C.6.1.1.2 **/
    class PatientInformation
    {
      public: 
        std::string PatientsName;       // tag 0010,0010
        std::string PatientID;          // tag 0010,0020
        std::string PatientsBirthDate;  // tag 0010,0030
        std::string PatientsBirthTime;  // tag 0010,0032
        std::string PatientsSex;        // tag 0010,0040
        std::string PatientComments;    // tag 0010,4000
    };

    typedef std::list<PatientInformation> PatientInformationList;

    /** Information about a study in the PACS. Fields should correspond to DICOM PS 3.4-2008 Annex C.6.1.1.3 **/
    class StudyInformation 
    {
      public:
        std::string StudyInstanceUID;   // tag 0020,000D
        std::string StudyID;            // tag 0020,0010
        std::string StudyDate;          // tag 0008,0020
        std::string StudyTime;          // tag 0008,0030
        std::string AccessionNumber;    // tag 0008,0050
        std::string ModalitiesInStudy;  // tag 0008,0061
        std::string ReferringPhysician; // tag 0008,0090
        std::string StudyDescription;   // tag 0008,1030
    };
    
    typedef std::list<StudyInformation> StudyInformationList;

    /** Information about a study in the PACS. Fields should correspond to DICOM PS 3.4-2008 Annex C.6.1.1.4 **/
    class SeriesInformation
    {
      public:
        std::string SeriesInstanceUID;  // tag 0020,000E
        int SeriesNumber;               // tag 0020,0011
        std::string SeriesDate;         // tag 0008,0021 
        std::string SeriesTime;         // tag 0008,0031
        std::string SeriesDescription;  // tag 0008,103E
        std::string BodyPartExamined;   // tag 0018,0015
        std::string FrameOfReferenceUID;// tag 0020,0052 
        int AcquisitionNumber;          // tag 0020,0012 image specific
        std::string ContrastAgent;      // tag 0018,0010 image specific 
        std::string ScanningSequence;   // tag 0018,0020 mr image specific
        int EchoNumber;                 // tag 0018,0086 mr image specific
        int TemporalPosition;           // tag 0020,0100 mr image specific

        SeriesInformation()
          : SeriesNumber(-1),
            AcquisitionNumber(-1),
            EchoNumber(-1),
            TemporalPosition(-1)
        {
        }
    };

    typedef std::list<SeriesInformation> SeriesInformationList;

    /** Information about a document in a series. Very roughly corresponds to DICOM PS 3.3-2008 Annex C.24 */
    class DocumentInformation
    {
      public:
        std::string SeriesInstanceUID;  // tag 0020,000E
        unsigned int InstanceNumber;    // tag 0020,0013
        std::string MimeType;           // tag 0042,0012
        std::string ContentDate;        // tag 0008,0023
        std::string DocumentTitle;      // tag 0042,0010

        DocumentInformation()
        :InstanceNumber( (unsigned int)-1 )
        {
        }
    };

    /** There can be lots of texts to one series, so we need a list. */
    typedef std::list<DocumentInformation> DocumentInformationList; 

    mitkClassMacro( PACSPlugin,itk::Object );

    /*!
     * \brief Return a singleton mitk::PACSPlugin-Instance
     *
     * \param destroyInstance Tell the specific implementation it should free/delete itself. 
     * \warning Application should not use the instance after calling GetInstance(true);
     * \TODO check deletion mechanism, should be done by the application, which knows about CHILI (because there is a QcMITK...
     */
    static PACSPlugin* GetInstance( bool destroyInstance = false ); 

    /*!
     * \brief Information about capabilities of the current implementation.
     */
    virtual PACSPluginCapability GetPluginCapabilities();

    /*!
     * \brief Get a list of all known patients
     */
    virtual PatientInformationList GetPatientInformationList();

    /*!
     * \brief Get a list of all studies for a patient 
     */
    virtual StudyInformationList GetStudyInformationList( const PatientInformation& patient );
 
    /*!
     * \brief Get a list of all series for a study instance UID
     */
    virtual SeriesInformationList GetSeriesInformationList( const std::string& studyInstanceUID = "" );

    /*!
     * \brief Get a list of all non-image documents for a series instance UID
     */
    virtual DocumentInformationList GetDocumentInformationList( const std::string& seriesInstanceUID = "" );

    /*!
     * \brief Patient information for a given series instance UID
     */
    virtual PatientInformation GetPatientInformation( const std::string& seriesInstanceUID = "" );

    /*!
     * \brief Study information for a given series instance UID
     */
    virtual StudyInformation GetStudyInformation( const std::string& seriesInstanceUID = "" );

    /*!
     * \brief Series information for a given series instance UID
     */
    virtual SeriesInformation GetSeriesInformation( const std::string& seriesInstanceUID = "" );

    /*!
     * \brief Document information for a given series instance UID and a document instance number
     */
    virtual DocumentInformation GetDocumentInformation( const std::string& seriesInstanceUID, 
                                                        unsigned int instanceNumber );

    /*!
     * \brief Number of image preview boxes (lightboxes)
     *
     * If the specific PACS implementation supports a kind of lightbox concept
     * (a preview of all images from a selected series), then this function should
     * return how many of such lightboxes there are.
    */
    virtual unsigned int GetLightboxCount();

    /*!
     * \brief Return the index of the active image preview box (lightbox)
     *
     * If the specific PACS implementation supports a kind of lightbox concept
     * (a preview of all images from a selected series), then this function should
     * return the index of the currently active lightbox (starting from 0).
     *
     * If no active lightbox exists (perhaps due to lack of such a concept), it
     * is the calling object's duty to check if the returned index is larger than
     * the number of existing lightboxes reported by GetLightboxCount().
    */
    virtual unsigned int GetActiveLightbox();

    /*!
     * Set type of "sorter" that stacks 2D images into 3D or 3D+t volumes
     * \param readerType   0 used for the ImageNumberFilter, 1 for the SingleSpacingFilter and 2 used for SpacingSetFilter.
     * \TODO this should take enum values
     */
    virtual void SetReaderType( unsigned int readerType = 0 );

    virtual void AbortPACSImport(); 

    /*!
     *
     * \brief Load all images from the given lightbox.
     * \TODO rename to LoadLightboxContent and load all images and texts, ignoring the lightbox!! make it use LoadSeriesContent for all series UIDs found in lightbox
     */
    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromLightbox( unsigned int lightboxIndex = 0 );

    /*!
     * \brief Load all objects from a given series instance UID
     * \TODO rename to LoadSeriesContent, make this take a list of UIDs
     */
    virtual std::vector<DataTreeNode::Pointer> LoadFromSeries( const std::string& seriesInstanceUID ); 

    /*!
     * \brief Load all image objects from a given series instance UID
     * \TODO rename to LoadSeriesImageContent, make this take a list of UIDs
     */
    virtual std::vector<DataTreeNode::Pointer> LoadImagesFromSeries( const std::string& seriesInstanceUID );
    
    virtual std::vector<mitk::DataTreeNode::Pointer> LoadImagesFromSeries( std::vector<std::string> seriesInstanceUIDs );

    /*!
     * \brief Load all objects from a given series instance UID
     * \TODO rename to LoadSeriesDocumentContent, make this take a list of UIDs
     */
    virtual std::vector<DataTreeNode::Pointer> LoadTextsFromSeries( const std::string& seriesInstanceUID );

    /*!
     * \brief Load a given document object for a series instance UID and a document instance number
     * \TODO rename to LoadDocument make it take a list of parameters
     */
    virtual DataTreeNode::Pointer LoadSingleText( const std::string& seriesInstanceUID, unsigned int instanceNumber ); 

    /*!
     * \brief Save given data as a new series
     * \param studyInstanceUID save into this study
     * \param seriesNumber a number identifying the new series
     * \param seriesDescription a string description for the new series
     */
    virtual void SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, 
                                  const std::string& studyInstanceUID, 
                                  int seriesNumber, 
                                  const std::string& seriesDescription );

    /*!
     * \brief Save given data into an existing series
     * \param seriesInstanceUID save into this series
     * \param seriesNumber a number identifying the new series
     * \param seriesDescription a string description for the new series
     */
    virtual void SaveToSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, 
                               const std::string& seriesInstanceUID, 
                               bool overwriteExistingSeries );

  protected:
    
    // All this is hidden, should be instantiated through the GetInstance() method.
    itkNewMacro( PACSPlugin );
    PACSPlugin();
    virtual ~PACSPlugin();

    int m_ReaderType;
};

} // end namespace

#endif

