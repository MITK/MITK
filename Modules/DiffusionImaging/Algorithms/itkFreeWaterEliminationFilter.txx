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
#include <C:\toolkits\mitk\bin2\MITK-superbuild\ITK-src\Utilities\vxl\core\vnl\vnl_vector_fixed.h>
#include <C:\toolkits\mitk\bin2\MITK-superbuild\ITK-src\Utilities\vxl\core\vnl\vnl_vector.h>
#include <C:\toolkits\mitk\bin2\MITK-superbuild\ITK-src\Utilities\vxl\core\vnl\vnl_matrix.h>
#include <C:\toolkits\mitk\bin2\MITK-superbuild\ITK-src\Utilities\vxl\core\vnl\algo\vnl_svd.h>
#include <C:\toolkits\mitk\bin2\MITK-superbuild\ITK-src\Utilities\vxl\core\vnl\algo\vnl_generalized_eigensystem.h>
#include <math.h>
#include <C:\toolkits\mitk\bin2\MITK-superbuild\ITK-src\Utilities\vxl\core\vnl\algo\vnl_symmetric_eigensystem.h>

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

    int nof = m_GradientDirectionContainer->Size();

    int cnt=0;

    for(int i=0; i<nof; i++)
    {
      vnl_vector_fixed< double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);


      if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001)
      {
        cnt++;
      }

    }

    int numberb0=cnt;

    float NOBRAIN= -100.0;







    int b= 1000;
    int in_g=0;
    int in_b=0;
    float temporaryb0sum=0.0;
    int cnt_dir=0;

    int etbt[6] = { 0, 4, 8, 1, 5, 2 };
    float temporary[3];
    float tensor[9];
    int badnumberpresent=0;
    int badnumberpast=0;
    int wasdetectedbad=0;
    int LOW_D=0.01;
    int HIGH_D = 5;
    int oldnumberofbads=100000000000;
    int newnumberofbads;
    int difff=1;


    int flag;


    vnl_matrix<float> H(nof-numberb0, 6);
    vnl_matrix<float> tem_tensor(3,3);




    vnl_vector< float> higher_thresh(nof-numberb0);
    vnl_vector< float> lower_thresh(nof-numberb0 );
    vnl_vector< float> diagonal(nof-numberb0);
    vnl_vector< float> b_vec(nof-numberb0);
    vnl_vector< float > b0index(nof);


    vnl_matrix<float> directions(nof-numberb0,3);




    int cnt_d=0;
    for(int i=0; i<nof; i++)
    {
      vnl_vector_fixed< double, 3 > vec = m_GradientDirectionContainer->ElementAt(i);


      if(vec[0]<0.0001 && vec[1]<0.0001 && vec[2]<0.0001)
      {
        b0index[i]=i;

      }
      else
      {
        directions[cnt_d][0]=vec[0];
        directions[cnt_d][1]=vec[1];
        directions[cnt_d][2]=vec[2];
        b0index[i]=-1;

      }
    }




    float ****table;// table in which all images except b0 are store
    table = new float***[nof-numberb0];
    for (int g=0; g<nof-numberb0; g++)
    {
      table[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        table[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          table[g][i][j] = new float[size[2]] ;
        }
      }
    }




    float ****b0image;// table with all b0images
    b0image = new float***[numberb0];
    for (int g=0; g<numberb0; g++)
    {
      b0image[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        b0image[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          b0image[g][i][j] = new float[size[2]] ;
        }
      }
    }

    float ***mask_all;
    mask_all = new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      mask_all[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        mask_all[i][j] = new float[size[2]] ;
      }
    }

    in_g=0;
    in_b=0;

    for(int x=0; x<size[0]; x++)
    {
      for(int y=0; y<size[1]; y++)
      {
        for(int z=0; z<size[2]; z++)
        {	

          itk::Index<3> ix;
          ix[0] = x;
          ix[1] = y;
          ix[2] = z;

          GradientVectorType pixel2 = gradientImagePointer->GetPixel(ix);

          for(int g=0; g<nof; g++)
          {
            if(g == b0index[g]){flag=1;}
            else{flag=0;}


            if(flag==0)
            {
              table[in_g][x][y][z] = pixel2[g];// data without b0
              in_g++;
            }
            else
            {
              b0image[in_b][x][y][z] = pixel2[g];// only b0 image
              in_b++;

            }

          }
          in_g=0;
          in_b=0;
        }
      }
    }


    float ***b0_mean_image;// table with mean b0 image
    b0_mean_image = new float**[size[0]];
    for (int g=0; g<size[0]; g++)
    {
      b0_mean_image[g] = new float*[size[1]];
      for ( int i = 0 ; i < size[1] ; i++ ) 
      {
        b0_mean_image[g][i] = new float[size[2]] ;

      }
    }

    for(int x =0;x<size[0];x++)
    {
      for(int y=0;y<size[1];y++)
      {
        for( int z=0;z<size[2];z++)
        {
          temporaryb0sum=0;
          for (int i=0;i<numberb0;i++)
          {
            temporaryb0sum=temporaryb0sum+b0image[i][x][y][z];         
          }
          b0_mean_image[x][y][z]=temporaryb0sum/numberb0;  
        }
      }
    }

    float ****sorted_eigs;// table in which all images except b0 are stored
    sorted_eigs = new float***[3];
    for (int g=0; g<3; g++)
    {
      sorted_eigs[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        sorted_eigs[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          sorted_eigs[g][i][j] = new float[size[2]] ;
        }
      }
    }

    float ****atten;// table in which all images except b0 are stored
    atten = new float***[nof-numberb0];
    for (int g=0; g<nof-numberb0; g++)
    {
      atten[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        atten[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          atten[g][i][j] = new float[size[2]] ;
        }
      }
    }

    float ****D;
    D = new float***[6];
    for (int g=0; g<6; g++)
    {
      D[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        D[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          D[g][i][j] = new float[size[2]] ;
        }
      }
    }







    cnt_dir=0;

    for(int g=0; g<nof; g++)
    {
      GradientDirectionType gradient = m_GradientDirectionContainer->ElementAt(g);




      if(gradient[0]>0.0001 || gradient[1]>0.0001 || gradient[2]>0.0001)
      {
        directions[cnt_dir][0] = gradient[0];
        directions[cnt_dir][1] = gradient[1];
        directions[cnt_dir][2] = gradient[2];

        cnt_dir++;
      }



    }// collection of all gradient directions except b0


    vnl_matrix<float> dirsTimesDirsTrans = directions*directions.transpose();




    for (int i=0;i<nof-numberb0;i++)
    {    
      diagonal[i]=dirsTimesDirsTrans[i][i];    
    }

    float max_diagonal = diagonal.max_value();//15:31 ok

    directions=directions*((float)b/1000.0)*(1.0/sqrt(max_diagonal));

    dirsTimesDirsTrans = directions*directions.transpose();


    for (int i=0;i<nof-numberb0;i++)
    {   
      b_vec[i]= dirsTimesDirsTrans[i][i];   
      std::cout << b_vec[i] << "\n";
    }
    std::cout << std::endl;

    for (int i = 0; i < nof-numberb0; i++) 
    {
      for (int j = 0; j < 3; j++) {temporary[j] = -directions[i][j];}

      for (int j = 0; j < 3; j++) 
      {
        for (int k = 0; k < 3; k++) {tensor[k + 3 * j] = temporary[k] * directions[i][j];}
      }
      for (int j = 0; j < 6; j++) {H[i][j] = tensor[etbt[j]];}
      for (int j = 0; j < 3; j++) {H[i][3 + j] *= 2.0;}
    }



    vnl_matrix<float> inputtopseudoinverse=H.transpose()*H;




    //vnl_svd<float> h_svd(H);
    vnl_symmetric_eigensystem<float> eig( inputtopseudoinverse);
    vnl_matrix<float> pseudoInverse = eig.pinverse()*H.transpose();

    /*for(int i = 0; i < nof-numberb0; i++)
    {
      for(int j = 0; j < nof-numberb0; j++)
      {
        std::cout << pseudoInverse[i][j] << " ";
      }
      std::cout << std::endl;
    }*/



    int negativitycheck=0;
    for ( int x=0;x<size[0];x++)
    {
      for ( int y=0;y<size[1];y++)
      {
        for ( int z=0;z<size[2];z++)
        { 
          if(b0_mean_image[x][y][z]<=0)
          {
            negativitycheck++;
          }
        }
      }
    }

    for ( int x=0;x<size[0];x++)
    {
      for ( int y=0;y<size[1];y++)
      {
        for ( int z=0;z<size[2];z++)
        { 
          if(b0_mean_image[x][y][z]<=0)
          {
            check_the_neighbors(x,y,z,b0_mean_image,size,NOBRAIN);
          }
        }
      }
    }
    int negativitycheckafter=0;

    for ( int x=0;x<size[0];x++)
    {
      for ( int y=0;y<size[1];y++)
      {
        for ( int z=0;z<size[2];z++)
        { 
          if(b0_mean_image[x][y][z]<=0)
          {
            negativitycheckafter++;
          }
        }
      }
    }
    std::cout << "before: " << negativitycheck << "\nafter: " << negativitycheckafter << std::endl;

    table[2][3][4][8]=-10;
    table[4][45][56][38]=-10;

    int negativitycheck1=0;

    for (int g=0;g<nof-numberb0;g++)
    {
      for ( int x=0;x<size[0];x++)
      {
        for ( int y=0;y<size[1];y++)
        {
          for ( int z=0;z<size[2];z++)
          { 
            if(table[g][x][y][z]<=0)
            {
              negativitycheck1++;
            }
          }
        }
      }
    }
    for (int g=0;g<nof-numberb0;g++)
    {
      for ( int x=0;x<size[0];x++)
      {
        for ( int y=0;y<size[1];y++)
        {
          for ( int z=0;z<size[2];z++)
          { 
            if(table[g][x][y][z]<=0)
            {

              check_the_neighbors4D(g,x,y,z,table,size,NOBRAIN);
            }
          }
        }
      }
    }
    int negativitycheckafter1=0;
    for (int g=0;g<nof-numberb0;g++)
    {
      for ( int x=0;x<size[0];x++)
      {
        for ( int y=0;y<size[1];y++)
        {
          for ( int z=0;z<size[2];z++)
          { 
            if(table[g][x][y][z]<=0)
            {
              negativitycheckafter1++;
            }
          }
        }
      }
    }
    std::cout << "before: " << negativitycheck1 << "\nafter: " << negativitycheckafter1 << std::endl;

    /*for ( int g=0;g<nof-numberb0;g++)
    {
    for ( int x=0;x<size[0];x++)
    {
    for ( int y=0;y<size[1];y++)
    {
    for ( int z=0;z<size[2];z++)
    { 
    if(table[g][x][y][z]<=0)
    {
    check_the_neighbors(x,y,z,table[g],size);
    }
    }
    }
    }
    }// removing voxels smaller than 0 from the data / i am not using this threshold for this step cause in my opinion its sensles - if data are below zero it would be below threshold for sure.

    */





    for (int i=1;i<nof-numberb0;i++)
    {
      higher_thresh[i]=exp(-b_vec[i]*LOW_D);
      lower_thresh[i]= exp(-b_vec[i]*HIGH_D);
    }

    int break_flag=0;


    while(difff>0)
    {
      wasdetectedbad=0;

      for ( int x=0;x<size[0];x++)
      {
        for ( int y=0;y<size[1];y++)
        {
          for ( int z=0;z<size[2];z++)
          {

            //if(b0_mean_image[x][y][z]>=0)

            //{


            remove_negative_eigs(x,y,z,table,nof,size,pseudoInverse, D, atten, higher_thresh, lower_thresh,wasdetectedbad, b0_mean_image,sorted_eigs,numberb0,break_flag,NOBRAIN,mask_all);

            //}

          }
        }
      }
      std::cout << "current: " << wasdetectedbad << std::endl;
      newnumberofbads=wasdetectedbad;
      difff=oldnumberofbads-newnumberofbads;
      //std::cout << "current: " << newnumberofbads << "\npast: " << oldnumberofbads << std::endl;
      oldnumberofbads=wasdetectedbad;
    }
    /// I will try to change it as well   std::cout << "current: " << newnumberofbads << "\npast: " << oldnumberofbads << std::endl;`



    for ( int a=0;a<size[0];a++)
      {
        for ( int b=0;b<size[1];b++)
        {
          for ( int c=0;c<size[2];c++)
          {
           if(b0_mean_image[a][b][c]>50.0)
           mask_all[a][b][c]=1.0;
           else
           mask_all[a][b][c]=0.0;

          }
          
        }
        
    }
    
    

    ImageType::IndexType start;
    start.Fill(0);

    ImageType::RegionType region(start,size);

    ImageType::Pointer image = ImageType::New();
    image->SetRegions(region);
    image->SetVectorLength(nof);
    image->Allocate();

    ImageType::IndexType pixelIndex;


    typedef itk::VariableLengthVector<short> VariableVectorType;
    VariableVectorType variableLengthVector;
    variableLengthVector.SetSize(nof);


    for(int x=0;x<size[0];x++)
    {
      for(int y=0;y<size[1];y++)
      {
        for(int z=0;z<size[2];z++)
        {
          pixelIndex[0] = x;
          pixelIndex[1] = y;
          pixelIndex[2] = z;

          for(int i=0;i<nof-numberb0;i++)
          {
            variableLengthVector[i] = (short)table[i][x][y][z];
          }
          variableLengthVector[nof-1]= (short)b0_mean_image[x][y][z];
          image->SetPixel(pixelIndex, variableLengthVector);

        }
      }
    }



 



 /// new filter
   
    float ****D_after_minimization;
    D_after_minimization = new float***[6];
    for (int g=0; g<6; g++)
    {
      D_after_minimization[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        D_after_minimization[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          D_after_minimization[g][i][j] = new float[size[2]] ;
        }
      }
    } // new D


    float ****regulated_tensors;
    regulated_tensors = new float***[6];
    for (int g=0; g<6; g++)
    {
      regulated_tensors[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        regulated_tensors[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          regulated_tensors[g][i][j] = new float[size[2]] ;
        }
      }
    } // temporary tensors

    float ***mean_difusivity;
    mean_difusivity = new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      mean_difusivity[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        mean_difusivity[i][j] = new float[size[2]] ;
      }
    }//declaration of mean difusivity map

    float ***max_f;
    max_f= new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      max_f[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        max_f[i][j] = new float[size[2]] ;
      }
    }//declaration of maximal f

    float ***f;
    f = new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      f[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        f[i][j] = new float[size[2]] ;
      }
    }// declaration of f

    float f_min=0.0;
    float f_max=0.0;


    float ***min_f;
    min_f = new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      min_f[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        min_f[i][j] = new float[size[2]] ;
      }
    }// declaration of minimal f
 
    float baseline=0.6;
    float max_eig=2.5;
    float min_eig=0.01;
    float watered=3.0;
    float temporary_b_component=0.0;

    float er = 0.0;

    float beta=0.0;

    float dt=0.0025;

    float dt_fidel=0.01;

    float dt_f=0.01;





    for (int i=0;i<nof-numberb0;i++)
    {
    temporary_b_component=temporary_b_component+b_vec[i];
    }

    float mean_b=temporary_b_component/(nof-numberb0);


    float temporary_mean_sum=0.0;

    for ( int x=0; x<size[0];x++)
    {
      for( int y=0;y<size[1];y++)
      {
        for (int z =0;z<size[2];z++)
        {
          for (int g=1;g<nof-numberb0;g++)
          {
            temporary_mean_sum=temporary_mean_sum + D[g][x][y][z];
          }
          mean_difusivity[x][y][z]=temporary_mean_sum/(nof-numberb0);
          temporary_mean_sum=0.0;
        }
      }
    }

    for ( int x=0; x<size[0];x++)
    {
      for( int y=0;y<size[1];y++)
      {
        for (int z =0;z<size[2];z++)
        {
          max_f[x][y][z]= (exp(-mean_b*sorted_eigs[0][x][y][z]))/(exp(-min_eig*mean_b))-(exp(-mean_b*watered));

          if(max_f[x][y][z]>1.0){max_f[x][y][z]=1.0;}
          else if(max_f[x][y][z]<0.0){max_f[x][y][z]=0.0;}
        }
      }
    }

    for ( int x=0; x<size[0];x++)
    {
      for( int y=0;y<size[1];y++)
      {
        for (int z =0;z<size[2];z++)
        {
          min_f[x][y][z]= (exp(-mean_b*sorted_eigs[2][x][y][z]))/(exp(-max_eig*mean_b))-(exp(-mean_b*watered));
          if(min_f[x][y][z]>1.0){min_f[x][y][z]=1.0;}
          else if(min_f[x][y][z]<0.0){min_f[x][y][z]=0.0;}
        }
      }
    }


    for ( int x=0; x<size[0];x++)
    {
      for( int y=0;y<size[1];y++)
      {
        for (int z =0;z<size[2];z++)
        {
          f[x][y][z]= (exp(-mean_b*mean_difusivity[x][y][z]))/(exp(-baseline*mean_b))-(exp(-mean_b*watered));

          if(f[x][y][z]>max_f[x][y][z]){f[x][y][z]=(min_f[x][y][z]+max_f[x][y][z])/2;}
          else if(f[x][y][z]<min_f[x][y][z]){f[x][y][z]=(min_f[x][y][z]+max_f[x][y][z])/2;}

        }
      }
    }

  

    float ****mask_extended;// table with all b0images
    mask_extended = new float***[6];
    for (int g=0; g<6; g++)
    {
      mask_extended[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        mask_extended[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          mask_extended[g][i][j] = new float[size[2]] ;
        }
      }
    }


    for ( int j=0;j<6;j++)
    {
      for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
              
                if (j==0)
                {
                  if(x==0)
                  {
                    mask_extended[j][x][y][z]= mask_all[x][y][z]*mask_all[x][y][z];
                    
                   }
                    else
                    {
                    mask_extended[j][x][y][z]= mask_all[x-1][y][z]*mask_all[x][y][z];
                    
                    }
                  }
                if (j==1)
                {
                   if(y==0)
                  {
                    mask_extended[j][x][y][z]= mask_all[x][y][z]*mask_all[x][y][z];
                    
                   }
                    else
                    {
                    mask_extended[j][x][y][z]= mask_all[x][y-1][z]*mask_all[x][y][z];
                    
                    }
                 }
                if (j==2)
                {
                 if(z==0)
                  {
                    mask_extended[j][x][y][z]= mask_all[x][y][z]*mask_all[x][y][z];
                    
                   }
                    else
                    {
                    mask_extended[j][x][y][z]= mask_all[x][y][z-1]*mask_all[x][y][z];
                    
                    }
                }

                if (j==3)
                {
                  if(x==size[0]-1)
                  {
                    mask_extended[j][x][y][z]= mask_all[x][y][z]*mask_extended[0][x][y][z]*mask_extended[1][x][y][z]*mask_extended[2][x][y][z];
                    
                   }
                    else
                    {
                    mask_extended[j][x][y][z]= mask_all[x+1][y][z]*mask_extended[0][x][y][z]*mask_extended[1][x][y][z]*mask_extended[2][x][y][z];
                    
                    }
                  }
                if (j==4)
                {
                   if(y==size[1]-1)
                  {
                    mask_extended[j][x][y][z]= mask_all[x][y][z]*mask_extended[0][x][y][z]*mask_extended[1][x][y][z]*mask_extended[2][x][y][z];
                    
                   }
                    else
                    {
                    mask_extended[j][x][y][z]= mask_all[x][y+1][z]*mask_extended[0][x][y][z]*mask_extended[1][x][y][z]*mask_extended[2][x][y][z];
                    
                    }
                 }
                if (j==5)
                {
                 if(z==size[2]-1)
                  {
                    mask_extended[j][x][y][z]= mask_all[x][y][z]*mask_extended[0][x][y][z]*mask_extended[1][x][y][z]*mask_extended[2][x][y][z];
                    
                   }
                    else
                    {
                    mask_extended[j][x][y][z]= mask_all[x][y][z+1]*mask_extended[0][x][y][z]*mask_extended[1][x][y][z]*mask_extended[2][x][y][z];
                    
                    }
                }

            }


          }
      
        }

      }

    float ****natten;// table in which all images except b0 are stored
    natten = new float***[nof-numberb0];
    for (int g=0; g<nof-numberb0; g++)
    {
      natten[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        natten[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          natten[g][i][j] = new float[size[2]] ;
        }
      }
    }



    for (int i=0;i<nof-numberb0;i++)
    {
      for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {

             if(atten[i][x][y][z]==-100.0)
             {
               natten[i][x][y][z]=-100.0;

             }
             else
             {
               natten[i][x][y][z]= (atten[i][x][y][z]-(exp(-3.0*b_vec[i])*(1.0-f[x][y][z])))/f[x][y][z];

              if (natten[i][x][y][z]<0.0)
              {
                natten[i][x][y][z]=0.01;
              }
             }

          }
        }
      }
    }

     
     vnl_vector<float> natten_log(nof-numberb0);
     vnl_vector<float> temp_D(6);

      for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
           if(mask_all[x][y][z]>0.0)
           {
             for(int i=0;i<nof-numberb0;i++)
             {
               natten_log[i]=log(natten[i][x][y][z]);
             }
              temp_D = pseudoInverse*natten_log;
              for (int u=0;u<6;u++){D_after_minimization[u][x][y][z]=temp_D[u];}
            }
           else{for (int u=0;u<6;u++){D_after_minimization[u][x][y][z]=0.0;}}
          }
        }
      }

    float ****diff;// table in which all images except b0 are stored
    diff = new float***[nof-numberb0];
    for (int g=0; g<nof-numberb0; g++)
    {
      diff[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        diff[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          diff[g][i][j] = new float[size[2]] ;
        }
      }
    }

    float ****diwa;// table in which all images except b0 are stored
    diwa = new float***[6];
    for (int g=0; g<6; g++)
    {
      diwa[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        diwa[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          diwa[g][i][j] = new float[size[2]] ;
        }
      }
    }

    float ****model;// table in which all images except b0 are stored
    model = new float***[6];
    for (int g=0; g<6; g++)
    {
      model[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        model[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          model[g][i][j] = new float[size[2]] ;
        }
      }
    }

    float ***df;
    df = new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      df[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        df[i][j] = new float[size[2]] ;
      }
    }

    float ***dif;
    dif = new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      dif[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        dif[i][j] = new float[size[2]] ;
      }
    }


    int defined_iter=50;
    int iter=1;

    vnl_vector<float> vox_dim(3);

    for(int i=0;i<3;i++)
    {
      vox_dim[i]=1.0;
    }


    float ****ten;
    ten = new float***[nof-numberb0];
    for (int g=0; g<6; g++)
    {
      ten[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        ten[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          ten[g][i][j] = new float[size[2]] ;
        }
      }
    } // ten variable




    float *****temp_tensors;
    temp_tensors = new float****[3];
    for (int g=0; g<3; g++)
    {
      temp_tensors[g] = new float***[6];
      for (int u=0;u<6;u++)
      {
      temp_tensors[g][u] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        temp_tensors[g][u][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          temp_tensors[g][u][i][j] = new float[size[2]] ;
        }
      }
      }
    } // temporary tensors


    float *****temp_tensors_2;
    temp_tensors_2 = new float****[3];
    for (int g=0; g<3; g++)
    {
      temp_tensors_2[g] = new float***[6];
      for (int u=0;u<6;u++)
      {
      temp_tensors_2[g][u] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        temp_tensors_2[g][u][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          temp_tensors_2[g][u][i][j] = new float[size[2]] ;
        }
      }
      }
    }


    float *****temp_tensors_3;
    temp_tensors_3 = new float****[3];
    for (int g=0; g<3; g++)
    {
      temp_tensors_3[g] = new float***[6];
      for (int u=0;u<6;u++)
      {
      temp_tensors_3[g][u] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        temp_tensors_3[g][u][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          temp_tensors_3[g][u][i][j] = new float[size[2]] ;
        }
      }
      }
    }

    float ****induced_metric;
    induced_metric = new float***[6];
    for (int g=0; g<6; g++)
    {
      induced_metric[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        induced_metric[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          induced_metric[g][i][j] = new float[size[2]] ;
        }
      }
    } // temporary tensors

    float ***determinant;
    determinant = new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      determinant[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        determinant[i][j] = new float[size[2]] ;
      }
    }

    float ***determinant_g;
    determinant_g = new float**[size[0]];
    for ( int i = 0 ; i < size[0] ; i++ ) 
    {
      determinant_g[i] = new float*[size[1]] ;
      for ( int j = 0 ; j < size[1] ; j++ )
      {
        determinant_g[i][j] = new float[size[2]] ;
      }
    }

      float ****inverse_metric;
    inverse_metric = new float***[6];
    for (int g=0; g<6; g++)
    {
      inverse_metric[g] = new float**[size[0]];
      for ( int i = 0 ; i < size[0] ; i++ ) 
      {
        inverse_metric[g][i] = new float*[size[1]] ;
        for ( int j = 0 ; j < size[1] ; j++ )
        {
          inverse_metric[g][i][j] = new float[size[2]] ;
        }
      }
    }




    for (int iteration=0;iteration<defined_iter;iteration++)
    {

      if(dt>0.0)
      {
        beltrami_regulation_term(mask_extended,vox_dim,beta,dt,iter,D_after_minimization,regulated_tensors,er,size,nof,numberb0,temp_tensors,temp_tensors_2,temp_tensors_3,induced_metric,inverse_metric, determinant,determinant_g);
      }

      if(dt_fidel>0.0)
      {
        fidelity_term(D_after_minimization,mask_all,f,pseudoInverse,atten,dt_fidel,b_vec, df, model, diff, nof,numberb0,dif,size,ten);
      }

      for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            if(mask_all[x][y][z]>0.0)
            {
              for(int i=0;i<6;i++)
              {
             
                regulated_tensors[i][x][y][z]=regulated_tensors[i][x][y][z]+model[i][x][y][z];
                D_after_minimization[i][x][y][z]=D_after_minimization[i][x][y][z]+regulated_tensors[i][x][y][z];
                
              }
              f[x][y][z]=f[x][y][z]+dt_f*df[x][y][z];
              if(f[x][y][z] > 0.0)
                std::cout << "non-zero";

            }else
            {
              for(int i=0;i<6;i++){
                regulated_tensors[i][x][y][z]=0.0;
                D_after_minimization[i][x][y][z]=0.0;
              }
              f[x][y][z]=0.0;
            }

          }
        }
    }

   find_min_max(f, f_min,f_max,min_f,max_f,size);

   for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            if (f[x][y][z]<min_f[x][y][z])
            {
              f[x][y][z]= f_min;

            }

          }

        }
   }

     for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            if (f[x][y][z]>max_f[x][y][z])
            {
              f[x][y][z]= f_max;

            }

          }

        }
   }






   std::cout << "now: " << iteration << std::endl;

    }// main loop of minimization


       // fwe map
    typedef itk::Image<float, 3> ImageType_fw;

    ImageType_fw::IndexType start_fwe;
    start_fwe.Fill(0);

    ImageType_fw::RegionType region_fwe(start_fwe,size);

    ImageType_fw::Pointer fw_map = ImageType_fw::New();
    fw_map->SetRegions(region_fwe);
    fw_map->Allocate();

    ImageType_fw::IndexType pixelIndex_fwe;

    
    float pixel;


  


    for(int x=0;x<size[0];x++)
    {
      for(int y=0;y<size[1];y++)
      {
        for(int z=0;z<size[2];z++)
        {
          pixelIndex_fwe[0] = x;
          pixelIndex_fwe[1] = y;
          pixelIndex_fwe[2] = z;

         
           pixel = (float)f[x][y][z];
            if(pixel>0.0)
              std::cout << " non zero" ;
            fw_map->SetPixel(pixelIndex_fwe, pixel);

        }
      }
    }

    
    mitk::Image::Pointer imageInMitk;
    CastToMitkImage(fw_map, imageInMitk);


    mitk::ImageWriter::Pointer writer = mitk::ImageWriter::New();
    writer->SetInput(imageInMitk);


    std::string file;

    std::ostringstream filename;
    filename << "C:\\Users\\binczyk\\Desktop\\Nowy folder (3)";
    filename << "fwemap";
    filename << ".nrrd";

    file = filename.str();

    writer->SetFileName(file);
    writer->Update();








    m_OutputDiffusionImage = mitk::DiffusionImage<short>::New();
    m_OutputDiffusionImage->SetVectorImage( image );
    m_OutputDiffusionImage->SetB_Value(m_BValue);
    m_OutputDiffusionImage->SetDirections(m_GradientDirectionContainer);
    m_OutputDiffusionImage->SetOriginalDirections(m_GradientDirectionContainer);
    m_OutputDiffusionImage->InitializeFromVectorImage();


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
    /* normalization
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
    */
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
    ::check_the_neighbors4D(int g,int x, int y, int z, float ****table, itk::Size<3> size,float NOBRAIN)   
  {


    float temp_sum=0;
    float temp_number=0;
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
    else if(x==size[0]-1)
    {
      init_i=x-1;
      limit_i=x+1;
    }
    else
    {
      init_i=x-1;
      limit_i=x+2;
    }

    for (int i=init_i;i<limit_i;i+=2)

    {// 1
      if(y==0)
      {
        init_j=y;
        limit_j=y+2;
      }
      else if(y==size[1]-1)
      {
        init_j=y-1;
        limit_j=y+1;
      }
      else
      {
        init_j=y-1;
        limit_j=y+2;
      }

      for (int j=init_j;j<limit_j;j+=2)
      {//2

        if(z==0)
        {
          init_c=z;
          limit_c=z+2;
        }
        else if(z==size[2]-1)
        {
          init_c=z-1;
          limit_c=z+1;
        }
        else
        {
          init_c=z-1;
          limit_c=z+2;
        }
        for (int c=init_c;c<limit_c;c+=2)
        {//3


          if(table[g][i][j][c]<= 0)
          {
            temp_sum=temp_sum + 0;
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
      table[g][x][y][z]=NOBRAIN;
    }
    else
    {
      table[g][x][y][z]=temp_sum/temp_number;
    }


  }// end of void


  template <class TDiffusionPixelType, class TTensorPixelType>
  void
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    ::check_the_neighbors(int x, int y, int z, float ***table, itk::Size<3> size,float NOBRAIN)   
  {

    float temp_sum=0;
    float temp_number=0;
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
    else if(x==size[0]-1)
    {
      init_i=x-1;
      limit_i=x+1;
    }
    else
    {
      init_i=x-1;
      limit_i=x+2;
    }

    for (int i=init_i;i<limit_i;i+=2)

    {// 1
      if(y==0)
      {
        init_j=y;
        limit_j=y+2;
      }
      else if(y==size[1]-1)
      {
        init_j=y-1;
        limit_j=y+1;
      }
      else
      {
        init_j=y-1;
        limit_j=y+2;
      }

      for (int j=init_j;j<limit_j;j+=2)
      {//2

        if(z==0)
        {
          init_c=z;
          limit_c=z+2;
        }
        else if(z==size[2]-1)
        {
          init_c=z-1;
          limit_c=z+1;
        }
        else
        {
          init_c=z-1;
          limit_c=z+2;
        }
        for (int c=init_c;c<limit_c;c+=2)
        {//3


          if(table[i][j][c]<=0)
          {
            temp_sum=temp_sum + 0;
          }
          else
          {
            temp_sum=temp_sum+table[i][j][c];
            temp_number++;
          }
        }//3
      }//2
    }//1

    if (temp_number==0)
    {
      table[x][y][z]=NOBRAIN;
    }
    else
    {
      table[x][y][z]=temp_sum/temp_number;
    }


  }// end of void



  /////

    template <class TDiffusionPixelType, class TTensorPixelType>
    void 
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    :: fidelity_term(float ****D_after_minimization, float ***mask_all, float ***f,vnl_matrix<float> pseudoInverse,float ****atten, float dt_fidel,vnl_vector< float> b_vec, float ***df, float ****model, float ****diff, int nof, int numberb0, float ***dif,itk::Size<3> size,float ****ten)

    {

 


    vnl_vector<float> temp_tensor(6);
    vnl_vector<float> temp_ten(nof-numberb0);

    for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            if(mask_all[x][y][z]>0.0)
            {
              for(int i=0;i<6;i++)
              {
                temp_tensor[i]=D_after_minimization[i][x][y][z];
              }
              temp_ten=pseudoInverse*temp_tensor;

              for(int i=0;i<nof-numberb0;i++)
              {
                temp_ten[i]=exp(temp_ten[i]);
              }
              for (int u=0;u<nof-numberb0;u++){ten[u][x][y][z]=temp_ten[u];}

            }else
            {
              for (int u=0;u<nof-numberb0;u++){ten[u][x][y][z]=0.0;}
            }

          }
        }
    }
    float w_comp;

     for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            if(mask_all[x][y][z]>0.0)
            {
               for(int i=0;i<nof-numberb0;i++)
              {
                w_comp=exp(-b_vec[i]*3.0);
                diff[i][x][y][z]=(f[x][y][z]*ten[i][x][y][z])+((1.0-f[x][y][z])*w_comp -atten[i][x][y][z]);
                
              }

            }else
            {
              for (int u=0;u<nof-numberb0;u++){diff[u][x][y][z]=0.0;}
            }

          }
        }
    }

     float temp_sum_comp=0.0;

     for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            if(mask_all[x][y][z]>0.0)
            {
               for(int i=0;i<nof-numberb0;i++)
              {
               w_comp=exp(-b_vec[i]*3.0);
               temp_sum_comp=temp_sum_comp+(diff[i][x][y][z]*(ten[i][x][y][z]-w_comp));
                
              }
             df[x][y][z]=-temp_sum_comp;


            }else
            {
              df[x][y][z]=0.0;
            }

          }
        }
    }


     vnl_matrix<float> inverse_transposed = -dt_fidel*pseudoInverse.transpose();
     vnl_matrix<float> temporary_attenuation(nof-numberb0,1);
     vnl_matrix<float> tct(6,1);



      for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            if(mask_all[x][y][z]>0.0)
            {
               for(int i=0;i<nof-numberb0;i++)
              {
                temporary_attenuation[i][0]=diff[i][x][y][z]*f[x][y][z]*ten[i][x][y][z];
                
              }
               tct=inverse_transposed*temporary_attenuation;
               for (int u=0;u<6;u++){model[u][x][y][z]=tct[u][0];}


            }else
            {
              for (int u=0;u<6;u++){model[u][x][y][z]=0.0;}
            }

          }
        }
    }

    float temporary_var=0.0;


    for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
            if(mask_all[x][y][z]>0.0)
            {
               for(int i=0;i<nof-numberb0;i++)
              {
                temporary_var=temporary_var + diff[i][x][y][z]*diff[i][x][y][z];
                
              }
               dif[x][y][z]=temporary_var/(nof-numberb0);
              


            }else
            {
              dif[x][y][z]=0.0;
            }

          }
        }
    }




    }//end of fidelity term



    template <class TDiffusionPixelType, class TTensorPixelType>
    void 
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    :: find_min_max(float ***f, float &f_min, float &f_max, float ***min_f, float ***max_f,itk::Size<3> size)
    
    {


    float min_temp=10000.0;

    for (int x=1;x<size[0];x++)
      {
        for (int y=1;y<size[1];y++)
        {
          for (int z=1;z<size[2];z++)
          {

            if(f[x][y][z]>min_f[x][y][z] && f[x][y][z]<min_temp)
            {
              f_min=f[x][y][z];

            }
            min_temp=f[x][y][z];

          }


        }

    }


    float max_temp=-10000.0;

    for (int x=1;x<size[0];x++)
      {
        for (int y=1;y<size[1];y++)
        {
          for (int z=1;z<size[2];z++)
          {

            if(f[x][y][z]<max_f[x][y][z] && f[x][y][z]>max_temp)
            {
              f_min=f[x][y][z];

            }
             max_temp=f[x][y][z];

          }


        }

    }

      











    }// end of min































  template <class TDiffusionPixelType, class TTensorPixelType>
    void 
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    :: beltrami_regulation_term(float ****mask_extended, vnl_vector<float> vox_dim, float beta,float dt,float iter,float ****D, float ****regulated_tensors, float er, itk::Size<3> size,int nof, int numberb0,float *****temp_tensors,float *****temp_tensors_2,float *****temp_tensors_3,float ****induced_metric, float ****inverse_metric,float ***determinant,float ***determinant_g)
    {

    


    // declaratuon of temporary stuff

    
   

    

    //  end of temporary variables

  
  for (int j=0;j<3;j++)
   {
    for ( int i=0;i<6;i++)
    {
      for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
              
                if (j==0)
                {
                  if(x==0)
                  {
                    temp_tensors[j][i][x][y][z]=(D[i][x][y][z]-D[i][x][y][z])/vox_dim[0]*mask_extended[0][x][y][z];
                    
                   }
                    else
                    {
                    temp_tensors[j][i][x][y][z]=(D[i][x][y][z]-D[i][x-1][y][z])/vox_dim[0]*mask_extended[0][x][y][z];
                    
                    }
                  }
                if (j==1)
                {
                  if(y==0)
                  {
                    temp_tensors[j][i][x][y][z]=(D[i][x][y][z]-D[i][x][y][z])/vox_dim[1]*mask_extended[1][x][y][z];
                    
                    }
                    else
                    {
                    temp_tensors[j][i][x][y][z]=(D[i][x][y][z]-D[i][x][y-1][z])/vox_dim[1]*mask_extended[1][x][y][z];
                    
                    }
                 }
                if (j==2)
                {
                if(z==0)
                  {
                    temp_tensors[j][i][x][y][z]=(D[i][x][y][z]-D[i][x][y][z])/vox_dim[2]*mask_extended[2][x][y][z];
                    
                    }
                    else
                    {
                    temp_tensors[j][i][x][y][z]=(D[i][x][y][z]-D[i][x][y][z-1])/vox_dim[2]*mask_extended[2][x][y][z];
                    
                    }
                }

            }


          }
      
        }

      }


    }// end of temp_tensors loop


 
       for (int x=0;x<size[0];x++)
        {
          for (int y=0;y<size[1];y++)
          {
           for (int z=0;z<size[2];z++)
            {
             
               induced_metric[0][x][y][z]=(1.0+beta)*(temp_tensors[0][0][x][y][z]*temp_tensors[0][0][x][y][z]+temp_tensors[0][1][x][y][z]*temp_tensors[0][1][x][y][z]+temp_tensors[0][2][x][y][z]*temp_tensors[0][2][x][y][z]
               +temp_tensors[0][3][x][y][z]*temp_tensors[0][3][x][y][z]+temp_tensors[0][4][x][y][z]*temp_tensors[0][4][x][y][z]+temp_tensors[0][5][x][y][z]*temp_tensors[0][5][x][y][z]);

               induced_metric[1][x][y][z]=(1.0+beta)*(temp_tensors[1][0][x][y][z]*temp_tensors[1][0][x][y][z]+temp_tensors[1][1][x][y][z]*temp_tensors[1][1][x][y][z]+temp_tensors[1][2][x][y][z]*temp_tensors[1][2][x][y][z]
               +temp_tensors[1][3][x][y][z]*temp_tensors[1][3][x][y][z]+temp_tensors[1][4][x][y][z]*temp_tensors[1][4][x][y][z]+temp_tensors[1][5][x][y][z]*temp_tensors[1][5][x][y][z]);

               induced_metric[2][x][y][z]=(1.0+beta)*(temp_tensors[2][0][x][y][z]*temp_tensors[2][0][x][y][z]+temp_tensors[2][1][x][y][z]*temp_tensors[2][1][x][y][z]+temp_tensors[2][2][x][y][z]*temp_tensors[2][2][x][y][z]
             
              +temp_tensors[2][3][x][y][z]*temp_tensors[2][3][x][y][z]+temp_tensors[2][4][x][y][z]*temp_tensors[2][4][x][y][z]+temp_tensors[2][5][x][y][z]*temp_tensors[2][5][x][y][z]);

              induced_metric[3][x][y][z]=(beta)*(temp_tensors[0][0][x][y][z]*temp_tensors[1][0][x][y][z]+temp_tensors[0][1][x][y][z]*temp_tensors[1][1][x][y][z]+temp_tensors[0][2][x][y][z]*temp_tensors[1][2][x][y][z]
               +temp_tensors[0][3][x][y][z]*temp_tensors[1][3][x][y][z]+temp_tensors[0][4][x][y][z]*temp_tensors[1][4][x][y][z]+temp_tensors[0][5][x][y][z]*temp_tensors[1][5][x][y][z]);

              induced_metric[4][x][y][z]=(beta)*(temp_tensors[0][0][x][y][z]*temp_tensors[2][0][x][y][z]+temp_tensors[0][1][x][y][z]*temp_tensors[2][1][x][y][z]+temp_tensors[0][2][x][y][z]*temp_tensors[2][2][x][y][z]
               +temp_tensors[0][3][x][y][z]*temp_tensors[2][3][x][y][z]+temp_tensors[0][4][x][y][z]*temp_tensors[2][4][x][y][z]+temp_tensors[0][5][x][y][z]*temp_tensors[2][5][x][y][z]);

               induced_metric[5][x][y][z]=(beta)*(temp_tensors[1][0][x][y][z]*temp_tensors[2][0][x][y][z]+temp_tensors[1][1][x][y][z]*temp_tensors[2][1][x][y][z]+temp_tensors[1][2][x][y][z]*temp_tensors[2][2][x][y][z]
               +temp_tensors[1][3][x][y][z]*temp_tensors[2][3][x][y][z]+temp_tensors[1][4][x][y][z]*temp_tensors[2][4][x][y][z]+temp_tensors[1][5][x][y][z]*temp_tensors[2][5][x][y][z]);

            }

          }
        }


        for (int x=0;x<size[0];x++)
        {
          for (int y=0;y<size[1];y++)
          {
           for (int z=0;z<size[2];z++)
            {

              determinant[x][y][z]=induced_metric[0][x][y][z]*(induced_metric[1][x][y][z]*induced_metric[2][x][y][z]-induced_metric[5][x][y][z]*induced_metric[5][x][y][z])
                -induced_metric[3][x][y][z]*(induced_metric[3][x][y][z]*induced_metric[2][x][y][z]-induced_metric[5][x][y][z]*induced_metric[4][x][y][z])
                + induced_metric[4][x][y][z]*(induced_metric[3][x][y][z]*induced_metric[5][x][y][z]-induced_metric[1][x][y][z]*induced_metric[4][x][y][z]);


           }
          }
        }

  

      for (int x=0;x<size[0];x++)
        {
          for (int y=0;y<size[1];y++)
          {
           for (int z=0;z<size[2];z++)
            {
              inverse_metric[0][x][y][z]=(induced_metric[1][x][y][z]*induced_metric[2][x][y][z]-induced_metric[5][x][y][z]*induced_metric[5][x][y][z]);
              inverse_metric[1][x][y][z]=(induced_metric[0][x][y][z]*induced_metric[2][x][y][z]-induced_metric[4][x][y][z]*induced_metric[4][x][y][z]);
              inverse_metric[2][x][y][z]=(induced_metric[0][x][y][z]*induced_metric[1][x][y][z]-induced_metric[3][x][y][z]*induced_metric[3][x][y][z]);
              inverse_metric[3][x][y][z]=(induced_metric[3][x][y][z]*induced_metric[5][x][y][z]-induced_metric[1][x][y][z]*induced_metric[4][x][y][z]);
              inverse_metric[4][x][y][z]=(induced_metric[0][x][y][z]*induced_metric[5][x][y][z]-induced_metric[3][x][y][z]*induced_metric[4][x][y][z]);
              inverse_metric[5][x][y][z]=(induced_metric[3][x][y][z]*induced_metric[2][x][y][z]-induced_metric[5][x][y][z]*induced_metric[4][x][y][z]);

           }

          }

      }

   
    float temporary_variable=0.0;
    float power=0.5;


        for (int x=0;x<size[0];x++)
        {
          for (int y=0;y<size[1];y++)
          {
           for (int z=0;z<size[2];z++)
            {

              temporary_variable=pow(temporary_variable+determinant[x][y][z],power);
              determinant_g[x][y][z]=pow(determinant[x][y][z],-power);

           }

          }

        }

        er=temporary_variable/(size[0]*size[1]*size[2]);



   for(int u=0;u<6;u++)
   {
 
      for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {

           
            temp_tensors_2[0][u][x][y][z]= (1.0/determinant_g[x][y][z])*(inverse_metric[0][x][y][z]*temp_tensors[0][u][x][y][z]
            +inverse_metric[5][x][y][z]*temp_tensors[1][u][x][y][z]
            +inverse_metric[3][x][y][z]*temp_tensors[2][u][x][y][z]);


            temp_tensors_2[1][u][x][y][z]= (1.0/determinant_g[x][y][z])*(inverse_metric[5][x][y][z]*temp_tensors[0][u][x][y][z]
            +inverse_metric[1][x][y][z]*temp_tensors[1][u][x][y][z]
            +inverse_metric[4][x][y][z]*temp_tensors[2][u][x][y][z]);


            temp_tensors_2[2][u][x][y][z]= (1.0/determinant_g[x][y][z])*(inverse_metric[3][x][y][z]*temp_tensors[0][u][x][y][z]
            +inverse_metric[4][x][y][z]*temp_tensors[1][u][x][y][z]
            +inverse_metric[2][x][y][z]*temp_tensors[2][u][x][y][z]);


          }
           
    
      
        }

      }


    }


   for (int j=0;j<3;j++)
   {
    for ( int i=0;i<6;i++)
    {
      for (int x=0;x<size[0];x++)
      {
        for (int y=0;y<size[1];y++)
        {
          for (int z=0;z<size[2];z++)
          {
              
                if (j==0)
                {
                  if(x==size[0]-1)
                  {
                    temp_tensors_3[j][i][x][y][z]=((D[i][x][y][z]-D[i][x][y][z])/vox_dim[0])*mask_extended[3][x][y][z]*determinant[x][y][z];
                    
                   }
                   else
                   {
                   temp_tensors_3[j][i][x][y][z]=((D[i][x+1][y][z]-D[i][x][y][z])/vox_dim[0])*mask_extended[3][x][y][z]*determinant[x][y][z];
                    
                   }
                  }
                if (j==1)
                {
                  if(y==size[1]-1)
                  {
                    temp_tensors_3[j][i][x][y][z]=((D[i][x][y][z]-D[i][x][y][z])/vox_dim[1])*mask_extended[4][x][y][z]*determinant[x][y][z];
                    
                    }
                    else
                    {
                    temp_tensors_3[j][i][x][y][z]=((D[i][x][y+1][z]-D[i][x][y][z])/vox_dim[1])*mask_extended[4][x][y][z]*determinant[x][y][z];
                    
                    }
                 }
                if (j==2)
                {
                if(z==size[3]-1)
                  {
                    temp_tensors_3[j][i][x][y][z]=((D[i][x][y][z]-D[i][x][y][z])/vox_dim[2])*mask_extended[5][x][y][z]*determinant[x][y][z];
                    
                    }
                    else
                    {
                    temp_tensors_3[j][i][x][y][z]=((D[i][x][y][z+1]-D[i][x][y][z])/vox_dim[2])*mask_extended[5][x][y][z]*determinant[x][y][z];
                    
                    }
                }

            }


          }
      
        }

      }


    }// end of temp_tensors loop








        for (int x=0;x<size[0];x++)
        {
          for (int y=0;y<size[1];y++)
          {
           for (int z=0;z<size[2];z++)
            {
              for(int u=0;u<6;u++)
              {

              regulated_tensors[u][x][y][z]= dt*(temp_tensors_3[0][u][x][y][z]+temp_tensors_3[1][u][x][y][z]+temp_tensors_3[2][u][x][y][z]);
            
              }

           }


          }


        }





    
    

    // declaratuon of temporary stuff


    }// end of beltrami


  template <class TDiffusionPixelType, class TTensorPixelType>
  void 
    FreeWaterEliminationFilter<TDiffusionPixelType, TTensorPixelType>
    ::remove_negative_eigs(int x, int y, int z, float ****table, 
    int nof, itk::Size<3> size, 
    vnl_matrix<float> pseudoInverse, float ****D, float ****atten, 
    vnl_vector<float> higher_thresh, 
    vnl_vector<float> lower_thresh, int& wasdetectedbad, 
    float ***b0_mean_image, float ****sorted_eigs, int numberb0, int break_flag,float NOBRAIN, float ***mask_all)
  {//0


    /* for (int g=0;g<6;g++)
    {
    for ( int x=0;x<size[0];x++)
    {
    for ( int y=0;y<size[1];y++)
    {
    for ( int z=0;z<size[2];z++)
    { 
    std::cout <<  table [g][x][y][z] << std::endl;
    std::cin.get();

    }
    }
    }
    }*/




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

    vnl_matrix<float> temporary(nof-1,3);

    vnl_matrix<float> temp_tensor(3,3);

    vnl_vector<float> atten_log(nof-numberb0);
    vnl_vector<float> temp_atten(nof-numberb0);
    vnl_vector<float> temp_D(6);
    vnl_vector<float> eigen_vals(3);

    break_flag=0;

    for (int u=0;u<nof-numberb0;u++)
    {  
      
      if(table[u][x][y][z]<50.0 || b0_mean_image[x][y][z]<0.01){break_flag=1;mask_all[x][y][z]=0.0;}
      else{break_flag=0;
      float temp_table = table[u][x][y][z];
      float temp_b0 = b0_mean_image[x][y][z];
      atten[u][x][y][z]=temp_table/temp_b0;
      if(atten[u][x][y][z]==1.0){atten[u][x][y][z]=1.1;}
      float temp_f =atten[u][x][y][z];
      atten_log[u]=log(temp_f);
      temp_atten[u]=atten_log[u];
      }
    }

    if(break_flag==0)
    {

      temp_D = pseudoInverse*temp_atten;




      for (int u=0;u<6;u++){D[u][x][y][z]=temp_D[u];}

      /*double M11= temp_D[0];
      double M12= temp_D[3];
      double M13= temp_D[5];
      double M22= temp_D[1];
      double M23= temp_D[4];
      double M33= temp_D[2];*/


      float l1;
      float l2;
      float l3;

      temp_tensor[0][0]= temp_D[0];
      temp_tensor[1][0]= temp_D[3];
      temp_tensor[2][0]= temp_D[5];
      temp_tensor[0][1]= temp_D[3];
      temp_tensor[1][1]= temp_D[1];
      temp_tensor[2][1]= temp_D[4];
      temp_tensor[0][2]= temp_D[5];
      temp_tensor[1][2]= temp_D[4];
      temp_tensor[2][2]= temp_D[2];


      /*for (int i=0; i<3;i++)

      {
      for(int j=0;j<3;j++)
      {
      std::cout<<temp_tensor[i][j]<<std::endl;
      }
      }*/


      vnl_symmetric_eigensystem <float> eigen_tensor(temp_tensor);


      eigen_vals[0]=eigen_tensor.get_eigenvalue(0);
      eigen_vals[1]=eigen_tensor.get_eigenvalue(1);
      eigen_vals[2]=eigen_tensor.get_eigenvalue(2);

      /*for (int i=0;i<3;i++)
      {


      eigen_vals[i]=eigen_tensor.get_eigenvalue(i);

      }*/

      float temp1;
      float temp2;

      if (eigen_vals[0]>eigen_vals[1])

      {
        temp1=eigen_vals[0];
        temp2=eigen_vals[1];
        eigen_vals[0]=temp2;
        eigen_vals[1]=temp1;

      }

       if (eigen_vals[1]>eigen_vals[2])

      {
        temp1=eigen_vals[1];
        temp2=eigen_vals[2];
        eigen_vals[1]=temp2;
        eigen_vals[2]=temp1;

      }

      if (eigen_vals[0]>eigen_vals[1])

      {
        temp1=eigen_vals[0];
        temp2=eigen_vals[1];
        eigen_vals[0]=temp2;
        eigen_vals[1]=temp1;

      }

    



      l1=eigen_vals[0];
      l2=eigen_vals[1];
      l3=eigen_vals[2];



      



      //std::cout <<l1<<l2<<l3 << std::endl;
      //std::cin.get();

      if( l1>0.01 && l2>0.01 && l3>0.01)
      {
      flag=0; 
      mask_all[x][y][z]=1.0;
      sorted_eigs[0][x][y][z]=l1;
      sorted_eigs[1][x][y][z]=l2;
      sorted_eigs[2][x][y][z]=l3;
      }
      else{flag=1;}

      sorted_eigs[0][x][y][z]=l1;
      sorted_eigs[1][x][y][z]=l2;
      sorted_eigs[2][x][y][z]=l3;

      if(flag==1)// 1
      {
        wasdetectedbad=wasdetectedbad++;

        for (int u=0;u<nof-numberb0;u++)//2  
        {
          if(table[u][x][y][z]<lower_thresh[u] || table[u][x][y][z]>higher_thresh[u])   
          {// 3
            if(x==0)
            {
              init_i=x;
              limit_i=x+2;
            }
            else if(x==size[0]-1)
            {
              init_i=x-1;
              limit_i=x+1;
            }
            else
            {
              init_i=x-1;
              limit_i=x+2;
            }

            for (int i=init_i;i<limit_i;i+=2)//4
            {
              if(y==0)
              {
                init_j=y;
                limit_j=y+2;
              }
              else if(y==size[1]-1)
              {
                init_j=y-1;
                limit_j=y+1;
              }
              else
              {
                init_j=y-1;
                limit_j=y+2;
              }
              for (int j=init_j;j<limit_j;j+=2)//5
              {
                if(z==0)
                {
                  init_c=z;
                  limit_c=z+2;
                }
                else if(z==size[2]-1)
                {
                  init_c=z-1;
                  limit_c=z+1;
                }
                else
                {
                  init_c=z-1;
                  limit_c=z+2;
                }
                for (int c=init_c;c<limit_c;c+=2)
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

                if (temp_number==0){table[u][x][y][z]=NOBRAIN;}
                else{ table[u][x][y][z]=temp_sum/temp_number;}     

              } //end if//4

            } // end of for//3

          } // end of if//2
        }//1

      }else 
      {
        for (int u=0;u<6;u++){D[u][x][y][z]=0.0;}
        for (int u=0;u<6;u++){atten[u][x][y][z]=-100.0;}
        mask_all[x][y][z]=0.0;
        sorted_eigs[0][x][y][z]=0.0;
        sorted_eigs[1][x][y][z]=0.0;
        sorted_eigs[2][x][y][z]=0.0;
        //for (int u=0;u<nof-numberb0;u++){atten[u][x][y][z]=0.0;}

      }

    }// end of void//0



  } // end of namespace

}
//}





