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


   int ****table;// table in which all images except b0 are store
    table = new int***[int nof-int numberb0];
    for (int g=0; g<int nof-int numberb0; g++)
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

    int ****sorted_eigs;// table in which all images except b0 are stored
    sorted_eigs = new int***[3];
    for (int g=0; g<3; g++)
    {
      sorted_eigs[g] = new int**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        sorted_eigs[g][i] = new int*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          sorted_eigs[g][i][j] = new int[size[2]] ;
        }
      }
    }
    
    int ****atten;// table in which all images except b0 are stored
    atten = new int***[int nof-int numberb0];
    for (int g=0; g<int nof-int numberb0; g++)
    {
      atten[g] = new int**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        atten[g][i] = new int*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          atten[g][i][j] = new int[size[2]] ;
        }
      }
    }
    
    int ****D;// table in which all images except b0 are stored
    D = new int***[6];
    for (int g=0; g<6; g++)
    {
      D[g] = new int**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        D[g][i] = new int*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          D[g][i][j] = new int[size[2]] ;
        }
      }
    }
    
    int ****b0image// table with all b0images
    b0image = new int***[int numberb0];
    for (int g=0; g<int numberb0; g++)
    {
      b0image[g] = new int**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        b0image[g][i] = new int*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          b0image[g][i][j] = new int[size[2]] ;
        }
      }
    }
    
   int ***b0_mean_image// table with mean b0 image
    b0_mean_image = new int**[size[0]];
    for (int g=0; g<size[0]; g++)
    {
      b0image[g] = new int*[size[1]];
      for ( int i = 0 ; i < size[1] ; i++ ) 
      {
        b0image[g][i] = new int[size[2]] ;
       
      }
    }

    int **directions;// declaration of directions int nofx3 table
        directions = new int*[int nof-int numberb0];
        for (int g=0; g<int nof; g++)
        {
          directions[g] = new int*[3];
        }

    vnl_matrix<float> directions(int int nof-int int numberb0,3);
    vnl_matrix<float> H(int int nof-int int numberb0, 6);
    vnl_matrix<float> tem_tensor(3,3);
    vnl_matrix<float> cell_temporary(int int nof-int int numberb0,int int nof-int int numberb0);
    vnl_matrix<int> H(int nof-int numberb0,6);
    vnl_matrix<float> b_temporary(int nof-int numberb0,int nof-int numberb0);
    vnl_vector_fixed< double, int nof-int numberb0 > higher_thresh;
    vnl_vector_fixed< double, int nof-int numberb0 > higher_thresh;
    vnl_vector_fixed< double, int nof-int numberb0 > diagonal;
    vnl_vector_fixed< double, int nof-int numberb0 > b_vec;
    vnl_vector_fixed< double, int numberb0 > b0index;

    int b= 1000;
    int in_g=0;
    int in_b=0;
    int temporaryb0sum=0;
    int cnt_dir=0;
    int max_diagonal=max(diagonal);        
    int etbt = { 0, 4, 8, 1, 5, 2 };
    int temporary[3];
    int tensor[9];
    int badnumberpresent=0;
    int badnumberpast=0;
    int wasdetectedbad=0;
    int LOW_D=0.01;
    int HIGH_D = 5;
    int oldnumberofbads=1000000000000;
    int newnumberofbads;
    int diff=1;
    int nof = m_GradientDirectionContainer->Size();
    int numberb0;
    int cnt=0;
    // end of declarations

    for(int i=0; i<int nof; i++)
    {
      vnl_vector_fixed< double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);

      
      if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001)
      {
        b0Index[cnt]=i;
        cnt++;
      }
      else
      {
        directions.push_back(vec);
      }
    }
    
    int numberb0=cnt;

    if(b0Index == -1)
    {
      MITK_ERROR << "No B0 found!!!";
      return;
    }


    for(int g=0; g<int nof; g++)
    {
      
      for(int x=0; x<size[0]; x++)
      {
        for(int y=0; y<size[1]; y++)
        {
          for(int z=0; z<size[2]; z++)
          {	   
            
            for (int c=0;c<int nof;c++)
            {
                if(g == b0index[c]){int flag=1;}
                else{int flag=0;}
            }

            if(flag==0)
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
              b0image[in_b][x][y][z] = pixel2.GetElement(g);// only b0 image
              in_b++;
            }
          }
        }
      }

      in_g++;
    }// stoaring data in table and b0images in b0 table
    

    for(int x =0;x<size[0];x++)
    {
        for(int y=0;y<size[1];y++)
        {
            for( int z=0;z<size[2];z++)
            {
                temporaryb0sum=0;
                for (int i=0;i<int numberb0;i++)
                {
                    temporaryb0sum=temporaryb0sum+b0image[i][x][y][z];         
                }
                b0_mean_image[x][y][z]=temporaryb0sum/int numberb0;  
            }
        }
    }

    for(int g=0; g<int nof; g++)
    {
      GradientDirectionType gradient = m_GradientDirectionContainer->ElementAt(g);
      for(int x=0; x<size[0]; x++)
      {
                for ( int i=0;i<int numberb0;i++)
                {
             
                    if(g!=b0index[i])
                        {
                         directions[cnt_dir][0] = gradient.GetElement(0);
                         directions[cnt_dir][1] = gradient.GetElement(1);
                         directions[cnt_dir][2] = gradient.GetElement(2);
                         }
                  }
      }

   }// collection of all gradient directions except b0
   
  cell_temporary=directions*transpose(directions);

  for (int i=0;i<int nof-int numberb0;i++)
           {
              for (int j=0;j<int nof-int numberb0;j++)
               {
                 if(i==j){diagonal[i]=cell_temporary;}
                }
             }// end of calculating diagonal
                        
  directions=directions*(b/1000)*(1/(sqrt(max_diag)));
  b_temporary=directions*transpose(directions);
             
  for (int i=0;i<int nof-int numberb0;i++)
         {
            for (int j=0;j<int nof-int numberb0;j++)
             {
               if(i==j){b_vec[i]=b_temporary;}  
              }
          }// end of calculating b_vector
   
    for (int i = 0; i < int nof-int numberb0; i++) 
    {
      for (int j = 0; j < 3; j++) {temporary[j] = -direction[i][j];}
   
      for (int j = 0; j < 3; j++) 
      {
        for (int k = 0; k < 3; k++) {tensor[k + 3 * j] = temporary[k] * direction[i][j];}
      }
      for (int j = 0; j < 6; j++) {H[i][j] = tensor[etbt[j]];}
      for (int j = 0; j < 3; j++) {H[i][3 + j] *= 2.0;}
    }
  
  H=pinverse(transpose(H)*H)*transpose(H);

  // Create Design Matrix end

   for ( int g=0;g<int nof-int numberb0;g++)
   {
       for ( int x=0;x<size[0];x++)
       {
           for ( int y=0;y<size[1];y++)
           {
               for ( int z=0;z<size[2];z++)

               { // if condition
                   check_the_neighbors(x,y,z,table,g,size);
               }
           }
       }
   }// removing voxels smaller than 0 from the data / i am not using this threshold for this step cause in my opinion its sensles - if data are below zero it would be below threshold for sure.
  
  for (int i=1;i<int nof-int numberb0,i++)
   {
      higher_thresh[i]=exp(-b_vec[i]*LOW_D);
      lower_thresh[i]= exp(-b_vec[i]*HIGH_D);
   }

   while(diff>0)
   {

     for ( int x=0;x<size[0];x++)
     {
         for ( int y=0;y<size[1];y++)
         {
             for ( int z=0;z<size[2];z++)
             {
                 remove_negative_eigs(x,y,z,&table,int nof,size,H,&D,&atten,higher_thresh,lower_thresh, &wasdetected bad,b0_image,&sorted_eigs,int numberb0);
             }
         }
     }
     newnumberofbads=wasdetectedbad;
     diff=oldnumberofbads-newnumberofbads;
     oldnumberofbads=wasdetectedbad;
   }


  }// end of class

 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::check_the_neighbors(int g, int x, int y, int z, int ****table, itk::Size<3> size)   
  {
   
     int temp_sum=0;
     int temp_number=0;
     int init_i;
     int init_j;
     int init_c;
     int limit_i;
     int limit_j;
     int limit_c;
    
     if(x==0)
     {
       init_i=x;
       limit_i=x+2;
     }
     else if(x==size[0])
     {
       init_i=x-1;
       limit_i=x+1;
     }
     else
     {
       init_i=x-1;
       limit_i=x+2;
     }
     
     for (int i=init_i;i<limit_i;i+2)
         
     {// 1
       if(y==0)
          {
             init_j=y;
             limit_j=y+2;
           }
       else if(y==size[1])
           {
              init_j=y-1;
              limit_j=y+1;
            }
       else
           {
              init_j=y-1;
              limit_j=y+2;
            }

        for (int j=init_j;j<limit_j;j+2)
        {//2

           if(z==0)
              {
               init_c=z;
               limit_c=z+2;
               }
           else if(z==size[2])
              {
                init_c=z-1;
                limit_c=z+1;
              }
           else
               {
                init_c=z-1;
                limit_c=z+2;
                }
           for (int c=init_c;c<limit_c;c+2)
           {//3
               
               
               if(table[g][i][j][c]<=0)
               {
                temp_sum=temp_sum+ 0;
               }
               else
               {
                 temp_sum=temp_sum+table[g][i][j][c];
                 temp_number++;
               }
            }//3
         }//2
    }//1

   if (temp_number==0)
    {
    table[g][x][y][z]=0;
    }
     else
    {
      table[g][x][y][z]=temp_sum/temp_number;
     }
   

   }// end of void

  template <class TDiffusionPixelType, class TTensorPixelType>
  void 
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::remove_negative_eigs(int x, int y, int z, int ****table, 
                            int nof, itk::Size<3> size, 
                            vnl_matrix<float> H, int ****D, int ****atten, 
                            vnl_vector<float> higher_thresh, 
                            vnl_vector<float> lower_thresh, int wasdetectedbad, 
                            int ***b0_mean_image, int ****sorted_eigs, int numberb0)
  {//0
   int temp_sum=0;
   int temp_number=0;
   int init_i;
   int init_j;
   int init_c;
   int limit_i;
   int limit_j;
   int limit_c;
   int temp_product;
   int flag;
   vnl_matrix<float> temporary(int nof-1,3);
   vnl_vector<float> atten_log(int nof-1);
   vnl_vector<float> temp_atten(int nof-1);
   vnl_vector<float> temp_D(6);
   
   for (int u=0;u<int nof-int numberb0;u++)
   {
     atten[u][x][y][z]=table[u][x][y][z]/b0_mean_image[x][y][z];
     atten_log=log(atten[u][x][y][z]);
     temp_atten[u]=atten[u][x][y][z];
   }
   temp_D = H*temp_atten;
   
   for (int u=0;u<6;u++){D[u][x][y][z]=temp_D[u];}
   
   int a= temp_D[0];
   int aa= temp_D[3];
   int aaa= temp_D[5];
   int b= temp_D[3];
   int bb= temp_D[1];
   int bbb= temp_D[4];
   int d= temp_D[5];
   int dd= temp_D[4];
   int ddd= temp_D[2];/// check it- 16.02 its ok.
   int l1;
   int l2;
   int l3;
   vnl_symetric_eigensystem_compute_eigenvals<float>(a,aa,aaa,b,bb,bbb,d,dd,ddd,&l1,&l2,&l3);

   if( l1>0.01 && l2>0.01 &&l3>0.01){flag=0;}
   else{flag=1;}
   
   sorted_eigs[0][x][y][z]=l1;
   sorted_eigs[1][x][y][z]=l2;
   sorted_eigs[2][x][y][z]=l3;
           
   if(flag==1)// 1
   {
     wasdetectedbad=wasdetectedbad++;
       
     for (int u=0;u<int nof-int numberb0;u++)//2  
    {
      if(table[u][x][y][z]<lower_thresh[u] || table[u][x][y][z]>higher_thresh[u])   
      {// 3
        if(x==0)
        {
          init_i=x;
          limit_i=x+2;
        }
        else if(x==size[0])
        {
          init_i=x-1;
          limit_i=x+1;
        }
        else
        {
          init_i=x-1;
          limit_i=x+2;
        }

        for (int i=init_i;i<limit_i;i+2)//4
        {
        if(y==0)
        {
          init_j=y;
          limit_j=y+2;
        }
        else if(y==size[1])
        {
          init_j=y-1;
          limit_j=y+1;
        }
        else
        {
          init_j=y-1;
          limit_j=y+2;
        }
        for (int j=init_j;j<limit_j;j+2)//5
        {
        if(z==0)
        {
          init_c=z;
          limit_c=z+2;
        }
        else if(z==size[2])
        {
          init_c=z-1;
          limit_c=z+1;
        }
        else
        {
          init_c=z-1;
          limit_c=z+2;
        }
        for (int c=init_c;c<limit_c;c+2)
        {// 6
          if(table[u][i][j][c]<=0)
          {
           temp_sum=temp_sum+ 0;
          }
          else
          {
           temp_sum=temp_sum + table[u][i][j][c];
           temp_number++;
          }
        }//5

        if (temp_number==0){table[u][x][y][z]=0;}
        else{ table[u][x][y][z]=temp_sum/temp_number;}     

      } //end if//4
           
    } // end of for//3
   
  } // end of if//2
 }//1
}// end of void//0

} // end of namespace


}


