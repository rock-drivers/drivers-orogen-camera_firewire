#! /usr/bin/env ruby

require 'vizkit'
require 'orocos'
require 'optparse'
include Orocos

gain = nil
exposure = nil
display = false

parser = OptionParser.new do |opt|
    opt.banner = "camera-config.rb [--host hostname] [--display] [--gain gain] [--exposure exposure]"
    opt.on('--host=HOSTNAME', String, "the corba nameserver we should contact") do |name|
        Orocos::CORBA.name_service = name
    end
    opt.on('--gain=gain', Integer, "gain value for the cameras") do |value|
	gain = value
    end
    opt.on('--exposure=exposure', Integer, "exposure value for the cameras") do |value|
	exposure = value
    end
    opt.on('--display') do
	display = true
    end
    opt.on('--help') do
        puts parser
        exit 0
    end
end

remaining = parser.parse(ARGV)

Orocos.initialize

left_camera = TaskContext.get 'camera_left'  
right_camera = TaskContext.get 'camera_right'

if gain
    left_camera.setIntAttrib(:GainValue, gain)
    right_camera.setIntAttrib(:GainValue, gain)
end

if exposure
    left_camera.setIntAttrib(:ExposureValue, exposure)
    right_camera.setIntAttrib(:ExposureValue, exposure)
end

if display
    Vizkit.display left_camera.frame
    Vizkit.display right_camera.frame

    Vizkit.exec
end

