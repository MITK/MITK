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
#include "MitkDiffusionImagingExports.h"
#include <mitkImage.h>


//includes storing fiberdata
#include <vtkSmartPointer.h> //may be replaced by class precompile argument
#include <vtkPolyData.h> // may be replaced by class
#include <vtkPoints.h> // my be replaced by class
#include <vtkDataSet.h>

#include <QStringList>

#include <mitkPlanarFigure.h>

namespace mitk {

/**
   * \brief Base Class for Fiber Bundles;   */
class  MitkDiffusionImaging_EXPORT FiberBundleX : public BaseData
{
public:

    // fiber colorcodings
    static const char* COLORCODING_ORIENTATION_BASED;
    static const char* COLORCODING_FA_BASED;
    static const char* COLORCODING_CUSTOM;
    static const char* FIBER_ID_ARRAY;

    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );

    mitkClassMacro( FiberBundleX, BaseData )
    itkNewMacro( Self )
    mitkNewMacro1Param(Self, vtkSmartPointer<vtkPolyData>) // custom constructor

    // colorcoding related methods
    void SetColorCoding(const char*);
    void SetFAMap(mitk::Image::Pointer);
    void DoColorCodingOrientationBased();
    void DoColorCodingFaBased();
    void DoUseFaFiberOpacity();
    void ResetFiberOpacity();

    // fiber smoothing/resampling
    void ResampleFibers(float pointDistance = 1);
    void DoFiberSmoothing(int pointsPerCm);
    void DoFiberSmoothing(int pointsPerCm, double tension, double continuity, double bias );
    bool RemoveShortFibers(float lengthInMM);
    bool RemoveLongFibers(float lengthInMM);
    bool ApplyCurvatureThreshold(float minRadius, bool deleteFibers);
    void MirrorFibers(unsigned int axis);
    void RotateAroundAxis(double x, double y, double z);
    void TranslateFibers(double x, double y, double z);

    // add/subtract fibers
    FiberBundleX::Pointer AddBundle(FiberBundleX* fib);
    FiberBundleX::Pointer SubtractBundle(FiberBundleX* fib);

    // fiber subset extraction
    FiberBundleX::Pointer           ExtractFiberSubset(PlanarFigure *pf);
    std::vector<long>               ExtractFiberIdSubset(PlanarFigure* pf);
    vtkSmartPointer<vtkPolyData>    GeneratePolyDataByIds( std::vector<long> ); // TODO: make protected
    void                            GenerateFiberIds(); // TODO: make protected



    // get/set data
    void SetFiberPolyData(vtkSmartPointer<vtkPolyData>, bool updateGeometry = true);
    vtkSmartPointer<vtkPolyData> GetFiberPolyData();
    QStringList GetAvailableColorCodings();
    char* GetCurrentColorCoding();
    itkGetMacro( NumFibers, int)
    itkGetMacro( FiberSampling, int)
    itkGetMacro( MinFiberLength, float )
    itkGetMacro( MaxFiberLength, float )
    itkGetMacro( MeanFiberLength, float )
    itkGetMacro( MedianFiberLength, float )
    itkGetMacro( LengthStDev, float )

    std::vector<int> GetPointsRoi()
    {
        return m_PointsRoi;
    }

    // copy fiber bundle
    mitk::FiberBundleX::Pointer GetDeepCopy();

    // compare fiber bundles
    bool Equals(FiberBundleX* fib);

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

    std::vector<int> m_PointsRoi; // this global variable needs to be refactored

};

} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
