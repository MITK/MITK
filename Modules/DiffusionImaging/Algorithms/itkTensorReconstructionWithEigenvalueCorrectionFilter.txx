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
#ifndef _itk_TensorReconstructionWithEigenvalueCorrectionFilter_txx_
#define _itk_TensorReconstructionWithEigenvalueCorrectioFiltern_txx_
#endif
#include "itkImageRegionConstIterator.h"
#include <math.h>
#include <mitkImageWriter.h>

namespace itk
{


  template <class TDiffusionPixelType, class TTensorPixelType>
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
    ::TensorReconstructionWithEigenvalueCorrectionFilter()
  {
    m_B0Threshold = 50.0;
  }

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
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
        b0index[i]=5;
      }
      else
      {
        b0index[i]=-1;
        directions[cnt][0] = vec[0];
        directions[cnt][1] = vec[1];
        directions[cnt][2] = vec[2];
        cnt++;
      }

    }

    vnl_matrix<double> dirsTimesDirsTrans = directions*directions.transpose();

    vnl_vector< double> diagonal(nof-numberb0);
    vnl_vector< double> b_vec(nof-numberb0);
    vnl_vector< double> temporary(3);

    for (int i=0;i<nof-numberb0;i++)
    {    
      diagonal[i]=dirsTimesDirsTrans[i][i];    
    }
  
    double max_diagonal = diagonal.max_value();    

    directions=directions*sqrt(m_BValue/1000.0)*(1.0/sqrt(max_diagonal));


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
    double temp_pixel;
   
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

    // end of the biggest data structure for this filter
    // begining of removing of negative values from the data set

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
            if(pixel2[f]<=0.0)
            {
              check_the_neighbors(x,y,z,f,size,ix,gradientImagePointer,pixelIndex,corrected_diffusion,temp_pixel);
              variableLengthVector[f] = temp_pixel;
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
    }
   
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
    mask->SetSpacing(gradientImagePointer->GetSpacing());
    mask->SetOrigin(gradientImagePointer->GetOrigin());
    mask->Allocate();
    ImageType_mask::IndexType pixelIndex_mask;

    //
    int mask_cnt=0;

    //
  
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
              mean_b = mean_b + pixel4[i];
            }
          }
          mean_b=mean_b/numberb0;
          if(mean_b>m_B0Threshold)
          {
            pixel = 3.0;
            //
            mask_cnt++;
            //
          }
          else
          {
            pixel=0.0;
          }
          mask->SetPixel(pixelIndex_mask, pixel);
        }
      }
    }

    //
    std::cout << "Number of voxels in mask: " << mask_cnt << std::endl;
    //
    
    // Declaration of tensor image- output of the filter
    typedef itk::Image< itk::DiffusionTensor3D<float>, 3 > TensorImageType;
    TensorImageType::IndexType start_tensorImg;
    start_tensorImg.Fill(0);
    TensorImageType::RegionType region_tensorImg(start_tensorImg,size);
    TensorImageType::Pointer tensorImg = TensorImageType::New();
    tensorImg->SetRegions(region_tensorImg);
    tensorImg->SetSpacing(gradientImagePointer->GetSpacing());
    tensorImg->SetOrigin(gradientImagePointer->GetOrigin());
    tensorImg->Allocate();
    itk::Index<3> pixelIndex_tensorImg;
    // end of tensor declaration
    // start of the main loop for removing of voxels with negative eigen values tensors
    //some important variables
    vnl_matrix<double> temp_tensor(3,3);
    vnl_vector<double> eigen_vals(3);
