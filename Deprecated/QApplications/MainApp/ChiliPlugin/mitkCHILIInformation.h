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

#ifndef MITKCHILIINFORMATION_H_HEADER_INCLUDED
#define MITKCHILIINFORMATION_H_HEADER_INCLUDED

//CHILI
#include <chili/cdbTypes.h>  //series_t*, study_t*, ...
//MITK
#include <mitkPACSPlugin.h>  //StudyInformation, PatientInformation, ...

class QcPlugin;

namespace mitk {

class CHILIInformation: public itk::Object
{
  public:

    mitkClassMacro( CHILIInformation, itk::Object );
    itkNewMacro( CHILIInformation );
    virtual ~CHILIInformation();

    PACSPlugin::StudyInformation GetStudyInformation( QcPlugin* instance, const std::string& seriesInstanceUID = "" );

    PACSPlugin::PatientInformation GetPatientInformation( QcPlugin* instance, const std::string& seriesInstanceUID = "" );

    PACSPlugin::SeriesInformation GetSeriesInformation( QcPlugin* instance, const std::string& seriesInstanceUID = "" );

    PACSPlugin::SeriesInformationList GetSeriesInformationList( QcPlugin* instance, const std::string& studyInstanceUID = "" );

    PACSPlugin::DocumentInformation GetDocumentInformation( QcPlugin* instance, const std::string& seriesInstanceUID, 
                                                                                unsigned int textInstanceNumber );

    PACSPlugin::DocumentInformationList GetDocumentInformationList( QcPlugin* instance, const std::string& seriesInstanceUID );

  private:

    static ipBool_t GlobalIterateSeries( int rows, int row, series_t* series, void* user_data );

    static ipBool_t GlobalIterateText( int rows, int row, text_t *text, void *user_data );

    PACSPlugin::SeriesInformationList m_SeriesInformationList;

    PACSPlugin::DocumentInformationList m_DocumentInformationList;
    std::list<std::string> m_DocumentTextOIDList;

  protected:

    CHILIInformation();
};

} // namespace mitk

#endif
