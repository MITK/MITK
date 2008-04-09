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

#ifndef MITKSAVETOCHILI_H_HEADER_INCLUDED
#define MITKSAVETOCHILI_H_HEADER_INCLUDED

//CHILI
#include <chili/cdbTypes.h>  //series_t*, study_t*, ...
//MITK
#include "mitkParentChild.h"
#include "mitkDataStorage.h"
#include "mitkImageToPicDescriptor.h"  //using struct TagInformationList

class QcPlugin;

namespace mitk {

class SaveToCHILI: public itk::Object
{
  public:

    void SaveToChili( QcPlugin* instance, DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& tmpDirectory );

    void SaveAsNewSeries( QcPlugin* instance, DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& studyOID, int seriesNumber, const std::string& seriesDescription, const std::string& tmpDirectory );

    void SaveToSeries( QcPlugin* instance, DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& seriesOID, bool overrideExistingSeries, const std::string& tmpDirectory );

    mitkClassMacro( SaveToCHILI, itk::Object );
    itkNewMacro( SaveToCHILI );
    virtual ~SaveToCHILI();

  private:

    ParentChild::Pointer m_ParentChild;

    int m_MaximumImageNumber;

    unsigned int GetMaximumImageNumber( QcPlugin* instance, const std::string& seriesOID );

    ImageToPicDescriptor::TagInformationList GetNeededTagList( study_t* study, patient_t* patient, series_t* series );

    static ipBool_t GlobalIterateImagesForImageNumber( int rows, int row, image_t* image, void* user_data );

  protected:

    SaveToCHILI();

};

} // namespace mitk

#endif
