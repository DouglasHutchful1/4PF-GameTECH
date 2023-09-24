#include "entity.hpp"
#include "../deserialize-utils.hpp"
#include "../components/component-deserializer.hpp"

#include <glm/gtx/euler_angles.hpp>

namespace our {

    // This function returns the transformation matrix from the entity's local space to the world space
    // Remember that you can get the transformation matrix from this entity to its parent from "localTransform"
    // To get the local to world matrix, you need to combine this entities matrix with its parent's matrix and
    // its parent's parent's matrix and so on till you reach the root.
    glm::mat4 Entity::getLocalToWorldMatrix() const {
        //DONE: (Req 8) Write this function
        // Check if this entity has a parent, if it does, then we will combine the local to world matrix
        // of the parent with the local to world matrix of this entity
        if(!parent) // base case: the entity has no parent
        {
            return localTransform.toMat4();
        } 
        else // recursive case: the entity has a parent so we will combine the local to world matrix of the parent with the local to world matrix of this entity
            return parent->getLocalToWorldMatrix()*localTransform.toMat4();
    }

    // Deserializes the entity data and components from a json object
    void Entity::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        name = data.value("name", name);
        localTransform.deserialize(data);
        if(data.contains("components")){
            if(const auto& components = data["components"]; components.is_array()){
                for(auto& component: components){
                    deserializeComponent(component, this);
                }
            }
        }
    }

}