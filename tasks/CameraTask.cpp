#include "CameraTask.hpp"
#include <dc1394/dc1394.h>
#include <rtt/NonPeriodicActivity.hpp>
#include "../../../drivers/camera_interface/src/CamTypes.h"
#include <opencv/highgui.h>

using namespace camera_firewire;
using namespace camera;
using namespace base::samples::frame;

RTT::NonPeriodicActivity* CameraTask::getNonPeriodicActivity()
{ return dynamic_cast< RTT::NonPeriodicActivity* >(getActivity().get()); }

CameraTask::CameraTask(std::string const& name)
    : CameraTaskBase(name)
{ }

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See CameraTask.hpp for more detailed
// documentation about them.

bool CameraTask::configureHook()
{
    stereo = true;
    dc1394_t *dc_device;

    //frame_size_t size(752,480);
    frame_size_t size(640,480);
    
    left_frame.init(size.width,size.height,3,MODE_BAYER_RGGB,false);
    if(stereo) right_frame.init(size.width,size.height,3,MODE_BAYER_RGGB,false);

    // create a new firewire bus device
    dc_device = dc1394_new();
    
    left_camera.setDevice(dc_device);
    left_camera.cleanup();
    
    right_camera.setDevice(dc_device);

    camera::CamInterface &left_cam = left_camera;
    camera::CamInterface &right_cam = right_camera;

    //find and display all cameras
    std::vector<CamInfo> cam_infos ;
    left_cam.listCameras(cam_infos);
    left_cam.open(cam_infos[0], Master);
    left_cam.setAttrib(camera::int_attrib::IsoSpeed, 400);
    left_cam.setAttrib(camera::double_attrib::FrameRate, 15);        

    if(stereo) 
    {
        if(!right_cam.open(cam_infos[1], Monitor))
        {
	    left_cam.setAttrib(camera::double_attrib::FrameRate, 15);
	    right_cam.setAttrib(camera::double_attrib::FrameRate, 15);
	    left_cam.close();
	    right_cam.close();
	    left_cam.open(cam_infos[0], Master);
	    right_cam.open(cam_infos[1], Monitor);
	    right_cam.setAttrib(camera::int_attrib::IsoSpeed, 400);
        }
    }

    frame_size_t fs;
    fs.height = 480;
    fs.width = 640; //fs.width = 752;
    
    left_cam.setFrameSettings(fs, MODE_BAYER_RGGB, 8, false);
    left_cam.setAttrib(int_attrib::GainValue, 16);
    left_cam.setAttrib(enum_attrib::GammaToOn);
    left_cam.setAttrib(int_attrib::ExposureValue, 150);
    left_cam.setAttrib(int_attrib::WhitebalValueBlue, 580);
    left_cam.setAttrib(int_attrib::WhitebalValueRed, 650);
    left_cam.setAttrib(int_attrib::AcquisitionFrameCount, 200);
    // left_cam.setAttrib(enum_attrib::ExposureModeToManual);
    left_cam.setAttrib(enum_attrib::ExposureModeToAuto);

    if(stereo)
    {
        right_cam.setFrameSettings(fs, MODE_BAYER_RGGB, 8, false);
        right_cam.setAttrib(int_attrib::GainValue, 16);
        right_cam.setAttrib(enum_attrib::GammaToOn);
        right_cam.setAttrib(int_attrib::ExposureValue,150);
        right_cam.setAttrib(int_attrib::WhitebalValueBlue, 580);
        right_cam.setAttrib(int_attrib::WhitebalValueRed, 650);
	right_cam.setAttrib(int_attrib::AcquisitionFrameCount, 200);
        // right_cam.setAttrib(enum_attrib::ExposureModeToManual);
        right_cam.setAttrib(enum_attrib::ExposureModeToAuto);
    }

    timeval ts, te, tcurr, tprev;
    gettimeofday(&ts,NULL);
    gettimeofday(&tcurr,NULL);
    lastUpdateTime = 0;

    left_cam.setAttrib(camera::double_attrib::FrameRate, 60);
    if(stereo) right_cam.setAttrib(camera::double_attrib::FrameRate, 60);

    left_camera.clearBuffer();
    if(stereo) right_camera.clearBuffer();

    left_cam.grab(SingleFrame, 20);
       
    //usleep(1000000);
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

    if(stereo) right_camera.retrieveFrame(right_frame,0); 
    left_camera.retrieveFrame(left_frame,0);
    _left_frame.write(left_frame);
    if(stereo)_right_frame.write(right_frame);
   
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
