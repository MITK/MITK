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


#ifndef MITKCOLOURIMAGEPROCESSOR_H
#define MITKCOLOURIMAGEPROCESSOR_H


#include <mitkImage.h>
#include <itkImage.h>

#include <itkRGBAPixel.h>
#include <mitkTransferFunction.h>
#include <itkImageRegionIterator.h>


namespace mitk
{

class mitkColourImageProcessor
{

  public:

    typedef itk::RGBAPixel<unsigned char> RGBAPixel;
    typedef itk::Image<RGBAPixel, 3> RGBAImage;


    mitkColourImageProcessor();
    ~mitkColourImageProcessor () ;

    mitk::Image::Pointer convertToRGBAImage( mitk::Image::Pointer input , mitk::TransferFunction::Pointer tf );
    mitk::Image::Pointer convertWithBinaryToRGBAImage( mitk::Image::Pointer input1 , mitk::Image::Pointer input2 , mitk::TransferFunction::Pointer tf );
    mitk::Image::Pointer convertWithBinaryAndColorToRGBAImage( mitk::Image::Pointer input1 , mitk::Image::Pointer input2 , mitk::TransferFunction::Pointer tf, int* color );
    mitk::Image::Pointer combineRGBAImage( mitk::Image::Pointer input1 , mitk::Image::Pointer input2);


  private:

    template<class TType>
    mitk::Image::Pointer ScalarToRGBA(itk::Image<TType, 3>* input  , mitk::TransferFunction::Pointer tf);

    template<class TType,class BType>
    mitk::Image::Pointer ScalarAndBinaryToRGBA(itk::Image<TType, 3>* input  ,itk::Image<BType, 3>* input2  , mitk::TransferFunction::Pointer tf);

    template<class TType,class BType>
    mitk::Image::Pointer ScalarAndBinaryAndColorToRGBA(itk::Image<TType, 3>* input  ,itk::Image<BType, 3>* input2  , mitk::TransferFunction::Pointer tf, int * color);

    mitk::Image::Pointer CombineRGBAImage(const unsigned char *input  , const unsigned char *input2, int sizeX, int sizeY, int sizeZ );

};
}//end namespace mitk
#endif /* MITKCOLOURIMAGEPROCESSOR_H */
