#include <mitkStandardFileLocations.h>

#include <mitkConfig.h>
#include <itksys/SystemTools.hxx>


const std::string mitk::StandardFileLocations::FindFile(const char* filename, const char* pathInSourceDir)
{
  const char* mitkConf = itksys::SystemTools::GetEnv("MITKCONF");
  std::string xmlFileName;

  if (mitkConf)
  {
    // 1. look for MITKCONF environment variable
    xmlFileName = mitkConf;

    if ( mitkConf ) 
    {
      xmlFileName += "/";
      xmlFileName = itksys::SystemTools::ConvertToOutputPath(xmlFileName.c_str());
      xmlFileName += filename;
      
      if(itksys::SystemTools::FileExists(xmlFileName.c_str())) return xmlFileName;
    } 
  }
   
  // 2. look in the current working directory
  xmlFileName = filename;

  if(itksys::SystemTools::FileExists(xmlFileName.c_str())) return xmlFileName;

  // 3. use a source tree location from compile time
  xmlFileName = MITK_ROOT;
  if (pathInSourceDir)
  {
    xmlFileName += pathInSourceDir;
  }
  xmlFileName += '/';
  xmlFileName += filename;
  
  if(itksys::SystemTools::FileExists(xmlFileName.c_str())) return xmlFileName;

  return std::string("");
}
