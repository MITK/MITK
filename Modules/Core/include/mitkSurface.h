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

#ifndef mitkSurface_h
#define mitkSurface_h

#include "itkImageRegion.h"
#include "mitkBaseData.h"
#include <vtkSmartPointer.h>

class vtkPolyData;

namespace mitk
{
  /**
    * \brief Class for storing surfaces (vtkPolyData).
    * \ingroup Data
    */
  class MITKCORE_EXPORT Surface : public BaseData
  {
  public:
    typedef itk::ImageRegion<5> RegionType;

    mitkClassMacro(Surface, BaseData);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      void CalculateBoundingBox();
    void CopyInformation(const itk::DataObject *data) override;
    void ExecuteOperation(Operation *operation) override;
    void Expand(unsigned int timeSteps = 1) override;
    const RegionType &GetLargestPossibleRegion() const;
    virtual const RegionType &GetRequestedRegion() const;
    unsigned int GetSizeOfPolyDataSeries() const;
    virtual vtkPolyData *GetVtkPolyData(unsigned int t = 0) const;
    void Graft(const DataObject *data) override;
    bool IsEmptyTimeStep(unsigned int t) const override;
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    void SetRequestedRegion(const itk::DataObject *data) override;
    virtual void SetRequestedRegion(Surface::RegionType *region);
    void SetRequestedRegionToLargestPossibleRegion() override;
    virtual void SetVtkPolyData(vtkPolyData *polydata, unsigned int t = 0);
    virtual void Swap(Surface &other);
    void Update() override;
    void UpdateOutputInformation() override;
    bool VerifyRequestedRegion() override;

  protected:
    mitkCloneMacro(Self);

    Surface();
    ~Surface() override;

    Surface(const Surface &other);
    Surface &operator=(Surface other);

    void ClearData() override;
    void InitializeEmpty() override;

  private:
    std::vector<vtkSmartPointer<vtkPolyData>> m_PolyDatas;
    mutable RegionType m_LargestPossibleRegion;
    mutable RegionType m_RequestedRegion;
    bool m_CalculateBoundingBox;
  };

  /**
  * @brief Equal Compare two surfaces for equality, returns true if found equal.
  * @warning This method is deprecated and will not be available in the future. Use the \a bool mitk::Equal(const
  * mitk::Surface& s1, const mitk::Surface& s2) instead
  * @ingroup MITKTestingAPI
  * @param rightHandSide Surface to compare.
  * @param leftHandSide Surface to compare.
  * @param eps Epsilon to use for floating point comparison. Most of the time mitk::eps will be sufficient.
  * @param verbose Flag indicating if the method should give a detailed console output.
  * @return True if every comparison is true, false in any other case.
  */
  DEPRECATED(MITKCORE_EXPORT bool Equal(
    mitk::Surface *leftHandSide, mitk::Surface *rightHandSide, mitk::ScalarType eps, bool verbose));

  /**
  * @brief Equal Compare two surfaces for equality, returns true if found equal.
  * @ingroup MITKTestingAPI
  * @param rightHandSide Surface to compare.
  * @param leftHandSide Surface to compare.
  * @param eps Epsilon to use for floating point comparison. Most of the time mitk::eps will be sufficient.
  * @param verbose Flag indicating if the method should give a detailed console output.
  * @return True if every comparison is true, false in any other case.
  */
  MITKCORE_EXPORT bool Equal(mitk::Surface &leftHandSide,
                             mitk::Surface &rightHandSide,
                             mitk::ScalarType eps,
                             bool verbose);

  /**
  * @brief Equal Compare two vtk PolyDatas for equality, returns true if found equal.
  * @warning This method is deprecated and will not be available in the future. Use the \a bool mitk::Equal(const
  * vtkPolyData& p1, const vtkPolyData& p2) instead
  * @ingroup MITKTestingAPI
  * @param rightHandSide Surface to compare.
  * @param leftHandSide Surface to compare.
  * @param eps Epsilon to use for floating point comparison. Most of the time mitk::eps will be sufficient.
  * @param verbose Flag indicating if the method should give a detailed console output.
  * @return True if every comparison is true, false in any other case.
  *
  * This will only check if the number of cells, vertices, polygons, stripes and lines is the same and whether
  * all the two poly datas have the same number of points with the same coordinates. It is not checked whether
  * all points are correctly connected.
  */
  DEPRECATED(MITKCORE_EXPORT bool Equal(
    vtkPolyData *leftHandSide, vtkPolyData *rightHandSide, mitk::ScalarType eps, bool verbose));

  /**
  * @brief Equal Compare two vtk PolyDatas for equality, returns true if found equal.
  * @ingroup MITKTestingAPI
  * @param rightHandSide Surface to compare.
  * @param leftHandSide Surface to compare.
  * @param eps Epsilon to use for floating point comparison. Most of the time mitk::eps will be sufficient.
  * @param verbose Flag indicating if the method should give a detailed console output.
  * @return True if every comparison is true, false in any other case.
  *
  * This will only check if the number of cells, vertices, polygons, stripes and lines is the same and whether
  * all the two poly datas have the same number of points with the same coordinates. It is not checked whether
  * all points are correctly connected.
  */
  MITKCORE_EXPORT bool Equal(vtkPolyData &leftHandSide, vtkPolyData &rightHandSide, mitk::ScalarType eps, bool verbose);
}

#endif
