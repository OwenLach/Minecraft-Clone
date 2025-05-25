#include <glm/glm.hpp>
#include <vector>

#include "Constants.h"
#include "BlockTypes.h"
#include "TextureAtlas.h"

class Block
{
public:
    BlockType block_type;
    TextureAtlas *textureAtlas;
    glm::vec3 position;
    BlockTextureUVs textureUVs;

    Block(BlockType type, TextureAtlas *atlas, glm::vec3 pos);
    std::vector<float> getVerticies();

private:
    std::vector<float> verticies;

    void initBlockTextures();
    void setPosition(glm::vec3 pos);
    glm::vec3 getPosition();
};