//    double l1;
  //  double l2;
    //double l3;
    int number_of_bads=0;
    int old_number_of_bads=10000000000000000;
    int diff=1;

    vnl_vector< double> pixel_max(nof);
    vnl_vector< double> pixel_min(nof);

    for (int i=1;i<nof;i++)
    {
      pixel_max[i]=exp(-b_vec[i]*0.01);// here some values for low and high diffusivity were pre define as 0.01 for low and 5 for high
      pixel_min[i]= exp(-b_vec[i]*5);
    }
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
            pixel = mask->GetPixel(ix);
            itk::DiffusionTensor3D<float> ten;
            
            pixelIndex_tensorImg[0]=x;
            pixelIndex_tensorImg[1]=y;
            pixelIndex_tensorImg[2]=z;            
            
            if(pixel>2.0)
            {
              ix[0] = x;
              ix[1] = y;
              ix[2] = z;
              GradientVectorType pt = corrected_diffusion->GetPixel(ix);
               mean_b=0.0;

  
              for (int i=0;i<nof;i++)
              {
                org_data[i]=pt[i];
              }
              calculate_attenuation(org_data,b0index,atten,mean_b,nof,numberb0);
              calculate_tensor(pseudoInverse,atten,tensor,nof,numberb0);

              temp_tensor[0][0]= tensor[0];temp_tensor[0][1]= tensor[3];temp_tensor[0][2]= tensor[5];
              temp_tensor[1][0]= tensor[3];temp_tensor[1][1]= tensor[1];temp_tensor[1][2]= tensor[4];
              temp_tensor[2][0]= tensor[5];temp_tensor[2][1]= tensor[4];temp_tensor[2][2]= tensor[2];

              vnl_symmetric_eigensystem <double> eigen_tensor(temp_tensor);

              eigen_vals[0]=eigen_tensor.get_eigenvalue(0);eigen_vals[1]=eigen_tensor.get_eigenvalue(1);eigen_vals[2]=eigen_tensor.get_eigenvalue(2);
                     

              if( eigen_vals[0]>0.0 && eigen_vals[1]>0.0 && eigen_vals[2]>0.0)
              {
               
                
                ten(0,0) = tensor[0];
                ten(0,1) = tensor[3];
                ten(0,2) = tensor[5];
                ten(1,1) = tensor[1];
                ten(1,2) = tensor[4];
                ten(2,2) = tensor[2];


               if(x==47 && y==69 && z==28)
               {
               std::cout <<"Joe tensor"<< "\n";

              std::cout <<tensor[0]<< "\n";
              std::cout <<tensor[1]<< "\n";
              std::cout <<tensor[2]<< "\n";
              std::cout <<tensor[3]<< "\n";
              std::cout <<tensor[4]<< "\n";
              std::cout <<tensor[5]<< "\n";

               }


                tensorImg->SetPixel(pixelIndex_tensorImg, ten);
                mask->SetPixel(ix,1.0);
              }//end of if eigenvalues
              else
              {
                number_of_bads++;
                ten.Fill(0.0);
                tensorImg->SetPixel(pixelIndex_tensorImg, ten);

                for (int f=0;f<nof;f++)
                {
                  if(pt[f]>pixel_max[f] || pt[f]<pixel_min[f])
                  {
                    check_the_neighbors(x,y,z,f,size,ix,gradientImagePointer,pixelIndex,corrected_diffusion,temp_pixel);
                    variableLengthVector[f] = temp_pixel;
                  }
                  
                  
                }

                mask->SetPixel(ix,3.0);
                corrected_diffusion->SetPixel(ix, variableLengthVector);
              } 
            }// end of if
            
            else if(pixel<1.0)
            {
              ten.Fill(0.0);
              tensorImg->SetPixel(pixelIndex_tensorImg, ten);
            }


          }// end of for 3
        }

      }// end of for 1
      diff=old_number_of_bads-number_of_bads;
      old_number_of_bads=number_of_bads;
      std::cout << "bad voxels: " << number_of_bads << std::endl;
      number_of_bads=0;

      
    } //while

    this->SetNthOutput(0, tensorImg);
    m_VectorImage = corrected_diffusion;
    m_MaskImage = mask;
    m_PseudoInverse = pseudoInverse;
    m_H = H;
    m_BVec=b_vec;

  }
  
  

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
    TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
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
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::check_the_neighbors(int x, int y, int z,int f, itk::Size<3> size,itk::Index<3> ix,
  typename GradientImagesType::Pointer gradientImagePointer,ImageType::IndexType pixelIndex,
    ImageType::Pointer corrected_diffusion,double &temp_pixel)   
  {
    typedef typename GradientImagesType::PixelType         GradientVectorType;
  
    int init_i;int init_j;int init_c;int limit_i;int limit_j;int limit_c;
    double tempsum=0.0;
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

          GradientVectorType p = gradientImagePointer->GetPixel(ix);
          
          if(p[f]<=0.0)
          {
            tempsum=tempsum + 0;
          }
          else
          {
            tempsum=tempsum+p[f];
            temp_number=temp_number+1;
          }
        }
      }
    }// end of for 1
    if (temp_number==0.0)
    {
      tempsum=0.0;
    }
    else
    {
      tempsum=tempsum/temp_number;
    }
        
    temp_pixel = (short)tempsum;
  }


  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::calculate_attenuation(vnl_vector<double> org_data,vnl_vector< double > b0index,vnl_vector<double> &atten, double mean_b,int nof, int numberb0)
  {
    mean_b=0.0;

    for (int i=0;i<nof;i++)
    {
      if(b0index[i]>0)
      {
        mean_b=mean_b+org_data[i];
      }
        
    }
    mean_b=mean_b/numberb0;
    int cnt=0;
    for (int i=0;i<nof;i++)
    {
      if(b0index[i]<0)
      {
        //if(org_data[i]<0.001){org_data[i]=0.01;}
        atten[cnt]=org_data[i]/mean_b;
        double frank=org_data[i]/mean_b;
        cnt++;
      }
    }
  }


  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::calculate_tensor(vnl_matrix<double> pseudoInverse,vnl_vector<double> atten,vnl_vector<double> &tensor, int nof,int numberb0)
  {
    for (int i=0;i<nof-numberb0;i++)
    {
      atten[i]=log((double)atten[i]);
    }
      tensor = pseudoInverse*atten;
  }// end of void calculate tensor
   
} // end of namespace



  

  

