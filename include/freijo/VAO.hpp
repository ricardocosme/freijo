
// Copyright Ricardo Calheiros de Miranda Cosme 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "freijo/buffer.hpp"

#include <cstddef>
#include <algorithm>

namespace freijo {

class VAO_base
{
public:
    VAO_base() { glGenVertexArrays(1, &_id); }
    ~VAO_base() { glDeleteVertexArrays(1, &_id); }
    
    void bind() const { glBindVertexArray(_id); }    
    void unbind() const { glBindVertexArray(0); }
    
    unsigned int id() const noexcept { return _id; }
protected:    
    unsigned int _id{0};
};
    
/* RAII to bind()/unbind() */ 
class scoped_vao_bind
{
public:
    explicit scoped_vao_bind(const VAO_base& vao_base)
        : _vao_base(vao_base)
    { _vao_base.bind(); }
    
    ~scoped_vao_bind() { _vao_base.unbind(); }
    
    scoped_vao_bind(const scoped_vao_bind&) = delete;
    scoped_vao_bind& operator=(const scoped_vao_bind&) = delete;
private:
    const VAO_base& _vao_base;
};
    
/* VAO (Vertex Array Object)
 *
 * Model the concepts Movable and EqualityComparable.
 */            
class VAO : public VAO_base
{
public:
    VAO() = default;
    
    VAO(VAO&& o) noexcept
    { std::swap(_id, o._id); }
    
    VAO& operator=(VAO&& o) noexcept
    {
        std::swap(_id, o._id);
        return *this;
    }
    
    template<typename VBO>
    void attach(std::size_t index, const VBO& vbo,
                GLsizei stride = 0,
                std::size_t offset = 0,
                GLboolean normalized = GL_FALSE) const
    {
        scoped_vao_bind sb(*this);
        scoped_buffer_bind<VBO> sbb(vbo);
        glVertexAttribPointer(index,
                              VBO::target::size,
                              VBO::target::type,
                              normalized, stride,
                              reinterpret_cast<void*>(offset));
        glEnableVertexAttribArray(index);
    }

    template<typename EBO>
    void attach(const EBO& ebo) const
    {
        scoped_vao_bind sb(*this);
        ebo.bind();
    }    
                
    void detach(std::size_t index) const
    {
        scoped_vao_bind sb(*this);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(index);
    }

    template<typename EBO>
    void detach(const EBO& ebo) const
    {
        scoped_vao_bind sb(*this);
        ebo.unbind();
    }
    
    void enable_attrib(std::size_t index) const
    {
        scoped_vao_bind sb(*this);
        glEnableVertexAttribArray(index);
    }
            
    void disable_attrib(std::size_t index) const
    {
        scoped_vao_bind sb(*this);
        glDisableVertexAttribArray(index);
    }    
};

inline
bool operator==(const VAO& lhs, const VAO& rhs)
{ return lhs.id() == rhs.id(); }
 
inline
bool operator!=(const VAO& lhs, const VAO& rhs)
{ return !(lhs == rhs); }

}
