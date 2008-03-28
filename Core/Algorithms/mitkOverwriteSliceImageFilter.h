/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkOverwriteSliceImageFilter_h_Included
#define mitkOverwriteSliceImageFilter_h_Included

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"

#include <itkImage.h>

namespace mitk
{

/**
  \brief Writes a 2D slice into a 3D image.

  \sa SegTool2D
  \sa ContourTool
  \sa ExtractImageFilter

  \ingroup Process
  \ingroup Reliver
  
  There is a separate page describing the general design of QmitkSliceBasedSegmentation: \ref QmitkSliceBasedSegmentationTechnicalPage

  This class takes a 3D mitk::Image as input and tries to replace one slice in it with the second input image, which is specified
  by calling SetSliceImage with a 2D mitk::Image.

  Two parameters determine which slice is replaced: the "slice dimension" is that one, which is constant for all points in the plane, e.g. transversal would mean 2.
  The "slice index" is the slice index in the image direction you specified with "affected dimension". Indices count from zero.

  This class works with all kind of image types, the only restrictions being that the input is 3D, and the slice image is 2D

  Last contributor: $Author$
*/
class MITK_CORE_EXPORT OverwriteSliceImageFilter : public ImageToImageFilter
{
  public:
    
    mitkClassMacro(OverwriteSliceImageFilter, ImageToImageFilter);
    itkNewMacro(OverwriteSliceImageFilter);

    /**
      \brief Which slice to overwrite (first one has index 0).
    */
    itkSetMacro(SliceIndex, unsigned int);
    itkGetConstMacro(SliceIndex, unsigned int);

    /**
      \brief The orientation of the slice to overwrite.

      \param dim Number of the dimension which is constant for all pixels of the desired slices (e.g. 0 for transversal)
    */
    itkSetMacro(SliceDimension, unsigned int);
    itkGetConstMacro(SliceDimension, unsigned int);
    
    /**
      \brief Time step of the slice to overwrite
    */
    itkSetMacro(TimeStep, unsigned int);
    itkGetConstMacro(TimeStep, unsigned int);
    
    itkSetMacro(CreateUndoInformation, bool);
    itkGetConstMacro(CreateUndoInformation, bool);
    
    itkSetObjectMacro(SliceImage, Image);
    const Image* GetSliceImage() { return m_SliceImage.GetPointer(); }
    
    const Image* GetLastDifferenceImage() { return m_SliceDifferenceImage.GetPointer(); }

  protected:

    OverwriteSliceImageFilter(); // purposely hidden
    virtual ~OverwriteSliceImageFilter();
    
    virtual void GenerateData();
    
    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageSwitch( itk::Image<TPixel,VImageDimension>* image );
    
    template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void ItkImageProcessing( itk::Image<TPixel1,VImageDimension1>* itkImage1, itk::Image<TPixel2,VImageDimension2>* itkImage2 );

    std::string EventDescription( unsigned int sliceDimension, unsigned int sliceIndex, unsigned int timeStep );

    Image::ConstPointer m_SliceImage;
    Image::Pointer m_SliceDifferenceImage;
    
    unsigned int m_SliceIndex;
    unsigned int m_SliceDimension;
    unsigned int m_TimeStep;
    unsigned int m_Dimension0;
    unsigned int m_Dimension1;

    bool m_CreateUndoInformation;
};

} // namespace

#endif


