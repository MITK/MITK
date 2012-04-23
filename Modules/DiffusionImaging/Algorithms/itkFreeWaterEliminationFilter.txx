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
    m_B0Threshold = 50.0;
  }

  template <class TDiffusionPixelType, class TTensorPixelType>
  void
  FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
  ::GenerateData ( )
  {

    typedef typename GradientImagesType::PixelType         GradientVectorType;


    int nof= m_B0Mask.size(); /// !!!!!!!!!!!! ned to pass it too
    int number_b0= m_B0Mask.size()-m_BVec.size();

    // we need to take care about size variable
    typename GradientImagesType::SizeType size = m_CorrectedVols->GetLargestPossibleRegion().GetSize();

    // Declaration of tensor image- output of the filter
    typedef itk::Image< itk::DiffusionTensor3D<float>, 3 > TensorImageType;
    TensorImageType::IndexType start_tensorImg;
    start_tensorImg.Fill(0);
    TensorImageType::RegionType region_tensorImg(start_tensorImg,size);
    TensorImageType::Pointer tensorImg = TensorImageType::New();
    tensorImg->SetRegions(region_tensorImg);
    tensorImg->SetSpacing(m_CorrectedVols->GetSpacing());
    tensorImg->SetOrigin(m_CorrectedVols->GetOrigin());
    tensorImg->Allocate();
    itk::Index<3> ix;
    // end of tensor declaration

    // declaration of corrected diffusion image///
    ImageType::IndexType start;
    start.Fill(0);
    ImageType::RegionType region(start,size);
    FloatVectorImageType::Pointer f_w = FloatVectorImageType::New();
    f_w->SetRegions(region);
    f_w->SetVectorLength(nof);
    f_w->Allocate();
    typedef itk::VariableLengthVector<float> VariableVectorType;
    VariableVectorType variableLengthVector;
    variableLengthVector.SetSize(3);
    // end of the biggest data structure for this filter

    double pixel=0.0;

    // final correction of the mask, It was undone in previous filter
    for ( int x=0;x<size[0];x++)
    {
      for ( int y=0;y<size[1];y++)
      {
        for ( int z=0;z<size[2];z++)
        {
           ix[0]=x; ix[1]=y; ix[2]=z;
           pixel = m_MaskImage->GetPixel(ix);

           if(pixel>2.0)
           {
             pixel=0.0;
             m_MaskImage->SetPixel(ix, pixel);
           }
        }
      }
    }// end of for 1

    // estimation of f_map
    vnl_vector<double> org_data(nof);
    vnl_vector<double> tensor(6);
    vnl_vector<double> atten(nof-number_b0);
    vnl_matrix<double> temp_tensor(3,3);
    vnl_matrix<double> copy(nof-number_b0,6);
    vnl_vector<double> eigen_vals(3);
    vnl_vector<double> f_vec(3);
    vnl_vector<double> eigs(2);
    double mean_b=m_BVec.mean();
    itk::DiffusionTensor3D<float> ten;

    float MD=0.0;

    for ( int x=0;x<size[0];x++)
    {
      for ( int y=0;y<size[1];y++)
      {
        for ( int z=0;z<size[2];z++)
        {
          ix[0]=x; ix[1]=y; ix[2]=z;
          pixel = m_MaskImage->GetPixel(ix);

          if(pixel>0.0)
          {

            if(x==22 && y==48 && z==20)
            {
               std::cout <<"21 48 20 tensor"<< "\n";
               std::cout <<tensor[0]<< "\n";
               std::cout <<tensor[1]<< "\n";
               std::cout <<tensor[2]<< "\n";
               std::cout <<tensor[3]<< "\n";
               std::cout <<tensor[4]<< "\n";
               std::cout <<tensor[5]<< "\n" <<std::endl;
            }


            GradientVectorType data_vec = m_CorrectedVols->GetPixel(ix);
            for (int i=0;i<nof;i++)
            {
              org_data[i]=data_vec[i];
            }

            calculate_attenuation(org_data,m_B0Mask,atten,mean_b,nof,number_b0);
            calculate_tensor(m_PseudoInverse,atten,tensor,nof,number_b0);


            temp_tensor[0][0]= tensor[0];temp_tensor[0][1]= tensor[3];temp_tensor[0][2]= tensor[5];
            temp_tensor[1][0]= tensor[3];temp_tensor[1][1]= tensor[1];temp_tensor[1][2]= tensor[4];
            temp_tensor[2][0]= tensor[5];temp_tensor[2][1]= tensor[4];temp_tensor[2][2]= tensor[2];

            vnl_symmetric_eigensystem <double> eigen_tensor(temp_tensor);

            eigen_vals[0]=eigen_tensor.get_eigenvalue(0);
            eigen_vals[1]=eigen_tensor.get_eigenvalue(1);
            eigen_vals[2]=eigen_tensor.get_eigenvalue(2);
            eigs[0]=(double)eigen_vals.max_value();
            eigs[1]=(double)eigen_vals.min_value();

            MD=(tensor[0]+tensor[1]+tensor[2])/3.0;// calculating of mean_diffusivity


            // Hard coded. check!
            mean_b=1.0;

            if(MD < 2.3)
            {
              f_vec[0] = ((exp(-mean_b*eigs[0]))-(exp(-mean_b*3.0)))/((exp(-2.5*mean_b))-(exp(-mean_b*3.0)));
              f_vec[1] = ((exp(-mean_b*MD))-(exp(-mean_b*3.0)))/((exp(-0.6*mean_b))-(exp(-mean_b*3.0)));
              f_vec[2] = ((exp(-mean_b*eigs[1]))-(exp(-mean_b*3.0)))/((exp(-0.01*mean_b))-(exp(-mean_b*3.0)));


              if(f_vec[2]>1.0)
                f_vec[2]=1.0;

              if(f_vec[2]<0.0)
                f_vec[2]=0.0;// finish min value for the voxel

              if(f_vec[0]>1.0)
                f_vec[0]=1.0;

              if(f_vec[0]<0.0)
                f_vec[0]=0.0;// finish max value for the voxel


              if(f_vec[1]>f_vec[0])
              {
                f_vec[1]=(f_vec[0]+ f_vec[2])/2.0;
              }

              if(f_vec[1]<f_vec[2])
              {
                f_vec[1]=(f_vec[0] + f_vec[2])/2.0;
              }// finish of f_map calculation




              for (int i=0;i<nof-number_b0;i++)
              {

               if(f_vec[1] == 0)
               {
                 std::cout << "f_vec[1] == 0" << std::endl;
               }
               atten[i]= (atten[i]-(exp(-3.0*m_BVec[i])*(1.0-f_vec[1])))/f_vec[1];
               if (atten[i]<0.1){atten[i]=0.01;}
              }// estimation of new attenuation value- after subtracting initial water part

              calculate_tensor(m_PseudoInverse,atten,tensor,nof,number_b0);

              ten(0,0) = tensor[0]; ten(0,1) = tensor[3]; ten(0,2) = tensor[5];
              ten(1,1) = tensor[1]; ten(1,2) = tensor[4]; ten(2,2) = tensor[2];

              tensorImg->SetPixel(ix, ten);// calculation and setting of new tensor

              VariableVectorType tempvec;
              tempvec.SetSize(3);
              tempvec[0] = (float)f_vec[0];
              tempvec[1] = (float)f_vec[1];
              tempvec[2] = (float)f_vec[2];
              f_w->SetPixel(ix, tempvec);// setting value of f-mask, its max and min as well


              if(f_vec[1] != f_vec[1])
              {
                std::cout << "nan" << std::endl;
              }



            }
            else
            {
              ten.Fill(0.0);
              tensorImg->SetPixel(ix, ten);
              f_vec.fill(0.0);

              VariableVectorType tempvec;
              tempvec.SetSize(3);
              tempvec[0] = (float)f_vec[0];
              tempvec[1] = (float)f_vec[1];
              tempvec[2] = (float)f_vec[2];
              f_w->SetPixel(ix, tempvec);
              m_MaskImage->SetPixel(ix,0.0);
            }

          }
          else
          {
            ten.Fill(0.0);
            tensorImg->SetPixel(ix, ten);

            VariableVectorType tempvec;
            tempvec.SetSize(3);

            tempvec.Fill(0);
            f_w->SetPixel(ix, tempvec);
          }
        }
      }
    }
    /// Main loop of minimization

    /// Parameters declaration

    vnl_vector<double> vox_dim(3);
    for(int i=0;i<3;i++)
    {
      vox_dim[i]=1.0;
    }// need to be fixed

    double dt_reg=0.0025;double mask_x=0.0;double mask_y=0.0;double mask_z=0.0;

    double temporary_mask_px =0.0;double temporary_mask_py =0.0;double temporary_mask_pz =0.0;

    double temporary_component_x =0.0;double temporary_component_y =0.0;double temporary_component_z =0.0;

    double tensor_x=0.0;double tensor_y=0.0;double tensor_z=0.0;

    vnl_matrix<double> temp_tensor_n(6,1);
    vnl_matrix<double> rom(nof-number_b0,1);

    double temp_sum_comp=0.0;
    copy=m_H;
    vnl_matrix<double> inverse_transposed = copy.transpose();
    vnl_matrix<double> temporary_attenuation(nof-number_b0,1);
    vnl_matrix<double> tct(6,1);
    vnl_vector <double> regulated_tensors (6);

    double mult =0.0;double tensor_n_x=0.0;double tensor_n_y=0.0;double tensor_n_z=0.0;

    // all of this is declared because of the shifting that exist in Beltrami -Iwasawa method.

    double pixel_x_p=0.0; double pixel_x_n=0.0;
    double pixel_y_p=0.0; double pixel_y_n=0.0;
    double pixel_z_p=0.0; double pixel_z_n=0.0;

    vnl_vector<double> tensor_x_p(6); vnl_vector<double> tensor_x_n(6);
    vnl_vector<double> tensor_y_p(6); vnl_vector<double> tensor_y_n(6);
    vnl_vector<double> tensor_z_p(6); vnl_vector<double> tensor_z_n(6);

    /// end of [parameter declaration


    // Copy of tensorImage
    TensorImageType::Pointer regTensorImg = TensorImageType::New();
    regTensorImg->SetRegions(tensorImg->GetLargestPossibleRegion().GetSize());
    regTensorImg->SetSpacing(tensorImg->GetSpacing());
    regTensorImg->SetOrigin(tensorImg->GetOrigin());
    regTensorImg->Allocate();


    for(int t=0; t<10; t++)
    {

      std::cout << "beltrami iteration " << t << std::endl;


      for ( int x=0;x<size[0];x++)
      {
        for ( int y=0;y<size[1];y++)
        {
          for ( int z=0;z<size[2];z++)
          {


             ix[0]=x; ix[1]=y; ix[2]=z;
             pixel = m_MaskImage->GetPixel(ix);


             if(pixel>0.0)
             {
                ten = tensorImg->GetPixel(ix);
                mask_x=pixel*pixel;
                mask_y=pixel*pixel;
                mask_z=pixel*pixel;
                tensor[0]=ten(0,0);
                tensor[3]=ten(0,1);
                tensor[5]=ten(0,2);
                tensor[1]=ten(1,1);
                tensor[4]=ten(1,2);
                tensor[2]=ten(2,2);

                for(int t_id=0;t_id<6;t_id++)
                {
                  tensor_x=0;
                  tensor_y=0;
                  tensor_z=0;

                  if (x>0)
                  {
                    // shifting x negative
                    ix[0]=x-1;
                    ix[1]=y;
                    ix[2]=z;
                    pixel_x_n = m_MaskImage->GetPixel(ix);
                    ten = tensorImg->GetPixel(ix);
                    tensor_x_n[0]=ten(0,0);
                    tensor_x_n[3]=ten(0,1);
                    tensor_x_n[5]=ten(0,2);
                    tensor_x_n[1]=ten(1,1);
                    tensor_x_n[4]=ten(1,2);
                    tensor_x_n[2]=ten(2,2);
                    tensor_x=(tensor[t_id]-tensor_x_n[t_id])*pixel_x_n*pixel;
                    mask_x=pixel_x_n*pixel;
                  }
                  if (y>0)
                  {
                    // shifting y negative
                    ix[0]=x;
                    ix[1]=y-1;
                    ix[2]=z;
                    pixel_y_n = m_MaskImage->GetPixel(ix);
                    ten = tensorImg->GetPixel(ix);
                    tensor_y_n[0]=ten(0,0);
                    tensor_y_n[3]=ten(0,1);
                    tensor_y_n[5]=ten(0,2);
                    tensor_y_n[1]=ten(1,1);
                    tensor_y_n[4]=ten(1,2);
                    tensor_y_n[2]=ten(2,2);
                    tensor_y=(tensor[t_id]-tensor_y_n[t_id])*pixel_y_n*pixel;
                    mask_y=pixel_y_n*pixel;
                  }
                  if (z>0)
                  {
                    // shifting z negative
                    ix[0]=x;
                    ix[1]=y;
                    ix[2]=z-1;
                    pixel_z_n = m_MaskImage->GetPixel(ix);
                    ten = tensorImg->GetPixel(ix);
                    tensor_z_n[0]=ten(0,0);
                    tensor_z_n[3]=ten(0,1);
                    tensor_z_n[5]=ten(0,2);
                    tensor_z_n[1]=ten(1,1);
                    tensor_z_n[4]=ten(1,2);
                    tensor_z_n[2]=ten(2,2);

                    tensor_z=(tensor[t_id]-tensor_z_n[t_id])*pixel_z_n*pixel;
                    mask_z=pixel_z_n*pixel;
                  }

                  mult=mask_x*mask_y*mask_z;

                  if (x==(size[0]-1))
                  {
                    temporary_mask_px =pixel*mult;
                    temporary_component_x=0;
                  }

                  if (y==(size[1]-1))
                  {
                    temporary_mask_py =pixel*mult;
                    temporary_component_y=0;
                  }
                  if (z==(size[2]-1))
                  {
                    temporary_mask_pz =pixel*mult;
                    temporary_component_z=0;
                  }

                  // shifting z positive


                  if(x<(size[0]-1))
                  {
                    // shifting x positive
                    ix[0]=x+1;
                    ix[1]=y;
                    ix[2]=z;
                    pixel_x_p = m_MaskImage->GetPixel(ix);
                    ten = tensorImg->GetPixel(ix);

                    if(ten(0,0) != ten(0,0))
                    {
                      std::cout << "bad tensor" << std::endl;
                    }

                    tensor_x_p[0]=ten(0,0);
                    tensor_x_p[3]=ten(0,1);
                    tensor_x_p[5]=ten(0,2);
                    tensor_x_p[1]=ten(1,1);
                    tensor_x_p[4]=ten(1,2);
                    tensor_x_p[2]=ten(2,2);
                    tensor_n_x=(tensor_x_p[t_id]-tensor[t_id])*pixel*pixel_x_p-tensor_x;
                    temporary_mask_px =pixel_x_p*mult;
                    temporary_component_x =(tensor_n_x-tensor_x)/vox_dim[0];
                  }
                  if(y<(size[1]-1))
                  {
                    // shifting y positive
                    ix[0]=x;
                    ix[1]=y+1;
                    ix[2]=z;
                    pixel_y_p = m_MaskImage->GetPixel(ix);
                    ten = tensorImg->GetPixel(ix);
                    tensor_y_p[0]=ten(0,0);
                    tensor_y_p[3]=ten(0,1);
                    tensor_y_p[5]=ten(0,2);
                    tensor_y_p[1]=ten(1,1);
                    tensor_y_p[4]=ten(1,2);
                    tensor_y_p[2]=ten(2,2);
                    tensor_n_y=(tensor_y_p[t_id]-tensor[t_id])*pixel*pixel_y_p-tensor_y;
                    temporary_mask_py =pixel_y_p*mult;
                    temporary_component_y =(tensor_n_y-tensor_y)/vox_dim[1];
                  }

                  if(z<(size[2]-1))
                  {
                    // shifting z positive15
                    ix[0]=x;
                    ix[1]=y;
                    ix[2]=z+1;
                    pixel_z_p = m_MaskImage->GetPixel(ix);
                    ten = tensorImg->GetPixel(ix);
                    tensor_z_p[0]= ten(0,0);
                    tensor_z_p[3]=ten(0,1);
                    tensor_z_p[5]=ten(0,2);
                    tensor_z_p[1]=ten(1,1);
                    tensor_z_p[4]=ten(1,2);
                    tensor_z_p[2]=ten(2,2);
                    tensor_n_z=(tensor_z_p[t_id]-tensor[t_id])*pixel*pixel_z_p-tensor_z;
                    temporary_mask_pz =pixel_z_p*mult;
                    temporary_component_z =(tensor_n_z-tensor_z)/vox_dim[2];
                  }
                  regulated_tensors[t_id]=dt_reg*(temporary_mask_px*temporary_component_x + temporary_mask_py*temporary_component_y + temporary_mask_pz*temporary_component_z);
                  temp_tensor_n[t_id][0]=tensor[t_id];



                }



                // setting ix to original positio15n- it is used later on
                ix[0]=x; ix[1]=y; ix[2]=z;

                temp_sum_comp=0.0;


                rom=m_H*temp_tensor_n;



                GradientVectorType data_vec = m_CorrectedVols->GetPixel(ix);
                for (int i=0;i<nof;i++)
                {
                  org_data[i]=data_vec[i];
                }

                calculate_attenuation(org_data,m_B0Mask,atten,mean_b,nof,number_b0);// calculating attenuation

                VariableVectorType varvec=f_w->GetPixel(ix);
                f_vec[0]=(float)varvec[0];
                f_vec[1]=(float)varvec[1];
                f_vec[2]=(float)varvec[2];


                if(f_vec[1] != f_vec[1])
                {
                  std::cout << "nan" << std::endl;
                }


                for(int i=0;i<nof-number_b0;i++)
                {

                  if (rom[i][0] != rom[i][0])
                  {
                    std::cout << "nan" << std::endl;
                  }

                  rom[i][0]=exp(rom[i][0]);

                  if (rom[i][0] != rom[i][0])
                  {
                    std::cout << "nan" << std::endl;
                  }


                  if (temp_sum_comp != temp_sum_comp)
                  {
                    std::cout << "nan" << std::endl;
                  }


                  temp_sum_comp=temp_sum_comp+((f_vec[1]*rom[i][0]+(1.0-f_vec[1])*exp(-m_BVec[i]*3.0) -atten[i])*(rom[i][0]-exp(-m_BVec[i]*3.0)));


                  if (temp_sum_comp != temp_sum_comp)
                  {
                    std::cout << "nan" << std::endl;
                  }



                  temporary_attenuation[i][0]=(f_vec[1]*rom[i][0]+(1.0-f_vec[1])*exp(-m_BVec[i]*3.0)-atten[i])*f_vec[1]*rom[i][0];
                }
                tct=-0.01*inverse_transposed*temporary_attenuation;

                for (int u=0;u<6;u++)
                {
                  regulated_tensors[u]=regulated_tensors[u]+ tct[u][0];
                  tensor[u]=tensor[u]+regulated_tensors[u];
                }

                ten(0,0) = tensor[0]; ten(0,1) = tensor[3]; ten(0,2) = tensor[5];
                ten(1,1) = tensor[1]; ten(1,2) = tensor[4]; ten(2,2) = tensor[2];
                //tensorImg->SetPixel(ix, ten);// saving new tensor
                regTensorImg->SetPixel(ix, ten);// saving new tensor

                f_vec[1]= f_vec[1]- 0.01*temp_sum_comp;
                if (f_vec[1]<f_vec[2])
                {
                  f_vec[1]= f_vec[2];
                }
                if (f_vec[1]> f_vec[0])
                {
                  f_vec[1]= f_vec[0];
                }

                VariableVectorType tempvec;
                tempvec.SetSize(3);
                tempvec[0] = (float)f_vec[0];
                tempvec[1] = (float)f_vec[1];
                tempvec[2] = (float)f_vec[2];

                if(f_vec[1] != f_vec[1])
                {
                  std::cout << "nan" << std::endl;
                }

                f_w->SetPixel(ix, tempvec);// saving new f map
              }
            }
          }
        }



      }



      this->SetNthOutput(0, tensorImg);

      // Set fmap output

      m_FWImage = itk::Image<float,3>::New();
      m_FWImage->SetRegions(region_tensorImg);
      m_FWImage->SetSpacing(m_CorrectedVols->GetSpacing());
      m_FWImage->SetOrigin(m_CorrectedVols->GetOrigin());
      m_FWImage->Allocate();



      size = m_FWImage->GetLargestPossibleRegion().GetSize();

      for(int i=0; i<size[0]; i++)
      {
        for(int j=0; j<size[1]; j++)
        {
          for(int k=0; k<size[2]; k++)
          {
            ix[0] = i;
            ix[1] = j;
            ix[2] = k;


            VariableVectorType tempvec = f_w->GetPixel(ix);
            float f = 1.0-tempvec[1];

            m_FWImage->SetPixel(ix, f);

          }
        }
      }


    }// end of generate data




    template <class TDiffusionPixelType, class TTensorPixelType>
    void
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    ::calculate_attenuation(vnl_vector<double> org_data,vnl_vector< double > b0index,vnl_vector<double> &atten, double mean_b,int nof, int numberb0)
    {
      mean_b=0.0;

      for (int i=0;i<nof;i++)
      {
        if(b0index[i]>0)
        {
          mean_b=mean_b+org_data[i];
        } // setting ix to original position- it is used later on


      }
      mean_b=mean_b/numberb0;
      int cnt=0; // setting ix to original position- it is used later on

      for (int i=0;i<nof;i++)
      {
        if(b0index[i]<0)
        {
          if(org_data[i]<0.001){org_data[i]=0.01;}
          atten[cnt]=org_data[i]/mean_b;
          double frank=org_data[i]/mean_b;
          cnt++;
        }
      }
    }


    template <class TDiffusionPixelType, class TTensorPixelType>
    void
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    ::calculate_tensor(vnl_matrix<double> pseudoInverse,vnl_vector<double> atten,vnl_vector<double> &tensor, int nof,int numberb0)
    {
      for (int i=0;i<nof-numberb0;i++)
      {
        atten[i]=log((double)atten[i]);
      }
        tensor = pseudoInverse*atten;
    }// end of void calculate tensor

  } // end of namespace
