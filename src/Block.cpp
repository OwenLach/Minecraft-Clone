#include <glad/glad.h>
#include "Block.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Block::Block(BlockType type, glm::vec3 pos)
    : block_type(type), position(pos)
{
}

std::vector<float> Block::generateVerticies(BlockTextureUVs textureUVs)
{
    // Vertex positions for one cube face, repeated for each face
    static const std::vector<glm::vec3> basePositions = {
        // Back face
        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f},

        // Front face
        {-0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},
        {-0.5f, -0.5f, 0.5f},

        // Left face
        {-0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},

        // Right face
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},

        // Bottom face
        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {-0.5f, -0.5f, 0.5f},
        {-0.5f, -0.5f, -0.5f},

        // Top face
        {-0.5f, 0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, -0.5f},
    };

    // Corresponding UVs
    std::vector<glm::vec2> uvs = {
        textureUVs.side.bottomLeft,
        textureUVs.side.bottomRight,
        textureUVs.side.topRight,
        textureUVs.side.topRight,
        textureUVs.side.topLeft,
        textureUVs.side.bottomLeft,

        textureUVs.side.bottomLeft,
        textureUVs.side.bottomRight,
        textureUVs.side.topRight,
        textureUVs.side.topRight,
        textureUVs.side.topLeft,
        textureUVs.side.bottomLeft,

        textureUVs.side.topRight,
        textureUVs.side.topLeft,
        textureUVs.side.bottomLeft,
        textureUVs.side.bottomLeft,
        textureUVs.side.bottomRight,
        textureUVs.side.topRight,

        textureUVs.side.topRight,
        textureUVs.side.topLeft,
        textureUVs.side.bottomLeft,
        textureUVs.side.bottomLeft,
        textureUVs.side.bottomRight,
        textureUVs.side.topRight,

        textureUVs.bottom.bottomLeft,
        textureUVs.bottom.bottomRight,
        textureUVs.bottom.topRight,
        textureUVs.bottom.topRight,
        textureUVs.bottom.topLeft,
        textureUVs.bottom.bottomLeft,

        textureUVs.top.bottomLeft,
        textureUVs.top.bottomRight,
        textureUVs.top.topRight,
        textureUVs.top.topRight,
        textureUVs.top.topLeft,
        textureUVs.top.bottomLeft,
    };

    std::vector<float> verticies;
    for (unsigned int i = 0; i < basePositions.size(); i++)
    {
        // calculate block position
        glm::vec3 blockPos = basePositions[i] + position;

        verticies.push_back(blockPos.x);
        verticies.push_back(blockPos.y);
        verticies.push_back(blockPos.z);

        verticies.push_back(uvs[i].x);
        verticies.push_back(uvs[i].y);
    }

    return verticies;
}
