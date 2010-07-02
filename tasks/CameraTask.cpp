#include <highgui.h>
#include <cv.h>
#include "CameraTask.hpp"

#include <rtt/NonPeriodicActivity.hpp>
#include "../../../drivers/camera_interface/src/CamTypes.h"






using namespace camera_firewire;
using namespace camera;
using namespace base::samples::frame;

RTT::NonPeriodicActivity* CameraTask::getNonPeriodicActivity()
{ return dynamic_cast< RTT::NonPeriodicActivity* >(getActivity().get()); }


CameraTask::CameraTask(std::string const& name)
    : CameraTaskBase(name)
{
}





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
    
    std::cerr << 1;
    
    // create a new firewire bus device
    dc_device = dc1394_new();
    
        std::cerr << 2;

    

    std::cerr << "left cam created\n";
    left_camera.setDevice(dc_device);
    left_camera.cleanup();
    

    std::cerr << "right cam created\n";
    right_camera.setDevice(dc_device);


    //left_camera.cleanup();
    camera::CamInterface &left_cam = left_camera;
        std::cerr << "left cam linked\n";
    camera::CamInterface &right_cam = right_camera;




    std::cerr << "interfaces created";
    
    //find and display all cameras
    std::vector<CamInfo> cam_infos ;
    left_cam.listCameras(cam_infos);
        std::cerr << "c0" << std::endl;   
    std::cerr << "cam.isOpen = " << left_cam.isOpen() << std::endl;
        std::cerr << "c1" << std::endl;   
    cvWaitKey(100);

    std::cerr << "c1" << std::endl;   

    left_cam.open(cam_infos[0], Master);
    std::cerr << "c2" << std::endl;
    left_cam.setAttrib(camera::int_attrib::IsoSpeed, 400);
    std::cerr << "c3" << std::endl;
	left_cam.setAttrib(camera::double_attrib::FrameRate, 15);        
    std::cerr << "c4" << std::endl;

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

    std::cerr << "c" << std::endl;

    cvWaitKey(100);
    std::cerr << "cam.isOpen = " << left_cam.isOpen() << std::endl;
    frame_size_t fs;
  
    fs.height = 480;
    fs.width = 640; //fs.width = 752;
    cv::namedWindow("left",1);
    if(stereo) cv::namedWindow("right",1);
    cvMoveWindow("left",10,10);
    if(stereo) cvMoveWindow("right",800,10);

std::cerr << "b" << std::endl;

    cvWaitKey(100);

    left_cam.setFrameSettings(fs, MODE_BAYER_RGGB, 8, false);
    left_cam.setAttrib(int_attrib::GainValue, 16);
    left_cam.setAttrib(enum_attrib::GammaToOn);
    left_cam.setAttrib(int_attrib::ExposureValue, 90);
    left_cam.setAttrib(int_attrib::WhitebalValueBlue, 580);
    left_cam.setAttrib(int_attrib::WhitebalValueRed, 650);
    left_cam.setAttrib(int_attrib::AcquisitionFrameCount, 200);
    left_cam.setAttrib(enum_attrib::ExposureModeToManual);
//left_cam.setAttrib(enum_attrib::ExposureModeToAuto);

    if(stereo)
    {
        right_cam.setFrameSettings(fs, MODE_BAYER_RGGB, 8, false);
        right_cam.setAttrib(int_attrib::GainValue, 16);
        right_cam.setAttrib(enum_attrib::GammaToOn);
        right_cam.setAttrib(int_attrib::ExposureValue,90);
        right_cam.setAttrib(int_attrib::WhitebalValueBlue, 580);
        right_cam.setAttrib(int_attrib::WhitebalValueRed, 650);
	right_cam.setAttrib(int_attrib::AcquisitionFrameCount, 200);
	right_cam.setAttrib(enum_attrib::ExposureModeToManual);

    }

    cvWaitKey(50);
    timeval ts, te, tcurr, tprev;
    
    gettimeofday(&ts,NULL);
    gettimeofday(&tcurr,NULL);

    left_cam.setAttrib(camera::double_attrib::FrameRate, 30);
    if(stereo) right_cam.setAttrib(camera::double_attrib::FrameRate, 30);

    
    left_camera.clearBuffer();
    if(stereo) right_camera.clearBuffer();

   left_cam.grab(SingleFrame, 20);
	  //  if(stereo)   right_cam.grab(camera::Continuously, 20);
     return true;
 }
 bool CameraTask::startHook()
 {
     return true;
 }

void CameraTask::updateHook()
 {

	std::cerr << "retrieving..." << std::endl;
        cvWaitKey(1);
	left_camera.retrieveFrame(left_frame,0);
	if(stereo) right_camera.retrieveFrame(right_frame,0);
	imshow("left",left_frame.convertToCvMat());
	if(stereo) imshow("right",right_frame.convertToCvMat());
	cvWaitKey(2);
	_left_frame.write(left_frame);
	_right_frame.write(right_frame);

	
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

