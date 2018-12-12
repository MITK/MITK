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

#ifndef __itkShortestPathCostFunctionLiveWire_h
#define __itkShortestPathCostFunctionLiveWire_h

#include "itkShortestPathCostFunction.h"

#include "itkImageRegionConstIterator.h"

namespace itk
{
  /** \brief Cost function for LiveWire purposes.
  Specific features are considered to calculate cummulative
  costs of a link between two pixels. These are:

  - Gradient Magnitude
  - Gradient Direction
  - Laplacian Zero Crossing

  By default the Gradient Magnitude is mapped linearly to cost values
  between 0 (good) and 1 (bad). Via SetDynamicCostMap( std::map< int, int > &costMap)
  a cost map can be set to dynamically map Gradient Magnitude (non
  linear). Thus, lower values can be considered with lower costs
  than higher values of gradient magnitudes.
  To compute  the costs of the gradient magnitude dynamically
  an iverted map of the histogram of gradient magnitude image is used.

  */
  template <class TInputImageType>
  class ITK_EXPORT ShortestPathCostFunctionLiveWire : public ShortestPathCostFunction<TInputImageType>
  {
  public:
    /** Standard class typedefs. */
    typedef ShortestPathCostFunctionLiveWire Self;
    typedef ShortestPathCostFunction<TInputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;
    typedef itk::ImageRegionConstIterator<TInputImageType> ConstIteratorType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      /** Run-time type information (and related methods). */
      itkTypeMacro(ShortestPathCostFunctionLiveWire, ShortestPathCostFunction);

    typedef itk::Image<unsigned char, 2> UnsignedCharImageType;
    typedef itk::Image<float, 2> FloatImageType;

    typedef float ComponentType;
    typedef itk::CovariantVector<ComponentType, 2> OutputPixelType;
    typedef itk::Image<OutputPixelType, 2> VectorOutputImageType;

    typedef typename TInputImageType::IndexType IndexType;
    typedef TInputImageType ImageType;
    typedef itk::ImageRegion<2> RegionType;

    /** \brief calculates the costs for going from p1 to p2*/
    double GetCost(IndexType p1, IndexType p2) override;

    /** \brief returns the minimal costs possible (needed for A*)*/
    double GetMinCost() override;

    /** \brief Initialize the metric*/
    void Initialize() override;

    /** \brief Add void pixel in cost map*/
    virtual void AddRepulsivePoint(const IndexType &index);

    /** \brief Remove void pixel in cost map*/
    virtual void RemoveRepulsivePoint(const IndexType &index);

    /** \brief Clear repulsive points in cost function*/
    virtual void ClearRepulsivePoints();

    itkSetMacro(RequestedRegion, RegionType);
    itkGetMacro(RequestedRegion, RegionType);

    void SetImage(const TInputImageType *_arg) override;

    void SetDynamicCostMap(std::map<int, int> &costMap)
    {
      this->m_CostMap = costMap;
      this->m_UseCostMap = true;
      this->m_MaxMapCosts = -1;
      this->Modified();
    }

    void SetUseCostMap(bool useCostMap) { this->m_UseCostMap = useCostMap; }
    /**
     \brief Set the maximum of the dynamic cost map to save computation time.
    */
    void SetCostMapMaximum(double max) { this->m_MaxMapCosts = max; }
    enum Constants
    {
      MAPSCALEFACTOR = 10
    };

    /** \brief Returns the y value of gaussian with given offset and amplitude

       gaussian approximation
       f(x) = v(bin) * e^ ( -1/2 * (|x-k(bin)| / sigma)^2 )

       \param x
       \param xOfGaussian - offset
       \param yOfGaussian - amplitude
    */
    static double Gaussian(double x, double xOfGaussian, double yOfGaussian);

    const UnsignedCharImageType *GetMaskImage() { return this->m_MaskImage.GetPointer(); };
    const FloatImageType *GetGradientMagnitudeImage() { return this->m_GradientMagnitudeImage.GetPointer(); };
    const FloatImageType *GetEdgeImage() { return this->m_EdgeImage.GetPointer(); };
    const VectorOutputImageType *GetGradientImage() { return this->m_GradientImage.GetPointer(); };
  protected:
    ShortestPathCostFunctionLiveWire();

    ~ShortestPathCostFunctionLiveWire() override{};

    FloatImageType::Pointer m_GradientMagnitudeImage;
    FloatImageType::Pointer m_EdgeImage;
    UnsignedCharImageType::Pointer m_MaskImage;
    VectorOutputImageType::Pointer m_GradientImage;

    double m_MinCosts;

    bool m_UseRepulsivePoints;

    typename Superclass::PixelType val;

    typename Superclass::PixelType startValue;
    typename Superclass::PixelType endValue;

    double m_GradientMax;

    RegionType m_RequestedRegion;

    bool m_Initialized;

    std::map<int, int> m_CostMap;

    bool m_UseCostMap;

    double m_MaxMapCosts;

  private:
    double SigmoidFunction(double I, double max, double min, double alpha, double beta);
  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkShortestPathCostFunctionLiveWire.txx"
#endif

#endif /* __itkShortestPathCostFunctionLiveWire_h */
