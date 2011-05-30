#include "CameraTask.hpp"
#include <dc1394/dc1394.h>
#include <rtt/Activity.hpp>
#include <camera_interface/CamTypes.h>
#include <camera_interface/CamInfoUtils.h>
#include <camera_firewire/CamFireWire.h>
#include <TimestampSynchronizer.hpp>

#include <rtt/extras/FileDescriptorActivity.hpp>

using namespace camera_firewire;
using namespace camera;
using namespace base::samples::frame;

//RTT::Activity* CameraTask::getActivity()
//{ return dynamic_cast< RTT::Activity* >(getActivity().get()); }

CameraTask::CameraTask(std::string const& name)
    : CameraTaskBase(name)
    , camera(new camera::CamFireWire)
    , timestamp_synchronizer(0)
{ }

CameraTask::~CameraTask()
{
    delete camera;
    delete timestamp_synchronizer;
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See CameraTask.hpp for more detailed
// documentation about them.

bool CameraTask::configureHook()
{
    timestamp_synchronizer = new aggregator::TimestampSynchronizer<base::samples::frame::Frame>
	(base::Time::fromSeconds(0.6),
	 base::Time::fromSeconds(0),
	 base::Time::fromSeconds(1.0/_frame_rate),
	 base::Time::fromSeconds(20),
	 base::Time::fromSeconds(1.0/_frame_rate));

    std::cerr << "requested camera id: " << _camera_id << std::endl;

    dc1394_t *dc_device;

    //frame_size_t size(752,480);
    frame_size_t size(640,480);
    
    frame.init(size.width,size.height,3,MODE_BAYER_RGGB,false);
	
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

    for(int i = 0 ; i<cam_infos.size() ; i++)
    {
      std::cerr << "cam's uid is " << cam_infos[i].unique_id << " and desired id is " << _camera_id << std::endl;
      if(cam_infos[i].unique_id == strtoul(cam_id.c_str(),NULL,0))
        camera->open(cam_infos[i],Master);
    }
    std::cerr << 1;
    camera->setAttrib(camera::int_attrib::IsoSpeed, 400);
    
    camera->setAttrib(camera::double_attrib::FrameRate, 15);        
    std::cerr << 2;
    frame_size_t fs;
    fs.height = 480;
    fs.width = 640; //fs.width = 752;
    
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

    std::cerr << 3;
    camera->setAttrib(camera::double_attrib::FrameRate, 30);
    camera->clearBuffer();
    std::cerr << 5;
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
    base::Time trigger_ts;
    while (_trigger_timestamp.read(trigger_ts) == RTT::NewData)
	timestamp_synchronizer->pushReference(trigger_ts);

    if (camera->retrieveFrame(frame, 0)) {
	if (timestamp_synchronizer->getTimeFor(frame.time))
	    _frame.write(frame);
	else
	    timestamp_synchronizer->pushItem(frame,frame.time);
    }

    if (timestamp_synchronizer->itemAvailable(base::Time::now())) {
	timestamp_synchronizer->item().item.time = timestamp_synchronizer->item().time;
	_frame.write(timestamp_synchronizer->item().item);
	timestamp_synchronizer->popItem();
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
    delete timestamp_synchronizer;
    timestamp_synchronizer = 0;
}
