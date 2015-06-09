
#include "scitos_mira/ScitosCharger.h"

#include <scitos_msgs/BatteryState.h>
#include <scitos_msgs/ChargerStatus.h>

#include <scitos_mira/ScitosG5.h>


ScitosCharger::ScitosCharger() : ScitosModule(std::string ("Charger")), reconfigure_srv_(name_)  {
}

void ScitosCharger::initialize() {
	battery_pub_ = robot_->getRosNode().advertise<scitos_msgs::BatteryState>("/battery_state", 20);
	robot_->getMiraAuthority().subscribe<mira::robot::BatteryState>("/robot/charger/Battery",
							&ScitosCharger::battery_data_callback, this);

	charger_pub_ = robot_->getRosNode().advertise<scitos_msgs::ChargerStatus>("/charger_status", 20);
	robot_->getMiraAuthority().subscribe<uint8>("/robot/charger/ChargerStatus",
							&ScitosCharger::charger_status_callback, this);

	reconfigure_srv_.setCallback(boost::bind(&ScitosCharger::reconfigure_callback, this, _1, _2));

	save_persistent_errors_service_ = robot_->getRosNode().advertiseService("charger/save_persistent_errors", 
									       &ScitosCharger::save_persistent_errors, 
									       this);

}

bool ScitosCharger::save_persistent_errors(scitos_msgs::SavePersistentErrors::Request  &req, scitos_msgs::SavePersistentErrors::Response &res) {
  ROS_INFO_STREAM("Saving persistent error log to '" << req.filename << "'");
  
  //  call_mira_service
  try {
    mira::RPCFuture<void> r = robot_->getMiraAuthority().callService<void>("/robot/Robot", 
									 std::string("savePersistentErrors"),
									 req.filename);
  } catch (mira::XRPC& e) {
    ROS_ERROR_STREAM("Problem with RPC savePersistentErrors: " << e.message());
    return false;
  }
  return true;

}


void ScitosCharger::battery_data_callback(mira::ChannelRead<mira::robot::BatteryState> data) {
	ros::Time time_now = ros::Time::now(); // must be something better? data->timestamp.toUnixNS();?
	scitos_msgs::BatteryState b;

	b.header.seq = data->sequenceID;
	b.header.stamp = time_now; 
	b.charging = data->charging;
	b.current = data->current;
	b.lifePercent = data->lifePercent;
	b.lifeTime = data->lifeTime;
	b.powerSupplyPresent = data->powerSupplyPresent;
	b.voltage = data->voltage;
	b.cellVoltage = data->cellVoltage;

	battery_pub_.publish(b);
}


void ScitosCharger::charger_status_callback(mira::ChannelRead<uint8> data) {
	ros::Time time_now = ros::Time::now(); 

	scitos_msgs::ChargerStatus s;
	s.header.stamp=time_now;
	s.charging = (*data) & 1;
	s.empty = (*data) & (1 << 1);
	s.full = (*data) & (1 << 2);
	s.derating = (*data) & (1 << 3);
	s.charging_disabled = (*data) & (1 << 4);
	s.const_volt_mode = (*data) & (1 << 5);
	s.const_current_mode = (*data) & (1 << 6);
	s.internal_error_flag = (*data) & (1 << 7);

	charger_pub_.publish(s);
}

void ScitosCharger::reconfigure_callback( scitos_mira::ChargerParametersConfig& config, uint32_t level) {
	ROS_DEBUG("Reconfigure request on ScitosCharger module.");
	//Set the MIRA parameters to what was selected...

	// TODO: Decide if this is a good idea: all parameters relate to contact control, which might be dangerous and not needed.
}
