#include "mitkEndoDebug.h"
#include <itksys/SystemTools.hxx>
#include <itkFastMutexLock.h>
#include <itkMutexLockHolder.h>

namespace mitk
{
  struct EndoDebugData
  {
    EndoDebugData()
      : m_DebugEnabled(false)
      , m_ShowImagesInDebug(false)
      , m_ShowImagesTimeOut(false)
      , m_Mutex(itk::FastMutexLock::New())
    {

    }

    std::set<std::string> m_FilesToDebug;
    std::set<std::string> m_SymbolsToDebug;
    bool m_DebugEnabled;
    bool m_ShowImagesInDebug;
    size_t m_ShowImagesTimeOut;
    itk::FastMutexLock::Pointer m_Mutex;
  };

  EndoDebug::EndoDebug()
    : d ( new EndoDebugData )
  {

  }

  EndoDebug::~EndoDebug()
  {
    delete d;
  }

  EndoDebug& EndoDebug::GetInstance()
  {
    static EndoDebug instance;
    return instance;
  }

  std::string EndoDebug::GetFilenameWithoutExtension(const std::string& s)
  {
    return itksys::SystemTools::GetFilenameWithoutExtension( s );
  }

  void EndoDebug::AddFileToDebug(const std::string& s)
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      d->m_FilesToDebug.insert( s );
    }
  }

  void EndoDebug::AddSymbolToDebug(const std::string& symbolToDebug)
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      d->m_SymbolsToDebug.insert( symbolToDebug );
    }
  }

  bool EndoDebug::DebugSymbol(const std::string& s)
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      return d->m_SymbolsToDebug.find(s)
          != d->m_SymbolsToDebug.end();
    }
  }

  bool EndoDebug::DebugFile(const std::string& s)
  {
    std::string filename = GetFilenameWithoutExtension(s);

    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      return d->m_FilesToDebug.find(filename)
          != d->m_FilesToDebug.end();
    }
  }

  bool EndoDebug::Debug( const std::string& fileToDebug, const std::string& symbol )
  {
    bool debug = false;

    {
      bool debugEnabled = false;
      size_t filesSize = 0;
      size_t symbolsSize = 0;
      bool symbolFound = false;
      {
        itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
        debugEnabled = d->m_DebugEnabled;
        filesSize = d->m_FilesToDebug.size();
        symbolsSize = d->m_SymbolsToDebug.size();
        symbolFound = d->m_SymbolsToDebug.find(symbol) != d->m_SymbolsToDebug.end();
      }

      if( debugEnabled )
      {
        if( filesSize  == 0 )
          debug = true;
        else
          debug = DebugFile(fileToDebug);

        // ok debug is determined so far, now check if symbol set
        if( symbolsSize > 0 )
        {
          debug = symbolFound;
        }
        else
        {
          // do not show symbol debug output if no symbols are set at all
          if( !symbol.empty() )
            debug = false;
        }
      }
    }    

    return debug;
  }

  void EndoDebug::SetDebugEnabled(bool _DebugEnabled)
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      d->m_DebugEnabled = _DebugEnabled;
    }
  }

  bool EndoDebug::GetDebugEnabled()
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      return d->m_DebugEnabled;
    }
  }

  void EndoDebug::SetShowImagesInDebug(bool _ShowImagesInDebug)
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      d->m_ShowImagesInDebug = _ShowImagesInDebug;
    }
  }

  bool EndoDebug::GetShowImagesInDebug()
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      return d->m_ShowImagesInDebug;
    }
  }

  void EndoDebug::SetShowImagesTimeOut(size_t _ShowImagesTimeOut)
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      d->m_ShowImagesTimeOut = _ShowImagesTimeOut;
    }
  }

  bool EndoDebug::GetShowImagesTimeOut()
  {
    {
      itk::MutexLockHolder<itk::FastMutexLock> lock(*d->m_Mutex);
      return d->m_ShowImagesTimeOut;
    }
  }
}
