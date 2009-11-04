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

#include "itkImageFileWriter.h"
#include "itkWarpImageFilter.h"
#include "itkImageRegionIterator.h"

#include "mitkDemonsRegistration.h"

namespace mitk {

  DemonsRegistration::DemonsRegistration():
    m_Iterations(50),
    m_StandardDeviation(1.0),
    m_FieldName("newField.mhd"),
    m_ResultName("deformedImage.mhd"),
    m_SaveField(true),
    m_SaveResult(true),
    m_DeformationField(NULL)
  {

  }

  DemonsRegistration::~DemonsRegistration()
  {
  }

  void DemonsRegistration::SetNumberOfIterations(int iterations)
  {
    m_Iterations = iterations;
  }

  void DemonsRegistration::SetStandardDeviation(float deviation)
  {
    m_StandardDeviation = deviation;
  }

  void DemonsRegistration::SetSaveDeformationField(bool saveField)
  {
    m_SaveField = saveField;
  }

  void DemonsRegistration::SetDeformationFieldFileName(const char* fieldName)
  {
    m_FieldName = fieldName;
  }

  void DemonsRegistration::SetSaveResult(bool saveResult)
  {
    m_SaveResult = saveResult;
  }

  void DemonsRegistration::SetResultFileName(const char* resultName)
  {
    m_ResultName = resultName;
  }

  itk::Image<itk::Vector<float, 3>,3>::Pointer DemonsRegistration::GetDeformationField()
  {
    return m_DeformationField;
  }

