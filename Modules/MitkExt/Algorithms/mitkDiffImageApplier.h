/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkDiffImageApplier_h_Included
#define mitkDiffImageApplier_h_Included

#include "mitkCommon.h"
#include "mitkOperationActor.h"
#include "mitkImage.h"

#include <itkObjectFactory.h>
#include <itkImage.h>

namespace mitk
{

/**
  \brief Applies difference images to 3D images.

  This class is supposed to execute ApplyDiffImageOperations, which contain information about pixel changes within one image slice.
  Class should be called from the undo stack. At the moment, ApplyDiffImageOperations are only created by OverwriteSliceImageFilter.

  $Author: maleike $
*/
class MITK_CORE_EXPORT DiffImageApplier : public itk::Object, public OperationActor
{
  public:
    
    mitkClassMacro(DiffImageApplier, Object);
    itkNewMacro(DiffImageApplier);

    virtual void ExecuteOperation( Operation* operation );

    static DiffImageApplier* GetInstanceForUndo();

  protected:

    DiffImageApplier(); // purposely hidden
    virtual ~DiffImageApplier();

    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageSwitch2DDiff( itk::Image<TPixel,VImageDimension>* image );
    
    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageSwitch3DDiff( itk::Image<TPixel,VImageDimension>* image );

    template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void ItkImageProcessing2DDiff( itk::Image<TPixel1,VImageDimension1>* itkImage1, itk::Image<TPixel2,VImageDimension2>* itkImage2 );
    
    template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void ItkImageProcessing3DDiff( itk::Image<TPixel1,VImageDimension1>* itkImage1, itk::Image<TPixel2,VImageDimension2>* itkImage2 );

    template<typename TPixel, unsigned int VImageDimension>
    void ItkInvertPixelValues( itk::Image<TPixel,VImageDimension>* itkImage );

    Image::Pointer m_Image;
    Image::ConstPointer m_SliceDifferenceImage;
    
    unsigned int m_SliceIndex;
    unsigned int m_SliceDimension;
    unsigned int m_TimeStep;
    unsigned int m_Dimension0;
    unsigned int m_Dimension1;

    double m_Factor;
};

} // namespace

#endif


