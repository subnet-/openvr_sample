#ifndef SKYBOX_H
#define SKYBOX_H

#include "geometry/drawable.h"
#include "gui/config.h"

class Skybox : public Drawable
{
public:
    Skybox(std::string name = "SKYBOX");

    /**
     * @see Drawable::init()
     */
    virtual void init() override;

    /**
     * @see Drawable::draw(glm::mat4)
     */
    virtual void draw(glm::mat4 projection_matrix, glm::mat4 model_matrix) const override;

    /**
     * @see Drawable::update(float, glm::mat4)
     */
    virtual void update(float elapsedTimeMs, glm::mat4 modelViewMatrix) override;

protected:

    /**
     * @see Drawable::getVertexShader()
     */
    virtual std::string getVertexShader() const override;

    /**
     * @see Drawable::getFragmentShader()
     */
    virtual std::string getFragmentShader() const override;

    /**
     * @see Drawable::createObject()
     */
    virtual void createObject() override;

    /**
     * @brief loadTexture loads the textures for the skybox
     */
    virtual void loadTexture();
};

#endif // SKYBOX_H
