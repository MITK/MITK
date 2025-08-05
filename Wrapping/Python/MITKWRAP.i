%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#define SWIG_FILE_WITH_INIT
%}

%pythonbegin %{
import os
os.environ["PYMITK"] = "1"
%}

%module pyMITK
%include <mitk_swig_std.i>

%naturalvar; // All non-primitive types will use const reference typemaps
%include "numpy.i"
%nothread mitk::Image::GetAsNumpy;
%nothread mitk::Image::GetSpacing;
%nothread mitk::Image::GetOrigin;

%constant int ComponentTypeUInt8 = TypeDefinitions::ComponentTypeUInt8;
%constant int ComponentTypeInt8 = TypeDefinitions::ComponentTypeInt8;
%constant int ComponentTypeUInt16 = TypeDefinitions::ComponentTypeUInt16;
%constant int ComponentTypeInt16 = TypeDefinitions::ComponentTypeInt16;
%constant int ComponentTypeUInt32 = TypeDefinitions::ComponentTypeUInt32;
%constant int ComponentTypeInt32 = TypeDefinitions::ComponentTypeInt32;
%constant int ComponentTypeFloat = TypeDefinitions::ComponentTypeFloat;
%constant int ComponentTypeDouble = TypeDefinitions::ComponentTypeDouble;

#define MITKCORE_EXPORT
#define ITKCommon_EXPORT
%{
#include <itkSmartPointer.h>
#include <mitkIOUtil.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

using namespace mitk;
using itk::DataObject;
using itk::LightObject;
using itk::SmartPointer;

struct TypeDefinitions
{
   static const int ComponentTypeUInt8 = static_cast<int>(mitk::MapPixelType<uint8_t, mitk::isPrimitiveType<uint8_t>::value>::IOComponentType);
   static const int ComponentTypeInt8 = static_cast<int>(mitk::MapPixelType<int8_t, mitk::isPrimitiveType<int8_t>::value>::IOComponentType);
   static const int ComponentTypeUInt16 = static_cast<int>(mitk::MapPixelType<uint16_t, mitk::isPrimitiveType<uint16_t>::value>::IOComponentType);
   static const int ComponentTypeInt16 = static_cast<int>(mitk::MapPixelType<int16_t, mitk::isPrimitiveType<int16_t>::value>::IOComponentType);
   static const int ComponentTypeUInt32 = static_cast<int>(mitk::MapPixelType<uint32_t, mitk::isPrimitiveType<uint32_t>::value>::IOComponentType);
   static const int ComponentTypeInt32 = static_cast<int>(mitk::MapPixelType<int32_t, mitk::isPrimitiveType<int32_t>::value>::IOComponentType);
   static const int ComponentTypeFloat = static_cast<int>(mitk::MapPixelType<float, mitk::isPrimitiveType<float>::value>::IOComponentType);
   static const int ComponentTypeDouble = static_cast<int>(mitk::MapPixelType<double, mitk::isPrimitiveType<double>::value>::IOComponentType);
};

NPY_TYPES GetNumpyTypeFromTypeID(int componentTypeID)
{
   switch (componentTypeID)
   {
        case TypeDefinitions::ComponentTypeUInt8 :
            return NPY_UBYTE;
        case TypeDefinitions::ComponentTypeInt8 :
            return NPY_BYTE;
        case TypeDefinitions::ComponentTypeUInt16 :
            return NPY_USHORT;
        case TypeDefinitions::ComponentTypeInt16 :
            return NPY_SHORT;
        case TypeDefinitions::ComponentTypeUInt32 :
            return NPY_UINT;
        case TypeDefinitions::ComponentTypeInt32 :
            return NPY_INT;
        case TypeDefinitions::ComponentTypeFloat :
            return NPY_FLOAT;
        case TypeDefinitions::ComponentTypeDouble :
            return NPY_DOUBLE;
        default:
            return NPY_DOUBLE;
   }
}

mitk::PixelType MakePixelTypeFromTypeID(int componentTypeID, int numberOfComponents)
{
   switch (componentTypeID)
   {
      case TypeDefinitions::ComponentTypeUInt8 :
      return mitk::MakePixelType<uint8_t, uint8_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt8 :
      return mitk::MakePixelType<int8_t, int8_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeUInt16 :
      return mitk::MakePixelType<uint16_t, uint16_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt16 :
      return mitk::MakePixelType<int16_t, int16_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeUInt32 :
      return mitk::MakePixelType<uint32_t, uint32_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt32 :
      return mitk::MakePixelType<int32_t, int32_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeFloat :
      return mitk::MakePixelType<float, float>(numberOfComponents);
      case TypeDefinitions::ComponentTypeDouble :
      return mitk::MakePixelType<double, double>(numberOfComponents);
    default:
      return mitk::MakePixelType<double, double>(numberOfComponents);
   }
}

static PyObject* mitk_SetImageFromArray(PyObject * SWIGUNUSEDPARM(self), PyObject * args)
{
    PyObject* pyObj = NULL;
    PyObject* pyImage = NULL;

    Py_buffer pyBuffer;
    memset(&pyBuffer, 0, sizeof(Py_buffer));

    mitk::Image* mitkImage = NULL;
    void *mitkBufferPtr = NULL;
    std::vector<unsigned int> size;
    size_t pixelSize = 1;
    size_t len = 1;
    unsigned int accuValue = 1;
    unsigned int dimension = 0;

    if (!PyArg_ParseTuple(args, "OO", &pyObj, &pyImage))
    {
        return NULL;
    }
    if (PyObject_GetBuffer(pyObj, &pyBuffer, PyBUF_FULL_RO) != 0)
    {
        return NULL;
    }

    /* Cast over to a mitk Image. */
    void *voidImage;
    int res = 0;
    res = SWIG_ConvertPtr(pyImage, &voidImage, SWIGTYPE_p_mitk__Image, 0);
    if (!SWIG_IsOK(res))
    {
        SWIG_exception_fail(SWIG_ArgError(res), "in method 'SetImageFromArray', argument needs to be of type 'mitk::Image*'");
    }
    mitkImage = reinterpret_cast<mitk::Image*>(voidImage);
    try
    {
        mitk::ImageWriteAccessor writeAccess(mitkImage);
        mitkBufferPtr = writeAccess.GetData();
        pixelSize = mitkImage->GetPixelType().GetBitsPerComponent() / 8;
    }
    catch(const std::exception & e)
    {
        std::string msg = "Exception thrown in pyMITK new Image: ";
        msg += e.what();
        PyErr_SetString(PyExc_RuntimeError, msg.c_str());
        goto fail;
    }
    dimension = mitkImage->GetDimension();
    for (unsigned int i = 0; i < dimension; ++i)
    {
        size.push_back(mitkImage->GetDimension(i));
    }
    // if the image is a vector just treat is as another dimension
    if (mitkImage->GetPixelType().GetNumberOfComponents() > 1)
    {
        size.push_back(mitkImage->GetPixelType().GetNumberOfComponents());
    }
    len = std::accumulate(size.begin(), size.end(), accuValue, std::multiplies<unsigned int>());
    len *= pixelSize;
    if (PyBuffer_ToContiguous(mitkBufferPtr, &pyBuffer, len, 'C') != 0)
    {
      goto fail;
    }
    PyBuffer_Release(&pyBuffer);
    Py_RETURN_NONE;

  fail:
    PyBuffer_Release(&pyBuffer);
    return NULL;
}

%}
%init %{
    import_array();
%}

