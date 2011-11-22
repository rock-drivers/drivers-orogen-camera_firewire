#include "CameraTask.hpp"
#include <dc1394/camera.h>
#include <camera_interface/CamTypes.h>
#include <camera_interface/CamInfoUtils.h>
#include <camera_firewire/CamFireWire.h>
#include <aggregator/TimestampEstimator.hpp>

using namespace camera_firewire;
using namespace camera;
using namespace base::samples::frame;

CameraTask::CameraTask(std::string const& name)
    : CameraTaskBase(name)
    , camera(new camera::CamFireWire)
    , timestampEstimator(0)
{
    // default properties
    _grab_mode = SingleFrame;
    _frame_buffer_size = 20;
    _fps = 30;
    _exposure = 150;
    _exposure_mode = "manual";
    _gain = 16;
    _camera_format = MODE_BAYER_BGGR;
    _whitebalance_blue = 580;
    _whitebalance_red = 650;
    _gamma = false;
    _acquisition_frame_count = 200;
    _whitebalance_mode = "manual";
    _trigger_mode = "fixed";
    _frame_start_trigger_event = "EdgeRising";
}

CameraTask::~CameraTask()
{
    if(camera)
    {
        delete camera;
        camera = NULL;
    }
    if(timestampEstimator)
    {
        delete timestampEstimator;
        timestampEstimator = NULL;
    }
}

/**
 * Callback function, will triggered if a new frame was retrieved.
 * 
 * @param frame the new frame
 */
void CameraTask::onRetrieveNewFrame(base::samples::frame::Frame & frame)
{
    base::Time trigger_ts;
    while (_trigger_timestamp.read(trigger_ts) == RTT::NewData)
        timestampEstimator->updateReference(trigger_ts);

    frame.time = timestampEstimator->update(frame.time);
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See CameraTask.hpp for more detailed
// documentation about them.

bool CameraTask::configureHook()
{
    if (! CameraTaskBase::configureHook())
      return false;

    if (_fps.value() != 0)
    {
        timestampEstimator = new aggregator::TimestampEstimator
            (base::Time::fromSeconds(20), base::Time::fromSeconds(1.0/_fps), 2);
    }
    else
    {
        timestampEstimator = new aggregator::TimestampEstimator
            (base::Time::fromSeconds(20), 2);
    }

    RTT::log(RTT::Info) << "requested firewire camera id: " << _camera_id <<  RTT::endlog();

    dc1394_t *dc_device;
    RTT::log(RTT::Info) << "creating new dc1394 bus device...";
    // create a new firewire bus device
    dc_device = dc1394_new();
    RTT::log(RTT::Info) << "done." <<  RTT::endlog();

    camera->setDevice(dc_device);

    //find and display all cameras
    std::vector<CamInfo> cam_infos ;
    int num_cams = camera->listCameras(cam_infos);
    if(num_cams <= 0){
        RTT::log(RTT::Error) << "Unable to find any firewire camera! (Abort)" <<  RTT::endlog();
        return false;
    }
    showCamInfos(cam_infos);

    std::string cam_id = _camera_id;

    bool opened = false;
    for(unsigned int i = 0 ; i<cam_infos.size() ; i++)
    {
      RTT::log(RTT::Info) << "cam's uid is " << cam_infos[i].unique_id << " and desired id is " << _camera_id <<  RTT::endlog();
      if(cam_infos[i].unique_id == strtoul(cam_id.c_str(),NULL,0))
      {
          if(camera->open(cam_infos[i],Master))
	  {
	      opened = true;
	  }
	  else
          {
              RTT::log(RTT::Error) << "Failed to open firewire Camera" <<  RTT::endlog();
              return false;
          }
      }
    }
    if( !opened )
    {
	RTT::log(RTT::Error) << "Did not find camera with id " << cam_id <<  RTT::endlog();
	return false;
    }
    
    cam_interface = camera;
    
    camera->setAttrib(int_attrib::IsoSpeed, 400);
    
    if(camera->isAttribAvail(int_attrib::HDRValue))
        setHDRValues(_hdr_voltage_1, _hdr_voltage_2, _hdr_voltage_3, _hdr_voltage_4);
    else
        RTT::log(RTT::Info) << "HDR mode is not supported by the camera" << RTT::endlog();
       
    RTT::log(RTT::Info) << "end of configureHook" << RTT::endlog();
    return true;
}

//bool CameraTask::startHook()
//{
//    if (! CameraTaskBase::startHook())
//      return false;
//    return true;
//}

// void CameraTask::updateHook()
// {
// }

// void CameraTask::errorHook()
// {
// }

//void CameraTask::stopHook()
//{
//    CameraTaskBase::stopHook();
//}

void CameraTask::cleanupHook()
{
    CameraTaskBase::cleanupHook();
    if(timestampEstimator)
    {
        delete timestampEstimator;
        timestampEstimator = NULL;
    }
}

bool CameraTask::setHDRValues(int hdr_voltage_1, int hdr_voltage_2, int hdr_voltage_3, int hdr_voltage_4)
{
    if(hdr_voltage_1 < 0 || hdr_voltage_1 > 255 || hdr_voltage_2 < 0 || hdr_voltage_2 > 255 
        || hdr_voltage_3 < 0 || hdr_voltage_3 > 255 || hdr_voltage_4 < 0 || hdr_voltage_4 > 255)
    {
        RTT::log(RTT::Error) << "The HDR voltage values have to be between 0..255! (Abort)" << RTT::endlog();
        return false;
    }
    
    u_int32_t value = 0;
    value = (value & 0xFFFFFF00UL) | (((u_int32_t)hdr_voltage_1) & 0xFFUL);
    value = (value & 0xFFFF00FFUL) | ((((u_int32_t)hdr_voltage_2) & 0xFFUL) << 8);
    value = (value & 0xFF00FFFFUL) | ((((u_int32_t)hdr_voltage_3) & 0xFFUL) << 16);
    value = (value & 0x00FFFFFFUL) | ((((u_int32_t)hdr_voltage_4) & 0xFFUL) << 24);
    
    try
    {
        cam_interface->setAttrib(int_attrib::HDRValue, (int)value);
    }
    catch(std::runtime_error e)
    {
        return false;
    }
    return true;
}