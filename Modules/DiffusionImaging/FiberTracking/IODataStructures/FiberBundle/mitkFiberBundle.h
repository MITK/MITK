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


#ifndef _MITK_FiberBundle_H
#define _MITK_FiberBundle_H

//includes for MITK datastructure
#include <mitkBaseData.h>
#include <MitkFiberTrackingExports.h>
#include <mitkImage.h>
#include <mitkDataStorage.h>
#include <mitkPlanarFigure.h>
#include <mitkPixelTypeTraits.h>
#include <mitkPlanarFigureComposite.h>


//includes storing fiberdata
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkDataSet.h>
#include <vtkTransform.h>
#include <vtkFloatArray.h>


namespace mitk {

/**
   * \brief Base Class for Fiber Bundles;   */
class MITKFIBERTRACKING_EXPORT FiberBundle : public BaseData
{
public:

    typedef itk::Image<unsigned char, 3> ItkUcharImgType;

    // fiber colorcodings
    static const char* FIBER_ID_ARRAY;

    virtual void UpdateOutputInformation() override;
    virtual void SetRequestedRegionToLargestPossibleRegion() override;
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    virtual bool VerifyRequestedRegion() override;
    virtual void SetRequestedRegion(const itk::DataObject*) override;

    mitkClassMacro( FiberBundle, BaseData )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro1Param(Self, vtkSmartPointer<vtkPolyData>) // custom constructor

    // colorcoding related methods
    void ColorFibersByCurvature();
    void ColorFibersByScalarMap(mitk::Image::Pointer, bool opacity);
    template <typename TPixel>
    void ColorFibersByScalarMap(const mitk::PixelType pixelType, mitk::Image::Pointer, bool opacity);
    void ColorFibersByOrientation();
    void SetFiberOpacity(vtkDoubleArray *FAValArray);
    void ResetFiberOpacity();
    void SetFiberColors(vtkSmartPointer<vtkUnsignedCharArray> fiberColors);
    void SetFiberColors(float r, float g, float b, float alpha=255);
    vtkSmartPointer<vtkUnsignedCharArray> GetFiberColors() const { return m_FiberColors; }

    // fiber compression
    void Compress(float error = 0.0);

    // fiber resampling
    void ResampleSpline(float pointDistance=1);
    void ResampleSpline(float pointDistance, double tension, double continuity, double bias );

    bool RemoveShortFibers(float lengthInMM);
    bool RemoveLongFibers(float lengthInMM);
    bool ApplyCurvatureThreshold(float minRadius, bool deleteFibers);
    void MirrorFibers(unsigned int axis);
    void RotateAroundAxis(double x, double y, double z);
    void TranslateFibers(double x, double y, double z);
    void ScaleFibers(double x, double y, double z, bool subtractCenter=true);
    void TransformFibers(double rx, double ry, double rz, double tx, double ty, double tz);
    void RemoveDir(vnl_vector_fixed<double,3> dir, double threshold);
    itk::Point<float, 3> TransformPoint(vnl_vector_fixed< double, 3 > point, double rx, double ry, double rz, double tx, double ty, double tz);
    itk::Matrix< double, 3, 3 > TransformMatrix(itk::Matrix< double, 3, 3 > m, double rx, double ry, double rz);

    // add/subtract fibers
    FiberBundle::Pointer AddBundle(FiberBundle* fib);
    FiberBundle::Pointer SubtractBundle(FiberBundle* fib);

    // fiber subset extraction
    FiberBundle::Pointer           ExtractFiberSubset(DataNode *roi, DataStorage* storage);
    std::vector<long>              ExtractFiberIdSubset(DataNode* roi, DataStorage* storage);
    FiberBundle::Pointer           ExtractFiberSubset(ItkUcharImgType* mask, bool anyPoint, bool invert=false);
    FiberBundle::Pointer           RemoveFibersOutside(ItkUcharImgType* mask, bool invert=false);

    vtkSmartPointer<vtkPolyData>    GeneratePolyDataByIds( std::vector<long> ); // TODO: make protected
    void                            GenerateFiberIds(); // TODO: make protected

    // get/set data
    vtkSmartPointer<vtkFloatArray> GetFiberWeights() const { return m_FiberWeights; }
    float GetFiberWeight(unsigned int fiber);
    void SetFiberWeights(float newWeight);
    void SetFiberWeight(unsigned int fiber, float weight);
    void SetFiberWeights(vtkSmartPointer<vtkFloatArray> weights);
    void SetFiberPolyData(vtkSmartPointer<vtkPolyData>, bool updateGeometry = true);
    vtkSmartPointer<vtkPolyData> GetFiberPolyData() const;
    itkGetMacro( NumFibers, int)
    //itkGetMacro( FiberSampling, int)
    int GetNumFibers() const {return m_NumFibers;}
    itkGetMacro( MinFiberLength, float )
    itkGetMacro( MaxFiberLength, float )
    itkGetMacro( MeanFiberLength, float )
    itkGetMacro( MedianFiberLength, float )
    itkGetMacro( LengthStDev, float )
    itkGetMacro( UpdateTime2D, itk::TimeStamp )
    itkGetMacro( UpdateTime3D, itk::TimeStamp )
    void RequestUpdate2D(){ m_UpdateTime2D.Modified(); }
    void RequestUpdate3D(){ m_UpdateTime3D.Modified(); }
    void RequestUpdate(){ m_UpdateTime2D.Modified(); m_UpdateTime3D.Modified(); }

    unsigned long GetNumberOfPoints();

    // copy fiber bundle
    mitk::FiberBundle::Pointer GetDeepCopy();

    // compare fiber bundles
    bool Equals(FiberBundle* fib, double eps=0.0001);

    itkSetMacro( ReferenceGeometry, mitk::BaseGeometry::Pointer )
    itkGetConstMacro( ReferenceGeometry, mitk::BaseGeometry::Pointer )

protected:

    FiberBundle( vtkPolyData* fiberPolyData = nullptr );
    virtual ~FiberBundle();

    itk::Point<float, 3> GetItkPoint(double point[3]);

    // calculate geometry from fiber extent
    void UpdateFiberGeometry();

private:

    // actual fiber container
    vtkSmartPointer<vtkPolyData>  m_FiberPolyData;

    // contains fiber ids
    vtkSmartPointer<vtkDataSet>   m_FiberIdDataSet;

    int   m_NumFibers;

    vtkSmartPointer<vtkUnsignedCharArray> m_FiberColors;
    vtkSmartPointer<vtkFloatArray> m_FiberWeights;
    std::vector< float > m_FiberLengths;
    float   m_MinFiberLength;
    float   m_MaxFiberLength;
    float   m_MeanFiberLength;
    float   m_MedianFiberLength;
    float   m_LengthStDev;
    int     m_FiberSampling;
    itk::TimeStamp m_UpdateTime2D;
    itk::TimeStamp m_UpdateTime3D;
    mitk::BaseGeometry::Pointer m_ReferenceGeometry;
};

} // namespace mitk

#endif /*  _MITK_FiberBundle_H */
