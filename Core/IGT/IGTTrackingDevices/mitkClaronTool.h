#ifndef MITKCLARONTOOL_H_HEADER_INCLUDED_
#define MITKCLARONTOOL_H_HEADER_INCLUDED_

#include <mitkClaronInterface.h>
#include <mitkTrackingTool.h>
#include <itkFastMutexLock.h>

namespace mitk
{
  class ClaronTool : public TrackingTool
  {
  public:

    mitkClassMacro(ClaronTool, TrackingTool);
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
    * \brief Writes the positions of the tooltip in the 3 variables. If the tool is not found the position is set to 0:0:0.
    */
    void GetPosition (float &x, float &y, float &z) const;

    /**
    * \brief Writes the positions of the tooltip in the position object. If the tool is not found the position is set to 0:0:0.
    */
    void GetPosition(mitk::Point3D& position) const;


    /**
    * \brief Sets the position of the Tooltip
    */
    void SetPosition (float x, float y, float z);

    /**
    * \brief Writes the quarternions of the tooltip in the 4 variables. If the tool is not found the quarternions are set to 0:0:0:0.
    */
    void GetQuaternion (float &q0, float &qx, float &qy, float &qz) const;

    /**
    * \brief Writes the quarternions of the tooltip in the quarternion object. If the tool is not found the quarternions are set to 0:0:0:0.
    */
    void GetQuaternion(mitk::Quaternion& orientation) const;


    /**
    * \brief Sets the quaternions of the tool
    */
    void SetQuaternion (float q0, float q1, float q2, float q3);

    /**
    * \brief Sets the name.
    */
    void SetToolName (std::string name);

    /**
    * @return Returns the name of the tool.
    */
    std::string GetToolName();

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
    * If you want to change the toolname use the method setToolName instead!
    */
    void SetCalibrationName(std::string name);

    /**
    * @return Returns the toolhandle of the tool.
    */
    claronToolHandle GetToolHandle();

    //NICHT IMPLEMENTIERTE GEERBTE FUNKTIONEN
    virtual bool Enable();                     
    virtual bool Disable();                      
    virtual bool IsEnabled() const;             
    virtual bool IsDataValid() const;           
    virtual float GetTrackingError() const;     

  protected:

    ClaronTool(void);

    virtual ~ClaronTool(void);

    claronToolHandle toolHandle;

    float m_Position[3];
    float m_Quaternion[4];
    std::string m_calibrationName;
    ClaronInterface* theDevice;
    std::string m_filename;
    bool m_Enabled;
    bool m_DataValid;
  };
}//mitk
#endif // MITKCLARONTOOL_H_HEADER_INCLUDED_ 
