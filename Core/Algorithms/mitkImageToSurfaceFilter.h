/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _MITKIMAGETOSURFACEFILTER_h__
#define _MITKIMAGETOSURFACEFILTER_h__

#include <vtkPolyData.h>
#include <mitkCommon.h>
#include <mitkSurfaceSource.h>
#include <mitkSurface.h>

#include <mitkImage.h>
#include <vtkImageData.h>

#include <vtkSmoothPolyDataFilter.h>
#include <vtkMarchingCubes.h>


namespace mitk {
  /**
   * @brief Converts pixel data to surface data
   *
   * The resulting vtk-surface has the same size as the input image. The surface 
   * can be generally smoothed by vtkDecimatePro reduce complexity of triangles 
   * and vtkSmoothPolyDataFilter to relax the mesh. Both are enabled by default 
   * and connected in the common way of pipelining in ITK. It's also possible
   * to create time sliced surfaces.
   *
   * @ingroup ImageFilters
   * @ingroup Process
   */

  class MITK_CORE_EXPORT ImageToSurfaceFilter : public SurfaceSource
  {
    public:

      enum DecimationType {NoDecimation,Decimate,DecimatePro};

      mitkClassMacro(ImageToSurfaceFilter, SurfaceSource);
      itkNewMacro(Self);

      /**
       * For each image time slice a surface will be created. This method is 
       * called by Update().
       */
      virtual void GenerateData();
      virtual void GenerateOutputInformation();

      /**
       * Returns a const reference to the input image (original)
       */
      const mitk::Image *GetInput(void);

      /**
       * Set the source image for this filter class. input can be everey mitk 
       * 3D or 3D+t image.
       */
      virtual void SetInput(const mitk::Image *image);

      /**
       * Set number of iterations of the Laplacian filter. not 
       *
       * @param smoothIteration default 50
       */
      void SetSmoothIteration(int smoothIteration);

      /**
       * Set number of relaxation. Specify the relaxation factor for Laplacian 
       * smoothing. The VTK documentation recommends small relaxation factors 
       * and large numbers of iterations.
       *
       * @param smoothRelaxation default 0.1
       */
      void SetSmoothRelaxation(float smoothRelaxation);


      /**
       * Set desired threshold for input image. The threshold referees to 
       * vtkMarchingCube. For binary images use integer values use 1.0 to 
       * separate fore- and background.
       */
      itkSetMacro(Threshold, ScalarType);

      /**
       * Get Threshold from vtkMarchingCube. Threshold can be manipulated by 
       * inherited classes.
       */
      itkGetConstMacro(Threshold, ScalarType);

      /**
       * Enables vtkSmoothPolyDataFilter. With Laplacian smoothing this filter 
       * will relax the surface. You can control the Filter by manipulating the 
       * number of iterations and the relaxing factor.      
       * */
      itkSetMacro(Smooth,bool);
      
      /*
       * Enable/Disable surface smoothing.
       */
      itkBooleanMacro(Smooth);
      
      /*
       * Returns if surface smoothing is enabled
       */
      itkGetConstMacro(Smooth,bool);

      /** 
       * Get the state of decimation mode to reduce triangle in the
       * surface represantation
       * */
      itkGetConstMacro(Decimate,DecimationType);
      
      /**
       * Enable the decimation filter to reduce the number of triangles in the 
       * mesh and produce a good approximation to the original image. The filter
       * has support for vtk-5 and earlier versions. Mor detailed information 
       * check the vtkDecimatePro and vtkDecimate.      
       * */
      itkSetMacro(Decimate,DecimationType);

      /**
       * Set desired TargetReduction of triangles in the range from 0.0 to 1.0. 
       * The destroyed triangles is in relation with the size of data. For example 0.9 
       * will reduce the data set to 10%.
       *
       * @param TargetReduction float from 0.0 to 1.0
       * */
      itkSetMacro(TargetReduction, float);

      /**
       * Return reduction factor for VtkDecimatePro
       */
      itkGetConstMacro(TargetReduction, float);

      /** 
       * Transforms a point by a 4x4 matrix
       */
      template <class T1, class T2, class T3>
      inline void mitkVtkLinearTransformPoint(T1 matrix[4][4], T2 in[3], T3 out[3])
      {
        T3 x = matrix[0][0]*in[0]+matrix[0][1]*in[1]+matrix[0][2]*in[2]+matrix[0][3];
        T3 y = matrix[1][0]*in[0]+matrix[1][1]*in[1]+matrix[1][2]*in[2]+matrix[1][3];
        T3 z = matrix[2][0]*in[0]+matrix[2][1]*in[1]+matrix[2][2]*in[2]+matrix[2][3];
        out[0] = x;
        out[1] = y;
        out[2] = z;
      }

    protected:
      ImageToSurfaceFilter();
      virtual ~ImageToSurfaceFilter();

      /**
       * With the given threshold vtkMarchingCube creates the surface. By default nothing 
       * will be done. Optional its possible to reduce the number of triangles (vtkDecimat).
       * or smooth the data (vtkSmoothPolyDataFilter).
       * 
       * @param time selected slice or "0" for single
       * @param *vtkimage input image
       * @param *surface output
       * @param threshold can be different from SetThreshold()
       */
      void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold);

      bool m_Smooth;
      DecimationType m_Decimate;
      ScalarType m_Threshold; 
      float m_TargetReduction;
      int m_SmoothIteration;
      float m_SmoothRelaxation;

  };

} // namespace mitk

#endif //_MITKIMAGETOSURFACEFILTER_h__
