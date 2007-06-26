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
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <list>

#include "mitkChiliPluginEvents.h"
#include <mitkDataTreeNode.h>

#include <mitkDataTree.h>

#define CHILI_VERSION 36

class QcPlugin;

namespace mitk {

//TODO wird aktuell so genutzt mitk::m_QmitkChiliPluginConferenceID (QmitkChili3ConferenceKit), nicht über die GetConferenceID-Funktion
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

  this is the default-implementation, for the real implementation look at mitkChiliPluginImpl
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
      std::string ImportTime;
      std::string ChiliSenderID;
      std::string AccessionNumber;
      std::string InstitutionName;
      std::string WorkflowState;
      std::string Flags;
      std::string PerformingPhysician;
      std::string ReportingPhysician;
      std::string LastAccess;
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

    /** there can be lots of series to one study, so we need a list */
    typedef std::list<SeriesInformation> SeriesList;

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

    /** This struct contain all possible informations about the TextFile (all other then PIC-Files). */
    struct TextFileInformation
    {
      std::string OID;
      std::string MimeType;
      std::string ChiliText;
      std::string Status;
      std::string FrameOfReferenceUID;
      std::string TextDate;
      std::string Description;
    };
    /** there can be lots of series to one study, so we need a list */
    typedef std::list<TextFileInformation> TextFileList;

    /** return the StudyInformation of the current selected study */
    virtual StudyInformation GetCurrentSelectedStudy();
    /** return the list of the series to the current selected study */
    virtual SeriesList GetCurrentSelectedSeries();
    /** return the PatientInformation of the current selected study */
    virtual PatientInformation GetCurrentSelectedPatient();
    /** return a list of all TextFiles
    * - of the current selected series (if the user specify no series_OID)
      - of a specified series (if the user set the parameter) */
    virtual TextFileList GetTextFileInformation( std::string seriesOID = "" );

    /** return the number of Lightboxes in chili */
    virtual unsigned int GetLightBoxCount();

    /** return the number of the current ChiliVersion */
    virtual unsigned int GetChiliVersion();

    /** return if the application run as standalone or as chiliplugin*/
    virtual bool IsPlugin();
    /** return the conferenceid */
    virtual int GetConferenceID();

    /** Set the properties from the list to the datatreenode. The description of the properties get the prefix "Chili: ". */
    virtual void AddPropertyListToNode( const mitk::PropertyList::Pointer, mitk::DataTreeNode* );

    /** return the Plugin */
    static ChiliPlugin* GetInstance( bool destroyInstance = false );

    mitkClassMacro( ChiliPlugin,itk::Object );
    itkNewMacro( ChiliPlugin );
    virtual ~ChiliPlugin();

    /** With this function you can save a file to the chilidatabase.
    * If you want to save the file to the current selected study and series, then set the datatreenode only.
    * If you want to save the file to a specific study and series, then you have to set all parameter. */
    virtual void UploadViaFile( DataTreeNode*, std::string studyInstanceUID = "", std::string patientOID = "", std::string studyOID = "", std::string seriesOID = "" );

    /** with this function you can load a file from the chilidatabase
    * For the chiliText and MimeType use GetTextFileInformation(). */
    virtual void DownloadViaFile( std::string chiliText, std::string MimeType, DataTreeIteratorBase* parentIterator );

    //UnderConstruction
    virtual void UploadViaBuffer( DataTreeNode* );
    virtual DataTreeNode* DownloadViaBuffer();

  protected:
    ChiliPlugin();
};

} // namespace mitk

#endif /* MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD */
