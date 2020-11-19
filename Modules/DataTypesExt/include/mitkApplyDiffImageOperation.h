/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkApplyDiffImageIIncluded
#define mitkApplyDiffImageIIncluded

#include "MitkDataTypesExtExports.h"
#include "mitkCompressedImageContainer.h"
#include "mitkOperation.h"

namespace mitk
{
  /**
   @brief Operation, that holds information about some image difference

   This class stores undo information for DiffImageApplier.
   Instances of this class are created e.g. by OverwriteSliceImageFilter.
   This works only for images with 1 channel (gray scale images, no color images).

   ApplyDiffImageOperation of course refers to an image (a segmentation usually).
   The refered image is observed for itk::DeleteEvent, because there is no SmartPointer
   used to keep the image alive -- the purpose of this class is undo and the undo
   stack should not keep things alive forever.

   To save memory, zlib compression is used via CompressedImageContainer.

   @ingroup Undo
   @ingroup ToolManagerEtAl
  */
  class MITKDATATYPESEXT_EXPORT ApplyDiffImageOperation : public Operation
  {
  protected:
    void OnImageDeleted();

    Image *m_Image;
    unsigned int m_SliceIndex;
    unsigned int m_SliceDimension;
    unsigned int m_TimeStep;
    double m_Factor;
    bool m_ImageStillValid;

    unsigned long m_DeleteTag;

    CompressedImageContainer::Pointer zlibContainer;

  public:
    /**
      Pass only 2D images here.
      \param operationType
      \param image
      \param diffImage
      \param timeStep
      \param sliceIndex brief Which slice to extract (first one has index 0).
      \param sliceDimension Number of the dimension which is constant for all pixels of the desired slice (e.g. 0 for
      axial)
    */
    ApplyDiffImageOperation(OperationType operationType,
                            Image *image,
                            Image *diffImage,
                            unsigned int timeStep = 0,
                            unsigned int sliceDimension = 2,
                            unsigned int sliceIndex = 0);
    ~ApplyDiffImageOperation() override;

    // Unfortunately cannot use itkGet/SetMacros here, since Operation does not inherit itk::Object
    unsigned int GetSliceIndex() { return m_SliceIndex; }
    unsigned int GetSliceDimension() { return m_SliceDimension; }
    unsigned int GetTimeStep() { return m_TimeStep; }
    void SetFactor(double factor) { m_Factor = factor; }
    double GetFactor() { return m_Factor; }
    Image *GetImage() { return m_Image; }
    Image::Pointer GetDiffImage();

    bool IsImageStillValid() { return m_ImageStillValid; }
  };

} // namespace mitk

#endif
