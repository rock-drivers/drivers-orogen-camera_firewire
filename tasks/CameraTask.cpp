#include "CameraTask.hpp"
#include <dc1394/dc1394.h>
#include <rtt/Activity.hpp>
#include <camera_interface/CamTypes.h>
#include <camera_interface/CamInfoUtils.h>
#include <camera_firewire/CamFireWire.h>
#include <Timestamper.hpp>

#include <rtt/extras/FileDescriptorActivity.hpp>

using namespace camera_firewire;
using namespace camera;
using namespace base::samples::frame;

//RTT::Activity* CameraTask::getActivity()
//{ return dynamic_cast< RTT::Activity* >(getActivity().get()); }

CameraTask::CameraTask(std::string const& name)
    : CameraTaskBase(name)
    , camera(new camera::CamFireWire)
    , timestamper(0)
{ }

CameraTask::~CameraTask()
{
    delete camera;
    delete timestamper;
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See CameraTask.hpp for more detailed
// documentation about them.

bool CameraTask::configureHook()
{
    timestamper = new aggregator::Timestamper<base::samples::frame::Frame>
	(base::Time::fromSeconds(0.6),
	 base::Time::fromSeconds(0),
	 base::Time::fromSeconds(1.0/_frame_rate),
	 base::Time::fromSeconds(20),
	 base::Time::fromSeconds(1.0/_frame_rate));

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
    camera->setAttrib(camera::int_attrib::IsoSpeed, 400);
    
    camera->setAttrib(camera::double_attrib::FrameRate, 15);        
    frame_size_t fs;
    fs.height = _height;
    fs.width = _width;

    camera->setFrameSettings(fs, MODE_BAYER_RGGB, 8, false);
    camera->setAttrib(int_attrib::GainValue, 16);
    camera->setAttrib(enum_attrib::GammaToOn);
    camera->setAttrib(int_attrib::ExposureValue, 150);//_exposure_value);
    std::cout << "set exposure to " << _exposure_value << std::endl;
    camera->setAttrib(int_attrib::WhitebalValueBlue, 580);
    camera->setAttrib(int_attrib::WhitebalValueRed, 650);
    camera->setAttrib(int_attrib::AcquisitionFrameCount, 200);
    camera->setAttrib(enum_attrib::ExposureModeToManual);
    //camera->setAttrib(enum_attrib::ExposureModeToAuto);

    camera->setAttrib(camera::double_attrib::FrameRate, 30);
    camera->grab(SingleFrame, 20);
       
    //usleep(1000000);
    std::cerr << "end of configureHook" << std::endl;
    return true;
}

bool CameraTask::startHook()
{
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

void CameraTask::updateHook()
{
    if (_trigger_timestamp.connected())
	timestamper->enableSynchronization();

    base::Time trigger_ts;
    while (_trigger_timestamp.read(trigger_ts) == RTT::NewData)
	timestamper->pushReference(trigger_ts);

    aggregator::Timestamper<base::samples::frame::Frame>::ItemIterator
      frame_it = timestamper->getSpareItem();

    frame_it->item.init(_width,_height,3,MODE_BAYER_RGGB,false);

    if (camera->retrieveFrame(frame_it->item, 10))
    {
	frame_it->time = frame_it->item.time;
	timestamper->pushItem(frame_it);
    }
    else
    {
	timestamper->putSpareItem(frame_it);
        exception(IO_ERROR);
        return;
    }

    while (timestamper->itemAvailable(base::Time::now())) {
	timestamper->item().item.time = timestamper->item().time;
	_frame.write(timestamper->item().item);
	timestamper->popItem();
    }
}

// void CameraTask::errorHook()
// {
// }

void CameraTask::stopHook()
{
    RTT::extras::FileDescriptorActivity* fd_activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
        fd_activity->clearAllWatches();
}

void CameraTask::cleanupHook()
{
    delete timestamper;
    timestamper = 0;
}
