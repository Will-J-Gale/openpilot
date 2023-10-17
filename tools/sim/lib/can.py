#!/usr/bin/env python3
import cereal.messaging as messaging
from opendbc.can.packer import CANPacker
from opendbc.can.parser import CANParser
from openpilot.selfdrive.boardd.boardd_api_impl import can_list_to_can_capnp
from openpilot.selfdrive.car import crc8_pedal

# packer = CANPacker("honda_civic_touring_2016_can_generated")
packer = CANPacker("honda_civic_hatchback_ex_2017_can_generated")
rpacker = CANPacker("acura_ilx_2016_nidec")


def get_car_can_parser():
  dbc_f = 'honda_civic_hatchback_ex_2017_can_generated'
  checks = [
  ]
  return CANParser(dbc_f, checks, 0)
cp = get_car_can_parser()

def can_function(pm, speed, angle, idx, cruise_button, is_engaged):

  msg = []

  # *** powertrain bus ***
  #pt_bus = 0 # For  honda_civic_touring_2016_can_generated
  pt_bus = 1

  speed = speed * 3.6 # convert m/s to kph
  msg.append(packer.make_can_msg("ENGINE_DATA", pt_bus, {"XMISSION_SPEED": speed}))
  msg.append(packer.make_can_msg("WHEEL_SPEEDS", pt_bus, {
    "WHEEL_SPEED_FL": speed,
    "WHEEL_SPEED_FR": speed,
    "WHEEL_SPEED_RL": speed,
    "WHEEL_SPEED_RR": speed
  }))

  msg.append(packer.make_can_msg("SCM_BUTTONS", pt_bus, {"CRUISE_BUTTONS": cruise_button}))

#   values = {"COUNTER_PEDAL": idx & 0xF}
#   checksum = crc8_pedal(packer.make_can_msg("GAS_SENSOR", 0, {"COUNTER_PEDAL": idx & 0xF})[2][:-1])
#   values["CHECKSUM_PEDAL"] = checksum
#   msg.append(packer.make_can_msg("GAS_SENSOR", 0, values))

#   msg.append(packer.make_can_msg("GEARBOX", pt_bus, {"GEAR": 4, "GEAR_SHIFTER": 8}))
  msg.append(packer.make_can_msg("GAS_PEDAL_2", pt_bus, {}))
  msg.append(packer.make_can_msg("SEATBELT_STATUS", pt_bus, {"SEATBELT_DRIVER_LATCHED": 1}))
  msg.append(packer.make_can_msg("STEER_STATUS", pt_bus, {}))
  msg.append(packer.make_can_msg("STEERING_SENSORS", pt_bus, {"STEER_ANGLE": angle}))
  msg.append(packer.make_can_msg("VSA_STATUS", pt_bus, {}))
  msg.append(packer.make_can_msg("STANDSTILL", pt_bus, {"WHEELS_MOVING": 1 if speed >= 1.0 else 0}))
  msg.append(packer.make_can_msg("STEER_MOTOR_TORQUE", pt_bus, {}))
  msg.append(packer.make_can_msg("EPB_STATUS", pt_bus, {}))
  msg.append(packer.make_can_msg("DOORS_STATUS", pt_bus, {}))
  msg.append(packer.make_can_msg("CRUISE_PARAMS", pt_bus, {}))
  msg.append(packer.make_can_msg("CRUISE", pt_bus, {}))
  msg.append(packer.make_can_msg("SCM_FEEDBACK", pt_bus, {"MAIN_ON": 1}))
  msg.append(packer.make_can_msg("POWERTRAIN_DATA", pt_bus, {"ACC_STATUS": int(is_engaged)}))
  msg.append(packer.make_can_msg("HUD_SETTING", pt_bus, {}))
  msg.append(packer.make_can_msg("CAR_SPEED", pt_bus, {}))
  
  #For some reason this is on bus 1
  msg.append(packer.make_can_msg("ACC_HUD", 1, {"CRUISE_SPEED": 40}))
  msg.append(packer.make_can_msg("ACC_CONTROL", 1, {}))

  # *** cam bus ***
  msg.append(packer.make_can_msg("STEERING_CONTROL", 2, {}))
  msg.append(packer.make_can_msg("LKAS_HUD", 2, {}))
  msg.append(packer.make_can_msg("BRAKE_COMMAND", 2, {}))

  # *** radar bus ***
  if idx % 5 == 0:
    msg.append(rpacker.make_can_msg("RADAR_DIAGNOSTIC", 1, {"RADAR_STATE": 0x79}))
    for i in range(16):
      msg.append(rpacker.make_can_msg("TRACK_%d" % i, 1, {"LONG_DIST": 255.5}))

  pm.send('can', can_list_to_can_capnp(msg))

def sendcan_function(sendcan):
  sc = messaging.drain_sock_raw(sendcan)
  cp.update_strings(sc, sendcan=True)

  if cp.vl[0x1fa]['COMPUTER_BRAKE_REQUEST']:
    brake = cp.vl[0x1fa]['COMPUTER_BRAKE'] / 1024.
  else:
    brake = 0.0

  if cp.vl[0x200]['GAS_COMMAND'] > 0:
    gas = ( cp.vl[0x200]['GAS_COMMAND'] + 83.3 ) / (0.253984064 * 2**16)
  else:
    gas = 0.0

  if cp.vl[0xe4]['STEER_TORQUE_REQUEST']:
    steer_torque = cp.vl[0xe4]['STEER_TORQUE']/3840
  else:
    steer_torque = 0.0

  return gas, brake, steer_torque
