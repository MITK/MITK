/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 18:06:46 +0200 (Mi, 13 Mai 2009) $
Version:   $Revision: 5308 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_DIFFUSION_VOLUMES_DATA_SOURCE_H_HEADER_
#define _MITK_DIFFUSION_VOLUMES_DATA_SOURCE_H_HEADER_

#include "mitkBaseProcess.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk {

  template<typename TPixelType>
  class DiffusionVolumes;
  /*class DiffusionVolumes<double>;
  class DiffusionVolumes<int>;
  class DiffusionVolumes<unsigned int>;
  class DiffusionVolumes<short>;
  class DiffusionVolumes<unsigned short>;
  class DiffusionVolumes<char>;
  class DiffusionVolumes<unsigned char>;
  class DiffusionVolumes<long>;
  class DiffusionVolumes<unsigned long>;*/

//##Documentation
//## @brief Superclass of all classes generating diffusion volumes (instances 
//## of class DiffusionVolumes) as output. 
//##
//## @ingroup Process
template<typename TPixelType>
class MitkDiffusionImaging_EXPORT DiffusionVolumesSource : public BaseProcess
{
public:
  mitkClassMacro(DiffusionVolumesSource, BaseProcess);

  itkNewMacro(Self);  

  typedef DiffusionVolumes<TPixelType> OutputType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  virtual DataObjectPointer MakeOutput(unsigned int idx);


  void SetOutput(OutputType* output);

  OutputType* GetOutput();

  OutputType* GetOutput(unsigned int idx);

  virtual void GraftOutput(OutputType* graft);

  virtual void GraftNthOutput(unsigned int idx, OutputType *graft);

protected:
  DiffusionVolumesSource();

  virtual ~DiffusionVolumesSource();
};

} // namespace mitk

#include "mitkDiffusionVolumesSource.cpp"


#endif /* _MITK_DIFFUSION_VOLUMES_DATA_SOURCE_H_HEADER_ */
