/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef ImageGenerator_H_HEADER_INCLUDED
#define ImageGenerator_H_HEADER_INCLUDED

#include <MitkExports.h>
#include <mitkImage.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

namespace mitk {
  //##Documentation
  //## @brief generator for random MITK images
  //## This is a helper class to generate MITK images filled with random values.
  //## The parameters dimX, dimY, dimZ, and dimT are used to set the dimensions of the MITK image.
  //## Default parameters are dimT = 1 and dimZ = 1 which is a 2D image (or a 4D image with just 1 slice and 1 time step).
  //## The parameters randomMax and randomMin are boundary values for the random generator.
  //## In other words: the generator will generate values between randomMin and randomMax.
  //## @ingroup IO
  class MITK_CORE_EXPORT ImageGenerator
  {
  public:
    template <typename TPixelType>
        static mitk::Image::Pointer GenerateRandomImage(unsigned int dimX,
                                                    unsigned int dimY,
                                                    unsigned int dimZ = 1,
                                                    unsigned int dimT = 1,
                                                    const double randomMax = 1000.0f, const double randMin = 0.0f)
        {
          //set the data type according to the template
            mitk::PixelType type = MakeScalarPixelType<TPixelType>();
          //type.Initialize(typeid(TPixelType));

          //initialize the MITK image with given dimenion and data type
          mitk::Image::Pointer output = mitk::Image::New();
          unsigned int* dimensions = new unsigned int[4];
          unsigned int numberOfDimensions = 0;
          unsigned int bufferSize = 0;

          //check which dimension is needed
          if(dimT <= 1)
          {
            if(dimZ <= 1)
            { //2D
              numberOfDimensions = 2;
              dimensions[0] = dimX;
              dimensions[1] = dimY;
              bufferSize = dimX*dimY;
            }
            else
            { //3D
              numberOfDimensions = 3;
              dimensions[0] = dimX;
              dimensions[1] = dimY;
              dimensions[2] = dimZ;
              bufferSize = dimX*dimY*dimZ;
            }
          }
          else
          { //4D
            numberOfDimensions = 4;
            dimensions[0] = dimX;
            dimensions[1] = dimY;
            dimensions[2] = dimZ;
            dimensions[3] = dimT;
            bufferSize = dimX*dimY*dimZ*dimT;
          }
          output->Initialize(type, numberOfDimensions, dimensions);

          //get a pointer to the image buffer to write into
          TPixelType* imageBuffer = (TPixelType*)output->GetData();

          //initialize the random generator
          itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
          randomGenerator->Initialize();

          //fill the buffer for each pixel/voxel
          for(unsigned int i = 0; i < bufferSize; i++)
          {
            if(type == typeid(int)) //call integer function
            {
              imageBuffer[i] = (TPixelType)randomGenerator->GetIntegerVariate((int)randomMax);
              //TODO random generator does not support integer values in a given range (e.g. from 5-10)
              //range is always [0, (int)randomMax]
            }else if((type == typeid(double)) || (type == typeid(float))) //call integer function
            {
              imageBuffer[i] = (TPixelType)randomGenerator->GetUniformVariate(randMin,randomMax);
            }else if(type == typeid(unsigned char))
            {
              //use the integer randomGenerator with mod 256 to generate unsigned char values
              imageBuffer[i] = (unsigned char) ((int)randomGenerator->GetIntegerVariate((int)randomMax)) % 256;
            }else{
              MITK_ERROR << "Datatype not supported yet.";
              //TODO call different methods for other datatypes
            }
          }
          return output;
        }
  };
} // namespace mitk

#endif /* ImageGenerator_H_HEADER_INCLUDED */
