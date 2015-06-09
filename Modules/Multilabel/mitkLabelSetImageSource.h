/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef __mitkLabelSetImageSource_H_
#define __mitkLabelSetImageSource_H_

#include "mitkImageSource.h"
#include "MitkMultilabelExports.h"

namespace mitk {

class LabelSetImage;

//##Documentation
//## @brief Superclass of all classes generating labelset images (instances
//## of class LabelSetImage) as output.
//##
//## @ingroup Process
class MITKMULTILABEL_EXPORT LabelSetImageSource : public ImageSource
{
public:
  mitkClassMacro(LabelSetImageSource, ImageSource);
  itkNewMacro(Self);

  typedef LabelSetImage OutputType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  mitkBaseDataSourceGetOutputDeclarations
  /**
   * Allocates a new output object and returns it. Currently the
   * index idx is not evaluated.
   * @param idx the index of the output for which an object should be created
   * @returns the new object
   */
  virtual itk::DataObject::Pointer MakeOutput ( DataObjectPointerArraySizeType idx ) override;

  /**
   */
  virtual itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

protected:
  LabelSetImageSource();

  virtual ~LabelSetImageSource();
};

} // namespace mitk

//#include "mitkLabelSetImageSource.cpp"


#endif // __mitkLabelSetImageSource_H_
