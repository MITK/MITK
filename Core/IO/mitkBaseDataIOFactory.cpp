/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "mitkBaseDataIOFactory.h"
#include "mitkBaseProcess.h"
#include "mitkIOAdapter.h"
#include "mitkConfig.h"

#include "mitkPicFileIOFactory.h"
#include "mitkPointSetIOFactory.h"
#include "mitkItkImageFileIOFactory.h"
#include "mitkParRecFileIOFactory.h"
#include "mitkSTLFileIOFactory.h"
#include "mitkObjFileIOFactory.h"
#include "mitkVtkSurfaceIOFactory.h"
#include "mitkVtkImageIOFactory.h"
#include "mitkVtiFileIOFactory.h"
#include "mitkPicVolumeTimeSeriesIOFactory.h"
#include "mitkStlVolumeTimeSeriesIOFactory.h"
#include "mitkVtkVolumeTimeSeriesIOFactory.h"

#include "itkMutexLock.h"
#include "itkMutexLockHolder.h"

namespace mitk
{

std::vector<BaseData::Pointer>* BaseDataIOFactory::CreateBaseDataIO(const std::string path, const std::string filePrefix, const std::string filePattern, FileModeType mode, bool series)
{
  RegisterBuiltInFactories();
  std::vector<BaseData::Pointer>* baseDataVector = new std::vector<BaseData::Pointer>;
  std::list<IOAdapterBase::Pointer> possibleIOAdapter;
  std::list<LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("mitkIOAdapter");
  for(std::list<LightObject::Pointer>::iterator i = allobjects.begin();
    i != allobjects.end(); ++i)
  {
    IOAdapterBase* io = dynamic_cast<IOAdapterBase*>(i->GetPointer());
    if(io)
    {
      possibleIOAdapter.push_back(io);
    }
    else
    {
      std::cerr << "Error BaseDataIO factory did not return an IOAdapterBase: "
        << (*i)->GetNameOfClass() 
        << std::endl;
    }
  }
  for(std::list<IOAdapterBase::Pointer>::iterator k = possibleIOAdapter.begin();
    k != possibleIOAdapter.end(); ++k)
  { 
    if( mode == ReadMode )
    {
      bool canReadFile = false;
      
      if ( series )
        canReadFile = (*k)->CanReadFile(filePrefix, filePrefix, filePattern); // we have to provide a filename without extension here to 
                                                                              // prevent the "normal" (non-series) readers to report that
                                                                              // they could read the file
      else
        canReadFile = (*k)->CanReadFile(path, filePrefix, filePattern);
      if(canReadFile)
      {
        BaseProcess::Pointer ioObject = (*k)->CreateIOProcessObject(path, filePrefix, filePattern);
        ioObject->Update();
        //return dynamic_cast<BaseData*>(ioObject->GetOutputs()[0].GetPointer());
        int numberOfContents = (int)ioObject->GetNumberOfOutputs();
        baseDataVector->resize(numberOfContents-1);
        BaseData::Pointer baseData;
        std::vector <BaseData::Pointer>::iterator it;
        it = baseDataVector->begin();
        for(int i=0; i<numberOfContents; i++){
          baseData = dynamic_cast<BaseData*>(ioObject->GetOutputs()[i].GetPointer());
          it = baseDataVector->insert(it, baseData);
        }
        return baseDataVector;
      }
    }
    //else if( mode == WriteMode )
    //{
    //  if((*k)->CanWriteFile(path))
    //  {
    //    return *k;
    //  }

    //}
  }
  return baseDataVector;
}

void BaseDataIOFactory::RegisterBuiltInFactories()
{
  static bool firstTime = true;

  static itk::SimpleMutexLock mutex;
  {
    // This helper class makes sure the Mutex is unlocked 
    // in the event an exception is thrown.
    itk::MutexLockHolder<itk::SimpleMutexLock> mutexHolder( mutex );
    if( firstTime )
    {
      itk::ObjectFactoryBase::RegisterFactory( PicFileIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( PointSetIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( ParRecFileIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( STLFileIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( ObjFileIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( VtkSurfaceIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( VtkImageIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( VtiFileIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( ItkImageFileIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( PicVolumeTimeSeriesIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( StlVolumeTimeSeriesIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( VtkVolumeTimeSeriesIOFactory::New() );

      //ObjectFactoryBase::RegisterFactory( LSMBaseDataIOFactory::New()); //should be before TIFF
      //ObjectFactoryBase::RegisterFactory( NiftiBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( AnalyzeBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( StimulateBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( JPEGBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( TIFFBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( NrrdBaseDataIOFactory::New() );
      //ObjectFactoryBase::RegisterFactory( BMPBaseDataIOFactory::New() );
      //ObjectFactoryBase::RegisterFactory( DICOMImageIO2Factory::New() );
      firstTime = false;
    }
  }

}

} // end namespace itk
