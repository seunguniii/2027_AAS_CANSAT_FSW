#include <gz/sim/System.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Link.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/components/Link.hh>
#include <gz/sim/components/Name.hh>

#include <gz/transport/Node.hh>

#include <gz/msgs/boolean.pb.h>

#include <gz/plugin/Register.hh>

#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>


namespace cansat {
  class J425R:
    public gz::sim::System,
    public gz::sim::ISystemConfigure,
    public gz::sim::ISystemPreUpdate {
      public:
        void Configure(const gz::sim::Entity &entity,
                       const std::shared_ptr<const sdf::Element> &, 
                       gz::sim::EntityComponentManager &ecm,
                       gz::sim::EventManager &)
        {
          gz::sim::Model model(entity);
          this->linkEntity = model.LinkByName(ecm, "rocket_base_link");

          if(this->linkEntity == gz::sim::kNullEntity) {
            std::cerr << "[J425R] Couldn't find rocket_base_link" << std::endl;
            return;
          }
          std::cout << "[J425R] Found rocket_base_link: " << this->linkEntity << std::endl;
          
          std::random_device rd;
          this->rng.seed(rd());
          
          this->node.Subscribe("/rocket/ignite", &J425R::OnIgnite, this);
        }


        void PreUpdate(const gz::sim::UpdateInfo &info, 
                       gz::sim::EntityComponentManager &ecm)
        {
          this->simTime = std::chrono::duration<double>(info.simTime).count();
          
          if(!this->ignited) return;
          
          if (this->simTime - this->ignitionTime >= 1.6) {
            this->ignited = false;
            
            std::cout<< "[J425R] BURNOUT" << std::endl;
            return;
          }
          
          gz::sim::Link link(this->linkEntity);
          
          auto pose = gz::sim::worldPose(this->linkEntity, ecm);
          
          double zThrust = std::max(0.0, this->thrustDist(this->rng));
          double xThrust = this->lateralDist(this->rng);
          double yThrust = this->lateralDist(this->rng);
          
          gz::math::Vector3d bodyThrust(xThrust, yThrust, zThrust);
          gz::math::Vector3d worldThrust = pose.Rot().RotateVector(bodyThrust);
          
          link.AddWorldForce(ecm, worldThrust);
        }         

        
        void OnIgnite(const gz::msgs::Boolean &msg) {
          if (msg.data() && !this->ignited) {
            this->ignited = true;
            this->ignitionTime = this->simTime;

            std::cout << "[J425R] IGNITED" << std::endl;
          }
        }
        
      private:
        gz::sim::Entity linkEntity{gz::sim::kNullEntity};
        gz::transport::Node node;

        double simTime{0.0};
        
        bool ignited{false};
        double ignitionTime{0.0};
        
        std::mt19937 rng;
        std::normal_distribution<double> thrustDist{420.0, 5.0};
        std::normal_distribution<double> lateralDist{0.0, 2.0};
    };
}

GZ_ADD_PLUGIN(
    cansat::J425R,
    gz::sim::System,
    gz::sim::ISystemConfigure,
    gz::sim::ISystemPreUpdate
)
