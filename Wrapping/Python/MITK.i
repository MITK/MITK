%module pyMITK

%include <mitk_swig_common.i>

%{
#include "mitkNumpyArrayConversion.cxx"
%}

// Numpy array conversion support
%native(_GetMemoryViewFromImage) PyObject *mitk_GetMemoryViewFromImage( PyObject *self, PyObject *args );
%native(_SetImageFromArray) PyObject *mitk_SetImageFromArray( PyObject *(self), PyObject *args );


%pythoncode %{

HAVE_NUMPY = True
try:
    import numpy
except ImportError:
    HAVE_NUMPY = False


def _get_numpy_dtype( mitkImage ):
    """Given a MITK image, returns the numpy.dtype which describes the data"""

    if not HAVE_NUMPY:
        raise ImportError('Numpy not available.')

    # this is a mapping from MITK's pixel id to numpy's dtype
    _mitk_np = {ComponentTypeUInt8:numpy.uint8,
                ComponentTypeUInt16:numpy.uint16,
                ComponentTypeUInt32:numpy.uint32,
                ComponentTypeInt8:numpy.int8,
                ComponentTypeInt16:numpy.int16,
                ComponentTypeInt32:numpy.int32,
                ComponentTypeFloat:numpy.float32,
                ComponentTypeDouble:numpy.float64,
                }

    return _mitk_np[ mitkImage.GetPixelType().GetComponentType() ]



def _get_mitk_pixelid(numpy_array_type):
    """Returns a MITK PixelID given a numpy array."""

    if not HAVE_NUMPY:
        raise ImportError('Numpy not available.')

    # This is a Mapping from numpy array types to sitks pixel types.
    _np_mitk = {numpy.character:ComponentTypeUInt8,
                numpy.uint8:ComponentTypeUInt8,
                numpy.uint16:ComponentTypeUInt16,
                numpy.uint32:ComponentTypeUInt32,
                numpy.int8:ComponentTypeInt8,
                numpy.int16:ComponentTypeInt16,
                numpy.int32:ComponentTypeInt32,
                numpy.float32:ComponentTypeFloat,
                numpy.float64:ComponentTypeDouble,
                }

    try:
        return _np_mitk[numpy_array_type.dtype]
    except KeyError:
        for key in _np_mitk:
            if numpy.issubdtype(numpy_array_type.dtype, key):
                return _np_mitk[key]
        raise TypeError('dtype: {0} is not supported.'.format(numpy_array_type.dtype))

def _get_sitk_vector_pixelid(numpy_array_type):
    """Returns a MITK vecotr PixelID given a numpy array."""

    if not HAVE_NUMPY:
        raise ImportError('Numpy not available.')

    # This is a Mapping from numpy array types to sitks pixel types.
    _np_sitk = {numpy.character:sitkVectorUInt8,
                numpy.uint8:sitkVectorUInt8,
                numpy.uint16:sitkVectorUInt16,
                numpy.uint32:sitkVectorUInt32,
                numpy.uint64:sitkVectorUInt64,
                numpy.int8:sitkVectorInt8,
                numpy.int16:sitkVectorInt16,
                numpy.int32:sitkVectorInt32,
                numpy.int64:sitkVectorInt64,
                numpy.float32:sitkVectorFloat32,
                numpy.float64:sitkVectorFloat64,
                }

    try:
        return _np_sitk[numpy_array_type.dtype]
    except KeyError:
        for key in _np_sitk:
            if numpy.issubdtype(numpy_array_type.dtype, key):
                return _np_sitk[key]
        raise TypeError('dtype: {0} is not supported.'.format(numpy_array_type.dtype))


# MITK <-> Numpy Array conversion support.
#http://www.nickdarnell.com/swig-casting-revisited/
def GetArrayViewFromImage(image):
    """Get a NumPy ndarray view of a MITK Image.

    Returns a Numpy ndarray object as a "view" of the MITK's Image buffer. This reduces pixel buffer copies, but requires that the MITK image object is kept around while the buffer is being used.

    """

    if not HAVE_NUMPY:
        raise ImportError('NumPy not available.')

    dtype = _get_numpy_dtype( image )

    shape = GetImageSize(image);
    if image.GetPixelType().GetNumberOfComponents() > 1:
      shape = ( image.GetPixelType().GetNumberOfComponents(), ) + shape

    imageMemoryView =  _pyMITK._GetMemoryViewFromImage(image)
    arrayView = numpy.asarray(imageMemoryView).view(dtype = dtype)
    arrayView.shape = shape[::-1]

    return arrayView

def GetArrayFromImage(image):
    """Get a NumPy ndarray from a MITK Image.

    This is a deep copy of the image buffer and is completely safe and without potential side effects.
    """

    # TODO: If the image is already not unique then a second copy may be made before the numpy copy is done.
    arrayView = GetArrayViewFromImage(image)

    # perform deep copy of the image buffer
    return numpy.array(arrayView, copy=True)

def GetImageFromArray( arr, isVector=False):
    """Get a MITK Image from a numpy array. If isVector is True, then a 3D array will be treated as a 2D vector image, otherwise it will be treated as a 3D image"""

    if not HAVE_NUMPY:
        raise ImportError('Numpy not available.')

    z = numpy.asarray( arr )

    assert z.ndim in ( 2, 3, 4 ), \
      "Only arrays of 2, 3 or 4 dimensions are supported."

    id = _get_mitk_pixelid( z )
    #img = Image_New()

    if ( z.ndim == 3 and isVector ) or (z.ndim == 4):
      pixelType=MakePixelTypeFromTypeID(id, z.shape[-1])
      newShape=VectorUInt32(z.shape[-2::-1])
      img = MakeImage(pixelType, newShape)
      #img.Initialize(pixelType, z.ndim - 1, z.shape[-2::-1])
    elif z.ndim in ( 2, 3 ):
      pixelType=MakePixelTypeFromTypeID(id, 1)
      newShape=VectorUInt32(z.shape[::-1])
      img = MakeImage(pixelType, newShape)
      #img.Initialize(pixelType, z.ndim, z.shape[::-1])

    _pyMITK._SetImageFromArray( z.tostring(), img )

    return img

%}



