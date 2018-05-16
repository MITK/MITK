
//
// Defining some Macros that make problems with SWIG as the
// corresponding definitions are not included by default.
// Luckely, these includes are not necessary for SWIG.
//
#define ITK_NOEXCEPT
#define ITKCommon_EXPORT
#define ITK_OVERRIDE
#define MITKCORE_EXPORT
#define MITKCLCORE_EXPORT
#define MITKCLUTILITIES_EXPORT
#define ITKCommon_EXPORT

#define ITKCommon_EXPORT
#define ITK_FORWARD_EXPORT
#define ITK_OVERRIDE
#define ITK_NOEXCEPT


%include <itkMacro.h>
%include <mitkCommon.h>
%include <itkSmartPointer.h>
%include <mitkPixelType.h>
%include <mitkChannelDescriptor.h>
%include <mitkIOUtil.h>

#define DEPRECATED(func) func
#undef ITK_DISALLOW_COPY_AND_ASSIGN
#define ITK_DISALLOW_COPY_AND_ASSIGN(TypeName)

%pythoncode %{
 convertion_list = {}
%}

SWIG_ADD_MITK_CLASS(Object, itkObject.h, itk)
SWIG_ADD_MITK_CLASS(DataObject, itkDataObject.h, itk)

SWIG_ADD_MITK_CLASS(TimeGeometry, mitkTimeGeometry.h, mitk)
SWIG_ADD_MITK_CLASS(ArbitraryTimeGeometry, mitkArbitraryTimeGeometry.h, mitk)
SWIG_ADD_MITK_CLASS(ProportionalTimeGeometry, mitkProportionalTimeGeometry.h, mitk)
SWIG_ADD_MITK_CLASS(BaseGeometry, mitkBaseGeometry.h, mitk)
SWIG_ADD_MITK_CLASS(Geometry3D, mitkGeometry3D.h, mitk)
SWIG_ADD_MITK_CLASS(SlicedGeometry3D, mitkSlicedGeometry3D.h, mitk)
SWIG_ADD_MITK_CLASS(PlaneGeometry , mitkPlaneGeometry.h, mitk)

SWIG_ADD_NONOBJECT_NOVECTOR_CLASS(BoundingBox, mitkBaseGeometry.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(TimeBounds, mitkBaseGeometry.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(FixedArrayType, mitkBaseGeometry.h, mitk)

SWIG_ADD_NONOBJECT_CLASS(Point2D, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point3D, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point4D, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point2I, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point3I, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point4I, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(VnlVector, mitkVector.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Vector2D, mitkVector.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Vector3D, mitkVector.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Vector4D, mitkVector.h, mitk)

SWIG_ADD_MITK_CLASS(BaseData, mitkBaseData.h, mitk)
SWIG_ADD_MITK_CLASS(SlicedData, mitkSlicedData.h, mitk)
SWIG_ADD_MITK_CLASS(Image, mitkImage.h, mitk)
SWIG_ADD_MITK_CLASS(PointSet, mitkPointSet.h, mitk)

SWIG_ADD_MITK_CLASS(AbstractGlobalImageFeature, mitkAbstractGlobalImageFeature.h, mitk)
SWIG_ADD_MITK_CLASS(GIFImageDescriptionFeatures, mitkGIFImageDescriptionFeatures.h, mitk)
SWIG_ADD_MITK_CLASS(GIFFirstOrderStatistics, mitkGIFFirstOrderStatistics.h, mitk)
SWIG_ADD_MITK_CLASS(GIFFirstOrderHistogramStatistics, mitkGIFFirstOrderHistogramStatistics.h, mitk)
SWIG_ADD_MITK_CLASS(GIFVolumetricStatistics, mitkGIFVolumetricStatistics.h, mitk)
SWIG_ADD_MITK_CLASS(GIFVolumetricDensityStatistics, mitkGIFVolumetricDensityStatistics.h, mitk)
SWIG_ADD_MITK_CLASS(GIFCooccurenceMatrix2, mitkGIFCooccurenceMatrix2.h, mitk)
SWIG_ADD_MITK_CLASS(GIFNeighbouringGreyLevelDependenceFeature, mitkGIFNeighbouringGreyLevelDependenceFeatures.h, mitk)
SWIG_ADD_MITK_CLASS(GIFGreyLevelRunLength, mitkGIFGreyLevelRunLength.h, mitk)
SWIG_ADD_MITK_CLASS(GIFGreyLevelSizeZone, mitkGIFGreyLevelSizeZone.h, mitk)
SWIG_ADD_MITK_CLASS(GIFGreyLevelDistanceZone, mitkGIFGreyLevelDistanceZone.h, mitk)
SWIG_ADD_MITK_CLASS(GIFLocalIntensity, mitkGIFLocalIntensity.h, mitk)
SWIG_ADD_MITK_CLASS(GIFIntensityVolumeHistogramFeatures, mitkGIFIntensityVolumeHistogramFeatures.h, mitk)
SWIG_ADD_MITK_CLASS(GIFNeighbourhoodGreyToneDifferenceFeatures, mitkGIFNeighbourhoodGreyToneDifferenceFeatures.h, mitk)
SWIG_ADD_MITK_CLASS(GIFCurvatureStatistic, mitkGIFCurvatureStatistic.h, mitk)