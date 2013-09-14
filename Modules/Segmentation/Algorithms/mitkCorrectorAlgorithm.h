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
#include "SegmentationExports.h"
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
 *   \li a difference image from GetDifferenceImage()
 *   \li the modified input image from GetOutput()
 *
 * The difference image is an image of the same dimensions as the input. Each pixel has
 * one of three values -1, 0, +1 meaning
 *   \li <b>-1</b> this pixel was foreground in the input image and should be removed from the segmentation
 *   \li <b>0</b> this pixel needs no change
 *   \li <b>+1</b> this pixel was background in the input image and should be added to the segmentation
 *
 * The output image is a combination of the original input with the generated difference image.
 *
 * \sa CorrectorTool2D
 */
class Segmentation_EXPORT CorrectorAlgorithm : public ImageToImageFilter
{
  public:

    mitkClassMacro(CorrectorAlgorithm, ImageToImageFilter);
    itkNewMacro(CorrectorAlgorithm);

    /**
     * \brief User drawn contour
     */
    void SetContour( ContourModel* contour){this->m_Contour = contour;}

    /**
     * \brief Calculated difference image.
     */
    //itkGetObjectMacro(DifferenceImage, Image);

  protected:

    // used by TobiasHeimannCorrectionAlgorithm
    typedef struct
    {
      int  lineStart;
      int lineEnd;
      bool modified;
    }
    TSegData;

    CorrectorAlgorithm();
    virtual ~CorrectorAlgorithm();

    // does the actual processing
    virtual void GenerateData();

    void TobiasHeimannCorrectionAlgorithm(mitkIpPicDescriptor* pic);
    bool modifySegment( int lineStart, int lineEnd, ipMITKSegmentationTYPE state, mitkIpPicDescriptor *pic, int* _ofsArray );

    void CalculateDifferenceImage( Image* modifiedImage, Image* originalImage );
    template<typename TPixel, unsigned int VImageDimension>
    void ItkCalculateDifferenceImage( itk::Image<TPixel, VImageDimension>* originalImage, Image* modifiedMITKImage );

    Image::Pointer m_WorkingImage;
    ContourModel::Pointer m_Contour;
    Image::Pointer m_DifferenceImage;
};

}

#endif

