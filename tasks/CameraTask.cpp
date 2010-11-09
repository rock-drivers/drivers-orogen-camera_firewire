#include "CameraTask.hpp"
#include <dc1394/dc1394.h>

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

    std::cerr << "cam.isOpen = " << left_cam.isOpen() << std::endl;
    frame_size_t fs;
  
    fs.height = 480;
    fs.width = 640; //fs.width = 752;
    //cv::namedWindow("left",1);
    //if(stereo) cv::namedWindow("right",1);
    //cvMoveWindow("left",10,10);
    //if(stereo) cvMoveWindow("right",800,10);

std::cerr << "b" << std::endl;


    left_cam.setFrameSettings(fs, MODE_BAYER_RGGB, 8, false);
    left_cam.setAttrib(int_attrib::GainValue, 16);
    left_cam.setAttrib(enum_attrib::GammaToOn);
    left_cam.setAttrib(int_attrib::ExposureValue, 150);
    left_cam.setAttrib(int_attrib::WhitebalValueBlue, 580);
    left_cam.setAttrib(int_attrib::WhitebalValueRed, 650);
    left_cam.setAttrib(int_attrib::AcquisitionFrameCount, 200);
//    left_cam.setAttrib(enum_attrib::ExposureModeToManual);
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
//	right_cam.setAttrib(enum_attrib::ExposureModeToManual);
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


   //left_cam.retrieveFrame(left_frame,0);
   left_cam.grab(SingleFrame, 20);
   //left_cam.retrieveFrame(left_frame,0);
	  //  if(stereo)   right_cam.grab(camera::Continuously, 20);
     
usleep(1000000);

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
std::cerr << "now inside updateHook, time: " << t1 << " ms\n";

std::cerr << "fps estimate = " << 1.0/(t1/1000.0-lastUpdateTime/1000.0) << " fps\n";
lastUpdateTime = t1;

	
        if(stereo) std::cerr << "retrieving right..." << std::endl;
        if(stereo) right_camera.retrieveFrame(right_frame,0); 

	std::cerr << "retrieving left..." << std::endl;
        cvWaitKey(1);
	left_camera.retrieveFrame(left_frame,0);
	//if(stereo) std::cerr << "retrieving right..." << std::endl;
	//if(stereo) right_camera.retrieveFrame(right_frame,0);
	std::cerr << "showing image..." << std::endl;
	//imshow("left",left_frame.convertToCvMat());
	//if(stereo) imshow("right",right_frame.convertToCvMat());
	//cvWaitKey(2);
	std::cerr << "writing frame..." << std::endl;
	_left_frame.write(left_frame);
	if(stereo)_right_frame.write(right_frame);
        //left_camera.grab(SingleFrame, 20);
	//cvWaitKey(10);

   gettimeofday(&tim, NULL);
             t1=tim.tv_sec*1000.0+(tim.tv_usec/1000.0) -1.285582e12;
std::cerr << "leaving updatedHook at: " << t1 << " ms\n";

	
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

