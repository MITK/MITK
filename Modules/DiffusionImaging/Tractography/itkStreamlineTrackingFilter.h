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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkStreamlineTrackingFilter_h_
#define __itkStreamlineTrackingFilter_h_

#include "MitkDiffusionImagingExports.h"
#include <itkImageToImageFilter.h>
#include <itkVectorContainer.h>
#include <itkVectorImage.h>
#include <itkDiffusionTensor3D.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

namespace itk{
  /** \class StreamlineTrackingFilter
 */

  template< class TTensorPixelType, class TPDPixelType=double>
  class StreamlineTrackingFilter :
      public ImageToImageFilter< Image< DiffusionTensor3D<TTensorPixelType>, 3 >,
      Image< Vector< TPDPixelType, 3 >, 3 > >
  {

  public:

    typedef StreamlineTrackingFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< DiffusionTensor3D<TTensorPixelType>, 3 >,
    Image< Vector< TPDPixelType, 3 >, 3 > >
    Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(StreamlineTrackingFilter, ImageToImageFilter)

    typedef TTensorPixelType                        TensorComponentType;
    typedef TPDPixelType                            DirectionPixelType;
    typedef typename Superclass::InputImageType     InputImageType;
    typedef typename Superclass::OutputImageType    OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    typedef vtkSmartPointer< vtkPolyData >     FiberPolyDataType;

    itkGetMacro( FiberPolyData, FiberPolyDataType )

  protected:
    StreamlineTrackingFilter();
    ~StreamlineTrackingFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, int);
    void AfterThreadedGenerateData();

    FiberPolyDataType m_FiberPolyData;
    vtkSmartPointer<vtkPoints> m_Points;
    vtkSmartPointer<vtkCellArray> m_Cells;
    float m_FaThres;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkStreamlineTrackingFilter.cpp"
#endif

#endif //__itkStreamlineTrackingFilter_h_

