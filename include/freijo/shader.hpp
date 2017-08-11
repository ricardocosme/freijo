
// Copyright Ricardo Calheiros de Miranda Cosme 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#pragma once

// #include <GL/glew.h>
#include <GL/gl.h>

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace freijo {

/// Represents a vertex shader
struct vertex_shader
{ static const GLenum glType{GL_VERTEX_SHADER}; };

/// Represents a fragment shader
struct fragment_shader
{ static const GLenum glType{GL_FRAGMENT_SHADER}; };

/// Abstraction to Shader Objects
/// (Section 2.11.1 Shader Object at OpenGL 3.3 Core Profile)
///
/// /tparam Type Type of the shader. It can be a `vertex_shader` or
/// `fragment_shader`
///
template<typename Type>
class shader
{
public:
    shader() = default;

    /// Create a shader object(glCreateShader) and compile the
    /// shader source code(glCompileShader)
    ///
    /// /throw std::runtime_error if a compilation error occurs
    ///
    shader(std::string src)
        : _id(glCreateShader(Type::glType))
        , _src(std::move(src))
    {
        /// Section 2.11.1 - CreateShader()
        /// "If an error occurs, zero will be returned"
        assert(_id);

        /// Get a null-terminated array of the source code. It
        /// permits the last argument to glShaderSource, a NULL
        /// pointer.
        auto csrc = _src.c_str();
        glShaderSource(_id, 1, &csrc, NULL);
        
        glCompileShader(_id);
        GLint compiled;
        glGetShaderiv(_id, GL_COMPILE_STATUS, &compiled);
        if(!compiled)
        {
            GLint length;
            glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> log(length);
            glGetShaderInfoLog(_id, length, &length, log.data());
            throw std::runtime_error("Shader compile error: "
                                     + std::string(log.data()));
        }        
    }

    /// Mark the shader to be deleted when it's not attached to any
    /// program object
    ~shader()
    { glDeleteShader(_id); }        

    shader(shader&& rhs) noexcept
        : _id(rhs._id)
        , _src(std::move(rhs._src))
    {
        rhs._id = 0;
    }
    
    shader& operator=(shader&& rhs) noexcept
    {
        _id = rhs._id;
        rhs._id = 0;
        _src = std::move(rhs._src);
        return *this;
    }
    
    /// Return the shader's name
    GLuint id() const noexcept
    { return _id; }

    /// Return the source code
    const std::string& src() const noexcept
    { return _src; }    
private:
    // Shader object's name
    GLuint _id{0};
    
    // Source code
    std::string _src;
};
 
} 
