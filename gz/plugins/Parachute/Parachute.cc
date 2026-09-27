#include <gz/sim/System.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Link.hh>
#include <gz/sim/components/Pose.hh>

#include <gz/transport/Node.hh>
#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/empty.pb.h>

#include <gz/plugin/Register.hh>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/math/Quaternion.hh>
#include <gz/common/Console.hh>

#include <iostream>

namespace cansat {
  class Parachute : public gz::sim::System,
                    public gz::sim::ISystemConfigure,
                    public gz::sim::ISystemPreUpdate
  {
    public: void Configure(const gz::sim::Entity &_entity,
                           const std::shared_ptr<const sdf::Element> &_sdf,
                           gz::sim::EntityComponentManager &_ecm,
                           gz::sim::EventManager &) override
    {
      this->model_ = gz::sim::Model(_entity);
      
      std::string linkName = _sdf->Get<std::string>("link_name", "base_link").first;
      this->link_ = gz::sim::Link(this->model_.LinkByName(_ecm, linkName));

      if (!this->link_.Valid(_ecm)) {
        std::cerr << "[Parachute] Link [" << linkName << "] not found in model!" << std::endl;
        return;
      }

      this->cd_ = _sdf->Get<double>("cd", 1.3).first;
      this->area_ = _sdf->Get<double>("area", 0.2).first;
      this->air_density_ = _sdf->Get<double>("air_density", 1.225).first;

      //self-righting & damping params
      this->tether_offset_ = _sdf->Get<double>("tether_offset", 0.15).first; // 15cm above CoM
      this->angular_damping_ = _sdf->Get<double>("angular_damping", 0.05).first;

      //trigger : apgee, altitude, timer, manual
      this->trigger_mode_ = _sdf->Get<std::string>("trigger_mode", "apogee").first;
      this->trigger_alt_ = _sdf->Get<double>("trigger_altitude", 100.0).first;
      this->apogee_offset_ = _sdf->Get<double>("apogee_drop_offset", 1.5).first;
      this->timer_delay_ = _sdf->Get<double>("timer_delay", 10.0).first;

      std::string modelName = this->model_.Name(_ecm);
      std::string statusTopic = _sdf->Get<std::string>("status_topic", "/" + modelName + "/parachute/status").first;
      std::string cmdTopic = _sdf->Get<std::string>("cmd_topic", "/" + modelName + "/parachute/deploy").first;

      this->status_pub_ = this->node_.Advertise<gz::msgs::Boolean>(statusTopic);
      this->node_.Subscribe(cmdTopic, &Parachute::OnDeployCommand, this);

      std::cout << "[Parachute] Configured for model " << modelName << " on link " << linkName 
                << " in mode [" << this->trigger_mode_ << "]" << std::endl;
    }

    public: void PreUpdate(const gz::sim::UpdateInfo &_info,
                           gz::sim::EntityComponentManager &_ecm) override
    {
      if (_info.paused || !this->link_.Valid(_ecm))
        return;

      auto worldPoseOpt = this->link_.WorldPose(_ecm);
      if (!worldPoseOpt.has_value())
        return;

      gz::math::Pose3d currentPose = worldPoseOpt.value();
      double currentAlt = currentPose.Pos().Z();
      double simTime = std::chrono::duration<double>(_info.simTime).count();

      if (!this->initialized_) {
        this->lastPose_ = currentPose;
        this->lastSimTime_ = simTime;
        this->max_alt_ = currentAlt;
        this->initialized_ = true;
        return;
      }

      double dt = simTime - this->lastSimTime_;
      if (dt <= 0.0) return;

      gz::math::Vector3d vel = (currentPose.Pos() - this->lastPose_.Pos()) / dt;
      double v_z = vel.Z();

      gz::math::Quaterniond q_rel = currentPose.Rot() * this->lastPose_.Rot().Inverse();
      q_rel.Normalize();
      gz::math::Vector3d angVel = 2.0 * gz::math::Vector3d(q_rel.X(), q_rel.Y(), q_rel.Z()) / dt;
      if (q_rel.W() < 0.0) angVel = -angVel;

      if (currentAlt > this->max_alt_) {
        this->max_alt_ = currentAlt;
      }

      if (!this->deployed_) {
        if (this->trigger_mode_ == "apogee") {
          if (this->max_alt_ - currentAlt > this->apogee_offset_) {
            this->Deploy("Apogee Drop Detected");
          }
        } else if (this->trigger_mode_ == "altitude") {
          if (currentAlt <= this->trigger_alt_ && v_z < 0.0) {
            this->Deploy("Altitude Threshold Reached");
          }
        } else if (this->trigger_mode_ == "timer") {
          if (simTime >= this->timer_delay_) {
            this->Deploy("Timer Triggered");
          }
        }
      }

      if (this->deployed_) {
        double speed = vel.Length();
        if (speed > 0.01) {
          gz::math::Vector3d dragForce = -0.5 * this->air_density_ * this->cd_ * this->area_ * speed * vel;
          gz::math::Vector3d r_world = currentPose.Rot().RotateVector(gz::math::Vector3d(0, 0, this->tether_offset_));
          gz::math::Vector3d restoringTorque = r_world.Cross(dragForce);
          gz::math::Vector3d dampingTorque = -this->angular_damping_ * angVel;
          this->link_.AddWorldForce(_ecm, dragForce);
          this->link_.AddWorldWrench(_ecm, dragForce, restoringTorque + dampingTorque);
        }
      }

      this->lastPose_ = currentPose;
      this->lastSimTime_ = simTime;
    }

    private: void OnDeployCommand(const gz::msgs::Empty &) {
      if (!this->deployed_)
        this->Deploy("Manual Command Received");
    }

    private: void Deploy(const std::string &_reason)
    {
      this->deployed_ = true;
      std::cout << "[Parachute] DEPLOYED (" << _reason << ")" << std::endl;

      gz::msgs::Boolean msg;
      msg.set_data(true);
      this->status_pub_.Publish(msg);
    }

    private: gz::sim::Model model_;
    private: gz::sim::Link link_;
    private: gz::transport::Node node_;
    private: gz::transport::Node::Publisher status_pub_;

    private: bool deployed_{false};
    private: double max_alt_{0.0};
    
    private: gz::math::Pose3d lastPose_{gz::math::Pose3d::Zero};
    private: double lastSimTime_{0.0};
    private: bool initialized_{false};
    
    // Configurable parameters
    private: std::string trigger_mode_{"apogee"};
    private: double cd_{1.3};
    private: double area_{0.2};
    private: double air_density_{1.225};
    private: double trigger_alt_{100.0};
    private: double apogee_offset_{1.5};
    private: double timer_delay_{10.0};
    private: double tether_offset_{0.15};
    private: double angular_damping_{0.05};
  };
}

GZ_ADD_PLUGIN(
    cansat::Parachute,
    gz::sim::System,
    cansat::Parachute::ISystemConfigure,
    cansat::Parachute::ISystemPreUpdate
)
