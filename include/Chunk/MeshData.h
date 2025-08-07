#pragma once

#include "Vertex.h"

#include <vector>

struct MeshData
{
    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
};