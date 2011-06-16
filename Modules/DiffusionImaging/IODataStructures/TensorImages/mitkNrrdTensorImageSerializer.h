/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkNrrdTensorImageSerializer_h_included
#define mitkNrrdTensorImageSerializer_h_included

#include "MitkDiffusionImagingExports.h"
#include "mitkBaseDataSerializer.h"

namespace mitk
{
/**
  \brief Serializes mitk::Surface for mitk::SceneIO
*/
class MitkDiffusionImaging_EXPORT NrrdTensorImageSerializer : public BaseDataSerializer
{
  public:
    mitkClassMacro( NrrdTensorImageSerializer, BaseDataSerializer );
    itkNewMacro(Self);
    virtual std::string Serialize();
  protected:
    NrrdTensorImageSerializer();
    virtual ~NrrdTensorImageSerializer();
};
} // namespace
#endif
