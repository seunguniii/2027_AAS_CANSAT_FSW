#include <gz/sim/System.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Link.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/Link.hh>

#include <gz/plugin/Register.hh>
#include <gz/transport/Node.hh>
#include <gz/msgs/empty.pb.h>

#include <sdf/Element.hh>

#include <iostream>
#include <chrono>

namespace cansat {
  class RocketRecovery:
    public gz::sim::System,
    public gz::sim::ISystemConfigure,
    public gz::sim::ISystemPreUpdate {
      public:
        void Configure(const gz::sim::Entity &entity,
                       const std::shared_ptr<const sdf::Element> &sdf,
                       gz::sim::EntityComponentManager &ecm,
                       gz::sim::EventManager &) override
        {
          this->model = gz::sim::Model(entity);
          if(!this->model.Valid(ecm)){
            std::cerr << "[RocketRecovery] Plugin not attached to a <model>" << std::endl;
            return;
          }
          
          if(sdf->HasElement("ejection_force"))
            this->ejectionForce = sdf->Get<double>("ejection_force");

          this->detachPub = this->node.Advertise<gz::msgs::Empty>("/rocket/cone/detach");
          
          this->node.Subscribe("/rocket/test_eject", &RocketRecovery::OnTestEject, this);
          std::cout << "[RocketRecovery] Ground test topic active: /rocket/test_eject" << std::endl;
        }

        void PreUpdate(const gz::sim::UpdateInfo &info,
                       gz::sim::EntityComponentManager &ecm) override
        {
          double currentSimTime = std::chrono::duration<double>(info.simTime).count();
          if(this->coneLinkEntity == gz::sim::kNullEntity) {
            this->coneLinkEntity = ecm.EntityByComponents(
                gz::sim::components::Name("rocket_cone_link"),
                gz::sim::components::Link()
            );

            if(this->coneLinkEntity != gz::sim::kNullEntity) {
              std::cout << "[RocketRecovery] Found rocket_cone_link entity: " 
                        << this->coneLinkEntity << std::endl;
            } else return;
          }
          
          if(this->coneLinkEntity == gz::sim::kNullEntity) return;

          if (this->manualEjectRequested && !this->released) {
            this->released = true;
            this->ejecting = true;
            this->ejectionStartTime = currentSimTime;
            this->manualEjectRequested = false;
            std::cout << "[RocketRecovery] Manual pop executed!" << std::endl;
          }
          
          if(this->ejecting){
            if(currentSimTime - this->ejectionStartTime < this->ejectionDuration){
              gz::math::Pose3d pose = gz::sim::worldPose(this->coneLinkEntity, ecm);
              
              gz::math::Vector3d worldPopForce = pose.Rot() * gz::math::Vector3d(0, 0, this->ejectionForce);
              
              gz::sim::Link(this->coneLinkEntity).AddWorldForce(ecm, worldPopForce);

              gz::sim::Entity rocketLink = this->model.CanonicalLink(ecm);
              if(rocketLink != gz::sim::kNullEntity) 
                gz::sim::Link(rocketLink).AddWorldForce(ecm, -worldPopForce);
            } else this->ejecting = false;
          }

          if(this->released) return;
          
          gz::math::Pose3d pose = gz::sim::worldPose(this->model.Entity(), ecm);
          double currentZ = pose.Pos().Z();
          
          if(this->initialZ < 0){
            this->initialZ = currentZ;
            this->maxZ = currentZ;
            return;
          }
          
          if(!this->launched){
            if(currentZ > this->initialZ + 1.0){
              this->launched = true;
              std::cout << "[RocketRecovery] Launch detected" << std::endl;
            }
            return;
          }
          
          if(currentZ > this->maxZ) this->maxZ = currentZ;
          
          bool altitudeIsDescending = (currentZ < this->maxZ - 0.1);
          
          if(altitudeIsDescending){
            gz::msgs::Empty msg;
            this->detachPub.Publish(msg);
          
            std::cout << "[RocketRecovery] Apogee reached at " << this->maxZ << "m" << std::endl;
            std::cout << "[RocketRecovery] Opening payload bay" << std::endl;
            this->released = true;
            this->ejecting = true;
            this->ejectionStartTime = currentSimTime;
          }
        }
        
        void OnTestEject(const gz::msgs::Empty &) {
          if (this->released) return;

          gz::msgs::Empty msg;
          this->detachPub.Publish(msg);

          std::cout << "[RocketRecovery] Manual test trigger received" << std::endl;
          this->manualEjectRequested = true;
        }
      
      private:
        gz::sim::Model model{gz::sim::kNullEntity};
        gz::sim::Entity coneLinkEntity{gz::sim::kNullEntity};
        
        gz::transport::Node node;
        gz::transport::Node::Publisher detachPub;
        
        bool launched{false};
        bool released{false};
        bool ejecting{false};
        bool manualEjectRequested{false};
        
        double initialZ{-1.0};
        double maxZ{-1.0};
        
        double ejectionStartTime{0.0};
        double ejectionDuration{0.05};
        double ejectionForce{15.0};
    };
}

GZ_ADD_PLUGIN(
    cansat::RocketRecovery,
    gz::sim::System,
    gz::sim::ISystemConfigure,
    gz::sim::ISystemPreUpdate
)
