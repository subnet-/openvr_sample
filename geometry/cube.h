#ifndef CUBE_H
#define CUBE_H

#include "geometry/drawable.h"

#include <memory>
#include <vector>

class Cube : public Drawable
{
public:
	Cube(std::string name = "Default Cube",
            float radius = 1.0f,
            float distance = 10.0f,
            float hoursPerDay = 24.0f,
            unsigned int daysPerYear = 365,
            std::string textureLocation = ""
            );

    /**
     * @see Drawable::init()
     */
    virtual void init() override;

    /**
     * @see Drawable::recreate()
     */
    virtual void recreate() override;

    /**
     * @see Drawable::draw(glm::mat4)
     */
    virtual void draw(glm::mat4 projection_matrix, glm::mat4 view_matrix) const override;

    /**
     * @see Drawable::update(float, glm::mat4)
     */
    virtual void update(float elapsedTimeMs, glm::mat4 model_matrix) override;

    /**
     * @brief addChild adds a child to the hierarchy
     * @param child the child to add
     */
    virtual void addChild(std::shared_ptr<Cube> child);

    ~Cube();


protected:

    std::vector<std::shared_ptr<Cube>> _children; /**< All children that move around this planet */

    float _radius;      /**< the radius of the planet */
    float _distance;    /**< the distance between this planet and its parent*/

    unsigned int _daysPerYear;  /**< the number of days the planet needs for a full (global) rotation */

    float _localRotation;       /**< the current local roation */
    float _localRotationSpeed;  /**< the speed at which the planet spins */

    /**
     * @see Drawable::createObject()
     */
    virtual void createObject() override;

    /**
     * @see Drawable::getVertexShader()
     */
    virtual std::string getVertexShader() const override;

    /**
     * @see Drawable::getFragmentShader()
     */
    virtual std::string getFragmentShader() const override;
};

#endif // PLANET_H
