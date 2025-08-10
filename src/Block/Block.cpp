#include "Block/Block.h"

Block::Block() : type(BlockType::Air), position(glm::ivec3(0.0f)), skylight(0) {}

Block::Block(BlockType type, glm::ivec3 pos) : type(type), position(pos), skylight(0) {}