/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-01 08:56:22 +0200 (Di, 01 Apr 2008) $
Version:   $Revision: 13931 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKPARENTCHILD_H_HEADER_INCLUDED
#define MITKPARENTCHILD_H_HEADER_INCLUDED

//CHILI
#include <chili/cdbTypes.h>  //series_t*, study_t*, ...
//ITK
#include <itkObject.h>
//MITK
#include "mitkDataTree.h"
#include "mitkPACSPlugin.h"

class TiXmlDocument;
class QcPlugin;

namespace mitk {

class ParentChild: public itk::Object
{
  public:

    void InitParentChild( QcPlugin* instance, const std::string& studyOID, const std::string& instanceUID, const std::string& patientOID, const std::string& tmpDirectory );

    void AddEntry( DataTreeNode::Pointer node, std::list< std::string > CurrentImageInstanceUIDs, const std::string& seriesOID );

    void SaveRelationShip();

    std::string GetLabel( std::list<std::string> ImageInstanceUIDs );

    std::list<std::string> GetSlices( const std::string& label, const std::string& seriesOID );

    PACSPlugin::PSRelationInformationList GetSeriesRelationInformation( QcPlugin* instance, const std::string& seriesOID, const std::string& tmpDirectory );

    PACSPlugin::PSRelationInformationList GetStudyRelationInformation( QcPlugin* instance, const std::string& studyOID, const std::string& tmpDirectory );

    mitkClassMacro( ParentChild, itk::Object );
    itkNewMacro( ParentChild );
    virtual ~ParentChild();

  private:

    struct RelationCapabilityStruct
    {
      std::string xmlStudyOID;
      std::string xmlStudyInstanceUID;
      std::string xmlPatientOID;
      std::string xmlSeriesOID;
      std::string xmlTextOID;
      TiXmlDocument* xmlDocument;
      unsigned int elementCount;
    };
    RelationCapabilityStruct m_currentXML;

    QcPlugin* m_Instance;
    std::string m_tmpDirectory;
    std::list<DataTreeNode::Pointer> m_SavedNodeList;

    struct CircleCheckStruct
    {
      std::string VolumeLabel;
      int ParentCount;
      std::list<std::string> ChildList;
    };
    std::list<CircleCheckStruct> m_CircleCheckStructure;

    void InitCircleCheckStructure();
    bool SaveSingleRelation( const std::string& childVolumeLabel, const std::string& parentVolumeLabel );
    static ipBool_t GlobalIterateSearchStepOne( int rows, int row, series_t* series, void* user_data );
    static ipBool_t GlobalIterateSearchStepTwo( int rows, int row, text_t *text, void *user_data );

  protected:

    ParentChild();

};

} // namespace mitk

#endif
