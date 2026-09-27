#include <gz/sim/System.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Link.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Pose.hh>

#include <gz/plugin/Register.hh>
#include <gz/transport/Node.hh>
#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/empty.pb.h>

#include <sdf/Element.hh>
#include <iostream>
#include <chrono>
#include <string>

namespace cansat {
  class PayloadEjection : public gz::sim::System,
                          public gz::sim::ISystemConfigure,
                          public gz::sim::ISystemPreUpdate
  {
    public: void Configure(const gz::sim::Entity &_entity,
                           const std::shared_ptr<const sdf::Element> &_sdf,
                           gz::sim::EntityComponentManager &_ecm,
                           gz::sim::EventManager &) override
    {
      this->model_ = gz::sim::Model(_entity);

      this->payload_link_name_ = _sdf->Get<std::string>("payload_link_name", "container_link").first;
      this->detach_topic_ = _sdf->Get<std::string>("detach_topic", "/rocket/container/detach").first;
      this->force_ = _sdf->Get<double>("force", 45.75).first;
      this->duration_ = _sdf->Get<double>("duration", 0.05).first;

      this->trigger_mode_ = _sdf->Get<std::string>("trigger_mode", "apogee").first;
      this->trigger_alt_ = _sdf->Get<double>("trigger_altitude", 100.0).first;
      this->apogee_offset_ = _sdf->Get<double>("apogee_drop_offset", 1.0).first;
      this->min_altitude_ = _sdf->Get<double>("min_altitude", 20.0).first;
      this->timer_delay_ = _sdf->Get<double>("timer_delay", 10.0).first;

      this->detach_pub_ = this->node_.Advertise<gz::msgs::Empty>(this->detach_topic_);

      std::string modelName = this->model_.Name(_ecm);
      std::string cmdTopic = _sdf->Get<std::string>("cmd_topic", "/" + modelName + "/ejection/trigger").first;
      std::string statusTopic = _sdf->Get<std::string>("status_topic", "/" + modelName + "/ejection/status").first;

      this->status_pub_ = this->node_.Advertise<gz::msgs::Boolean>(statusTopic);
      this->node_.Subscribe(cmdTopic, &PayloadEjection::OnTriggerCommand, this);

      std::cout << "[PayloadEjection] Configured for payload " << this->payload_link_name_ 
                << " in mode [" << this->trigger_mode_ << "]" << std::endl;
    }

    public: void PreUpdate(const gz::sim::UpdateInfo &_info,
                           gz::sim::EntityComponentManager &_ecm) override
    {
      if (_info.paused) return;

      double simTime = std::chrono::duration<double>(_info.simTime).count();

      if (this->payload_link_entity_ == gz::sim::kNullEntity) {
        this->payload_link_entity_ = _ecm.EntityByComponents(
            gz::sim::components::Name(this->payload_link_name_),
            gz::sim::components::Link()
        );

        if (this->payload_link_entity_ != gz::sim::kNullEntity) {
          std::cout << "[PayloadEjection] Found " << this->payload_link_name_ 
                    << " entity: " << this->payload_link_entity_ << std::endl;
        } else return;
      }

      gz::math::Pose3d currentPose = gz::sim::worldPose(this->model_.Entity(), _ecm);
      double currentAlt = currentPose.Pos().Z();

      if (!this->initialized_) {
        this->initial_alt_ = currentAlt;
        this->max_alt_ = currentAlt;
        this->last_sim_time_ = simTime;
        this->initialized_ = true;
        return;
      }

      double dt = simTime - this->last_sim_time_;
      if (dt <= 0.0) return;

      if (currentAlt > this->max_alt_) {
        this->max_alt_ = currentAlt;
      }

      if (!this->released_) {
        bool armed = (this->max_alt_ - this->initial_alt_) >= this->min_altitude_;
        bool should_trigger = false;
        std::string reason = "";

        if (this->trigger_mode_ == "apogee" || this->trigger_mode_ == "any") {
          if (armed && (this->max_alt_ - currentAlt >= this->apogee_offset_)) {
            should_trigger = true;
            reason = "Apogee Drop Detected";
          }
        }
        if (!should_trigger && (this->trigger_mode_ == "altitude" || this->trigger_mode_ == "any")) {
          if (armed && currentAlt <= this->trigger_alt_) {
            should_trigger = true;
            reason = "Altitude Threshold Reached";
          }
        }
        if (!should_trigger && (this->trigger_mode_ == "timer" || this->trigger_mode_ == "any")) {
          if (simTime >= this->timer_delay_) {
            should_trigger = true;
            reason = "Timer Reached";
          }
        }

        if (should_trigger)
          this->ExecuteEjection(simTime, reason);
      }

      if (this->ejecting_) {
        if (simTime - this->ejection_start_time_ < this->duration_) {
          gz::math::Pose3d payloadPose = gz::sim::worldPose(this->payload_link_entity_, _ecm);
          gz::math::Vector3d worldPopForce = payloadPose.Rot() * gz::math::Vector3d(0, 0, this->force_);

          gz::sim::Link(this->payload_link_entity_).AddWorldForce(_ecm, worldPopForce);

          gz::sim::Entity canonicalLink = this->model_.CanonicalLink(_ecm);
          if (canonicalLink != gz::sim::kNullEntity) {
            gz::sim::Link(canonicalLink).AddWorldForce(_ecm, -worldPopForce);
          }
        } else {
          this->ejecting_ = false;
        }
      }

      this->last_sim_time_ = simTime;
    }

    private: void OnTriggerCommand(const gz::msgs::Empty &) {
      if (!this->released_) {
        this->ExecuteEjection(this->last_sim_time_, "Manual Command");
      }
    }

    private: void ExecuteEjection(double _simTime, const std::string &_reason) {
      this->released_ = true;
      this->ejecting_ = true;
      this->ejection_start_time_ = _simTime;

      gz::msgs::Empty msg;
      this->detach_pub_.Publish(msg);

      std::cout << "[PayloadEjection] EXECUTED (" << _reason << ")" << std::endl;

      gz::msgs::Boolean statusMsg;
      statusMsg.set_data(true);
      this->status_pub_.Publish(statusMsg);
    }

    private: gz::sim::Model model_;
    private: gz::sim::Entity payload_link_entity_{gz::sim::kNullEntity};

    private: gz::transport::Node node_;
    private: gz::transport::Node::Publisher detach_pub_;
    private: gz::transport::Node::Publisher status_pub_;

    private: bool initialized_{false};
    private: bool released_{false};
    private: bool ejecting_{false};

    private: double initial_alt_{0.0};
    private: double max_alt_{0.0};
    private: double last_sim_time_{0.0};
    private: double ejection_start_time_{0.0};

    //configurable parameters
    private: std::string payload_link_name_{"container_link"};
    private: std::string detach_topic_{"/rocket/container/detach"};
    private: std::string trigger_mode_{"apogee"};
    private: double force_{45.75};
    private: double duration_{0.05};
    private: double trigger_alt_{100.0};
    private: double apogee_offset_{1.0};
    private: double min_altitude_{20.0};
    private: double timer_delay_{10.0};
  };
}

GZ_ADD_PLUGIN(
    cansat::PayloadEjection,
    gz::sim::System,
    cansat::PayloadEjection::ISystemConfigure,
    cansat::PayloadEjection::ISystemPreUpdate
)
