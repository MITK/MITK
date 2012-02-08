/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorImageToDiffusionImageFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_FreeWaterEliminationFilter_txx_
#define _itk_FreeWaterEliminationFilter_txx_
#endif

#include "itkFreeWaterEliminationFilter.h"
#include "itkImageRegionConstIterator.h"

//vnl includes
#include <vnl/vnl_vector_fixed.h>

namespace itk
{


  template <class TDiffusionPixelType, class TTensorPixelType>
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::FreeWaterEliminationFilter()
  {

  }



  template <class TDiffusionPixelType, class TTensorPixelType>
  void
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    ::GenerateData ( )
  {


    typedef ImageRegionConstIterator< GradientImagesType > GradientIteratorType;
    typedef typename GradientImagesType::PixelType         GradientVectorType;
    typename GradientImagesType::Pointer gradientImagePointer = NULL;

    // Would have liked a dynamic_cast here, but seems SGI doesn't like it
    // The enum will ensure that an inappropriate cast is not done
    gradientImagePointer = static_cast< GradientImagesType * >(
                              this->ProcessObject::GetInput(0) );

    // Size of the complete image
    typename GradientImagesType::SizeType size = gradientImagePointer->GetLargestPossibleRegion().GetSize();

    // Find number of directions(B0 included)
    int nof = m_GradientDirectionContainer->Size();

    std::vector< vnl_vector_fixed< double, 3 > > directions;
    
    // Find te index of the B0
    int b0Index = -1;
    for(int i=0; i<nof; i++)
    {
      vnl_vector_fixed< double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);

      // Todo: verify if threshold is adequate
      if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001)
      {
        b0Index = i;
      }
      else
      {
        directions.push_back(vec);
      }
    }

    if(b0Index == -1)
    {
      MITK_ERROR << "No B0 found!!!";
      return;
    }


  

//declare table

    int ****table;
    table = new int***[nof-1];
    for (int g=0; g<nof-1; g++)
    {
      table[g] = new int**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        table[g][i] = new int*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          table[g][i][j] = new int[size[2]] ;
        }
      }
    }

    //example loop with step size of 10
    int in_g=0;

    for(int g=0; g<nof; g++)
    {
      
      for(int x=0; x<size[0]; x++)
      {
        for(int y=0; y<size[1]; y++)
        {
          for(int z=0; z<size[2]; z++)
          {	   

            if(g!= b0Index)

            {

            itk::Index<3> ix;
            ix[0] = x;
            ix[1] = y;
            ix[2] = z;

            GradientVectorType pixel2 = gradientImagePointer->GetPixel(ix);

            table[in_g][x][y][z] = pixel2.GetElement(g);// data without b0


            }
            else
            {
              in_g--;
            }
          }
        }
      }

      in_g++;
    }


    
    int *bvec;
    bvec = new int[nof-1];

    int *upper_threshold;
    upper_threshold = new int[nof-1];

    int *lower_threshold;
    lower_threshold = new int[nof-1];

    int *direction;
        direction = new int[nof-1];
    int *diagonal;
        diagonal = new int[nof-1];
    int *b_vec;
        b_vec = new int[nof-1];
    int counter_of_directions=0;

    int b= 1000;

    int **T_table;
        T_table = new int*[3];
        for (int f=0; f<3; f++)
        {
          T_table[f] = new int[3];
        }

    int **M_table;
        M_table = new int*[nof-1];
        for (int f=0; f<nof-1; f++)
        {
          M_table[f] = new int[nof-1];
        }
    int **H_table;
        H_table = new int*[nof-1];
        for (int f=0; f<nof-1; f++)
        {
          H_table[f] = new int[nof-1];
        }



        for(int i=0; i<m_GradientDirectionContainer->Size(); i++)
        {
          GradientDirectionType gradient = m_GradientDirectionContainer->ElementAt(i);          
        }

    // RAW to attenuation Main loop
 
      
      for(int x=0; x<size[0]; x++)
      {
        for(int y=0; y<size[1]; y++)
        {
          for(int z=0; z<size[2]; z++)
          {	   
              for(int g=0; g<nof-1; g++)
              {


                  table[g][x][y][z] = table[g][x][y][z]/directions[x][y][z];// realtive to b0, assuming that in directions we have loaded the image b0

                  signal[counter_of_directions] = table[g][x][y][z];
                  counter_of_directions++;

                }// end of directions loop

                      int temporary;
                      int cell_temporary;
                      for (int i=0;i<nof-1;i++)
                      {
                         for (int j=0;j<nof-1;j++)
                         {
                           cell_temporary=0;
                           for(int w=0;w<3;w++)
                           {
                           if(i==j)
                            {
                             temporary=direction[i][w]*direction[w][j];
                             temporary=cell_temporary+temporary;
                             cell_temporary=temporary;
                            }
                           }
                            if(i==j)
                            {
                             diagonal[i]=cell_temporary;
                            }
                         }
                      }// end of calculating diagonal


                      /// sprawdzic jak wyglada
                      for (int i=0;i<nof-1;i++)
                      {
                        direction[i]=direction[i]*(1/(sqrt(diagonal[i])));
                      }// end of final ccalculation of direction


                      int inner_counter=0;// second use of it
                      for (int i=0;i<nof-1;i++)
                      {
                         for (int j=0;j<nof-1;j++)
                         {
                           if(i==j)
                           {
                             b_vec[inner_counter]=direction[i]*direction[j];
                             inner_counter++;
                           }
                         }
                      }// end of calculating b_vector




                    for (int o=0;o<nof-1;o++)
                      {
                      for (int i=0;i<nof-1;i++)
                      {
                         for (int j=0;j<3;j++)
                                                                                     
                         {                                                 
                           =-direction[i]*direction[j];
                         }
                      }// end of calculating H_matrix
                      
                        
                           //M_table[o][1]=M_table[][]
                        
                      }// end of calculating H_matrix
                      
                    
                 

            }// end of z iteration


      } // end of y iteration


    }// end of x iteration


   





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////End/////////////////////////////////////////////////////////////////////////////////////

    std::string output;

    for(int g=0; g<nof; g++)
    {
      for(int x=0; x<size[0]; x+=25)
      {
        for(int y=0; y<size[1]; y+=25)
        {
          for(int z=0; z<size[2]; z+=25)
          {

            int value = table[g][x][y][z];
            std::stringstream stream;
            stream << table[g][x][y][z];
            output.append(stream.str());
            output.append(" ");
          }
          output.append("\n");
        }      
      }
      output.append("\n");
    }
    std::cout << output << std::endl;

    

      /////////////////vnl_vector_fixed<double, 3> vnlvec;