%native(_SetImageFromArray) PyObject *mitk_SetImageFromArray(PyObject *(self), PyObject *args);

// ignore reference count management
%ignore Delete;
%ignore SetReferenceCount;
%ignore Register;
%ignore UnRegister;

%define MITK_CLASS_SWIG_MACRO(namespace, class_name)
        
    %typemap(out) namespace ## :: ## class_name ## *, namespace ## :: ## class_name & {
        std::cout << "class_name *, class_name &" <<std::endl;
        $result = SWIG_NewPointerObj(SWIG_as_voidptr($1), $1_descriptor, 1);
        /*if ($1) {
                $1->Register();
        }*/
    }

    %typemap(out) namespace ## :: ## class_name ## ::Pointer 
    {
        std::cout << "namespace##::##class_name::Pointer" <<std::endl;
        namespace##::##class_name * ptr = $1.GetPointer();
        $result = SWIG_NewPointerObj(SWIG_as_voidptr(ptr), $descriptor(namespace::class_name *), 1);                
        if (ptr) {
            ptr->Register();
        }
    }
    
    %typemap(out) std::vector<namespace ## :: ## class_name ## ::Pointer> {
        std::cout << "Vector namespace##::##class_name::Pointer" <<std::endl;
        /*PyObject**/ $result = PyList_New($1.size());
        for (size_t i = 0; i < $1.size(); ++i) {
            namespace##::##class_name * ptr = ($1[i]).GetPointer();
            if (ptr) {
                ptr->Register();
                PyObject* pyObj = SWIG_NewPointerObj(SWIG_as_voidptr(ptr), $descriptor(namespace::class_name *), 1);
                PyList_SetItem($result, i, pyObj);
            }
        }
    }

    %extend namespace ## :: ## class_name
    {
        public:
        ~class_name()   
        {   std::cout << "namespace##::##class_name replace destructor" << std::endl;
            self->UnRegister(); 
        };
    }

    %ignore namespace ## :: ## class_name ## :: ## ~class_name ## ;
    %ignore namespace ## :: ## class_name ## _Pointer;

%enddef

MITK_CLASS_SWIG_MACRO(mitk, Image)


%inline{

    mitk::Image::Pointer MakeImage(mitk::PixelType pixelType, std::vector<unsigned int> shape)
    {
        mitk::Image::Pointer image = mitk::Image::New();
        image->Initialize(pixelType, shape.size(), shape.data());
        return image;
    }

}

