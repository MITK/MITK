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

    m_GradientImagePointer = static_cast< GradientImagesType * >(
      this->ProcessObject::GetInput(0) );



    itk::Index<3> testIx;
    testIx[0] = 48;
    testIx[0] = 41;
    testIx[0] = 31;

    GradientVectorType data_vec = m_GradientImagePointer->GetPixel(testIx);

    short data_vec1 = data_vec[0];
    short data_vec2 = data_vec[1];
    short data_vec3 = data_vec[2];
    short data_vec4 = data_vec[3];
    short data_vec5 = data_vec[4];
    short data_vec6 = data_vec[5];
    short data_vec7 = data_vec[6];





    typename GradientImagesType::SizeType size = m_GradientImagePointer->GetLargestPossibleRegion().GetSize();
    int nof = m_GradientDirectionContainer->Size();
    int numberb0=0;

    int cnt=0;

    for(int i=0; i<nof; i++)
    {
      vnl_vector_fixed <double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);

      if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001 && vec[0]>-0.0001&& vec[1]>-0.0001 && vec[2]>-0.0001)
      {
        numberb0++;
      }
    }


    itk::Vector<double,3u> spacing_term = m_GradientImagePointer->GetSpacing();
    itk::Matrix<double,3u,3u> direction_term = m_GradientImagePointer->GetDirection();

    vnl_vector <double> spacing_vnl(3);
    vnl_matrix <double> dir_vnl (3,3);

    for (int i=0;i<3;i++)
    {
      spacing_vnl[i]=spacing_term[i];

      for(int j=0;j<3;j++)
      {
        dir_vnl[i][j]=direction_term[i][j];
      }
    }

    vnl_matrix <double> vox_dim_step (3,3);

    for (int i=0;i<3;i++)
    {
      for(int j=0;j<3;j++)
      {
        vox_dim_step[i][j]=spacing_vnl[i]*dir_vnl[i][j];
      }
    }

    vnl_symmetric_eigensystem <double> eigen_spacing(vox_dim_step);

    vnl_vector <double> vox_dim (3);

    vox_dim[0]=eigen_spacing.get_eigenvalue(0);
    vox_dim[1]=eigen_spacing.get_eigenvalue(1);
    vox_dim[2]=eigen_spacing.get_eigenvalue(2);

    vox_dim=vox_dim/(vox_dim.min_value());


    vnl_matrix<double> directions(nof-numberb0,3);
    m_B0Mask.set_size(nof);


    std::cout<<"logarithm test start"<<std::endl;

    double R=3.;
    double prev_x=0.5;

    double x;

    double delta=0;

    double one=0.0;

    cout.precision(32);


            for (double i=1;i< 100;++i)
            {

                delta= one + i*0.01;

                x= log((double)delta);

                std::cout<<x<<std::endl;

             }


         std::cout<<"logarithm test end"<<std::endl;




    for(int i=0; i<nof; i++)
    {
      vnl_vector_fixed <double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);

      if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001 && vec[0]>-0.001&& vec[1]>-0.001 && vec[2]>-0.001)
      {
        m_B0Mask[i]=1;
      }
      else
      {
        m_B0Mask[i]=0;
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


    dirsTimesDirsTrans = directions*directions.transpose();

    for (int i=0;i<nof-numberb0;i++)
    {
      b_vec[i]= dirsTimesDirsTrans[i][i];
    }

    vnl_matrix<double> H(nof-numberb0, 6);
    vnl_matrix<double> H_org(nof-numberb0, 6);
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

    H_org=H;

    vnl_matrix<double> inputtopseudoinverse=H.transpose()*H;
    vnl_symmetric_eigensystem<double> eig( inputtopseudoinverse);
    vnl_matrix<double> pseudoInverse = eig.pinverse()*H.transpose();


    itk::Index<3> ix;

    ImageType::Pointer corrected_diffusion = ImageType::New();
    corrected_diffusion->SetRegions(size);
    corrected_diffusion->SetSpacing(m_GradientImagePointer->GetSpacing());
    corrected_diffusion->SetOrigin(m_GradientImagePointer->GetOrigin());
    corrected_diffusion->SetVectorLength(nof);
    corrected_diffusion->Allocate();


    ImageType::Pointer corrected_diffusion_temp = ImageType::New();
    /*corrected_diffusion_temp->SetRegions(size);
    corrected_diffusion_temp->SetSpacing(m_GradientImagePointer->GetSpacing());
    corrected_diffusion_temp->SetOrigin(m_GradientImagePointer->GetOrigin());
    corrected_diffusion_temp->SetVectorLength(nof);
    corrected_diffusion_temp->Allocate();*/

    DeepCopyDiffusionImage(corrected_diffusion,corrected_diffusion_temp,nof);


    typedef itk::VariableLengthVector<short> VariableVectorType;
    VariableVectorType variableLengthVector;
    variableLengthVector.SetSize(nof);


    typedef itk::VariableLengthVector<short> VariableVectorType;
    VariableVectorType corrected_single;
    corrected_single.SetSize(nof-1);


    typedef itk::Image<short, 3> MaskImageType;
    MaskImageType::Pointer mask = MaskImageType::New();
    mask->SetRegions(m_GradientImagePointer->GetLargestPossibleRegion().GetSize());
    mask->SetSpacing(m_GradientImagePointer->GetSpacing());
    mask->SetOrigin(m_GradientImagePointer->GetOrigin());
    mask->Allocate();


    double mean_b=0.0;
    double pixel=0.0;
    vnl_vector<double> tensor (6);


    int mask_cnt=0;
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
          {
            mask->SetPixel(ix, 0);
          }

        }
      }
    }


   //Sometimes the gradient voxels may contain negative values ( even if B0 voxel is > = 50 ). This must be corrected


    double mask_val=0.0;
    vnl_vector<double> org_vec(nof-numberb0);

    int counter_corrected =0;

    for ( int x=0;x<size[0];x++)
    {
      for ( int y=0;y<size[1];y++)
      {
        for ( int z=0;z<size[2];z++)
        {
          ix[0] = x; ix[1] = y; ix[2] = z;

          mask_val = mask->GetPixel(ix);
          GradientVectorType pixel2 = m_GradientImagePointer->GetPixel(ix);

          for (int i=0;i<nof;i++)
          {
            org_vec[i]=pixel2[i];
          }

          if(mask_val >0)// we are dooing this only if the voxels are in the mask
          {

          for( int f=0;f<nof;f++)
          {
            if(org_vec[f] <= 0) // if in one of the gradients it is 0 or les than 0
            {
              org_vec[f] = CheckNeighbours(x,y,z,f,size,mask);
              counter_corrected++;

              // If method returned 0 it means that there is no valid ( correct) neighborhood. It is done outside the function because
              //later the same method is called in different concept when this is not important
            }
          }

          for (int i=0;i<nof;i++)
          {
            pixel2[i]=org_vec[i];
          }

          corrected_diffusion->SetPixel(ix, pixel2);
          //counter_corrected++;

          }
        }
      }
    }



    std::cout << "Number of voxels in mask: " << mask_cnt << std::endl;
    std::cout << "Number of voxels corrected: " << counter_corrected << std::endl;

    typedef itk::Image< itk::DiffusionTensor3D<float>, 3 > TensorImageType;
    TensorImageType::Pointer tensorImg = TensorImageType::New();
    tensorImg->SetRegions(m_GradientImagePointer->GetLargestPossibleRegion().GetSize());
    tensorImg->SetSpacing(m_GradientImagePointer->GetSpacing());
    tensorImg->SetOrigin(m_GradientImagePointer->GetOrigin());
    tensorImg->Allocate();





    TensorImageType::Pointer temp_tensorImg = TensorImageType::New();
    /*temp_tensorImg->SetRegions(m_GradientImagePointer->GetLargestPossibleRegion().GetSize());
    temp_tensorImg->SetSpacing(m_GradientImagePointer->GetSpacing());
    temp_tensorImg->SetOrigin(m_GradientImagePointer->GetOrigin());
    temp_tensorImg->Allocate();*/

    DeepCopyTensorImage(tensorImg,temp_tensorImg);

    vnl_vector< double> pixel_max(nof-numberb0);
    vnl_vector< double> pixel_min(nof-numberb0);

    for (int i=0;i<nof-numberb0;i++)
    {
      pixel_max[i]=exp(-b_vec[i]*0.01);// here some values for low and high diffusivity were pre define as 0.01 for low and 5 for high
      pixel_min[i]= exp(-b_vec[i]*5);
    }

    m_PseudoInverse = pseudoInverse;
    m_H = H_org;
    m_BVec=b_vec;



   // info for Thomas ( to be removed) :start of "lots of new code


    double what_mask=0;
    double set_mask=2.0;
    double previous_mask=0;

    double old_number_negative_eigs=0;
    double new_number_negative_eigs=0;

    std::cout << "I am here "<< std::endl;

    bool  stil_correcting = true;

    TurnMask(size,mask,previous_mask,set_mask);// simply defining all possible tensors as with negative eigenvalues



    //corrected_diffusion= m_GradientImagePointer;

    std::cout << "Stil good"<< std::endl;

    what_mask=1.0;// we want to calculate all the tensors

    int tensors_generated=0;

    GenerateTensorImage(nof,numberb0,size,corrected_diffusion,mask,what_mask,tensorImg);// generating of the tensors

    std::cout << "good_tensors_generated"<<tensors_generated<< std::endl;

    //this->SetNthOutput(0, tensorImg);

    old_number_negative_eigs = CheckNegatives (size,mask,tensorImg);// checking how many tensors has problems, this is working only for mask =2

    std::cout << "good now how many bads there are"<< std::endl;

    while (stil_correcting == true)
    {

    std::cout << "Number of negative eigenvalues: " << old_number_negative_eigs << std::endl;// info for Thomas: Debug stuff - to be removed

    CorrectDiffusionImage(nof,numberb0,size,corrected_diffusion_temp,corrected_diffusion,mask,pixel_max,pixel_min);

    GenerateTensorImage(nof,numberb0,size,corrected_diffusion_temp,mask,what_mask,temp_tensorImg);

    new_number_negative_eigs = CheckNegatives (size,mask, temp_tensorImg);

    /*typedef  itk::ImageFileWriter< MaskImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName("/Users/macbook_fb/Desktop/franksecondfound.nrrd");
    writer->SetInput(mask);
    writer->Update();*/

    if(new_number_negative_eigs<old_number_negative_eigs)
    {
        stil_correcting=true;
        old_number_negative_eigs=new_number_negative_eigs;

        tensorImg=temp_tensorImg;

        corrected_diffusion=corrected_diffusion_temp;
    }
    else
    {
        stil_correcting=false;
    }



    }//end of while


    TurnMask(size, mask,1,0);

    tensorImg=temp_tensorImg;

    //what_mask=0;

    //GenerateTensorImage(nof,numberb0,size,corrected_diffusion,mask,what_mask,tensorImg,pseudoInverse,tensors_generated );


   // info for Thomas ( to be removed) :end  of "lots of new code

    this->SetNthOutput(0, tensorImg);

    m_VectorImage = corrected_diffusion;
    m_MaskImage = mask;


    m_Voxdim = vox_dim;


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
  double
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::CheckNeighbours(int x, int y, int z,int f, itk::Size<3> size, itk::Image<short, 3>::Pointer mask)
  {
    double back_x=std::max(0,x-1);
    double back_y=std::max(0,y-1);
    double back_z=std::max(0,z-1);

    int x_max=size[0];int y_max=size[1];int z_max=size[2];// converting short to int

    double forth_x=std::min((x+1),x_max);
    double forth_y=std::min((y+1),y_max);
    double forth_z=std::min((z+1),z_max);// setting constraints for neigborhood


    double tempsum=0;
    itk::Index<3> ix;
    double temp_number=0;
    double temp_mask=0;// declaration of variables
    double one =1.0;

    for(int i=back_x; i<forth_x+1; i++)
    {
      for (int j=back_y; j<forth_y+1; j++)
      {
        for (int c=back_z; c<forth_z+1;c++)
        {

          ix[0] = i;ix[1] = j;ix[2] = c;

          temp_mask=mask->GetPixel(ix);

          if(temp_mask > 0 && temp_mask < 2 )
          {
              GradientVectorType p = m_GradientImagePointer->GetPixel(ix);

              double test= p[f];

              if (test > 0.0 )// hmm this must be here becaus the method is used in multiple ocasions. Sometiems we may deal with negative values
              {

                tempsum=tempsum+p[f];// sum for calculation of mean
                temp_number++;// number for calculation of mean


            }//end of pf>0
          }//end of mask condition

        }
      }
    }// end of size


    ix[0] = x;ix[1] = y;ix[2] = z;

    if (temp_number <= 0.0)
    {
      tempsum=0;
      mask->SetPixel(ix,0);
    }
    else
    {
      tempsum=tempsum/temp_number;
    }



    double ret = tempsum;


    return ret;
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
        double o_d=org_data[i];
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
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::CalculateTensor(vnl_vector<double> atten,vnl_vector<double> &tensor, int nof,int numberb0)
  {
    for (int i=0;i<nof-numberb0;i++)
    {

      if(atten[i]==0.0)
     std::cout << "problem";
      atten[i]=log((double)atten[i]);
    }
    tensor = m_PseudoInverse*atten;


  }// end of void calculate tensor


  // info for Thomas ( to be removed) : start of "lots of new code"


  template <class TDiffusionPixelType, class TTensorPixelType>
  double
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::CheckNegatives ( itk::Size<3> size, itk::Image<short, 3>::Pointer mask, itk::Image< itk::DiffusionTensor3D<float>, 3 >::Pointer tensorImg )
  {

      // The method was created to simplif the flow of negative eigenvalue correction process. The method itself just return the number
      // of voxels (tensors) with negative eigenvalues. Then if the voxel was previously bad ( mask=2 ) but it is not bad anymore mask is
      //changed to 1.

      double badvoxels=0;
      double pixel=0;
      itk::Index<3> ix;
      itk::DiffusionTensor3D<float> ten;
      vnl_matrix<double> temp_tensor(3,3);
      vnl_vector<double> eigen_vals(3);// declaration of important structures and variables
      vnl_vector<double> tensor (6);


      for (int x=0;x<size[0];x++)
      {

        for (int y=0;y<size[1];y++)
        {

          for (int z=0;z<size[2];z++)
          {
              // for every pixel

              ix[0] = x; ix[1] = y; ix[2] = z;

              pixel = mask->GetPixel(ix);

              if(pixel == 2.0)
              {// but only if previously marked as bad or potentially bad one

              ten = tensorImg->GetPixel(ix);

              tensor[0] = ten(0,0);
              tensor[3] = ten(0,1);
              tensor[5] = ten(0,2);
              tensor[1] = ten(1,1);
              tensor[4] = ten(1,2);
              tensor[2] = ten(2,2);

              temp_tensor[0][0]= tensor[0]; temp_tensor[1][0]= tensor[3]; temp_tensor[2][0]= tensor[5];
              temp_tensor[0][1]= tensor[3]; temp_tensor[1][1]= tensor[1]; temp_tensor[2][1]= tensor[4];
              temp_tensor[0][2]= tensor[5]; temp_tensor[1][2]= tensor[4]; temp_tensor[2][2]= tensor[2];


              vnl_symmetric_eigensystem<double> eigen_tensor(temp_tensor);//creating an eigensystem out of 3x3 symmetric tensor matrix


              eigen_vals[0]=eigen_tensor.get_eigenvalue(0);
              eigen_vals[1]=eigen_tensor.get_eigenvalue(1);
              eigen_vals[2]=eigen_tensor.get_eigenvalue(2);// calculating eigenvalues for current tensor

              if( eigen_vals[0]>0.01 && eigen_vals[1]>0.01 && eigen_vals[2]>0.01)
              {
                mask->SetPixel(ix,1);

              }
              else
              {
                badvoxels++;// increasing a number of detected bad
              }

              }

          }
         }
       }

      double ret = badvoxels;//returning just the number of bad voxels

      return ret;

  }// end of void check negativity


  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::CorrectDiffusionImage(int nof,int numberb0,itk::Size<3> size,itk::VectorImage<short, 3>::Pointer corrected_diffusion_temp,itk::VectorImage<short, 3>::Pointer corrected_diffusion,itk::Image<short, 3>::Pointer mask,vnl_vector< double> pixel_max,vnl_vector< double> pixel_min)
  {
      // in this method the diffusion image temporray is corrected with use of information from updated mask. The diffusion image is tempora
      //-ry while we can't use corrected voxel to correct other voxel in the same iteration.




      itk::Index<3> ix;
      vnl_vector<double> org_data(nof-numberb0);
      vnl_vector<double> atten(nof-numberb0);
      double cnt_atten=0;// declaration of important variables

      for (int x=0;x<size[0];x++)
      {

        for (int y=0;y<size[1];y++)
        {

          for (int z=0;z<size[2];z++)
          {
              // for every pixel

              ix[0] = x; ix[1] = y; ix[2] = z;



              if(mask->GetPixel(ix) > 1.0)
              {
                  GradientVectorType  pt = corrected_diffusion->GetPixel(ix);

                  for (int i=0;i<nof;i++)
                  {
                    org_data[i]=pt[i];
                  }

                  //CalculateAttenuation(org_data,atten,nof,numberb0);

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
                      //if(org_data[i]<0.001){org_data[i]=0.01;}
                      atten[cnt]=org_data[i]/mean_b;
                      cnt++;
                    }
                  }


                  cnt_atten=0;

                  for (int f=0;f<nof;f++)
                  {
                    if(m_B0Mask[f]==0)
                    {

                    //if(atten[cnt_atten]<0.0067 || atten[cnt_atten]> 0.99)
                    //{
                       org_data[f] = CheckNeighbours(x,y,z,f,size,mask);
                    //}


                    cnt_atten++;

                   }

                  }//end for

                  for (int i=0;i<nof;i++)
                  {
                    pt[i]=org_data[i];
                  }

                  corrected_diffusion_temp->SetPixel(ix, pt);

              }
              else
              {
                  GradientVectorType  pt = corrected_diffusion->GetPixel(ix);
                  corrected_diffusion_temp->SetPixel(ix, pt);

              }
           }
        }
       }

      std::cout << "break point";

  }

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::GenerateTensorImage(int nof,int numberb0,itk::Size<3> size,itk::VectorImage<short, 3>::Pointer corrected_diffusion,itk::Image<short, 3>::Pointer mask,double what_mask,itk::Image< itk::DiffusionTensor3D<float>, 3 >::Pointer tensorImg)
  {
      // in this method the whole tensor image is updated with a tensors for defined voxels ( defined by a value of mask);


      itk::Index<3> ix;
      vnl_vector<double> org_data(nof-numberb0);
      vnl_vector<double> atten(nof-numberb0);
      vnl_vector<double> tensor(6);
      itk::DiffusionTensor3D<float> ten;
      double mask_val=0;


      for (int x=0;x<size[0];x++)
      {

        for (int y=0;y<size[1];y++)
        {

         for (int z=0;z<size[2];z++)
          {
              // for every pixel

              ix[0] = x; ix[1] = y; ix[2] = z;

              mask_val= mask->GetPixel(ix);

              if( mask_val > 0.0 )
              {
                 GradientVectorType pt = corrected_diffusion->GetPixel(ix);

                  for (int i=0;i<nof;i++)
                  {
                    org_data[i]=pt[i];
                  }

                  //CalculateAttenuation(org_data,atten,nof,numberb0);
                  //CalculateTensor(atten,tensor,nof,numberb0);

                  double mean_b=0.0;

                  for (int i=0;i<nof;i++)
                  {
                    if(m_B0Mask[i]>0)
                    {
                      double o_d=org_data[i];
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




                  for (int i=0;i<nof-numberb0;i++)
                  {

                    if(atten[i]==0.0)
                   std::cout << "problem";
                    atten[i]=log((double)atten[i]);
                  }
                  tensor = m_PseudoInverse*atten;

                  ten(0,0) = tensor[0];
                  ten(0,1) = tensor[3];
                  ten(0,2) = tensor[5];
                  ten(1,1) = tensor[1];
                  ten(1,2) = tensor[4];
                  ten(2,2) = tensor[2];

                  tensorImg->SetPixel(ix, ten);



              }
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




           }
        }
       }



  }// end of Generate Tensor


  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::TurnMask( itk::Size<3> size, itk::Image<short, 3>::Pointer mask, double previous_mask, double set_mask)
  {
      // in this method voxels in the mask that poses a certain value are substituded by other value. It is connected to the
      //idea in the algorithm. It is called more tna once so separate method spares some lines of code.


  itk::Index<3> ix;
  double temp_mask_value=0;

  for(int x=0;x<size[0];x++)
  {
    for(int y=0;y<size[1];y++)
    {
      for(int z=0;z<size[2];z++)
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
}//end of turn mas



  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::DeepCopyDiffusionImage(itk::VectorImage<short, 3>::Pointer corrected_diffusion, itk::VectorImage<short, 3>::Pointer corrected_diffusion_temp,int nof)
  {
    corrected_diffusion_temp->SetSpacing(corrected_diffusion->GetSpacing());
    corrected_diffusion_temp->SetOrigin(corrected_diffusion->GetOrigin());
    corrected_diffusion_temp->SetVectorLength(nof);
    corrected_diffusion_temp->SetRegions(corrected_diffusion->GetLargestPossibleRegion());
    corrected_diffusion_temp->Allocate();

    itk::ImageRegionConstIterator<ImageType> inputIterator(corrected_diffusion, corrected_diffusion->GetLargestPossibleRegion());
    itk::ImageRegionIterator<ImageType> outputIterator(corrected_diffusion_temp, corrected_diffusion_temp->GetLargestPossibleRegion());

    //inputIterator.begin();
    //outputIterator.begin();

    while(!inputIterator.IsAtEnd())
      {
      outputIterator.Set(inputIterator.Get());
      ++inputIterator;
      ++outputIterator;
      }
  }

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::DeepCopyTensorImage(itk::Image< itk::DiffusionTensor3D<float>, 3 >::Pointer tensorImg, itk::Image< itk::DiffusionTensor3D<float>, 3 >::Pointer temp_tensorImg)
  {

      temp_tensorImg->SetSpacing(tensorImg->GetSpacing());
      temp_tensorImg->SetOrigin(tensorImg->GetOrigin());
      temp_tensorImg->SetRegions(tensorImg->GetLargestPossibleRegion());
      temp_tensorImg->Allocate();

    itk::ImageRegionConstIterator<TensorImageType> inputIterator(tensorImg, tensorImg->GetLargestPossibleRegion());
    itk::ImageRegionIterator<TensorImageType> outputIterator(temp_tensorImg, temp_tensorImg->GetLargestPossibleRegion());

    //inputIterator.begin();
    //outputIterator.begin();

    while(!inputIterator.IsAtEnd())
      {
      outputIterator.Set(inputIterator.Get());
      ++inputIterator;
      ++outputIterator;
      }
  }





// info for Thomas ( to be removed) : end of "lots of new code"

} // end of namespace







