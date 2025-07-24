#include "Block/Block.h"

Block::Block() : type(BlockType::Air), position(glm::ivec3(0.0f)) {}

Block::Block(BlockType type, glm::ivec3 pos) : type(type), position(pos) {}