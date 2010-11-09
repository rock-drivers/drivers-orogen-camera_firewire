#ifndef CAMERA_FIREWIRE_CAMERATASK_TASK_HPP
#define CAMERA_FIREWIRE_CAMERATASK_TASK_HPP

#include "camera_firewire/CameraTaskBase.hpp"
#include <libraw1394/raw1394.h>
#include "camera_firewire/CamFireWire.h"
#include <highgui.h>
#include <cv.h>
//#include <dc1394/camera.h>

namespace RTT
{
    class NonPeriodicActivity;
}


namespace camera_firewire {
    class CameraTask : public CameraTaskBase
    {
	friend class CameraTaskBase;
    protected:
    
    

    public:
        CameraTask(std::string const& name = "camera_firewire::CameraTask");

        RTT::NonPeriodicActivity* getNonPeriodicActivity();

	bool stereo;
	base::samples::frame::Frame left_frame;
        base::samples::frame::Frame right_frame;
	double lastUpdateTime;

    camera::CamFireWire left_camera;
    camera::CamFireWire right_camera;
	


        /** This hook is called by Orocos when the state machine transitions
         * from PreOperational to Stopped. If it returns false, then the
         * component will stay in PreOperational. Otherwise, it goes into
         * Stopped.
         *
         * It is meaningful only if the #needs_configuration has been specified
         * in the task context definition with (for example):
         *
         *   task_context "TaskName" do
         *     needs_configuration
         *     ...
         *   end
         */
         bool configureHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to Running. If it returns false, then the component will
         * stay in Stopped. Otherwise, it goes into Running and updateHook()
         * will be called.
         */
         bool startHook();

        /** This hook is called by Orocos when the component is in the Running
         * state, at each activity step. Here, the activity gives the "ticks"
         * when the hook should be called. See README.txt for different
         * triggering options.
         *
         * The warning(), error() and fatal() calls, when called in this hook,
         * allow to get into the associated RunTimeWarning, RunTimeError and
         * FatalError states. 
         *
         * In the first case, updateHook() is still called, and recovered()
         * allows you to go back into the Running state.  In the second case,
         * the errorHook() will be called instead of updateHook() and in the
         * third case the component is stopped and resetError() needs to be
         * called before starting it again.
         *
         */
         void updateHook();
        

        /** This hook is called by Orocos when the component is in the
         * RunTimeError state, at each activity step. See the discussion in
         * updateHook() about triggering options.
         *
         * Call recovered() to go back in the Runtime state.
         */
        // void errorHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Running to Stopped after stop() has been called.
         */
         void stopHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to PreOperational, requiring the call to configureHook()
         * before calling start() again.
         */
         void cleanupHook();
    };
}

#endif

