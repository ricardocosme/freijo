
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

/// Abstraction to Program Objects
/// (Section 2.11.2 Program Object at OpenGL 3.3 Core Profile)
///
class program
{
public:
    using Shaders = std::initializer_list<GLuint>;

    program() = default;
    
    /// Create a program object(glCreateProgram), attach the
    /// `shaders`(glAttachShader) and link the program(glLinkProgram)
    ///
    /// /throw std::runtime_error if a link error occurs
    ///
    explicit program(Shaders shaders)
        : _id(glCreateProgram())
        , _shaders(std::move(shaders))
    {
        /// Section 2.11.2 - CreateProgram()
        /// "If an error occurs, zero will be returned"
        assert(_id);

        //TODO: INVALID_OPERATION
        for(auto shader : _shaders)
            glAttachShader(_id, shader);
        
        glLinkProgram(_id);
        GLint linked;
        glGetProgramiv(_id, GL_LINK_STATUS, &linked);
        if(!linked)
        {
            GLint length;
            glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> log(length);
            glGetProgramInfoLog(_id, length, &length, log.data());
            throw std::runtime_error("Program link error: "
                                     + std::string(log.data()));
        }
    }
    
    ~program()
    {
        /// Free the program from the current context
        glUseProgram(0);
        
        /// Section 2.11.3 - DeleteProgram()        
        /// "When a program object is deleted, all shader objects
        /// attached to it are detached."
        glDeleteProgram(_id);
    }

    program(program&& rhs)
        : _id(rhs._id)
        , _shaders(std::move(rhs._shaders))
    {
        rhs._id = 0;
    }
    
    program& operator=(program&& rhs)
    {
        _id = rhs._id;
        rhs._id = 0;
        _shaders = std::move(rhs._shaders);
        return *this;
    }

    /// Return the program's name
    GLuint id() const noexcept
    { return _id; }

    ///Return the attached shaders
    const Shaders& shaders() const noexcept
    { return _shaders; }
private:
    GLuint _id{0};
    Shaders _shaders;
};

inline bool operator==(const program& lhs,
                       const program& rhs)
{
    return lhs.id() == rhs.id()
        && std::equal(lhs.shaders().begin(), lhs.shaders().end(),
                      rhs.shaders().begin());
}

inline bool operator!=(const program& lhs,
                       const program& rhs)
{ return !(lhs == rhs); }

} 
