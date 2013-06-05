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

#ifndef _mitkImageLiveWireContourModelFilter_h__
#define _mitkImageLiveWireContourModelFilter_h__

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkContourModel.h"
#include "mitkContourModelSource.h"

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <itkShortestPathCostFunctionLiveWire.h>
#include <itkShortestPathImageFilter.h>


namespace mitk {

  /**

   \brief Calculates a LiveWire contour between two points in an image.

   For defining costs between two pixels specific features are extraced from the image and tranformed into a single cost value.
   \sa ShortestPathCostFunctionLiveWire

   The filter is able to create dynamic cost tranfer map and thus use on the fly training.
   \Note On the fly training will only be used for next update.
   The computation uses the last calculated segment to map cost according to features in the area of the segment.

   For time resolved purposes use ImageLiveWireContourModelFilter::SetTimestep( unsigned int ) to create the LiveWire contour
   at a specific timestep.

   \ingroup ContourModelFilters
   \ingroup Process
  */
  class Segmentation_EXPORT ImageLiveWireContourModelFilter : public ContourModelSource
  {

  public:

    mitkClassMacro(ImageLiveWireContourModelFilter, ContourModelSource);
    itkNewMacro(Self);


    typedef ContourModel OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::Image InputType;

    typedef itk::Image< float,  2 > FloatImageType;
    typedef itk::ShortestPathImageFilter< FloatImageType, FloatImageType > ShortestPathImageFilterType;
    typedef itk::ShortestPathCostFunctionLiveWire< FloatImageType >        CostFunctionType;


    /** \brief start point in worldcoordinates*/
    itkSetMacro(StartPoint, mitk::Point3D);
    itkGetMacro(StartPoint, mitk::Point3D);

    /** \brief end point in woorldcoordinates*/
    itkSetMacro(EndPoint, mitk::Point3D);
    itkGetMacro(EndPoint, mitk::Point3D);

    /** \brief Create dynamic cost tranfer map - use on the fly training.
    \Note On the fly training will be used for next update only.
    The computation uses the last calculated segment to map cost according to features in the area of the segment.
    */
    itkSetMacro(UseDynamicCostMap, bool);
    itkGetMacro(UseDynamicCostMap, bool);


    virtual void SetInput( const InputType *input);

    virtual void SetInput( unsigned int idx, const InputType * input);

    const InputType* GetInput(void);

    const InputType* GetInput(unsigned int idx);

    virtual OutputType* GetOutput();


    /** \brief Create dynamic cost tranfer map - on the fly training*/
    bool CreateDynamicCostMap(mitk::ContourModel* path=NULL);

    void SetTimestep( unsigned int timestep )
    {
      m_Timestep = timestep;
    }

    unsigned int GetTimestep()
    {
      return m_Timestep;
    }

  protected:
    ImageLiveWireContourModelFilter();

    virtual ~ImageLiveWireContourModelFilter();

    void GenerateOutputInformation() {};

    void GenerateData();

    /** \brief start point in worldcoordinates*/
    mitk::Point3D m_StartPoint;

    /** \brief end point in woorldcoordinates*/
    mitk::Point3D m_EndPoint;

    /** \brief Start point in index*/
    mitk::Point3D m_StartPointInIndex;

    /** \brief End point in index*/
    mitk::Point3D m_EndPointInIndex;

    /** \brief The cost function to compute costs between two pixels*/
    CostFunctionType::Pointer m_CostFunction;

    /** \brief Shortest path filter according to cost function m_CostFunction*/
    ShortestPathImageFilterType::Pointer m_ShortestPathFilter;

    /** \brief Flag to use a dynmic cost map or not*/
    bool m_UseDynamicCostMap;

    bool m_ImageModified;

    unsigned int m_Timestep;

    template<typename TPixel, unsigned int VImageDimension>
    void ItkProcessImage (itk::Image<TPixel, VImageDimension>* inputImage);

    template<typename TPixel, unsigned int VImageDimension>
    void CreateDynamicCostMapByITK(itk::Image<TPixel, VImageDimension>* inputImage, mitk::ContourModel* path=NULL);
  };

}

#endif
