/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageLiveWireContourModelFilter_h
#define mitkImageLiveWireContourModelFilter_h

#include <mitkCommon.h>
#include <mitkContourModel.h>
#include <mitkContourModelSource.h>
#include <MitkSegmentationExports.h>

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <itkShortestPathCostFunctionLiveWire.h>
#include <itkShortestPathImageFilter.h>

namespace mitk
{
  /**

   \brief Calculates a LiveWire contour between two points in an image.

   For defining costs between two pixels specific features are extracted from the image and transformed into a single cost
   value.
   \sa ShortestPathCostFunctionLiveWire

   The filter is able to create dynamic cost transfer map and thus use on the fly training.
   \note On the fly training will only be used for next update.
   The computation uses the last calculated segment to map cost according to features in the area of the segment.

   Caution: time support currently not available. Filter will always work on the first
   timestep in its current implementation.

   \ingroup ContourModelFilters
   \ingroup Process
  */
  class MITKSEGMENTATION_EXPORT ImageLiveWireContourModelFilter : public ContourModelSource
  {
  public:
    mitkClassMacro(ImageLiveWireContourModelFilter, ContourModelSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef ContourModel OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::Image InputType;

    typedef itk::Image<float, 2> InternalImageType;
    typedef itk::ShortestPathImageFilter<InternalImageType, InternalImageType> ShortestPathImageFilterType;
    typedef itk::ShortestPathCostFunctionLiveWire<InternalImageType> CostFunctionType;
    typedef std::vector<itk::Index<2>> ShortestPathType;

    /** \brief Start point in world coordinates. */
    itkSetMacro(StartPoint, mitk::Point3D);
    itkGetMacro(StartPoint, mitk::Point3D);

    /** \brief End point in world coordinates. */
    itkSetMacro(EndPoint, mitk::Point3D);
    itkGetMacro(EndPoint, mitk::Point3D);

    /** \brief Create dynamic cost transfer map - use on the fly training.
    \note On the fly training will be used for next update only.
    The computation uses the last calculated segment to map cost according to features in the area of the segment.
    */
    itkSetMacro(UseDynamicCostMap, bool);
    itkGetMacro(UseDynamicCostMap, bool);

    /** \brief Clear all repulsive points used in the cost function
    */
    void ClearRepulsivePoints();

    /** \brief Set a vector with repulsive points to use in the cost function
    */
    void SetRepulsivePoints(const ShortestPathType &points);

    /** \brief Add a single repulsive point to the cost function
    */
    void AddRepulsivePoint(const itk::Index<2> &idx);

    /** \brief Remove a single repulsive point from the cost function
    */
    void RemoveRepulsivePoint(const itk::Index<2> &idx);

    /** \brief Set the input image for the LiveWire computation.
      \param input The image to compute the LiveWire contour on.
    */
    virtual void SetInput(const InputType *input);

    using Superclass::SetInput;

    /** \brief Set the input image at a specific index.
      \param idx The input index.
      \param input The image to set.
    */
    virtual void SetInput(unsigned int idx, const InputType *input);

    /** \brief Get the first input image. */
    const InputType *GetInput(void);

    /** \brief Get the input image at a specific index. */
    const InputType *GetInput(unsigned int idx);

    /** \brief Get the output contour model. */
    virtual OutputType *GetOutput();

    /** \brief Debug helper that dumps the internal mask image. */
    virtual void DumpMaskImage();

    /** \brief Create a dynamic cost transfer map using on-the-fly training.
      \param path Optional contour path to use for training. If nullptr, the last computed segment is used.
      \return True if the cost map was created successfully.
    */
    bool CreateDynamicCostMap(mitk::ContourModel *path = nullptr);

    /** \brief Enable or disable the cost function in the shortest path filter.
      \param doUseCostFunction If true, the cost function is used; otherwise uniform cost is assumed.
    */
    void SetUseCostFunction(bool doUseCostFunction) { m_ShortestPathFilter->SetUseCostFunction(doUseCostFunction); };

  protected:
    ImageLiveWireContourModelFilter();

    ~ImageLiveWireContourModelFilter() override;

    void GenerateOutputInformation() override{};

    void GenerateData() override;

    void UpdateLiveWire();

    /** \brief Start point in world coordinates. */
    mitk::Point3D m_StartPoint;

    /** \brief End point in world coordinates. */
    mitk::Point3D m_EndPoint;

    /** \brief Start point in image index coordinates. */
    mitk::Point3D m_StartPointInIndex;

    /** \brief End point in image index coordinates. */
    mitk::Point3D m_EndPointInIndex;

    /** \brief The cost function to compute costs between two pixels*/
    CostFunctionType::Pointer m_CostFunction;

    /** \brief Shortest path filter according to cost function m_CostFunction*/
    ShortestPathImageFilterType::Pointer m_ShortestPathFilter;

    /** \brief Flag to use a dynamic cost map or not*/
    bool m_UseDynamicCostMap;

    unsigned int m_TimeStep;

    template <typename TPixel, unsigned int VImageDimension>
    void ItkPreProcessImage(const itk::Image<TPixel, VImageDimension> *inputImage);

    template <typename TPixel, unsigned int VImageDimension>
    void CreateDynamicCostMapByITK(const itk::Image<TPixel, VImageDimension> *inputImage,
                                   mitk::ContourModel *path = nullptr);

    InternalImageType::Pointer m_InternalImage;
  };
}

#endif
