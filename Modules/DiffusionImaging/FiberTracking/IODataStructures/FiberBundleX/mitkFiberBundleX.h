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


#ifndef _MITK_FiberBundleX_H
#define _MITK_FiberBundleX_H

//includes for MITK datastructure
#include <mitkBaseData.h>
#include <MitkFiberTrackingExports.h>
#include <mitkImage.h>


//includes storing fiberdata
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkDataSet.h>
#include <vtkTransform.h>

//#include <QStringList>

#include <mitkPlanarFigure.h>
#include <mitkPixelTypeTraits.h>
#include <mitkPlanarFigureComposite.h>

namespace mitk {

/**
   * \brief Base Class for Fiber Bundles;   */
class MitkFiberTracking_EXPORT FiberBundleX : public BaseData
{
public:

    typedef itk::Image<unsigned char, 3> ItkUcharImgType;

    // fiber colorcodings
    static const char* COLORCODING_ORIENTATION_BASED;
    static const char* COLORCODING_FA_BASED;
    static const char* COLORCODING_CUSTOM;
    static const char* FIBER_ID_ARRAY;

    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion(const itk::DataObject*);

    mitkClassMacro( FiberBundleX, BaseData )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro1Param(Self, vtkSmartPointer<vtkPolyData>) // custom constructor

    // colorcoding related methods
    void SetColorCoding(const char*);
    void SetFAMap(mitk::Image::Pointer);
    template <typename TPixel>
    void SetFAMap(const mitk::PixelType pixelType, mitk::Image::Pointer);
    void DoColorCodingOrientationBased();
    void DoColorCodingFaBased();
    void DoUseFaFiberOpacity();
    void ResetFiberOpacity();

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
    itk::Point<float, 3> TransformPoint(vnl_vector_fixed< double, 3 > point, double rx, double ry, double rz, double tx, double ty, double tz);
    itk::Matrix< double, 3, 3 > TransformMatrix(itk::Matrix< double, 3, 3 > m, double rx, double ry, double rz);

    // add/subtract fibers
    FiberBundleX::Pointer AddBundle(FiberBundleX* fib);
    FiberBundleX::Pointer SubtractBundle(FiberBundleX* fib);

    // fiber subset extraction
    FiberBundleX::Pointer           ExtractFiberSubset(BaseData* roi);
    std::vector<long>               ExtractFiberIdSubset(BaseData* roi);
    FiberBundleX::Pointer           ExtractFiberSubset(ItkUcharImgType* mask, bool anyPoint, bool invert=false);
    FiberBundleX::Pointer           RemoveFibersOutside(ItkUcharImgType* mask, bool invert=false);

    vtkSmartPointer<vtkPolyData>    GeneratePolyDataByIds( std::vector<long> ); // TODO: make protected
    void                            GenerateFiberIds(); // TODO: make protected

    // get/set data
    void SetFiberPolyData(vtkSmartPointer<vtkPolyData>, bool updateGeometry = true);
    vtkSmartPointer<vtkPolyData> GetFiberPolyData() const;
    std::vector< std::string > GetAvailableColorCodings();
    char* GetCurrentColorCoding();
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

    unsigned long GetNumberOfPoints();

    // copy fiber bundle
    mitk::FiberBundleX::Pointer GetDeepCopy();

    // compare fiber bundles
    bool Equals(FiberBundleX* fib, double eps=0.0001);

    itkSetMacro( ReferenceGeometry, mitk::BaseGeometry::Pointer )
    itkGetConstMacro( ReferenceGeometry, mitk::BaseGeometry::Pointer )

protected:

    FiberBundleX( vtkPolyData* fiberPolyData = NULL );
    virtual ~FiberBundleX();

    itk::Point<float, 3> GetItkPoint(double point[3]);

    // calculate geometry from fiber extent
    void UpdateFiberGeometry();

    // calculate colorcoding values according to m_CurrentColorCoding
    void UpdateColorCoding();

private:

    // actual fiber container
    vtkSmartPointer<vtkPolyData>  m_FiberPolyData;

    // contains fiber ids
    vtkSmartPointer<vtkDataSet>   m_FiberIdDataSet;

    char* m_CurrentColorCoding;
    int   m_NumFibers;

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

#endif /*  _MITK_FiberBundleX_H */
