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
    m_GradientImagePointer = static_cast< GradientImagesType * >(
      this->ProcessObject::GetInput(0) );

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
    typedef itk::VariableLengthVector<short> VariableVectorType;
    VariableVectorType variableLengthVector;
    variableLengthVector.SetSize(nof);


    //  removing negative values
    for ( int x=0;x<size[0];x++)
    {
      for ( int y=0;y<size[1];y++)
      {
        for ( int z=0;z<size[2];z++)
        {
          ix[0] = x;
          ix[1] = y;
          ix[2] = z;
          GradientVectorType pixel2 = m_GradientImagePointer->GetPixel(ix);
          for( int f=0;f<nof;f++)
          {
            if(pixel2[f]<0.0) //frank: pixel2[f]<=0.0
            {
              variableLengthVector[f] = CheckNeighbours(x,y,z,f,size);
            }
            else
            {
              variableLengthVector[f] = pixel2[f];
            }
          }

          corrected_diffusion->SetPixel(ix, variableLengthVector);
        }
      }
    }



    vnl_vector<double> org_data(nof);
    vnl_vector<double> atten(nof-numberb0);
    double mean_b=0.0;
    double pixel=0.0;
    vnl_vector<double> tensor (6);

    typedef itk::Image<short, 3> MaskImageType;
    MaskImageType::Pointer mask = MaskImageType::New();
    mask->SetRegions(m_GradientImagePointer->GetLargestPossibleRegion().GetSize());
    mask->SetSpacing(m_GradientImagePointer->GetSpacing());
    mask->SetOrigin(m_GradientImagePointer->GetOrigin());
    mask->Allocate();


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
          if(mean_b>m_B0Threshold)
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

    std::cout << "Number of voxels in mask: " << mask_cnt << std::endl;

    typedef itk::Image< itk::DiffusionTensor3D<float>, 3 > TensorImageType;
    TensorImageType::Pointer tensorImg = TensorImageType::New();
    tensorImg->SetRegions(m_GradientImagePointer->GetLargestPossibleRegion().GetSize());
    tensorImg->SetSpacing(m_GradientImagePointer->GetSpacing());
    tensorImg->SetOrigin(m_GradientImagePointer->GetOrigin());
    tensorImg->Allocate();


    vnl_matrix<double> temp_tensor(3,3);
    vnl_vector<double> eigen_vals(3);
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

    // Correct voxels with negative eigenvalues
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

            if(mask->GetPixel(ix) == 1)
            {
              ix[0] = x;
              ix[1] = y;
              ix[2] = z;
              GradientVectorType pt = corrected_diffusion->GetPixel(ix);

              for (int i=0;i<nof;i++)
              {
                org_data[i]=pt[i];
              }

              CalculateAttenuation(org_data,atten,nof,numberb0);
              CalculateTensor(pseudoInverse,atten,tensor,nof,numberb0);

              bool tensor_invalid = false;

              // verify tensor, check for -nan values
              for( unsigned int j=0; j<tensor.size(); j++)
              {
                if( tensor[j] != tensor[j])
                {
                  tensor[j] = 0;
                  tensor_invalid = true;
                  continue;
                }
              }

              // process only for valid tensors
              if(!tensor_invalid)
              {
                ///////////////testing
                ten(0,0) = tensor[0];
                ten(0,1) = tensor[3];
                ten(0,2) = tensor[5];
                ten(1,1) = tensor[1];
                ten(1,2) = tensor[4];
                ten(2,2) = tensor[2];

                typename TensorPixelType::EigenValuesArrayType eigenvalues;
                ten.ComputeEigenValues(eigenvalues);

                temp_tensor[0][0]= tensor[0]; temp_tensor[1][0]= tensor[3]; temp_tensor[2][0]= tensor[5];
                temp_tensor[0][1]= tensor[3]; temp_tensor[1][1]= tensor[1]; temp_tensor[2][1]= tensor[4];
                temp_tensor[0][2]= tensor[5]; temp_tensor[1][2]= tensor[4]; temp_tensor[2][2]= tensor[2];

                vnl_symmetric_eigensystem<double> eigen_tensor(temp_tensor);

                eigen_vals[0]=eigen_tensor.get_eigenvalue(0);
                eigen_vals[1]=eigen_tensor.get_eigenvalue(1);
                eigen_vals[2]=eigen_tensor.get_eigenvalue(2);

              }
              else
              // the tensor is invalid, i.e. contains some NAN entries
              {
                // set the eigenvalues manually to -1 to force the idx to be marked as bad voxel
                eigen_vals[0] = eigen_vals[1] = eigen_vals[2] = -1;
              }



              if( eigen_vals[0]>0.0 && eigen_vals[1]>0.0 && eigen_vals[2]>0.0)
              {
                tensorImg->SetPixel(ix, ten);
              }//end of if eigenvalues
              else
              {
                number_of_bads++;
                ten.Fill(0.0);
                tensorImg->SetPixel(ix, ten);

                for (int f=0;f<nof;f++)
                {
                  if(pt[f]>pixel_max[f] || pt[f]< pixel_min[f])
                  {
                    variableLengthVector[f] = CheckNeighbours(x,y,z,f,size);
                  }

                }

                corrected_diffusion->SetPixel(ix, variableLengthVector);
              }
            }

            else
            {
              ten.Fill(0.0);
              tensorImg->SetPixel(ix, ten);
            }


          }
        }

      }

      diff=old_number_of_bads-number_of_bads;
      old_number_of_bads=number_of_bads;
      std::cout << "bad voxels: " << number_of_bads << std::endl;
      number_of_bads=0;


    }



    this->SetNthOutput(0, tensorImg);
    m_VectorImage = corrected_diffusion;
    m_MaskImage = mask;
    m_PseudoInverse = pseudoInverse;
    m_H = H_org;
    m_BVec=b_vec;

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
  short
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::CheckNeighbours(int x, int y, int z,int f, itk::Size<3> size)
  {
    int init_i, init_j, init_c, limit_i, limit_j, limit_c;
    double tempsum=0.0;
    double temp_number=0.0;

    init_i=x-1;
    limit_i=x+2;
    if(x==0)
      init_i=x;
    else if(x==size[0]-1)
      limit_i=x+1;

    init_j=y-1;
    limit_j=y+2;
    if(y==0)
      init_j=y;
    else if(y==size[1]-1)
      limit_j=y+1;

    init_c=z-1;
    limit_c=z+2;
    if(z==0)
      init_c=z;
    else if(z==size[2]-1)
      limit_c=z+1;

    for(int i=init_i; i<limit_i; i+=2)
    {
      for (int j=init_j; j<limit_j; j+=2)
      {
        for (int c=init_c; c<limit_c ;c+=2)
        {

          itk::Index<3> ix;
          ix[0] = i;
          ix[1] = j;
          ix[2] = c;

          GradientVectorType p = m_GradientImagePointer->GetPixel(ix);

          if(p[f]>=0.0)
          {
            tempsum=tempsum+p[f];
            temp_number++;
          }
        }
      }
    }
    if (temp_number==0.0)
    {
      tempsum=0.0;
    }
    else
    {
      tempsum=tempsum/temp_number;
    }

    short ret = (short)tempsum;
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
      if(m_B0Mask[i]==1)
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
        if(org_data[i]<0.001){org_data[i]=0.01;}
        atten[cnt]=org_data[i]/mean_b;
        cnt++;
      }
    }
  }


  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  TensorReconstructionWithEigenvalueCorrectionFilter<TDiffusionPixelType, TTensorPixelType>
  ::CalculateTensor(vnl_matrix<double> pseudoInverse,vnl_vector<double> atten,vnl_vector<double> &tensor, int nof,int numberb0)
  {
    for (int i=0;i<nof-numberb0;i++)
    {
      atten[i]=log((double)atten[i]);
    }
    tensor = pseudoInverse*atten;
  }// end of void calculate tensor

} // end of namespace







