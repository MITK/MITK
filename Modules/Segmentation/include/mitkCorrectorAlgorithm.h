/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCorrectorAlgorithm_h
#define mitkCorrectorAlgorithm_h

#include <mitkContourModel.h>
#include <mitkImageToImageFilter.h>
#include <MitkSegmentationExports.h>
#include <mitkLabel.h>

#include <itkImage.h>

#define multilabelSegmentationType unsigned short
namespace mitk
{
  /**
    \brief Contour-based correction algorithm for 2D binary segmentation images.

    This class takes a 2D binary image and a user-drawn contour. The algorithm tests
    whether the line begins and ends inside or outside the segmentation and determines
    whether areas should be added to or subtracted from the segmentation shape.

    The output is the modified input image, combining the original input with the
    generated difference.

    \sa CorrectorTool2D
  */
  class MITKSEGMENTATION_EXPORT CorrectorAlgorithm : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CorrectorAlgorithm, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef mitk::Label::PixelType DefaultSegmentationDataType;

    /**
      \brief Set the user-drawn contour that defines the correction region.
    */
    void SetContour(ContourModel *contour) { this->m_Contour = contour; }
    itkSetMacro(FillColor, int);
    itkGetConstMacro(FillColor, int);

    itkSetMacro(EraseColor, int);
    itkGetConstMacro(EraseColor, int);

    // used by TobiasHeimannCorrectionAlgorithm
    typedef struct
    {
      int lineStart;
      int lineEnd;
      bool modified;

      std::vector<itk::Index<2>> points;
    } TSegData;

  protected:
    CorrectorAlgorithm();
    ~CorrectorAlgorithm() override;

    /** \brief Perform the actual contour correction processing. */
    void GenerateData() override;

    /** \brief Apply the improved Heilmann correction algorithm on the given 2D image.
      \return True if the correction was applied successfully.
    */
    bool ImprovedHeimannCorrectionAlgorithm(itk::Image<DefaultSegmentationDataType, 2>::Pointer pic);

    /** \brief Modify a single segment in the image according to the correction contour.
      \return True if the segment was modified successfully.
    */
    bool ModifySegment(const TSegData &segment, itk::Image<DefaultSegmentationDataType, 2>::Pointer pic);

    Image::Pointer m_WorkingImage;
    ContourModel::Pointer m_Contour;
    Image::Pointer m_DifferenceImage;

    int m_FillColor;
    int m_EraseColor;

  private:
    template <typename ScalarType>
    itk::Index<2> ensureIndexInImage(ScalarType i0, ScalarType i1);

    void ColorSegment(const mitk::CorrectorAlgorithm::TSegData &segment,
                      itk::Image<mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2>::Pointer pic);
    itk::Image<mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2>::Pointer CloneImage(
      itk::Image<mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2>::Pointer pic);
    itk::Index<2> GetFirstPoint(const mitk::CorrectorAlgorithm::TSegData &segment,
                                itk::Image<mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2>::Pointer pic);
    std::vector<itk::Index<2>> FindSeedPoints(
      const mitk::CorrectorAlgorithm::TSegData &segment,
      itk::Image<mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2>::Pointer pic);
    int FillRegion(const std::vector<itk::Index<2>> &seedPoints,
                   itk::Image<mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2>::Pointer pic);
    void OverwriteImage(itk::Image<mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2>::Pointer source,
                        itk::Image<mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2>::Pointer target);
  };
}

#endif
