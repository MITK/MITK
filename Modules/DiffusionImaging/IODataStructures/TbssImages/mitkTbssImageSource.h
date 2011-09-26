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


#ifndef _MITK_TBSS_IMAGE_DATA_SOURCE_H_HEADER_
#define _MITK_TBSS_IMAGE_DATA_SOURCE_H_HEADER_


// Should be changed in a new type for TBSS
#include "mitkImageSource.h"
#include "MitkDiffusionImagingExports.h"


namespace mitk {



  class TbssImage;


//##Documentation
//## @brief Superclass of all classes generating diffusion volumes (instances 
//## of class DiffusionImage) as output. 
//##
//## @ingroup Process

class MitkDiffusionImaging_EXPORT TbssImageSource : public ImageSource
{
public:
  mitkClassMacro(TbssImageSource, BaseProcess)
  itkNewMacro(Self)

  typedef TbssImage OutputType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  virtual DataObjectPointer MakeOutput(unsigned int idx);

  //OutputType * GetOutput(unsigned int idx);

  //void SetOutput(OutputType* output);

  //OutputType* GetOutput()
  //{return Superclass::GetOutput();}

  //OutputType* GetOutput(unsigned int idx);

  //virtual void GraftOutput(OutputType* graft);

  //virtual void GraftNthOutput(unsigned int idx, OutputType *graft);

protected:
  TbssImageSource();

  virtual ~TbssImageSource(){}
};

} // namespace mitk

//#include "mitkTbssImageSource.cpp"


#endif /* _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_ */
