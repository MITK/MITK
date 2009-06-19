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

#include "mitkImageToLookupTableFilter.h"
#include "mitkImageToItk.h"
#include "itkImage.h"
#include "itkVector.h"
#include "itkImageRegionIterator.h"
#include "itkNumericTraits.h"
#include <algorithm>

mitk::ImageToLookupTableFilter::ImageToLookupTableFilter()
{
    this->SetNumberOfRequiredInputs(1);    
}




mitk::ImageToLookupTableFilter::~ImageToLookupTableFilter()
{
    
}




void mitk::ImageToLookupTableFilter::SetInput( const mitk::ImageToLookupTableFilter::InputImageType *input)
{
    // Process object is not const-correct so the const_cast is required here
    this->itk::ProcessObject::SetNthInput( 0, const_cast< InputImageType * >( input ) );
    
}




void mitk::ImageToLookupTableFilter::SetInput( unsigned int index, const mitk::ImageToLookupTableFilter::InputImageType * input)
{
    if ( index + 1 > this->GetNumberOfInputs() )
    {
        this->SetNumberOfRequiredInputs( index + 1 );
    }
    // Process object is not const-correct so the const_cast is required here
    this->ProcessObject::SetNthInput( index, const_cast< InputImageType *>( input ) );
}




const mitk::ImageToLookupTableFilter::InputImageType * mitk::ImageToLookupTableFilter::GetInput(void)
{
    if ( this->GetNumberOfInputs() < 1 )
    {
        return 0;
    }
    return static_cast<const InputImageType * >( this->ProcessObject::GetInput( 0 ) );
}




const mitk::ImageToLookupTableFilter::InputImageType * mitk::ImageToLookupTableFilter::GetInput(unsigned int idx)
{
    return static_cast< const InputImageType * >( this->ProcessObject::GetInput( idx ) );
}
            



void mitk::ImageToLookupTableFilter::GenerateData()
{
    this->SetNumberOfOutputs( this->GetNumberOfInputs() );
    for (unsigned int inputIdx = 0; inputIdx < this->GetNumberOfInputs() ; ++inputIdx)
    {
        InputImagePointer image = const_cast<mitk::Image*>(this->GetInput( inputIdx ));
        OutputTypePointer output = dynamic_cast<OutputType*>(this->MakeOutput( inputIdx ).GetPointer());
        
        this->ProcessObject::SetNthOutput( inputIdx, output.GetPointer() );
        if (image.IsNull())
        {
            itkWarningMacro(<< inputIdx<<"'th input image is null!");    
            return;
        }
        
        // the default vtkLookupTable has range=[0,1]; and hsv ranges set 
        // up for rainbow color table (from red to blue).
        
        vtkLookupTable* vtkLut = vtkLookupTable::New();
        /*
        if ( ( image->GetPixelType().GetNumberOfComponents() == 3 ) && ( image->GetDimension() == 3 ) )
        {
            
            // some typedefs for conversion to an iterable itk image
            const unsigned int VectorDimension = 3;
            typedef float VectorComponentType;
            typedef itk::Vector< VectorComponentType, VectorDimension > VectorType;
            typedef itk::Image< VectorType, VectorDimension > VectorFieldType;
            typedef itk::ImageRegionIterator< VectorFieldType > VectorFieldIteratorType;
            typedef mitk::ImageToItk<VectorFieldType> ImageConverterType;
            
            // some local variables
            float minValue = itk::NumericTraits<float>::max();
            float maxValue = itk::NumericTraits<float>::NonpositiveMin();
            float norm = 0.0f;
            
            //determine the range of the vector magnitudes in the image
            ImageConverterType::Pointer imageConverter = ImageConverterType::New();
            imageConverter->SetInput(image);
            VectorFieldType::Pointer itkImage = imageConverter->GetOutput();
            VectorFieldIteratorType it( itkImage, itkImage->GetRequestedRegion() );
            it.GoToBegin();
            while ( !it.IsAtEnd() )
            {
                norm = it.Get().GetNorm();
                minValue = std::min(norm, minValue);
                maxValue = std::max(norm, maxValue);
            }
            LOG_INFO << "Range of vector magnitudes: [" << minValue << ", "<< maxValue << "]." << std::endl;
            vtkLut->SetRange(minValue, maxValue);      
        }
        else 
        {
            itkWarningMacro(<< "Sorry, only 3d vector images are currently supported!");    
        }*/
        vtkLut->SetRange(0, 10);
        output->SetVtkLookupTable( vtkLut );
        vtkLut->Delete();
    }
}
