#include "CameraTask.hpp"
#include <dc1394/dc1394.h>
#include <rtt/Activity.hpp>
#include "../../../drivers/camera_interface/src/CamTypes.h"
#include <opencv/highgui.h>
#include "../../../drivers/camera_interface/src/CamInfoUtils.h"

using namespace camera_firewire;
using namespace camera;
using namespace base::samples::frame;

//RTT::Activity* CameraTask::getActivity()
//{ return dynamic_cast< RTT::Activity* >(getActivity().get()); }

CameraTask::CameraTask(std::string const& name)
    : CameraTaskBase(name)
{ }

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See CameraTask.hpp for more detailed
// documentation about them.

bool CameraTask::configureHook()
{
    

    std::cerr << "requested camera id: " << _camera_id << std::endl;

    dc1394_t *dc_device;

    //frame_size_t size(752,480);
    frame_size_t size(640,480);
    
	frame.init(size.width,size.height,3,MODE_BAYER_RGGB,false);
	
    std::cerr << "creating new bus device...";
    // create a new firewire bus device
    dc_device = dc1394_new();
    std::cerr << "done." << std::endl;

	camera.setDevice(dc_device);
    //camera.cleanup();
    
    camera::CamInterface &cam = camera;

    //find and display all cameras
    std::vector<CamInfo> cam_infos ;
    cam.listCameras(cam_infos);
    showCamInfos(cam_infos);

    std::string cam_id = _camera_id;

    for(int i = 0 ; i<cam_infos.size() ; i++)
    {
      std::cerr << "cam's uid is " << cam_infos[i].unique_id << " and desired id is " << _camera_id << std::endl;
      if(cam_infos[i].unique_id == strtoul(cam_id.c_str(),NULL,0))
        cam.open(cam_infos[i],Master);
    }
    std::cerr << 1;
    cam.setAttrib(camera::int_attrib::IsoSpeed, 400);
    
    cam.setAttrib(camera::double_attrib::FrameRate, 15);        
    std::cerr << 2;
    frame_size_t fs;
    fs.height = 480;
    fs.width = 640; //fs.width = 752;
    
    cam.setFrameSettings(fs, MODE_BAYER_RGGB, 8, false);
    cam.setAttrib(int_attrib::GainValue, 16);
    cam.setAttrib(enum_attrib::GammaToOn);
    cam.setAttrib(int_attrib::ExposureValue, 150);
    cam.setAttrib(int_attrib::WhitebalValueBlue, 580);
    cam.setAttrib(int_attrib::WhitebalValueRed, 650);
    cam.setAttrib(int_attrib::AcquisitionFrameCount, 200);
    //cam.setAttrib(enum_attrib::ExposureModeToManual);
    cam.setAttrib(enum_attrib::ExposureModeToAuto);
    std::cerr << 3;
    timeval ts, te, tcurr, tprev;
    gettimeofday(&ts,NULL);
    gettimeofday(&tcurr,NULL);
    lastUpdateTime = 0;
    std::cerr << 4;
    cam.setAttrib(camera::double_attrib::FrameRate, 30);

    camera.clearBuffer();
    std::cerr << 5;
    cam.grab(SingleFrame, 20);
       
    //usleep(1000000);
    std::cerr << "end of configureHook" << std::endl;
    return true;
}

bool CameraTask::startHook()
{
    getActivity()->trigger();
    return true;
}

void CameraTask::updateHook()
{
    timeval tim;
    gettimeofday(&tim, NULL);
    double t1=tim.tv_sec*1000.0+(tim.tv_usec/1000.0) -1.285582e12;
    std::cerr << "fps estimate = " << 1.0/(t1/1000.0-lastUpdateTime/1000.0) << " fps\n";
    lastUpdateTime = t1;

    std::cerr << "retrieving...";
    camera.retrieveFrame(frame,0);
    std::cerr << "done" << std::endl;
    _frame.write(frame);
   
    getActivity()->trigger();
}

// void CameraTask::errorHook()
// {
// }

void CameraTask::stopHook()
{
}

void CameraTask::cleanupHook()
{
}
