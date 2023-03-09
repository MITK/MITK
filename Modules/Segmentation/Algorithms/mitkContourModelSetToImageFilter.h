/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSetToImageFilter_h
#define mitkContourModelSetToImageFilter_h

#include <MitkSegmentationExports.h>
#include <mitkImageSource.h>

namespace mitk
{
  class ContourModelSet;

  /**
    * @brief Fills a given mitk::ContourModelSet into a given mitk::Image
    * @ingroup Process
    */
  class MITKSEGMENTATION_EXPORT ContourModelSetToImageFilter : public ImageSource
  {
  public:
    mitkClassMacro(ContourModelSetToImageFilter, ImageSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
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
    itk::DataObject::Pointer MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
       * This is a default implementation to make sure we have something.
       * Once all the subclasses of ProcessObject provide an appopriate
       * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
       * virtual.
       */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    const mitk::ContourModelSet *GetInput(void);

    using itk::ProcessObject::SetInput;
    virtual void SetInput(const mitk::ContourModelSet *input);

    /**
       * @brief Set the image which will be used to initialize the output of this filter.
       * @param refImage the image used to initialize the output image
       */
    void SetImage(const mitk::Image *refImage);

    const mitk::Image *GetImage(void);

  protected:
    ContourModelSetToImageFilter();

    ~ContourModelSetToImageFilter() override;

    /**
       * @brief Initializes the volume of the output image with zeros
       */
    void InitializeOutputEmpty();

    bool m_MakeOutputBinary;

    unsigned int m_TimeStep;

    const mitk::Image *m_ReferenceImage;
  };
}
#endif
