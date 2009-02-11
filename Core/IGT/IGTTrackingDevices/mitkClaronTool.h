#ifndef MITKCLARONTOOL_H_HEADER_INCLUDED_
#define MITKCLARONTOOL_H_HEADER_INCLUDED_

#include <mitkClaronInterface.h>
#include <mitkInternalTrackingTool.h>
#include <itkFastMutexLock.h>

namespace mitk
{
  class ClaronTool : public InternalTrackingTool
  {
  public:

    mitkClassMacro(ClaronTool, InternalTrackingTool);
    itkNewMacro(Self);

    /**
    * \brief Loads a tool calibration file. Without this file the tool can not be tracked!
    */
    bool LoadFile(const char* filename);
    /**
    * \brief Loads a tool calibration file. Without this file the tool can not be tracked!
    */
    bool LoadFile(std::string filename);  

    const char* GetFile();
   
    /**
    * \brief Sets the handle of the tool.
    * @param handle The new handle of the tool.
    */
    void SetToolHandle (claronToolHandle handle);

    /**
    * @return Returns the calibration name which is used to identify the tool.
    */
    std::string GetCalibrationName();

    /**
    * \brief Sets the calibration name of the tool. Be careful, only use this method if you know what you are doing.
    * If you want to change the tool name use the method setToolName instead!
    */
    void SetCalibrationName(std::string name);

    /**
    * @return Returns the toolhandle of the tool.
    */
    claronToolHandle GetToolHandle();
   
  protected:
    ClaronTool();
    virtual ~ClaronTool();

    claronToolHandle m_ToolHandle;
 
    std::string m_CalibrationName;
    ClaronInterface* m_Device;
    std::string m_Filename;
  };
}//mitk
#endif // MITKCLARONTOOL_H_HEADER_INCLUDED_ 
