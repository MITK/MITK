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
#ifndef _itk_TensorReconstructionWithEigenvalueCorrectionFilter_txx_
#define _itk_TensorReconstructionWithEigenvalueCorrectionFilter_txx_
#endif
#include "itkImageRegionConstIterator.h"
#include <math.h>
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkImageRegionIterator.h"

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
::GenerateData ()
{

  m_GradientImagePointer = static_cast< GradientImagesType * >( this->ProcessObject::GetInput(0) );

  typename GradientImagesType::SizeType size = m_GradientImagePointer->GetLargestPossibleRegion().GetSize();

  // number of volumes
  int nof = m_GradientDirectionContainer->Size();

  // determine the number of b-zero values
  int numberb0=0;
  for(int i=0; i<nof; i++)
  {
    vnl_vector_fixed <double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);
    // due to roundings, the values are not always exactly zero
    if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001 && vec[0]>-0.0001&& vec[1]>-0.0001 && vec[2]>-0.0001)
    {
      numberb0++;
    }
  }


  // Matrix to store all diffusion encoding gradients
  vnl_matrix<double> directions(nof-numberb0,3);

  m_B0Mask.set_size(nof);


  int cnt=0;
  for(int i=0; i<nof; i++)
  {
    vnl_vector_fixed <double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);

    if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001 && vec[0]>-0.0001&& vec[1]>-0.0001 && vec[2]>-0.0001)
    {
      // the diffusion encoding gradient is approximately zero, wo we are dealing with a non-diffusion weighted volume
      m_B0Mask[i]=1;
    }
    else
    {
      // dealing with a diffusion weighted volume
      m_B0Mask[i]=0;

      // set the diffusion encoding gradient to the directions matrix
      directions[cnt][0] = vec[0];
      directions[cnt][1] = vec[1];
      directions[cnt][2] = vec[2];

      cnt++;
    }
  }

  // looking for maximal norm among gradients.
  // The norm is calculated with use of spectral radius theorem- based on determination of eigenvalue.

  vnl_matrix<double> dirsTimesDirsTrans = directions*directions.transpose();

  vnl_vector< double> diagonal(nof-numberb0);
  vnl_vector< double> b_vec(nof-numberb0);
  vnl_vector< double> temporary(3);


  for (int i=0;i<nof-numberb0;i++)
  {
    diagonal[i]=dirsTimesDirsTrans[i][i];
  }
  // normalization: free-water method assumes that directions matrix norm 1 is equal to b=1000
  directions=directions*sqrt(m_BValue/1000.0);

  //calculation of norms and storing them in vector

  dirsTimesDirsTrans = directions*directions.transpose();

  for (int i=0;i<nof-numberb0;i++)
  {
    b_vec[i]= dirsTimesDirsTrans[i][i];
  }


  // Calculation of so called design matrix that is used to obtain expected signal.

  vnl_matrix<double> H(nof-numberb0, 6);
  vnl_matrix<double> H_org(nof-numberb0, 6);
  vnl_vector<double> pre_tensor(9);

  //H is matrix that contains covariances for directions. It is stored twice because its original value is needed later
  // while H is changed

  int etbt[6] = { 0, 4, 8, 1, 5, 2 };// tensor order

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

  H_org=H;

  // calculation of inverse matrix by means of pseudoinverse

  vnl_matrix<double> inputtopseudoinverse=H.transpose()*H;
  vnl_symmetric_eigensystem<double> eig( inputtopseudoinverse);
  vnl_matrix<double> pseudoInverse = eig.pinverse()*H.transpose();


  typedef itk::Image<short, 3> MaskImageType;
  MaskImageType::Pointer mask = MaskImageType::New();
  mask->SetRegions(m_GradientImagePointer->GetLargestPossibleRegion().GetSize());
  mask->SetSpacing(m_GradientImagePointer->GetSpacing());
  mask->SetOrigin(m_GradientImagePointer->GetOrigin());
  mask->SetDirection( m_GradientImagePointer->GetDirection() );  // Set the image direction
  mask->SetLargestPossibleRegion( m_GradientImagePointer->GetLargestPossibleRegion() );
  mask->SetBufferedRegion( m_GradientImagePointer->GetLargestPossibleRegion() );
  mask->SetRequestedRegion( m_GradientImagePointer->GetLargestPossibleRegion() );
  mask->Allocate();

  // Image thresholding: For every voxel mean B0 image is calculated and then voxels of mean B0 less than the
  // treshold on the B0 image proviced by the userare excluded from the dataset with use of defined mask image.
  // 1 in mask voxel means that B0 > assumed treshold.

  int mask_cnt=0;
  for(unsigned int x=0;x<size[0];x++)
  {
    for(unsigned int y=0;y<size[1];y++)
    {
      for(unsigned int z=0;z<size[2];z++)
      {
        double mean_b=0.0;
        itk::Index<3> ix = {{x,y,z}};

        GradientVectorType pixel = m_GradientImagePointer->GetPixel(ix);
        for (int i=0;i<nof;i++)
        {
          if(m_B0Mask[i]==1)
          {
            mean_b = mean_b + pixel[i];
          }
        }
        mean_b=mean_b/numberb0;
        if(mean_b > m_B0Threshold)
        {
          mask->SetPixel(ix, 1);
          mask_cnt++;
        }
        else
          mask->SetPixel(ix, 0);
      }
    }
  }

  double mask_val=0.0;
  vnl_vector<double> org_vec(nof);

  int counter_corrected =0;

  for (unsigned int x=0;x<size[0];x++)
  {
    for (unsigned int y=0;y<size[1];y++)
    {
      for (unsigned int z=0;z<size[2];z++)
      {
        itk::Index<3> ix = {{x,y,z}};

        mask_val = mask->GetPixel(ix);

        GradientVectorType pixel2 = m_GradientImagePointer->GetPixel(ix);

        for (int i=0;i<nof;i++)
        {
          org_vec[i]=pixel2[i];
        }

        if(mask_val >0)
        {
          for( int f=0;f<nof;f++)
          {
            if(org_vec[f] <= 0)
            {
              org_vec[f] = CheckNeighbours(x,y,z,f,size,mask,m_GradientImagePointer);
              counter_corrected++;
            }
          }

          for (int i=0;i<nof;i++)
            pixel2[i]=org_vec[i];

          m_GradientImagePointer->SetPixel(ix, pixel2);

        }
      }
    }
  }

  typename TensorImageType::Pointer tensorImg;
  tensorImg = TensorImageType::New();
  tensorImg->SetSpacing( m_GradientImagePointer->GetSpacing() );   // Set the image spacing
  tensorImg->SetOrigin( m_GradientImagePointer->GetOrigin() );     // Set the image origin
  tensorImg->SetDirection( m_GradientImagePointer->GetDirection() );  // Set the image direction
  tensorImg->SetLargestPossibleRegion( m_GradientImagePointer->GetLargestPossibleRegion() );
  tensorImg->SetBufferedRegion( m_GradientImagePointer->GetLargestPossibleRegion() );
  tensorImg->SetRequestedRegion( m_GradientImagePointer->GetLargestPossibleRegion() );
  tensorImg->Allocate();

  //Declaration of vectors that contains too high or too low atenuation for each gradient. Attenuation is only calculated for
  //non B0 images so nof-numberb0.

  vnl_vector< double> pixel_max(nof-numberb0);
  vnl_vector< double> pixel_min(nof-numberb0);

  // to high and to low attenuation is calculated with use of highest allowed =5 and lowest allowed =0.01 diffusion coefficient

  for (int i=0;i<nof-numberb0;i++)
  {
    pixel_max[i]=exp(-b_vec[i]*0.01);
    pixel_min[i]= exp(-b_vec[i]*5);
  }

  m_PseudoInverse = pseudoInverse;
  m_H = H_org;
  m_BVec=b_vec;

  // in preprocessing we are dealing with 3 types of voxels: voxels excluded = 0 in mask, voxels correct =1 and voxels under correction
  //= 2. During pre processing most of voxels should be switched from 2 to 1.




  // what mask is a variable declared to simplify tensor calculaton. Tensors are obtained only for voxels that have a mask value bigger
  // than what_mask. Sometimes it is 1 sometimes 2.

  // initialization of required variables
  double what_mask=1.0;
  double set_mask=2.0;
  double previous_mask=0;

  double old_number_negative_eigs=0;
  double new_number_negative_eigs=0;

  bool  stil_correcting = true;

  TurnMask(size,mask,previous_mask,set_mask);
  // simply defining all possible tensors as with negative eigenvalues


  //Preprocessing is performed in multiple iterations as long as the next iteration does not increase the number of bad voxels.
  //The final DWI should be the one that has a smaller or equal number of bad voxels as in the
  //previous iteration. To obtain this temporary DWI image must be stored in memory.


  // ! Initial Calculation of Tensors
  std::cout << "Initial Tensor" << std::endl;
  GenerateTensorImage(nof,numberb0,size,m_GradientImagePointer,mask,what_mask,tensorImg);


  // checking how many tensors have problems, this is working only for mask =2
  old_number_negative_eigs = CheckNegatives (size,mask,tensorImg);


  //info for the user printed in the consol-debug information to be removed in the future
  std::cout << "Number of negative eigenvalues: " << old_number_negative_eigs << std::endl;

  CorrectDiffusionImage(nof,numberb0,size,m_GradientImagePointer,mask,pixel_max,pixel_min);


  while (stil_correcting == true)
  {
    //info for the user printed in the consol-debug information to be removed in the future
    std::cout << "Number of negative eigenvalues: " << old_number_negative_eigs << std::endl;

    GenerateTensorImage(nof,numberb0,size,m_GradientImagePointer,mask,what_mask,tensorImg);

    new_number_negative_eigs = CheckNegatives (size,mask,tensorImg);

    if(new_number_negative_eigs<old_number_negative_eigs)
    {
      // if the correction does not introduce new negative eigenvalues and corrects some of negative detected n previous iteration
      // smoothed DWI is used to substitute DWI from previous iteration
      stil_correcting=true;
      old_number_negative_eigs=new_number_negative_eigs;

    }

    else
    {
      stil_correcting=false;
    }

    //14.10.2013
    //CorrectDiffusionImage(nof,numberb0,size,corrected_diffusion_temp,mask,pixel_max,pixel_min);
    CorrectDiffusionImage(nof,numberb0,size,m_GradientImagePointer,mask,pixel_max,pixel_min);
  }

  // Changing the mask value for voxels that are still not corrcted. Original idea is to take them into analysis even though
  // eigenvalues are still negative
  TurnMask(size, mask,1,1);

  // Generation of final pre-processed tensor image that might be used as an input for FWE method

  // Final Tensor Reconstruction
  std::cout << "Final tensors " << std::endl;
  GenerateTensorImage(nof,numberb0,size,m_GradientImagePointer,mask,what_mask,tensorImg);

  m_MaskImage = mask;



  // Change diffusivity representation to standard convention
  itk::ImageRegionIterator<OutputType> outputIterator(tensorImg, tensorImg->GetLargestPossibleRegion());
  outputIterator.GoToBegin();
  while(!outputIterator.IsAtEnd())
    {
    TensorPixelType tens = outputIterator.Get();

    tens/= 1000.0;

    outputIterator.Set(tens);
    ++outputIterator;
    }

  this->SetNthOutput(0, tensorImg);
}