%extend mitk::Image
{
    public:
    PyObject* GetAsNumpy() 
    {
        unsigned int dim = self->GetDimension();
        mitk::ImageReadAccessor readAccess(self);
        const void *mitkBufferPtr = readAccess.GetData();
        std::vector<npy_intp> size;
        for (unsigned int i = 0; i < dim; ++i)
        {
            size.push_back(self->GetDimension(i));
        }
        if (self->GetPixelType().GetNumberOfComponents() > 1) // if the image is a vector just treat is as another dimension
        {    
            size.push_back(self->GetPixelType().GetNumberOfComponents() );
        }
        std::reverse(size.begin(), size.end());
        PyObject *result = PyArray_SimpleNewFromData(dim, size.data(), 
                GetNumpyTypeFromTypeID(static_cast<int>(self->GetPixelType().GetComponentType())),
                const_cast<void*>(mitkBufferPtr));
        return result;
    }

    PyObject* GetSpacing() 
    {
        unsigned int dim = self->GetDimension();
        mitk::Vector3D spacing = self->GetGeometry()->GetSpacing();
        PyObject* listObj = PyList_New(dim);
        for (unsigned int i = 0; i < dim; ++i)
        {
            PyObject* num = PyFloat_FromDouble(spacing[dim-i-1]);
            PyList_SET_ITEM(listObj, i, num);
        }
        return listObj;
    }

    PyObject* GetOrigin() 
    {
        unsigned int dim = self->GetDimension();
        mitk::Point3D origin = self->GetGeometry()->GetOrigin();
        PyObject* listObj = PyList_New(dim);
        for (unsigned int i = 0; i < dim; ++i)
        {
            PyObject* num = PyFloat_FromDouble(origin[dim-i-1]);
            PyList_SET_ITEM(listObj, i, num);
        }
        return listObj;
    }
}

%extend mitk::IOUtil
{
    public:
    static std::vector<mitk::Image::Pointer> imread(const std::string& filename)
    {
        auto images = mitk::IOUtil::Load(filename);
        std::vector<mitk::Image::Pointer> result;
        for(auto& image: images)
        {
            result.push_back(dynamic_cast<mitk::Image*>(image.GetPointer()));
        }
        return result;
    }
}

mitk::PixelType MakePixelTypeFromTypeID(int componentTypeID, int numberOfComponents);

%ignore mitk::BaseData;
%ignore mitk::SlicedData;
%ignore mitk::Image::GetPixelType;
%ignore mitk::Image::GetChannelDescriptor;
%ignore mitk::IOUtil::Save;
%ignore mitk::IOUtil::Load;
%rename(imsave) mitk::IOUtil::Save(const mitk::BaseData*, const std::string&, bool);

%include <itkMacro.h>
%include <itkSmartPointer.h>
%include <mitkCommon.h>
%include <mitkPixelType.h>
%include <mitkBaseData.h>
%include <mitkSlicedData.h>
%include <mitkImage.h>
%include <mitkIOUtil.h>

%template(VectorImageP) std::vector<mitk::Image::Pointer>;

%pythoncode %{

    HAVE_NUMPY = True
    try:
        import numpy
    except ImportError:
        HAVE_NUMPY = False

    
    def SayHi():
        print('Hi from pyMITK!')


    # Mapping from numpy array types to mitk pixel types.
    _np_mitk_mapping = {
                numpy.character:ComponentTypeUInt8,
                numpy.uint8:ComponentTypeUInt8,
                numpy.uint16:ComponentTypeUInt16,
                numpy.uint32:ComponentTypeUInt32,
                numpy.int8:ComponentTypeInt8,
                numpy.int16:ComponentTypeInt16,
                numpy.int32:ComponentTypeInt32,
                numpy.float32:ComponentTypeFloat,
                numpy.float64:ComponentTypeDouble
                }

    def _get_mitk_pixelid(numpy_array_type):
        """Returns a MITK PixelID given a numpy array."""
        if not HAVE_NUMPY:
            raise ImportError('Numpy not available.')
        try:
            return _np_mitk_mapping[numpy_array_type.dtype]
        except KeyError:
            for key in _np_mitk:
                if numpy.issubdtype(numpy_array_type.dtype, key):
                    return _np_mitk[key]
            raise TypeError('dtype: {0} is not supported.'.format(numpy_array_type.dtype))


    def GetImageFromArray(arr: numpy.ndarray, isVector: bool=False, isShared: bool=False):
        """
        Get a MITK Image from a numpy array. If isVector is True, then a 3D array will be treated as a 2D vector image,
        otherwise it will be treated as a 3D image. If isShared is True, then the new MITK Image and numpy array would 
        point to same underlying buffer.
        """
        if not HAVE_NUMPY:
            raise ImportError('Numpy not available.')

        assert arr.ndim in ( 2, 3, 4 ), \
        "Only arrays of 2, 3 or 4 dimensions are supported."

        id = _get_mitk_pixelid(arr)
        pixelType = MakePixelTypeFromTypeID(id, 1)
        newShape = VectorUInt32(arr.shape[::-1])
        img = MakeImage(pixelType, newShape)
        _SetImageFromArray(arr.tobytes(), img)
        if isShared:
            arr.data = img.GetAsNumpy().data
        return img

%}
