/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDiffImageApplier_h_Included
#define mitkDiffImageApplier_h_Included

#include "mitkCommon.h"
#include "mitkImage.h"
#include "mitkOperationActor.h"
#include <MitkSegmentationExports.h>

#include <itkImage.h>
#include <itkObjectFactory.h>

namespace mitk
{
  /**
    \brief Applies difference images to 3D images.

    This class is supposed to execute ApplyDiffImageOperations, which contain information about pixel changes within one
    image slice.
    Class should be called from the undo stack. At the moment, ApplyDiffImageOperations are only created by
    OverwriteSliceImageFilter.

    $Author: maleike $
  */
  class MITKSEGMENTATION_EXPORT DiffImageApplier : public itk::Object, public OperationActor
  {
  public:
    mitkClassMacroItkParent(DiffImageApplier, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      void ExecuteOperation(Operation *operation) override;

    static DiffImageApplier *GetInstanceForUndo();

  protected:
    DiffImageApplier(); // purposely hidden
    ~DiffImageApplier() override;

    template <typename TPixel, unsigned int VImageDimension>
    void ItkImageSwitch2DDiff(itk::Image<TPixel, VImageDimension> *image);

    template <typename TPixel, unsigned int VImageDimension>
    void ItkImageSwitch3DDiff(itk::Image<TPixel, VImageDimension> *image);

    template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void ItkImageProcessing2DDiff(itk::Image<TPixel1, VImageDimension1> *itkImage1,
                                  itk::Image<TPixel2, VImageDimension2> *itkImage2);

    template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void ItkImageProcessing3DDiff(itk::Image<TPixel1, VImageDimension1> *itkImage1,
                                  itk::Image<TPixel2, VImageDimension2> *itkImage2);

    template <typename TPixel, unsigned int VImageDimension>
    void ItkInvertPixelValues(itk::Image<TPixel, VImageDimension> *itkImage);

    Image::Pointer m_Image;
    Image::Pointer m_SliceDifferenceImage;

    unsigned int m_SliceIndex;
    unsigned int m_SliceDimension;
    unsigned int m_TimeStep;
    unsigned int m_Dimension0;
    unsigned int m_Dimension1;

    double m_Factor;
  };

} // namespace

#endif
