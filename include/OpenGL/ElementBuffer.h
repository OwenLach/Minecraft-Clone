#pragma once

#include <glad/glad.h>
#include <vector>

class ElementBuffer
{
public:
    ElementBuffer();
    ~ElementBuffer();

    void setData(const unsigned int *data, int size);
    void bind() const;
    void unbind() const;
    bool hasData() const { return hasData_; }

private:
    bool hasData_;
    unsigned int ID;
};