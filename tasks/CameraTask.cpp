#include "CameraTask.hpp"
#include <dc1394/dc1394.h>
#include <rtt/Activity.hpp>
#include <camera_interface/CamTypes.h>
#include <camera_interface/CamInfoUtils.h>
#include <camera_firewire/CamFireWire.h>
#include <aggregator/TimestampEstimator.hpp>

#include <rtt/extras/FileDescriptorActivity.hpp>

using namespace camera_firewire;
using namespace camera;
using namespace base::samples::frame;

//RTT::Activity* CameraTask::getActivity()
//{ return dynamic_cast< RTT::Activity* >(getActivity().get()); }

CameraTask::CameraTask(std::string const& name)
    : CameraTaskBase(name)
    , camera(new camera::CamFireWire)
    , timestampEstimator(0)
{
    _grab_mode = SingleFrame;
    _frame_buffer_size = 20;
    _fps = 5;
    _exposure = 150;
    _exposure_mode = "manual";
    _gain = 16;
    _camera_format = MODE_BAYER_RGGB;
    _whitebalance_blue = 580;
    _whitebalance_red = 650;
}

CameraTask::~CameraTask()
{
    delete camera;
    delete timestampEstimator;
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See CameraTask.hpp for more detailed
// documentation about them.

bool CameraTask::configureHook()
{
    if (! CameraTaskBase::configureHook())
      return false;

    timestampEstimator = new aggregator::TimestampEstimator
	(base::Time::fromSeconds(20),
	 base::Time::fromSeconds(1.0/_fps), 2);

    std::cerr << "requested camera id: " << _camera_id << std::endl;

    dc1394_t *dc_device;

    std::cerr << "creating new bus device...";
    // create a new firewire bus device
    dc_device = dc1394_new();
    std::cerr << "done." << std::endl;

    camera->setDevice(dc_device);

    //find and display all cameras
    std::vector<CamInfo> cam_infos ;
    camera->listCameras(cam_infos);
    showCamInfos(cam_infos);

    std::string cam_id = _camera_id;

    for(unsigned int i = 0 ; i<cam_infos.size() ; i++)
    {
      std::cerr << "cam's uid is " << cam_infos[i].unique_id << " and desired id is " << _camera_id << std::endl;
      if(cam_infos[i].unique_id == strtoul(cam_id.c_str(),NULL,0))
          if(!camera->open(cam_infos[i],Master))
          {
              std::cerr << "Failed to open Camera" << std::endl;
              return false;
          }
    }
    cam_interface = camera;
    
    camera->setAttrib(camera::int_attrib::IsoSpeed, 400);
    camera->setAttrib(enum_attrib::GammaToOn);
    camera->setAttrib(int_attrib::AcquisitionFrameCount, 200);
       
    //usleep(1000000);
    std::cerr << "end of configureHook" << std::endl;
    return true;
}

bool CameraTask::startHook()
{
    if (! CameraTaskBase::startHook())
      return false;
      
    RTT::extras::FileDescriptorActivity* fd_activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
    {
        std::cerr << "using FD activity !" << std::endl;
        std::cerr << "  FD=" << camera->getFileDescriptor() << std::endl;
        fd_activity->watch(camera->getFileDescriptor());
    }

    return true;
}

// void CameraTask::updateHook()
// {
// }

// void CameraTask::errorHook()
// {
// }

void CameraTask::stopHook()
{
    TaskBase::stopHook();
    RTT::extras::FileDescriptorActivity* fd_activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
        fd_activity->clearAllWatches();
}

void CameraTask::cleanupHook()
{
    TaskBase::cleanupHook();
    delete timestampEstimator;
    timestampEstimator = 0;
}

void CameraTask::onRetrieveNewFrame(base::samples::frame::Frame & frame)
{
    base::Time trigger_ts;
    while (_trigger_timestamp.read(trigger_ts) == RTT::NewData)
	    timestampEstimator->updateReference(trigger_ts);

    frame.time = timestampEstimator->update(frame.time);
}