template <class TDiffusionPixelType, class TTensorPixelType>
void
TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
::SetGradientImage( GradientDirectionContainerType *gradientDirection,
                    const GradientImagesType *gradientImage )
{

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
double
TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
::CheckNeighbours(int x, int y, int z,int f, itk::Size<3> size, itk::Image<short, 3>::Pointer mask, typename GradientImagesType::Pointer corrected_diffusion_temp)
{
  // method is used for finding a new value for the voxel with use of its 27 neighborhood. To perform such a smoothing correct voxels are
  // counted an arithmetical mean is calculated and stored as a new value for the voxel. If there is no proper neigborhood voxel is turned
  // to the value of 0.

  // Definition of neighbourhood avoiding crossing the image boundaries
  int x_max=size[0]-1;
  int y_max=size[1]-1;
  int z_max=size[2]-1;

  double back_x=std::max(0,x-1);
  double back_y=std::max(0,y-1);
  double back_z=std::max(0,z-1);

  double forth_x=std::min((x+1),x_max);
  double forth_y=std::min((y+1),y_max);
  double forth_z=std::min((z+1),z_max);


  double tempsum=0;
  double temp_number=0;

  for(int i=back_x; i<=forth_x; i++)
  {
    for (int j=back_y; j<=forth_y; j++)
    {
      for (int k=back_z; k<=forth_z; k++)
      {
        itk::Index<3> ix = {{i,j,k}};

        GradientVectorType p = corrected_diffusion_temp->GetPixel(ix);

        if (p[f] > 0.0 )// taking only positive values and counting them
        {
          if(!(i==x && j==y && k== z))
          {
            tempsum=tempsum+p[f];
            temp_number++;
          }
        }
      }
    }
  }

  //getting back to the original position of the voxel

  itk::Index<3> ix = {{x,y,z}};

  if (temp_number <= 0.0)
  {
    tempsum=0;
    mask->SetPixel(ix,0);
  }
  else
  {
    tempsum=tempsum/temp_number;

  }


  return tempsum;// smoothed value of voxel
}


template <class TDiffusionPixelType, class TTensorPixelType>
void
TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
::CalculateAttenuation(vnl_vector<double> org_data,vnl_vector<double> &atten,int nof, int numberb0)
{
  double mean_b=0.0;

  for (int i=0;i<nof;i++)
  {
    if(m_B0Mask[i]>0)
    {
   //   double o_d=org_data[i];
      mean_b=mean_b+org_data[i];
    }

  }
  mean_b=mean_b/numberb0;
  int cnt=0;
  for (int i=0;i<nof;i++)
  {
    if(m_B0Mask[i]==0)
    {
      //if(org_data[i]<0.001){org_data[i]=0.01;}
      atten[cnt]=org_data[i]/mean_b;
      cnt++;
    }
  }
}

template <class TDiffusionPixelType, class TTensorPixelType>
double
TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
::CheckNegatives ( itk::Size<3> size, itk::Image<short, 3>::Pointer mask, typename itk::Image< itk::DiffusionTensor3D<TTensorPixelType>, 3 >::Pointer tensorImg )
{

  // The method was created to simplif the flow of negative eigenvalue correction process. The method itself just return the number
  // of voxels (tensors) with negative eigenvalues. Then if the voxel was previously bad ( mask=2 ) but it is not bad anymore mask is
  //changed to 1.

  // declaration of important structures and variables
  double badvoxels=0;
  double pixel=0;
  itk::DiffusionTensor3D<double> ten;
  vnl_matrix<double> temp_tensor(3,3);
  vnl_vector<double> eigen_vals(3);
  vnl_vector<double> tensor (6);

  // for every pixel from the image
  for (unsigned int x=0;x<size[0];x++)
  {

    for (unsigned int y=0;y<size[1];y++)
    {

      for (unsigned int z=0;z<size[2];z++)
      {


        itk::Index<3> ix = {{x,y,z}};
        pixel = mask->GetPixel(ix);

        // but only if previously marked as bad one-negative eigen value

        if(pixel > 1)
        {

          ten = tensorImg->GetPixel(ix);

          // changing order from tensor structure in MITK into vnl structure 3x3 symmetric tensor matrix

          tensor[0] = ten(0,0);
          tensor[3] = ten(0,1);
          tensor[5] = ten(0,2);
          tensor[1] = ten(1,1);
          tensor[4] = ten(1,2);
          tensor[2] = ten(2,2);

          temp_tensor[0][0]= tensor[0]; temp_tensor[1][0]= tensor[3]; temp_tensor[2][0]= tensor[5];
          temp_tensor[0][1]= tensor[3]; temp_tensor[1][1]= tensor[1]; temp_tensor[2][1]= tensor[4];
          temp_tensor[0][2]= tensor[5]; temp_tensor[1][2]= tensor[4]; temp_tensor[2][2]= tensor[2];

          //checking negativity of tensor eigenvalues

          vnl_symmetric_eigensystem<double> eigen_tensor(temp_tensor);


          eigen_vals[0]=eigen_tensor.get_eigenvalue(0);
          eigen_vals[1]=eigen_tensor.get_eigenvalue(1);
          eigen_vals[2]=eigen_tensor.get_eigenvalue(2);

          //comparison to 0.01 instead of 0 was proposed by O.Pasternak

          if( eigen_vals[0]>0.01 && eigen_vals[1]>0.01 && eigen_vals[2]>0.01)
          {
            mask->SetPixel(ix,1);

          }
          else
          {
            badvoxels++;
          }

        }

      }
    }
  }

  double ret = badvoxels;

  return ret;

}


template <class TDiffusionPixelType, class TTensorPixelType>
void
TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
::CorrectDiffusionImage(int nof,int numberb0,itk::Size<3> size, typename GradientImagesType::Pointer corrected_diffusion,itk::Image<short, 3>::Pointer mask,vnl_vector< double> pixel_max,vnl_vector< double> pixel_min)
{
  // in this method the voxels that has tensor negative eigenvalues are smoothed. Smoothing is done on DWI image.For the voxel
  //detected as bad one, B0 image is smoothed obligatory. All other gradient images are smoothed only when value of attenuation
  //is out of declared bounds for too high or too low attenuation.

  // declaration of important variables
  vnl_vector<double> org_data(nof);
  vnl_vector<double> atten(nof-numberb0);
  double cnt_atten=0;

  for (unsigned int z=0;z<size[2];z++)
  {

    for (unsigned int x=0;x<size[0];x++)
    {

      for (unsigned int y=0;y<size[1];y++)
      {
        itk::Index<3> ix = {{x, y, z}};


        if(mask->GetPixel(ix) > 1.0)
        {
          GradientVectorType  pt = corrected_diffusion->GetPixel(ix);

          for (int i=0;i<nof;i++)
          {
            org_data[i]=pt[i];
          }


          double mean_b=0.0;

          for (int i=0;i<nof;i++)
          {
            if(m_B0Mask[i]>0)
            {
              mean_b=mean_b+org_data[i];
            }

          }
          mean_b=mean_b/numberb0;
          int cnt=0;
          for (int i=0;i<nof;i++)
          {
            if(m_B0Mask[i]==0)
            {

              atten[cnt]=org_data[i]/mean_b;
              cnt++;
            }
          }

          cnt_atten=0;

          //smoothing certain gradient images that are out of declared constraints

          for (int f=0;f<nof;f++)
          {
            if(m_B0Mask[f]==0)
            {

              if(atten[cnt_atten]<pixel_min[cnt_atten] || atten[cnt_atten]> pixel_max[cnt_atten])
              {

                int x_max=size[0]-1;
                int y_max=size[1]-1;
                int z_max=size[2]-1;

                double back_x=std::max(0,(int)x-1);
                double back_y=std::max(0,(int)y-1);
                double back_z=std::max(0,(int)z-1);

                double forth_x=std::min(((int)x+1),x_max);
                double forth_y=std::min(((int)y+1),y_max);
                double forth_z=std::min(((int)z+1),z_max);

                double tempsum=0;
                double temp_number=0;

                for(unsigned int i=back_x; i<=forth_x; i++)
                {
                  for (unsigned int j=back_y; j<=forth_y; j++)
                  {
                    for (unsigned int k=back_z; k<=forth_z; k++)
                    {
                      itk::Index<3> ix = {{i,j,k}};

                      GradientVectorType p = corrected_diffusion->GetPixel(ix);

                      //double test= p[f];

                      if (p[f] > 0.0 )// taking only positive values and counting them
                      {
                        if(!(i==x && j==y && k== z))
                        {
                          tempsum=tempsum+p[f];
                          temp_number++;
                        }

                      }


                    }
                  }
                }

                //getting back to the original position of the voxel

                itk::Index<3> ix = {{x,y,z}};

                if (temp_number <= 0.0)
                {
                  tempsum=0;
                  mask->SetPixel(ix,0);
                }
                else
                {
                  tempsum=tempsum/temp_number;

                }

                org_data[f] = tempsum;

              }


              cnt_atten++;

            }
            //smoothing B0
            if(m_B0Mask[f]==1)
            {

              int x_max=size[0] - 1;
              int y_max=size[1] - 1;
              int z_max=size[2] - 1;

              double back_x=std::max(0,(int)x-1);
              double back_y=std::max(0,(int)y-1);
              double back_z=std::max(0,(int)z-1);

              double forth_x=std::min(((int)x+1),x_max);
              double forth_y=std::min(((int)y+1),y_max);
              double forth_z=std::min(((int)z+1),z_max);


              double tempsum=0;
              double temp_number=0;

              for(unsigned int i=back_x; i<=forth_x; i++)
              {
                for (unsigned int j=back_y; j<=forth_y; j++)
                {
                  for (unsigned int k=back_z; k<=forth_z; k++)
                  {
                    itk::Index<3> ix = {{i,j,k}};

                    GradientVectorType p = corrected_diffusion->GetPixel(ix);

                    //double test= p[f];

                    if (p[f] > 0.0 )// taking only positive values and counting them
                    {
                      if(!(i==x && j==y && k== z))
                      {
                        tempsum=tempsum+p[f];
                        temp_number++;
                      }

                    }


                  }
                }
              }

              //getting back to the original position of the voxel

              itk::Index<3> ix = {{x,y,z}};

              if (temp_number <= 0.0)
              {
                tempsum=0;
                mask->SetPixel(ix,0);
              }
              else
              {
                tempsum=tempsum/temp_number;

              }

              org_data[f] = tempsum;

            }


          }

          for (int i=0;i<nof;i++)
          {
            pt[i]=org_data[i];
          }

          corrected_diffusion->SetPixel(ix, pt);

        }
        else
        {
          GradientVectorType  pt = corrected_diffusion->GetPixel(ix);
          corrected_diffusion->SetPixel(ix, pt);

        }
      }
    }
  }



}

template <class TDiffusionPixelType, class TTensorPixelType>
void
TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
::GenerateTensorImage(int nof,int numberb0,itk::Size<3> size,itk::VectorImage<short, 3>::Pointer corrected_diffusion,itk::Image<short, 3>::Pointer mask,double what_mask, typename itk::Image< itk::DiffusionTensor3D<TTensorPixelType>, 3 >::Pointer tensorImg)
{
  // in this method the whole tensor image is updated with a tensors for defined voxels ( defined by a value of mask);


  itk::Index<3> ix;

  itk::Index<3> idx;
  idx.Fill(5);

  vnl_vector<double> org_data(nof);
  vnl_vector<double> atten(nof-numberb0);
  vnl_vector<double> tensor(6);
  itk::DiffusionTensor3D<double> ten;
  double mask_val=0;


  for (unsigned int x=0;x<size[0];x++)
  {

    for (unsigned int y=0;y<size[1];y++)
    {

      for (unsigned int z=0;z<size[2];z++)
      {


        ix[0] = x; ix[1] = y; ix[2] = z;

        mask_val= mask->GetPixel(ix);

        //Tensors are calculated only for voxels above theshold for B0 image.

        if( mask_val > 0.0 )
        {

          // calculation of attenuation with use of gradient image and  and mean B0 image
          GradientVectorType pt = corrected_diffusion->GetPixel(ix);

          for (int i=0;i<nof;i++)
          {
            org_data[i]=pt[i];
          }

          double mean_b=0.0;

          for (int i=0;i<nof;i++)
          {
            if(m_B0Mask[i]>0)
            {
              mean_b=mean_b+org_data[i];
            }

          }
          mean_b=mean_b/numberb0;
          int cnt=0;
          for (int i=0;i<nof;i++)
          {
            if (org_data[i]<= 0)

            {
              org_data[i]=0.1;
            }
            if(m_B0Mask[i]==0)
            {
              atten[cnt]=org_data[i]/mean_b;
              cnt++;
            }
          }




          for (int i=0;i<nof-numberb0;i++)
          {

            atten[i]=log((double)atten[i]);
          }

          // Calculation of tensor with use of previously calculated inverse of design matrix and attenuation
          tensor = m_PseudoInverse*atten;

          ten(0,0) = tensor[0];
          ten(0,1) = tensor[3];
          ten(0,2) = tensor[5];
          ten(1,1) = tensor[1];
          ten(1,2) = tensor[4];
          ten(2,2) = tensor[2];

          tensorImg->SetPixel(ix, ten);



        }
        // for voxels with mask value 0 - tensor is simply 0 ( outside brain value)
        else if (mask_val < 1.0)
        {

          ten(0,0) = 0;
          ten(0,1) = 0;
          ten(0,2) = 0;
          ten(1,1) = 0;
          ten(1,2) = 0;
          ten(2,2) = 0;
          tensorImg->SetPixel(ix, ten);
        }

        if (ix == idx)
        {
          for (int ll = 0; ll < 6 ; ll++)
          std::cout << tensor[ll] << "," << std::endl;
        }




      }
    }
  }



}// end of Generate Tensor


template <class TDiffusionPixelType, class TTensorPixelType>
void
TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
::TurnMask( itk::Size<3> size, itk::Image<short, 3>::Pointer mask, double previous_mask, double set_mask)
{
  // The method changes voxels in the mask that poses a certain value with other value.

  itk::Index<3> ix;
  double temp_mask_value=0;

  for(unsigned int x=0;x<size[0];x++)
  {
    for(unsigned int y=0;y<size[1];y++)
    {
      for(unsigned int z=0;z<size[2];z++)
      {
        ix[0] = x; ix[1] = y; ix[2] = z;
        temp_mask_value=mask->GetPixel(ix);

        if(temp_mask_value>previous_mask)
        {
          mask->SetPixel(ix,set_mask);
        }
      }
    }
  }
}

} // end of namespace







