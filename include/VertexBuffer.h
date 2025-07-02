#pragma once

class VertexBuffer
{
public:
    VertexBuffer();
    ~VertexBuffer();

    void setData(const float *data, int size);
    void bind() const;
    void unbind() const;
    bool hasData() const { return hasData_; }

private:
    bool hasData_;
    unsigned int ID;
};