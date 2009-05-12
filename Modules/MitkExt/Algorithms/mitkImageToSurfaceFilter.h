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
  * @brief Converts pixel data to surface data by using a threshold
  * The mitkImageToSurfaceFilter is used to create a new surface out of an mitk image. The filter
  * uses a threshold to define the surface. It is based on the vtkMarchingCube algorithm. By default
  * a vtkPolyData surface based on an input threshold for the input image will be created. Optional
  * it is possible to reduce the number of triangles/polygones [SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro) and SetTargetReduction (float _arg)]
  * or smooth the surface-data [SetSmooth(true), SetSmoothIteration(int smoothIteration) and SetSmoothRelaxation(float smoothRelaxation)].
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

      /*
    * To decide whether a reduction of polygons in the created surface shall be
    * done or not by using the vtkDecimatePro Filter. Till vtk 4.x an vtkDecimateFilter existed,
    * but was patented. So since vtk 5.x it was replaced by the (worser?) vtkDecimateProFilter
    * Maybe another Filter will come soon.
    */
      enum DecimationType {NoDecimation,DecimatePro};

      mitkClassMacro(ImageToSurfaceFilter, SurfaceSource);
      itkNewMacro(Self);

      /**
       * For each image time slice a surface will be created. This method is
       * called by Update().
       */

      virtual void GenerateData();

    /**
    * Initializes the output information ( i.e. the geometry information ) of
    * the output of the filter
    */
      virtual void GenerateOutputInformation();

      /**
       * Returns a const reference to the input image (e.g. the original input image that ist used to create the surface)
       */
      const mitk::Image *GetInput(void);

      /**
       * Set the source image to create a surface for this filter class. As input every mitk
       * 3D or 3D+t image can be used.
       */
      virtual void SetInput(const mitk::Image *image);

      /**
       * Set the number of iterations that is used to smooth the surface. Used is the vtkSmoothPolydataFilter that uses the laplacian filter. The higher the number of iterations that stronger the smooth-result
       *
       * @param smoothIteration As smoothIteration default in that case 50 was choosen. The VTK documentation recommends small relaxation factors and large numbers of iterations.
       */
      void SetSmoothIteration(int smoothIteration);

      /**
       * Set number of relaxation. Specify the relaxation factor for Laplacian
       * smoothing. The VTK documentation recommends small relaxation factors
       * and large numbers of iterations.
       *
       * @param smoothRelaxation As smoothRelaxation default in that case 0.1 was choosen. The VTK documentation recommends small relaxation factors and large numbers of iterations.
       */
      void SetSmoothRelaxation(float smoothRelaxation);


      /**
    * Threshold that is used to create the surface. All pixel in the input image that are higher than that
    * value will be considered in the surface. The threshold referees to
    * vtkMarchingCube. Default value is 1. See also SetThreshold (ScalarType _arg)
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
       * surface represantation. Modes can only be NoDecimation or DecimatePro
     * (till vtk 4.x also Decimate)
       * */
      itkGetConstMacro(Decimate,DecimationType);

      /**
       * Enable the decimation filter to reduce the number of triangles in the
       * mesh and produce a good approximation to the original image. The filter
       * has support for vtk-5 and earlier versions. More detailed information
       * check the vtkDecimatePro and vtkDecimate.
       * */
      itkSetMacro(Decimate,DecimationType);

      /**
       * Set desired TargetReduction of triangles in the range from 0.0 to 1.0.
       * The destroyed triangles are in relation with the size of data. For example 0.9
       * will reduce the data set to 10%.
       *
       * @param Set a TargetReduction float-value from 0.0 to 1.0
       * */
      itkSetMacro(TargetReduction, float);

      /**
       * Returns the reduction factor for the VtkDecimatePro Decimation Filter as a float value
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

    /**
    * Destructor
    * */
      virtual ~ImageToSurfaceFilter();

      /**
       * With the given threshold vtkMarchingCube creates the surface. By default nothing a
       * vtkPolyData surface based on a threshold of the input image will be created. Optional
     * it is possible to reduce the number of triangles/polygones [SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro) and SetTargetReduction (float _arg)]
     * or smooth the data [SetSmooth(true), SetSmoothIteration(int smoothIteration) and SetSmoothRelaxation(float smoothRelaxation)].
       *
       * @param time selected slice or "0" for single
       * @param *vtkimage input image
       * @param *surface output
       * @param threshold can be different from SetThreshold()
       */
      void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold);

    /**
    * Flag whether the created surface shall be smoothed or not (default is "false"). SetSmooth (bool _arg)
    * */
      bool m_Smooth;

    /**
    * Decimation mode, default mode is "NoDecimation". See also SetDecimate (DecimationType _arg)
    * */
      DecimationType m_Decimate;

    /**
    * Threshold that is used to create the surface. All pixel in the input image that are higher than that
    * value will be considered in the surface. Default value is 1. See also SetThreshold (ScalarType _arg)
    * */
      ScalarType m_Threshold;

    /**
    * The Reduction factor of the Decimation Filter for the created surface. See also SetTargetReduction (float _arg)
    * */
      float m_TargetReduction;

    /**
    * The Iteration value for the Smooth Filter of the created surface. See also SetSmoothIteration (int smoothIteration)
    * */
      int m_SmoothIteration;

    /**
    * The Relaxation value for the Smooth Filter of the created surface. See also SetSmoothRelaxation (float smoothRelaxation)
    * */
      float m_SmoothRelaxation;

  };

} // namespace mitk

#endif //_MITKIMAGETOSURFACEFILTER_h__


