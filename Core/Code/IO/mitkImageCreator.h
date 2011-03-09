/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef ImageCreator_H_HEADER_INCLUDED
#define ImageCreator_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkImageSource.h"
#include <mitkImage.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

namespace mitk {
  //##Documentation
  //## @brief random generator for MITK images
  //## @ingroup IO
  class MITK_CORE_EXPORT ImageCreator
  {
  public:
    template <typename TPixelType>
        static mitk::Image::Pointer CreateRandomImage(unsigned int dimX,
                                                    unsigned int dimY,
                                                    unsigned int dimZ = 1,
                                                    unsigned int dimT = 1,
                                                    unsigned int channels = 1,
                                                    const double randomMax = 1000.0f, const double randMin = 0.0f)
    {
      mitk::PixelType type;
      type.Initialize(typeid(TPixelType));
      mitk::Image::Pointer output = mitk::Image::New();
      unsigned int* dimensions = new unsigned int[4];
      unsigned int numberOfDimensions = 0;
      unsigned int bufferSize = dimX*dimY;

      if(dimT == 0)
      {
        if(dimZ == 0)
        {
          numberOfDimensions = 2;
        }
        else
        {
          numberOfDimensions = 3;
          //      bufferSize = dimX*dimY*dimZ;
        }
      }
      else
      {
        numberOfDimensions = 4;
        //    bufferSize = dimX*dimY*dimZ*dimT;
      }

      dimensions[0] = dimX;
      dimensions[1] = dimY;
      dimensions[2] = dimZ;
      dimensions[3] = dimT;

      output->Initialize(type, numberOfDimensions, dimensions, channels);

      TPixelType* imageBuffer = (TPixelType*)output->GetData();
      itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();

      //for all time steps
      for(unsigned int t = 0; t < dimT; t++)
      { //for all slices
        for(unsigned int s = 0; s < dimZ; s++)
        { //for all pixels
          for(unsigned int i = 0; i < bufferSize; i++)
          {
            if(type == typeid(int)) //call integer function
            {
              MITK_INFO << "1";
              imageBuffer[i] = (TPixelType)randomGenerator->GetIntegerVariate((int)randomMax);
              //TODO random generator does not support integer values in a given range (e.g. from 5-10)
              //range is always [0, (int)randomMax]
            }else if((type == typeid(double)) || (type == typeid(float))) //call integer function
            {
              MITK_INFO << "2";
              imageBuffer[i] = (TPixelType)randomGenerator->GetUniformVariate(randMin,randomMax);
            }else if(type == typeid(unsigned char))
            {
              MITK_INFO << "3";
              imageBuffer[i] = ((int)randomGenerator->GetIntegerVariate((int)randomMax)) % 256;
            }else{
              MITK_INFO << "4";
            }
            //TODO call different methods for other datatypes
          }
          output->SetSlice(imageBuffer, s, t, channels);
        }
      }

      return output;
    }

  };
} // namespace mitk

#endif /* ImageCreator_H_HEADER_INCLUDED */
