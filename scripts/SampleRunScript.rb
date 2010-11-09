#! /usr/bin/env ruby
# -*- coding: utf-8 -*-

require 'orocos'
include Orocos
Orocos.initialize

# setup the environment so that ruby can find the test deployment
ENV['PKG_CONFIG_PATH'] = "#{File.expand_path("..", File.dirname(__FILE__))}/build:#{ENV['PKG_CONFIG_PATH']}"

Orocos.run 'camera_test' do
  camera = TaskContext.get 'camera'  

  canserial = TaskContext.get('canserial')

  can = TaskContext.get('can')

  can.device = '/dev/can0'
  can.checkBusOkCount = 1
  
  can.watch('canserial', 0, 0x7F0)

  can.canserial.connect_to(canserial.can_in, :type => :buffer, :size => 20)
  can.wcanserial.connect_to(canserial.can_out, :type => :buffer, :size => 20)

  Orocos.log_all_ports

  can.configure
  can.start

  sleep 1

  camera.configure  	
  camera.start

sleep 1

  canserial.configure
  canserial.start

# Pin_A1 => 4
# Pin_B1 => 5
# Pin_C1 => 6
# Pin_D1 => 7
# RisingFallingEdge => 2
# PullNone => 0
  canserial.watch_gpio("A1",4,2,0)
  canserial.watch_gpio("B1",5,2,0)
  canserial.watch_gpio("C1",6,2,0)
  canserial.watch_gpio("D1",7,2,0) # to have it in the logs

# Pin_A2 => 8
# Pin_B2 => 9
# Pin_C2 => 10
  canserial.watch_gpio("A2",8,2,0)
  canserial.watch_gpio("B2",9,2,0)
  canserial.watch_gpio("C2",10,2,0)

# Pin_B2 => 14
# FallingEdge => 1
# PullUp => 1
  canserial.watch_gpio("B3",13,1,1)


  #viewer = TaskContext.get 'camera_viewer'
  #viewer.frame.connect_to camera.frame

  #viewer.start

  for i in (1..1000000)
	sleep 0.01
  end 

  STDERR.puts "shutting down"
end

