
// Copyright Ricardo Calheiros de Miranda Cosme 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace freijo {

// RAII to glEnable and glDisable
//
// Calls glEnable(capability) when the object is initialized and
// glDisable(capability) when the object is destroyed
//
// Example:
// {    
//   enable blend(GL_BLEND); //Calls glEnable(GL_BLEND)
//   ...
// } //Calls glDisable(GL_BLEND)
class enable
{
public:    
    enable(GLenum cap)
        : _cap(cap)
    { glEnable(_cap); }
    
    ~enable()
    { glDisable(_cap);}

    GLenum capability() const noexcept
    { return _cap; }    
private:
    GLenum _cap;
};
    
// RAII to glEnable and glDisable that restores the before state
//
// Saves the current state before the calling to glEnable(capability)
// when the object is initialized and restores the saved state when
// the object is destroyed.
//
// Example:
// glEnable blend
//    
// {    
//   restore_enable blend(GL_BLEND); //Calls glEnable(GL_BLEND)
//   ...    
// } //Calls glDisable(GL_BLEND) if the capability 'GL_BLEND' was
//     disabled before the construction of 'blend'
class restore_enable
{
public:    
    restore_enable(GLenum cap)
        : _cap(cap)
    {
        glGetBooleanv(_cap, &_before);
        glEnable(_cap);
    }
    
    ~restore_enable()
    { if(_before == GL_FALSE) glDisable(_cap); }

    GLenum capability() const noexcept
    { return _cap; }    
private:
    GLenum _cap;
    GLboolean _before{GL_FALSE};
};
    
}
