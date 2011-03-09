#include <mitkImage.h>
#include <mitkImageCreator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

//creates a test image with given dimension filled with random values
//template <typename TPixelType>
//   static mitk::Image::Pointer mitk::ImageCreator::CreateTestImage(unsigned int dimX,
//                                                             unsigned int dimY,
//                                                             unsigned int dimZ,
//                                                             unsigned int dimT,
//                                                             unsigned int channels,
//                                                             const double randomMax, const double randMin)
//{
//  mitk::PixelType type;
//  type.Initialize(typeid(TPixelType));
//  mitk::Image::Pointer output = mitk::Image::New();
//  unsigned int* dimensions = new unsigned int[4];
//  unsigned int numberOfDimensions = 0;
//  unsigned int bufferSize = 0;
//  bufferSize = dimX*dimY;

//  if(dimT == 0)
//  {
//    if(dimZ == 0)
//    {
//      numberOfDimensions = 2;
//    }
//    else
//    {
//      numberOfDimensions = 3;
//      //      bufferSize = dimX*dimY*dimZ;
//    }
//  }
//  else
//  {
//    numberOfDimensions = 4;
//    //    bufferSize = dimX*dimY*dimZ*dimT;
//  }

//  dimensions[0] = dimX;
//  dimensions[1] = dimY;
//  dimensions[2] = dimZ;
//  dimensions[3] = dimT;

//  output->Initialize(type, numberOfDimensions, dimensions, channels);

//  TPixelType* imageBuffer = (TPixelType*)output->GetData();
//  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();

//  for(unsigned int t = 0; t < dimT; t++)
//  {
//    for(unsigned int s = 0; s < dimZ; s++)
//    {
//      for(unsigned int i = 0; i < bufferSize; i++)
//      {
//        if(type == typeid(int)) //call integer function
//        {
//          imageBuffer[i] = (TPixelType)randomGenerator->GetIntegerVariate((int)randomMax);
//          //TODO random generator does not support integer values in a given range (e.g. from 5-10)
//          //range is always [0, (int)randomMax]
//        }else if((type == typeid(double)) || (type == typeid(float)))
//        {
//          imageBuffer[i] = (TPixelType)randomGenerator->GetUniformVariate(randMin,randomMax);
//        }else if(type == typeid(unsigned char))
//        {
//          imageBuffer[i] = ((TPixelType)randomGenerator->GetIntegerVariate((int)randomMax)) % 256;
//        }
//        //TODO call different methods for other datatypes
//      }
//      output->SetSlice(imageBuffer, s, t, channels);
//    }
//  }

//  return output;
//}
