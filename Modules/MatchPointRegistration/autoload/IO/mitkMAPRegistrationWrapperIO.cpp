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

#include <iostream>
#include <fstream>

#include <clocale>

#include "mapRegistration.h"
#include "mapRegistrationFileWriter.h"
#include "mapRegistrationFileReader.h"
#include "mapLazyFileFieldKernelLoader.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>

#include "mitkMAPRegistrationWrapperIO.h"
#include "mitkMAPRegistrationWrapper.h"

namespace mitk
{

  /** Helper structure to change (and reset) the
  * local settings in a function scope*/
  struct LocaleSwitch
  {
    LocaleSwitch(const std::string& newLocale)
      : m_OldLocale(std::setlocale(LC_ALL, nullptr))
      , m_NewLocale(newLocale)
    {
      if (m_OldLocale == nullptr)
      {
        m_OldLocale = "";
      }
      else if (m_NewLocale != m_OldLocale)
      {
        // set the locale
        if (std::setlocale(LC_ALL, m_NewLocale.c_str()) == nullptr)
        {
          MITK_INFO << "Could not set locale " << m_NewLocale;
          m_OldLocale = nullptr;
        }
      }
    }

    ~LocaleSwitch()
    {
      if (m_OldLocale != nullptr && std::setlocale(LC_ALL, m_OldLocale) == nullptr)
      {
        MITK_INFO << "Could not reset locale " << m_OldLocale;
      }
    }

  private:
    const char* m_OldLocale;
    const std::string m_NewLocale;
  };

  /** Helper class that allows to use an functor in multiple combinations of
  * moving and target dimensions on a passed MAPRegistrationWrapper instance.\n
  * DimHelperSub is used DimHelper to iterate in a row of the dimension
  * combination matrix.
  */
  template< unsigned int i, unsigned int j, template < unsigned int,  unsigned int> class TFunctor>
  class DimHelperSub
  {
  public:
    static bool Execute(const mitk::MAPRegistrationWrapper* obj, const map::core::String& data)
    {
      if (TFunctor<i,j>::Execute(obj, data))
      {
        return true;
      }
      return DimHelperSub<i,j-1,TFunctor>::Execute(obj, data);
    }
  };

  /** Specialized template version of DimSubHelper that indicates the end
  * of the row in the dimension combination matrix, thus does nothing.
  */
  template< unsigned int i, template < unsigned int,  unsigned int> class TFunctor>
  class DimHelperSub<i,1,TFunctor >
  {
  public:
    static bool Execute(const mitk::MAPRegistrationWrapper*, const map::core::String&)
    {
      //just unwind. Go to the next "row" with DimHelper
      return false;
    }
  };

  /** Helper class that allows to use an functor in multiple combinations of
  * moving and target dimensions on a passed MAPRegistrationWrapper instance.\n
  * It is helpful if you want to ensure that all combinations are checked/touched
  * (e.g. 3D 3D, 3D 2D, 2D 3D, 2D 2D) without generating a large switch yard.
  * Think of n*m matrix (indicating the posible combinations). DimHelper walks from
  * one row to the next and uses DimHelperSub to iterate in a row.\n
  * For every element of the matrix the functor is executed on the passed object.
  */
  template< unsigned int i,  unsigned int j, template < unsigned int,  unsigned int> class TFunctor>
  class DimHelper{
  public:
    static bool Execute(const mitk::MAPRegistrationWrapper* obj, const map::core::String& data = "")
    {
      if (DimHelperSub<i,j,TFunctor>::Execute(obj, data))
      {
        return true;
      }
      return DimHelper<i-1,j,TFunctor>::Execute(obj, data);
    }
  };

  /** Specialized template version of DimHelper that indicates the end
  * of the dimension combination matrix, thus does nothing.
  */
  template< unsigned int j, template < unsigned int,  unsigned int> class TFunctor>
  class DimHelper<1,j, TFunctor >
  {
  public:
    static bool Execute(const mitk::MAPRegistrationWrapper*, const map::core::String&)
    {
      //just unwind. We are done.
      return false;
    }
  };

  /** Functor that checks of the dimension of the registration is supported and can
  * be written.
  */
  template<unsigned int i, unsigned int j>
  class CanWrite
  {
  public:
    static bool Execute(const mitk::MAPRegistrationWrapper* obj, const map::core::String& = "")
    {
      bool result = false;

      result = dynamic_cast<const map::core::Registration<i,j> *>(obj->GetRegistration()) != nullptr;

      return result;
    }
  };

  /** Functor that writes the registration to a file if it has the right dimensionality.
  */
  template<unsigned int i, unsigned int j>
  class WriteReg
  {
  public:
    static bool Execute(const mitk::MAPRegistrationWrapper* obj, const map::core::String& data)
    {
      const map::core::Registration<i,j>* pReg = dynamic_cast<const map::core::Registration<i,j>*>(obj->GetRegistration());
      if (pReg == nullptr)
      {
        return false;
      }

      typedef map::io::RegistrationFileWriter<i,j> WriterType;
      typename WriterType::Pointer writer = WriterType::New();

      writer->setExpandLazyKernels(false);

      try
      {
        writer->write(pReg,data);
      }
      catch (itk::ExceptionObject e)
      {
        std::cout << e << std::endl;
        throw;
      }

      return true;
    }
  };

