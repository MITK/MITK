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
#include <math.h>
#include <mitkImageWriter.h>

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

    /// First filter 
    int nof = m_GradientDirectionContainer->Size();
    int numberb0=0;
    vnl_vector< double > b0index(nof);
    int cnt=0;
    
    vnl_matrix<double> directions(nof-numberb0,3);

    for(int i=0; i<nof; i++)
      {
       vnl_vector_fixed <double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);

       if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001 && vec[0]>-0.001&& vec[1]>-0.001 && vec[2]>-0.001)
        {
         numberb0++;
         b0index[i]=i;
        }// end if
       else
        {
         b0index[i]=-1;
         directions[cnt][0] = vec[0];
         directions[cnt][1] = vec[1];
         directions[cnt][2] = vec[2];
         cnt++;
        }//end else

      }// end for

    vnl_matrix<double> dirsTimesDirsTrans = directions*directions.transpose();

    vnl_vector< double> diagonal(nof-numberb0);
    vnl_vector< double> b_vec(nof-numberb0);
    vnl_vector< double> temporary(3);

    for (int i=0;i<nof-numberb0;i++)
      {    
       diagonal[i]=dirsTimesDirsTrans[i][i];    
      }
    
    double max_diagonal = diagonal.max_value();
    double b_value=1000.0;

    directions=directions*sqrt(b_value/1000.0)*(1.0/sqrt(max_diagonal));


    for (int i=0;i<nof-numberb0;i++)
      {   
       b_vec[i]= dirsTimesDirsTrans[i][i];   
      }

    vnl_matrix<double> H(nof-numberb0, 6);
    vnl_matrix<double> temp_3_3(3,3);
    vnl_vector<double> pre_tensor(9);
  
    int etbt[6] = { 0, 4, 8, 1, 5, 2 };

    for (int i = 0; i < nof-numberb0; i++) 
       {
        for (int j = 0; j < 3; j++) 
          {
           temporary[j] = -directions[i][j];
          }
        for (int j = 0; j < 3; j++) 
          {
           for (int k = 0; k < 3; k++)
             {
              pre_tensor[k + 3 * j] = temporary[k] * directions[i][j];            
             }
           }
        for (int j = 0; j < 6; j++) 
          {
           H[i][j] = pre_tensor[etbt[j]];
          }
        for (int j = 0; j < 3; j++) 
          {
           H[i][3 + j] *= 2.0;
          }
        }

     vnl_matrix<double> inputtopseudoinverse=H.transpose()*H;
     vnl_symmetric_eigensystem<double> eig( inputtopseudoinverse);
     vnl_matrix<double> pseudoInverse = eig.pinverse()*H.transpose();
     
     itk::Index<3> ix;

    // declaration of corrected diffusion image///
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::RegionType region(start,size);
    ImageType::Pointer corrected_diffusion = ImageType::New();
    corrected_diffusion->SetRegions(region);
    corrected_diffusion->SetVectorLength(nof);
    corrected_diffusion->Allocate();
    ImageType::IndexType pixelIndex;
    typedef itk::VariableLengthVector<short> VariableVectorType;
    VariableVectorType variableLengthVector;
    variableLengthVector.SetSize(nof);

    // end of the bigest data structure for this filter

    // begening of removing of negative values from the data set

    for ( int x=0;x<size[0];x++)
      {
        for ( int y=0;y<size[1];y++)
        {
          for ( int z=0;z<size[2];z++)
          {          
           ix[0] = x;
           ix[1] = y;
           ix[2] = z;
           GradientVectorType pixel2 = gradientImagePointer->GetPixel(ix);
           for( int f=0;f<nof;f++)
           {
             if(pixel2[f]<0.0)
             {
             check_the_neighbors(x,y,z,f,size,ix,gradientImagePointer,pixelIndex,corrected_diffusion);
             }
             else
             { 
              variableLengthVector[f] = pixel2[f];
             }
           }
           pixelIndex[0] = x;
           pixelIndex[1] = y;
           pixelIndex[2] = z;
           corrected_diffusion->SetPixel(pixelIndex, variableLengthVector);
          }
        }
    }//end of calling the function for negative data points removal
    vnl_vector<double> org_data(nof);
    vnl_vector<double> atten(nof-numberb0);
    double mean_b=0.0;
    double pixel=0.0;
    vnl_vector<double> tensor (6);
    // Declaration of mask-1-good voxel , 0-bad one, initaialy good voxels indicated by 3
    typedef itk::Image<double, 3> ImageType_mask;
    ImageType_mask::IndexType start_mask;
    start_mask.Fill(0);
    ImageType_mask::RegionType region_mask(start_mask,size);
    ImageType_mask::Pointer mask = ImageType_mask::New();
    mask->SetRegions(region_mask);
    mask->Allocate();
    ImageType_mask::IndexType pixelIndex_mask;
    // end of mask declaration
    // start of filling mask
    for(int x=0;x<size[0];x++)
    {
      for(int y=0;y<size[1];y++)
      {
        for(int z=0;z<size[2];z++)
        {
         mean_b=0.0;
         ix[0] = x;
         ix[1] = y;
         ix[2] = z;
         pixelIndex_mask[0] = x;
         pixelIndex_mask[1] = y;
         pixelIndex_mask[2] = z;
         GradientVectorType pixel4 = gradientImagePointer->GetPixel(ix);
         for (int i=0;i<nof;i++)
         {
         if(b0index[i]>0)
           {
            mean_b=mean_b + pixel4[i];
           }
          }
         if(mean_b>50.0)
         {
           pixel = 3.0;
         }
         else
         {
           pixel=0.0;
         }
         mask->SetPixel(pixelIndex_mask, pixel);
        }
      }

    }// end of generation of mask
    // Declaration of tensor image- output of the filter
    typedef itk::Image<3,DiffusionTensor3D> ImageType_tensorImg;
    ImageType_tensorImg::IndexType start_tensorImg;
    start_tensorImg.Fill(0);
    ImageType_tensorImg::RegionType region_tensorImg(start_tensorImg,size);
    ImageType_tensorImg::Pointer tensorImg = ImageType_tensorImg::New();
    tensorImg->SetRegions(region_tensorImg);
    tensorImg->Allocate();
    ImageType_tensorImg::IndexType pixelIndex_tensorImg;
    // end of tensor declaration
    // start of the main loop for removing of voxels with negative eigen values tensors
    //some important variables
    vnl_matrix<double> temp_tensor(3,3);
    vnl_vector<double> eigen_vals(3);
    double l1;
    double l2;
    double l3;
    int number_of_bads=0;
    int old_number_of_bads=0;
    int diff=10000000000000000;
    while(diff>0)
    {
    for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            ix[0] = x;
            ix[1] = y;
            ix[2] = z;
            GradientVectorType pixel = mask->GetPixel(ix);
            if(pixel>1.0)
            {
              ix[0] = x;
              ix[1] = y;
              ix[2] = z;
              GradientVectorType pixel4 = corrected_diffusion->GetPixel(ix);
              for (int i=0;i<nof;i++)
              {
                org_data[i]=pixel4[i];
              }
              calculate_attenuation(org_data,b0index,atten,mean_b,nof);
              calculate_tensor(pseudoInverse,atten,tensor);

              temp_tensor[0][0]= tensor[0];temp_tensor[0][1]= tensor[3];temp_tensor[0][2]= tensor[5];
              temp_tensor[1][0]= tensor[3];temp_tensor[1][1]= tensor[1];temp_tensor[1][2]= tensor[4];
              temp_tensor[2][0]= tensor[5];temp_tensor[2][1]= tensor[4];temp_tensor[2][2]= tensor[2];

              vnl_symmetric_eigensystem <double> eigen_tensor(temp_tensor);

              eigen_vals[0]=eigen_tensor.get_eigenvalue(0);eigen_vals[1]=eigen_tensor.get_eigenvalue(1);eigen_vals[2]=eigen_tensor.get_eigenvalue(2);

               pixelIndex_tensorImg[0]=x;
               pixelIndex_tensorImg[1]=y;
               pixelIndex_tensorImg[2]=z;

              if( eigen_vals[0]>0.0 && eigen_vals[1]>0.0 && eigen_vals[2]>0.0)
               {
                 tensorImg->SetPixel(pixelIndex_tensorImg, tensor);
                 mask->SetPixel(ix,1.0);
               }//end of if eigenvalues
               else
               {
                 tensor[0]=0.0;tensor[1]=0.0;tensor[2]=0.0;tensor[3]=0.0;tensor[4]=0.0; pixelIndex_tensorImg[5]=x;
                 tensorImg->SetPixel(pixelIndex_tensorImg, tensor);
                 check_the_neighbors(x,y,z,f,size,ix,gradientImagePointer,pixelIndex,corrected_diffusion);
                 mask->SetPixel(ix,0.0);
                 number_of_bad++;
               }// 

            }// end of if

          }// end of for 3

        }

      }// end of for 1
      diff=old_number_of_bads-number_of_bads;
      old_number_of_bads=number_of_bads;
   } 
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
    ::check_the_neighbors(int x, int y, int z,int f, itk::Size<3> size,itk::Index<3> ix,
    typename GradientImagesType::Pointer gradientImagePointer,ImageType::IndexType pixelIndex,ImageType::Pointer corrected_diffusion)   
    {
    
    int init_i;int init_j;int init_c;int limit_i;int limit_j;int limit_c;
    double temp_sum=0.0;
    double temp_number=0.0;


    if(x==0){init_i=x;limit_i=x+2;}
    else if(x==size[0]-1){init_i=x-1;limit_i=x+1;}
    else{init_i=x-1;limit_i=x+2;}

    for (int i=init_i;i<limit_i;i+=2)
    {
      if(y==0){init_j=y;limit_j=y+2;}
      else if(y==size[1]-1){init_j=y-1;limit_j=y+1;}
      else{init_j=y-1;limit_j=y+2;}

      for (int j=init_j;j<limit_j;j+=2)
      {
        if(z==0){init_c=z;limit_c=z+2;}
        else if(z==size[2]-1){init_c=z-1;limit_c=z+1;}
        else{init_c=z-1;limit_c=z+2;}

        for (int c=init_c;c<limit_c;c+=2)
        {
          ix[0] = i;
          ix[1] = j;
          ix[2] = c;

          GradientVectorType pixel3 = gradientImagePointer->GetPixel(ix);
          
           if(pixel2[f]<=0.0)
            {
             temp_sum[f]=temp_sum[f] + 0;
             }
           else
             {
               temp_sum[f]=temp_sum[f]+pixel2[f];
               temp_number[f]=temp_number[f]+1;
             }
          }
        }
      }// end of for 1
      if (temp_number==0.0)
        {
         temp_sum=0.0;
        }
       else
         {
          tem_sum=temp_sum/temp_number;
         }
        
     
        variableLengthVector[f] = (short)temp_sum;
  }// end of void check the neighbors


  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::calculate_attenuation(vnl_vector<double> org_data,vnl_vector< double > b0index,vnl_vector<double> atten, double mean_b,int nof)
  {
    mean_b=0.0;

    for (int i=0;i<nof;i++)
    {
      if(b0index[i]>0)
      {
        mean_b=mean_b+org_data[i];
      }
    }
    int cnt=0;
    for (int i=0;i<nof;i++)
    {
      if(b0index[i]<0)
      {
        atten[cnt]=org_data[i]/mean_b;
        cnt++;
      }
    }
  }// end of void calculate attenuation


  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::calculate_tensor(vnl_matrix<double> pseudoInverse,vnl_vector<double> atten,vnl_vector<double> tensor)
  {
    for (int i=0;i<nof-numberb0;i++)
    {
      atten[i]=log((double)atten[i]);
    }
      tensor = pseudoInverse*atten;
  }// end of void calculate tensor
   
   } // end of namespace

  }

  

  

