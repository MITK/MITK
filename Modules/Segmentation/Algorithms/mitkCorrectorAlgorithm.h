/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCorrectorAlgorithmhIncluded
#define mitkCorrectorAlgorithmhIncluded

#include "ipSegmentation.h"
#include "mitkContourModel.h"
#include "mitkImageToImageFilter.h"
#include <MitkSegmentationExports.h>
#include <mitkLabel.h>

#include <itkImage.h>

#define multilabelSegmentationType unsigned short
namespace mitk
{
  /**
  * This class encapsulates an algorithm, which takes a 2D binary image and a contour.
  * The algorithm tests if the line begins and ends inside or outside a segmentation
  * and whether areas should be added to or subtracted from the segmentation shape.
  *
  * This class has two outputs:
  *   \li the modified input image from GetOutput()
  *
  * The output image is a combination of the original input with the generated difference image.
  *
  * \sa CorrectorTool2D
  */
  class MITKSEGMENTATION_EXPORT CorrectorAlgorithm : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CorrectorAlgorithm, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef mitk::Label::PixelType DefaultSegmentationDataType;

    /**
    * \brief User drawn contour
    */
    void SetContour(ContourModel *contour) { this->m_Contour = contour; }
    itkSetMacro(FillColor, int);
    itkGetConstMacro(FillColor, int);

    itkSetMacro(EraseColor, int);
    itkGetConstMacro(EraseColor, int);
    /**
    * \brief Calculated difference image.
    */
    // itkGetObjectMacro(DifferenceImage, Image);

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

    // does the actual processing
    void GenerateData() override;

    bool ImprovedHeimannCorrectionAlgorithm(itk::Image<DefaultSegmentationDataType, 2>::Pointer pic);
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