  MAPRegistrationWrapperIO::MAPRegistrationWrapperIO(const MAPRegistrationWrapperIO& other)
    : AbstractFileIO(other)
  {
  }

  MAPRegistrationWrapperIO::MAPRegistrationWrapperIO() : AbstractFileIO(mitk::MAPRegistrationWrapper::GetStaticNameOfClass())
  {
    std::string category = "MatchPoint Registration File";
    CustomMimeType customMimeType;
    customMimeType.SetCategory(category);
    customMimeType.AddExtension("mapr");
    this->AbstractFileIOWriter::SetMimeType(customMimeType);
    this->AbstractFileIOWriter::SetDescription(category);

    customMimeType.AddExtension("mapr.xml");
    customMimeType.AddExtension("MAPR");
    customMimeType.AddExtension("MAPR.XML");
    this->AbstractFileIOReader::SetMimeType(customMimeType);
    this->AbstractFileIOReader::SetDescription(category);

    this->RegisterService();
  }


  void MAPRegistrationWrapperIO::Write()
  {
    bool success = false;
    const BaseData* input = this->GetInput();
    if (input == nullptr)
    {
      mitkThrow() << "Cannot write data. Data pointer is nullptr.";
    }

    const mitk::MAPRegistrationWrapper* wrapper = dynamic_cast<const mitk::MAPRegistrationWrapper*>(input);
    if (wrapper == nullptr)
    {
      mitkThrow() << "Cannot write data. Data pointer is not a Registration wrapper.";
    }

    std::ostream* writeStream = this->GetOutputStream();
    std::string fileName = this->GetOutputLocation();
    if (writeStream)
    {
      fileName = this->GetLocalFileName();
    }

    // Switch the current locale to "C"
    LocaleSwitch localeSwitch("C");

    try
    {
      success = DimHelper<3,3,WriteReg>::Execute(wrapper, fileName);
    }
    catch (const std::exception& e)
    {
      mitkThrow() << e.what();
    }

    if (!success)
    {
      mitkThrow() << "Cannot write registration. Currently only registrations up to 4D are supported.";
    }
  }

  AbstractFileIO::ConfidenceLevel MAPRegistrationWrapperIO::GetWriterConfidenceLevel() const
  {
    const mitk::MAPRegistrationWrapper* regWrapper =  dynamic_cast<const mitk::MAPRegistrationWrapper*>(this->GetInput());

    if (regWrapper == nullptr)
    {
      return IFileWriter::Unsupported;
    }

    // Check if the registration dimension is supported
    if (! DimHelper<3,3,CanWrite>::Execute(regWrapper))
    {
      return IFileWriter::Unsupported;
    };

    return IFileWriter::Supported;
  }

  std::vector<BaseData::Pointer >  MAPRegistrationWrapperIO::Read()
  {
    std::vector<BaseData::Pointer > result;

    LocaleSwitch("C");

    std::string fileName = this->GetLocalFileName();
    if ( fileName.empty() )
    {
      mitkThrow() << "Cannot read file. Filename has not been set!";
    }

    /* Remove the following kernel loader provider because in MITK no lazy file loading should be used
    due to conflicts with session loading (end there usage of temporary directories)*/
    map::io::RegistrationFileReader::LoaderStackType::unregisterProvider(map::io::LazyFileFieldKernelLoader<2,2>::getStaticProviderName());
    map::io::RegistrationFileReader::LoaderStackType::unregisterProvider(map::io::LazyFileFieldKernelLoader<3,3>::getStaticProviderName());

    map::io::RegistrationFileReader::Pointer spReader = map::io::RegistrationFileReader::New();
    spReader->setPreferLazyLoading(true);
    map::core::RegistrationBase::Pointer spReg = spReader->read(fileName);
    mitk::MAPRegistrationWrapper::Pointer spRegWrapper = mitk::MAPRegistrationWrapper::New();
    spRegWrapper->SetRegistration(spReg);

    result.push_back(spRegWrapper.GetPointer());
    return result;
  }

  AbstractFileIO::ConfidenceLevel MAPRegistrationWrapperIO::GetReaderConfidenceLevel() const
  {
    AbstractFileIO::ConfidenceLevel result = IFileReader::Unsupported;

    std::string fileName = this->GetLocalFileName();
    std::ifstream in( fileName.c_str() );
    if ( in.good() )
    {
      result = IFileReader::Supported;
    }

    in.close();
    return result;
  }

  MAPRegistrationWrapperIO* MAPRegistrationWrapperIO::IOClone() const
  {
    return new MAPRegistrationWrapperIO(*this);
  }

}
