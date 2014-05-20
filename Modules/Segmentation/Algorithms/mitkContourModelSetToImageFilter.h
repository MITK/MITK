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

#ifndef _MITK_CONTOURMODEL_SOURCE_H
#define _MITK_CONTOURMODEL_SOURCE_H

#include <MitkSegmentationExports.h>
#include <mitkImageSource.h>

namespace mitk
{

class ContourModelSet;

/**
  * @brief Fills a given mitk::ContourModelSet into a given mitk::Image
  * @ingroup Process
  */
class MitkSegmentation_EXPORT ContourModelSetToImageFilter : public ImageSource
{
public:

  mitkClassMacro( ContourModelSetToImageFilter, ImageSource )
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  itkSetMacro(MakeOutputBinary, bool);
  itkGetMacro(MakeOutputBinary, bool);
  itkBooleanMacro(MakeOutputBinary);

  itkSetMacro(TimeStep, unsigned int);

  /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
  virtual itk::DataObject::Pointer MakeOutput ( DataObjectPointerArraySizeType idx );

  /**
     * This is a default implementation to make sure we have something.
     * Once all the subclasses of ProcessObject provide an appopriate
     * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
     * virtual.
     */
  virtual itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name);

  virtual void GenerateInputRequestedRegion();

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::ContourModelSet* GetInput(void);

  using itk::ProcessObject::SetInput;
  virtual void SetInput(const mitk::ContourModelSet* input);

  /**
     * @brief Set the image which will be used to initialize the output of this filter.
     * @param refImage the image used to initialize the output image
     */
  void SetImage(const mitk::Image *refImage);

  const mitk::Image *GetImage(void);

protected:

  ContourModelSetToImageFilter();

  virtual ~ContourModelSetToImageFilter();

  /**
     * @brief Initializes the volume of the output image with zeros
     */
  void InitializeOutputEmpty();

  bool m_MakeOutputBinary;

  unsigned int m_TimeStep;
};

}
#endif // #_MITK_CONTOURMODEL_SOURCE_H