/////////////////////vnlvec[0] = 0;
/////////////////vnlvec[1] = 1;
///////////////////vnlvec[2] = 2;





  }



  template <class TDiffusionPixelType, class TTensorPixelType>
  void
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::SetGradientImage( GradientDirectionContainerType *gradientDirection,
                          const GradientImagesType *gradientImage )
  {
    // Make sure crazy users did not call both AddGradientImage and
    // SetGradientImage
    if( m_GradientImageTypeEnumeration == GradientIsInManyImages )
      {
      itkExceptionMacro( << "Cannot call both methods:"
      << "AddGradientImage and SetGradientImage. Please call only one of them.");
      }

    this->m_GradientDirectionContainer = gradientDirection;

    unsigned int numImages = gradientDirection->Size();
    this->m_NumberOfBaselineImages = 0;
    for(GradientDirectionContainerType::Iterator it = this->m_GradientDirectionContainer->Begin();
        it != this->m_GradientDirectionContainer->End(); it++)
      {
      if(it.Value().one_norm() <= 0.0)
        {
        this->m_NumberOfBaselineImages++;
        }
      else // Normalize non-zero gradient directions
        {
        it.Value() = it.Value() / it.Value().two_norm();
        }
      }

    this->m_NumberOfGradientDirections = numImages - this->m_NumberOfBaselineImages;

    // ensure that the gradient image we received has as many components as
    // the number of gradient directions
    if( gradientImage->GetVectorLength() != this->m_NumberOfBaselineImages + this->m_NumberOfGradientDirections )
      {
      itkExceptionMacro( << this->m_NumberOfGradientDirections << " gradients + " << this->m_NumberOfBaselineImages
                         << "baselines = " << this->m_NumberOfGradientDirections + this->m_NumberOfBaselineImages
                         << " directions specified but image has " << gradientImage->GetVectorLength()
        << " components.");
      }

    this->ProcessObject::SetNthInput( 0,
        const_cast< GradientImagesType* >(gradientImage) );
    m_GradientImageTypeEnumeration = GradientIsInASingleImage;
  }



} // end of namespace
