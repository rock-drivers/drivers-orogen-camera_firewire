require 'orocos'
require 'vizkit'
include Orocos

Orocos::CORBA.name_service = "asguard.local"
Orocos.initialize

left_camera = TaskContext.get 'camera_firewire_left'  
right_camera = TaskContext.get 'camera_firewire_right'

#left_camera.setIntAttrib(:ExposureValue, 150)
#left_camera.setIntAttrib(:GainValue, 16)
#left_camera.setIntAttrib(:WhitebalValueRed, 580)
#left_camera.setIntAttrib(:WhitebalValueBlue, 650)
#left_camera.setIntAttrib(:AcquisitionFrameCount, 200)
#left_camera.setEnumAttrib(:GammaToOn)
#left_camera.setEnumAttrib(:GammaToOff)
#left_camera.setEnumAttrib(:ExposureModeToAuto)
#left_camera.setEnumAttrib(:ExposureModeToManual)
#left_camera.setEnumAttrib(:ExposureModeToAutoOnce)
#left_camera.setEnumAttrib(:GainModeToAuto)
#left_camera.setEnumAttrib(:GainModeToManual)
#left_camera.setEnumAttrib(:WhitebalModeToAuto)
#left_camera.setEnumAttrib(:WhitebalModeToAutoOnce)
#left_camera.setEnumAttrib(:WhitebalModeToManual)
#left_camera.setDoubleAttrib(:FrameRate, 30)
#left_camera.setHDRValues(120,60,60,40)

#right_camera.setIntAttrib(:ExposureValue, 150)
#right_camera.setIntAttrib(:GainValue, 16)
#right_camera.setIntAttrib(:WhitebalValueRed, 580)
#right_camera.setIntAttrib(:WhitebalValueBlue, 650)
#right_camera.setIntAttrib(:AcquisitionFrameCount, 200)
#right_camera.setEnumAttrib(:GammaToOn)
#right_camera.setEnumAttrib(:GammaToOff)
#right_camera.setEnumAttrib(:ExposureModeToAuto)
#right_camera.setEnumAttrib(:ExposureModeToManual)
#right_camera.setEnumAttrib(:ExposureModeToAutoOnce)
#right_camera.setEnumAttrib(:GainModeToAuto)
#right_camera.setEnumAttrib(:GainModeToManual)
#right_camera.setEnumAttrib(:WhitebalModeToAuto)
#right_camera.setEnumAttrib(:WhitebalModeToAutoOnce)
#right_camera.setEnumAttrib(:WhitebalModeToManual)
#right_camera.setDoubleAttrib(:FrameRate, 30)
#right_camera.setHDRValues(120,60,60,40)

Vizkit.display left_camera.frame
Vizkit.display right_camera.frame

Vizkit.exec
