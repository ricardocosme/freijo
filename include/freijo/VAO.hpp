
// Copyright Ricardo Calheiros de Miranda Cosme 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "freijo/buffer_object.hpp"

#include <cstddef>
#include <algorithm>

namespace freijo {

/* VAO (Vertex Array Object)
 *
 * Model the concepts Movable and EqualityComparable.
 */            
class VAO
{
public:
    VAO() { glGenVertexArrays(1, &_id); }
    
    ~VAO() { glDeleteVertexArrays(1, &_id); }
    
    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;
    
    VAO(VAO&& o) noexcept
    { std::swap(_id, o._id); }
    
    VAO& operator=(VAO&& o) noexcept
    {
        std::swap(_id, o._id);
        return *this;
    }
    
    void bind() const { glBindVertexArray(_id); }
    void unbind() const { glBindVertexArray(0); }

    template<typename VBO>
    void attach(std::size_t index, const VBO& vbo)
    {
        scoped_bind bg(*this);
        glBindBuffer(VBO::target::target, vbo.id());
        glVertexAttribPointer(index,
                              VBO::target::size,
                              VBO::target::type,
                              GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(index);
    }
            
    void detach(std::size_t index)
    {
        scoped_bind bg(*this);
        glVertexAttribPointer(index,
                              4,
                              GL_FLOAT,
                              GL_FALSE, 0, nullptr);
        glDisableVertexAttribArray(index);
    }
            
    template<typename IBO>
    void attach(const IBO& ibo)
    {
        scoped_bind bg(*this);
        glBindBuffer(IBO::target::target, ibo.id());
    }

    template<typename IBO>
    void detach(const IBO& ibo)
    {
        scoped_bind bg(*this);
        glBindBuffer(IBO::target::target, 0);
    }
    
    unsigned int id() const noexcept { return _id; }
    
    /* RAII to bind()/unbind() */ 
    class scoped_bind
    {
    public:
        explicit scoped_bind(const VAO& vao) : _vao(vao) { _vao.bind(); }
        ~scoped_bind() { _vao.unbind(); }
    
        scoped_bind(const scoped_bind&) = delete;
        scoped_bind& operator=(const scoped_bind&) = delete;
    private:
        const VAO& _vao;
    };
    
private:
    unsigned int _id{0};
};

inline
bool operator==(const VAO& lhs, const VAO& rhs)
{ return lhs.id() == rhs.id(); }
 
inline
bool operator!=(const VAO& lhs, const VAO& rhs)
{ return !(lhs == rhs); }

}
