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

#ifndef mitkOverwriteDirectedPlaneImageFilter_h_Included
#define mitkOverwriteDirectedPlaneImageFilter_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkImageToImageFilter.h"

#include <itkImage.h>

namespace mitk
{

/**
  \deprecated This class is deprecated. Use mitkVtkImageOverwrite instead.
  \sa mitkVtkImageOverwrite

  \brief Writes a 2D slice into a 3D image.

  \sa SegTool2D
  \sa ContourTool
  \sa ExtractImageFilter

  \ingroup Process
  \ingroup Reliver

  There is a separate page describing the general design of QmitkInteractiveSegmentation: \ref QmitkInteractiveSegmentationTechnicalPage

  This class takes a 3D mitk::Image as input and tries to replace one slice in it with the second input image, which is specified
  by calling SetSliceImage with a 2D mitk::Image.

  Two parameters determine which slice is replaced: the "slice dimension" is that one, which is constant for all points in the plane, e.g. axial would mean 2.
  The "slice index" is the slice index in the image direction you specified with "affected dimension". Indices count from zero.

  This class works with all kind of image types, the only restrictions being that the input is 3D, and the slice image is 2D.

  If requested by SetCreateUndoInformation(true), this class will create instances of ApplyDiffImageOperation for the undo stack.
  These operations will (on user request) be executed by DiffImageApplier to perform undo.

  Last contributor: $Author: maleike $
*/
class MITKSEGMENTATION_EXPORT OverwriteDirectedPlaneImageFilter : public ImageToImageFilter
{
  public:

    mitkClassMacro(OverwriteDirectedPlaneImageFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
      \brief Which plane to overwrite
    */
    const BaseGeometry* GetPlaneGeometry3D() const { return m_PlaneGeometry; }
    void SetPlaneGeometry3D( const BaseGeometry *geometry ) { m_PlaneGeometry = geometry; }

    /**
      \brief Time step of the slice to overwrite
    */
    itkSetMacro(TimeStep, unsigned int);
    itkGetConstMacro(TimeStep, unsigned int);

    /**
      \brief Whether to create undo operation in the MITK undo stack
     */
    itkSetMacro(CreateUndoInformation, bool);
    itkGetConstMacro(CreateUndoInformation, bool);

    itkSetObjectMacro(SliceImage, Image);
    const Image* GetSliceImage() { return m_SliceImage.GetPointer(); }

    const Image* GetLastDifferenceImage() { return m_SliceDifferenceImage.GetPointer(); }

  protected:

    OverwriteDirectedPlaneImageFilter(); // purposely hidden
    virtual ~OverwriteDirectedPlaneImageFilter();

    virtual void GenerateData() override;

    template<typename TPixel, unsigned int VImageDimension>
    void ItkSliceOverwriting ( itk::Image<TPixel, VImageDimension>* input3D);

    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageSwitch( itk::Image<TPixel,VImageDimension>* image );

    template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void ItkImageProcessing( itk::Image<TPixel1,VImageDimension1>* itkImage1, itk::Image<TPixel2,VImageDimension2>* itkImage2 );

    //std::string EventDescription( unsigned int sliceDimension, unsigned int sliceIndex, unsigned int timeStep );

    Image::ConstPointer m_SliceImage;
    Image::Pointer m_SliceDifferenceImage;

    const BaseGeometry  *m_PlaneGeometry;
    const BaseGeometry *m_ImageGeometry3D;
    unsigned int m_TimeStep;
    unsigned int m_Dimension0;
    unsigned int m_Dimension1;

    bool m_CreateUndoInformation;
};

} // namespace

#endif


