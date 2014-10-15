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

#ifndef mitkCorrectorAlgorithmhIncluded
#define mitkCorrectorAlgorithmhIncluded

#include "mitkImageToImageFilter.h"
#include <MitkSegmentationExports.h>
#include "mitkContourModel.h"
#include "ipSegmentation.h"

#include <itkImage.h>

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
class MitkSegmentation_EXPORT CorrectorAlgorithm : public ImageToImageFilter
{
  public:

    mitkClassMacro(CorrectorAlgorithm, ImageToImageFilter);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * \brief User drawn contour
     */
    void SetContour( ContourModel* contour){this->m_Contour = contour;}

    /**
     * \brief Calculated difference image.
     */
    //itkGetObjectMacro(DifferenceImage, Image);


    // used by TobiasHeimannCorrectionAlgorithm
    typedef struct
    {
      int  lineStart;
      int lineEnd;
      bool modified;

      std::vector< itk::Index<2> > points;
    }
    TSegData;

  protected:

    CorrectorAlgorithm();
    virtual ~CorrectorAlgorithm();

    // does the actual processing
    virtual void GenerateData();

    bool ImprovedHeimannCorrectionAlgorithm(itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer pic);
    bool ModifySegment(const TSegData &segment, itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer pic);

    Image::Pointer m_WorkingImage;
    ContourModel::Pointer m_Contour;
    Image::Pointer m_DifferenceImage;

    int m_FillColor;
    int m_EraseColor;
};
}

#endif