  template < typename TPixel, unsigned int VImageDimension >
    void DemonsRegistration::GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1)
  {
    typedef typename itk::Image< TPixel, VImageDimension >  FixedImageType;
    typedef typename itk::Image< TPixel, VImageDimension >  MovingImageType;

    typedef float InternalPixelType;
    typedef typename itk::Image< InternalPixelType, VImageDimension > InternalImageType;
    typedef typename itk::CastImageFilter< FixedImageType, 
                                  InternalImageType > FixedImageCasterType;
    typedef typename itk::CastImageFilter< MovingImageType, 
                                  InternalImageType > MovingImageCasterType;
    typedef typename itk::Image< InternalPixelType, VImageDimension > InternalImageType;
    typedef typename itk::Vector< float, VImageDimension >    VectorPixelType;
    typedef typename itk::Image<  VectorPixelType, VImageDimension > DeformationFieldType;
    typedef typename itk::DemonsRegistrationFilter<
                                  InternalImageType,
                                  InternalImageType,
                                  DeformationFieldType>   RegistrationFilterType;
    typedef typename itk::WarpImageFilter<
                            MovingImageType, 
                            MovingImageType,
                            DeformationFieldType  >     WarperType;
    typedef typename itk::LinearInterpolateImageFunction<
                                    MovingImageType,
                                    double          >  InterpolatorType;

    typedef  TPixel  OutputPixelType;
    typedef typename itk::Image< OutputPixelType, VImageDimension > OutputImageType;
    typedef typename itk::CastImageFilter< 
                          MovingImageType,
                          OutputImageType > CastFilterType;
    typedef typename itk::ImageFileWriter< OutputImageType >  WriterType;
    typedef typename itk::ImageFileWriter< DeformationFieldType >  FieldWriterType;

    typename FixedImageType::Pointer fixedImage = FixedImageType::New();
    mitk::CastToItkImage(m_ReferenceImage, fixedImage);
    typename MovingImageType::Pointer movingImage = itkImage1;

    if (fixedImage.IsNotNull() && movingImage.IsNotNull())
    {
      typename RegistrationFilterType::Pointer filter = RegistrationFilterType::New();

      this->AddStepsToDo(4);
      typename itk::ReceptorMemberCommand<DemonsRegistration>::Pointer command = itk::ReceptorMemberCommand<DemonsRegistration>::New();
      command->SetCallbackFunction(this, &DemonsRegistration::SetProgress);
      filter->AddObserver( itk::IterationEvent(), command );

      typename FixedImageCasterType::Pointer fixedImageCaster = FixedImageCasterType::New();
      fixedImageCaster->SetInput(fixedImage);
      filter->SetFixedImage( fixedImageCaster->GetOutput() );
      typename MovingImageCasterType::Pointer movingImageCaster = MovingImageCasterType::New();
      movingImageCaster->SetInput(movingImage);
      filter->SetMovingImage(movingImageCaster->GetOutput());
      filter->SetNumberOfIterations( m_Iterations );
      filter->SetStandardDeviations( m_StandardDeviation );
      filter->Update();

      typename WarperType::Pointer warper = WarperType::New();
      typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

      warper->SetInput( movingImage );
      warper->SetInterpolator( interpolator );
      warper->SetOutputSpacing( fixedImage->GetSpacing() );
      warper->SetOutputOrigin( fixedImage->GetOrigin() );
      warper->SetDeformationField( filter->GetOutput() );
      warper->Update();
      Image::Pointer outputImage = this->GetOutput();
      mitk::CastToMitkImage( warper->GetOutput(), outputImage );


      typename WriterType::Pointer      writer =  WriterType::New();
      typename CastFilterType::Pointer  caster =  CastFilterType::New(); 

      writer->SetFileName( m_ResultName );
  
      caster->SetInput( warper->GetOutput() );
      writer->SetInput( caster->GetOutput()   );
      if(m_SaveResult)
      {
        writer->Update();
      }

      if (VImageDimension == 2)
      {
        typedef DeformationFieldType  VectorImage2DType;
        typedef typename DeformationFieldType::PixelType Vector2DType;

        typename VectorImage2DType::ConstPointer vectorImage2D = filter->GetOutput();

        typename VectorImage2DType::RegionType  region2D = vectorImage2D->GetBufferedRegion();
        typename VectorImage2DType::IndexType   index2D  = region2D.GetIndex();
        typename VectorImage2DType::SizeType    size2D   = region2D.GetSize(); 


        typedef typename itk::Vector< float,       3 >  Vector3DType;
        typedef typename itk::Image< Vector3DType, 3 >  VectorImage3DType;

        typedef typename itk::ImageFileWriter< VectorImage3DType > WriterType;

        typename WriterType::Pointer writer3D = WriterType::New();

        typename VectorImage3DType::Pointer vectorImage3D = VectorImage3DType::New();
        
        typename VectorImage3DType::RegionType  region3D;
        typename VectorImage3DType::IndexType   index3D;
        typename VectorImage3DType::SizeType    size3D;

        index3D[0] = index2D[0];
        index3D[1] = index2D[1];
        index3D[2] = 0;
        
        size3D[0]  = size2D[0];
        size3D[1]  = size2D[1];
        size3D[2]  = 1;

        region3D.SetSize( size3D );
        region3D.SetIndex( index3D );

        typename VectorImage2DType::SpacingType spacing2D   = vectorImage2D->GetSpacing();
        typename VectorImage3DType::SpacingType spacing3D;

        spacing3D[0] = spacing2D[0];
        spacing3D[1] = spacing2D[1];
        spacing3D[2] = 1.0;

        vectorImage3D->SetSpacing( spacing3D );

        vectorImage3D->SetRegions( region3D );
        vectorImage3D->Allocate();
        
        typedef typename itk::ImageRegionConstIterator< VectorImage2DType > Iterator2DType;

        typedef typename itk::ImageRegionIterator< VectorImage3DType > Iterator3DType;

        Iterator2DType  it2( vectorImage2D, region2D );
        Iterator3DType  it3( vectorImage3D, region3D );

        it2.GoToBegin();
        it3.GoToBegin();

        Vector2DType vector2D;
        Vector3DType vector3D;

        vector3D[2] = 0; // set Z component to zero.

        while( !it2.IsAtEnd() )
        {
          vector2D = it2.Get();
          vector3D[0] = vector2D[0];  
          vector3D[1] = vector2D[1];  
          it3.Set( vector3D );
          ++it2;
          ++it3;
        }

        writer3D->SetInput( vectorImage3D );
        m_DeformationField = vectorImage3D;

        writer3D->SetFileName( m_FieldName );

        try
        {
          if(m_SaveField)
          {
            writer3D->Update();
          }
        }
        catch( itk::ExceptionObject & excp )
        {
          LOG_ERROR << excp << std::endl;
        }
      }
      else
      {
        typename FieldWriterType::Pointer      fieldwriter =  FieldWriterType::New();
        fieldwriter->SetFileName(m_FieldName);
        fieldwriter->SetInput( filter->GetOutput() );
        m_DeformationField = (itk::Image<itk::Vector<float, 3>,3> *)(filter->GetOutput());
        if(m_SaveField)
        {
          fieldwriter->Update();
        }
      }
      this->SetRemainingProgress(4);
    }
  }
} // end namespace
