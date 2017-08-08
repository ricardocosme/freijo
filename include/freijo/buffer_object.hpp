
// Copyright Ricardo Calheiros de Miranda Cosme 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <utility>
#include <vector>
#include <array>
// #include <GL/glew.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/detail/precision.hpp>

namespace freijo {

//Traits para definir um valor integral OpenGL para um dado tipo C++.
template<typename T>
struct GLTypeTraits;

template<>
struct GLTypeTraits<GLbyte> 
{
    static const GLenum type{GL_BYTE};
};

template<>
struct GLTypeTraits<GLubyte> 
{
    static const GLenum type{GL_UNSIGNED_BYTE};
};

template<>
struct GLTypeTraits<GLshort> 
{
    static const GLenum type{GL_SHORT};
};

template<>
struct GLTypeTraits<GLushort> 
{
    static const GLenum type{GL_UNSIGNED_SHORT};
};

template<>
struct GLTypeTraits<GLint> 
{
    static const GLenum type{GL_INT};
};

template<>
struct GLTypeTraits<GLuint> 
{
    static const GLenum type{GL_UNSIGNED_INT};
};

template<>
struct GLTypeTraits<GLfloat>
{
    static const GLenum type{GL_FLOAT};
};

template<>
struct GLTypeTraits<GLdouble>
{
    static const GLenum type{GL_DOUBLE};
};

//Traits para definição do tipo(type) da componente do vetor e do
//número de componentes(size).
template<typename T>
struct VertexTraits;

#define VertexTraits_GLM_TVEC(TVEC, SIZE)     \
template<typename T, glm::precision P> \
struct VertexTraits<glm::TVEC<T, P>> \
{\
    static const GLenum type{GLTypeTraits< \
        typename glm::tvec3<T, P>::value_type>::type}; \
    static const GLint size{SIZE}; \
};

//Suporte a vetores da biblioteca glm. 
VertexTraits_GLM_TVEC(tvec1, 1)
VertexTraits_GLM_TVEC(tvec2, 2)
VertexTraits_GLM_TVEC(tvec3, 3)
VertexTraits_GLM_TVEC(tvec4, 4)
               
//Modelo de target ARRAY_BUFFER. 
template<typename T>
struct ArrayBuffer
{
    static const GLenum target = GL_ARRAY_BUFFER;
    static const GLint size = VertexTraits<T>::size;
    static const GLenum type = VertexTraits<T>::type;
};

//Modelo de target ELEMENT_ARRAY. 
struct ElementArray
{
    static const GLenum target = GL_ELEMENT_ARRAY_BUFFER;    
    using type = std::size_t;
};

//RAII para glBindBuffer.
struct bind_buffer_guard
{
    bind_buffer_guard(GLenum target, GLuint id) : target(target)
    { glBindBuffer(target, id); }
    
    ~bind_buffer_guard() { glBindBuffer(target, 0); }

    GLenum target;
};

/* BufferObject é um objeto OpenGL que armazena um array de elementos
   do tipo ValueType. O array é alocado por um contexto OpenGL, ou seja,
   na placa gráfica. Tipicamente são utilizados para armazenar os
   vértices de uma geometria e os seus respectivos atributos de desenho.

   Baseado na especificação OpenGL 3.3 (Core Profile) - March 11, 2010
   Seção 2.9.1
   https://khronos.org/registry/OpenGL/specs/gl/glspec33.core.pdf

   /tparam ValueType Tipo do elemento armazenado no buffer.
   /tparam Target Define a finalidade de uso do buffer:
                  ARRAY_BUFFER -> Atributos de vértices.
                  ELEMENT_ARRAY_BUFFER -> Índices de vértices.

   Modela o concept Regular.
 */
template<typename ValueType, typename Target>
class BufferObject
{
public:
    using value_type = ValueType;
    using target = Target;

    /* poscondition: size() == 0 && id() == 0 && usage() == GL_DYNAMIC_DRAW */
    BufferObject() = default;

    /* Aloca um buffer com a cópia de std::vector<value_type>.
     *
     * /param c container.
     * /param usage Dica de uso do buffer. Ver especificação do OpenGL.
     */
    BufferObject(std::vector<value_type> c,
                 GLenum usage = GL_DYNAMIC_DRAW)
    {
        _size = c.size();
        alloc_cpy_buffer(c.data(), usage); 
    }

    /* Aloca um buffer com a cópia de std::array<value_type, N>.
     *
     * /param c container.
     * /param usage Dica de uso do buffer. Ver especificação do OpenGL.
     */
    template<std::size_t N>
    BufferObject(std::array<value_type, N> c,
                 GLenum usage = GL_DYNAMIC_DRAW)
    {
        _size = c.size();
        alloc_cpy_buffer(c.begin(), usage); 
    }
    
    /* Aloca um buffer com a cópia de [first, last).
     *
     * precondition: [first, last) é um range válido.
     *
     * /param first Iterador para o primeiro elemento 
     *              (modela ContiguousIterator).
     * /param last Iterador para o sucessor do último elemento 
     &             (modela ContiguousIterator).
     * /param usage Dica de uso do buffer. Ver especificação do OpenGL.
     */
    template<typename ContiguousIt>
    BufferObject(ContiguousIt first, ContiguousIt last,
                 GLenum usage = GL_DYNAMIC_DRAW)
    {
        _size = std::distance(first, last);
        alloc_cpy_buffer(first, usage); 
    }
    
    /* Aloca um buffer como cópia de uma std::initializer_list<value_type.
     *
     * poscondition: ilist == this
     *
     * /param ilist Lista de elementos do tipo value_type.
     * /param usage Dica de uso do buffer. Ver especificação do OpenGL.
     */
    BufferObject(const std::initializer_list<value_type>& ilist,
                 GLenum usage = GL_DYNAMIC_DRAW)
    {
        _size = std::distance(ilist.begin(), ilist.end());
        alloc_cpy_buffer(ilist.begin(), usage); 
    }

    /* Aloca um buffer de área sizeof(value_type) * count.
     *
     * poscondition: conteúdo do buffer indefinido.
     *
     * /param count Número de elementos alocados.
     * /param usage Dica de uso do buffer. Ver especificação do OpenGL.
     */
    BufferObject(std::size_t count,
                 GLenum usage = GL_DYNAMIC_DRAW)
    {
        _size = count;
        alloc_buffer(usage); 
    }
    
    BufferObject(const BufferObject& rhs)
    {
        copy_from(rhs);
    }
    
    BufferObject& operator=(const BufferObject& rhs)
    {
        copy_from(rhs);
        return *this;
    }
        
    friend void swap(BufferObject& a, BufferObject& b) noexcept
    {
        std::swap(a._id, b._id);
        std::swap(a._size, b._size);
        std::swap(a._usage, b._usage);
    }

    /* poscondition: rhs assume estado de BufferObject(). */
    BufferObject(BufferObject&& rhs) noexcept
        : BufferObject ()
    {
        swap(*this, rhs);
    }
    
    /* poscondition: rhs assume estado de BufferObject(). */
    BufferObject& operator=(BufferObject&& rhs) noexcept
    {
        _id = rhs._id;
        _size = rhs._size;
        _usage = rhs._usage;
        rhs._id = 0;
        rhs._size = 0;
        rhs._usage = GL_DYNAMIC_DRAW;
        return *this;
    }
    
    ~BufferObject() { del_buffer(); }

    /* Redefine o conteúdo do buffer.
     * 
     * Se c.size() == size() o conteúdo de c sobrescreve o conteúdo do buffer,
     * caso contrário há uma realocação do espaço seguida de uma cópia do 
     * conteúdo de c.
     *
     * precondition: o buffer não está mapeado(map). 
     */
    void reset(std::vector<value_type> c,
               GLenum usage = GL_DYNAMIC_DRAW)
    {
        reset(c.data(), c.data() + c.size(), usage); 
    }

    /* Redefine o conteúdo do buffer.
     * 
     * Se c.size() == size() o conteúdo de c sobrescreve o conteúdo do buffer,
     * caso contrário há uma realocação do espaço seguida de uma cópia do 
     * conteúdo de c.
     *
     * precondition: o buffer não está mapeado(map). 
     */
    template<std::size_t N>
    void reset(std::array<value_type, N> c,
               GLenum usage = GL_DYNAMIC_DRAW)
    {
        reset(c.begin(), c.begin() + c.size(), usage); 
    }
    
    /* Redefine o conteúdo do buffer.
     * 
     * Se c.size() == size() o conteúdo de c sobrescreve o conteúdo do buffer,
     * caso contrário há uma realocação do espaço seguida de uma cópia do 
     * conteúdo de c.
     *
     * precondition: o buffer não está mapeado(map). 
     */
    template<typename ContiguousIt>
    void reset(ContiguousIt first, ContiguousIt last,
               GLenum usage = GL_DYNAMIC_DRAW)
    {
        auto nsize = std::distance(first, last);
        if (nsize == _size)
            glBufferSubData(target::target, 0, area(), first);
        else
        {
            del_buffer();
            _size = nsize;
            alloc_cpy_buffer(first, usage);
        }
    }

    /* Retorna um ponteiro para value_type para o começo do buffer.
     *
     * preconditions: this != BufferObject() && 
     *
     * /param access Modo de acesso ao buffer GL_READ_ONLY, GL_WRITE_ONLY 
     *               ou GL_READ_WRITE. Deve ser compatível com BUFFER_USAGE
     *               para se obter a melhor performance possível.
     *               !Atenção! Não cumprir a recomendação acima possivelmente
     *               implicará em um acesso de ordem de magnitude mais lento.
     * /return Ponteiro para value_type que aponta para o começo do buffer. NULL
     *         caso se o buffer já estiver mapeado.
     */    
    value_type* map(GLenum access) const
    {
        assert(*this != BufferObject());
        bind_buffer_guard bbg(target::target, _id);
        auto p = reinterpret_cast<value_type*>
            (glMapBuffer(target::target, access));
        return p;
    }
    
    /* Invalida o mapeamento do buffer.
     * 
     * precondition: this != BufferObject() 
     * poscondition: map(access) invalidado.
     * 
     * /return false se o conteúdo foi corrompido ou se não estiver mapeado.
     */    
    GLboolean unmap() const
    {
        assert(*this != BufferObject());
        bind_buffer_guard bbg(target::target, _id);
        auto res = glUnmapBuffer(target::target);
        return res;
    }

    /* Número de elementos alocados. */    
    std::size_t size() const noexcept {return _size;}
    
    /* Retorna true se o buffer estiver vazio. */    
    bool empty() const noexcept { return _size == 0; }

    /* Retorna o nome do buffer. */
    GLuint id() const noexcept { return _id; }
    
    /* Retorna o usage do buffer; */    
    GLenum usage() const noexcept { return _usage; }
private:
    GLuint _id{0};
    std::size_t _size{0};
    GLenum _usage{GL_DYNAMIC_DRAW};
    
    /* Área ocupada pelo buffer em bytes */
    std::size_t area() const noexcept { return sizeof(value_type) * _size; }
        
    void alloc_buffer(GLenum usage)
    {
        alloc_cpy_buffer(nullptr, usage);
    }

    template<typename ContiguousIt>
    void alloc_cpy_buffer(ContiguousIt first, GLenum usage)
    {
        _usage = usage;
        glGenBuffers(1, &_id);
        bind_buffer_guard bbg(target::target, _id);
        glBufferData(target::target,
                     area(),
                     first,
                     _usage);
    }

    template<typename BufferObject>
    void copy_from(BufferObject&& o)
    {
        _size = o.size();
        alloc_buffer(o._usage);
        bind_buffer_guard bbgr(GL_COPY_READ_BUFFER, o._id);
        bind_buffer_guard bbgw(GL_COPY_WRITE_BUFFER, _id);
        glCopyBufferSubData(GL_COPY_READ_BUFFER,
                            GL_COPY_WRITE_BUFFER,
                            0, 0,
                            o.area());
    }

    void del_buffer() { glDeleteBuffers(1, &_id); }
};

template<typename T, typename Target>
inline bool operator==(const BufferObject<T, Target>& lhs,
                       const BufferObject<T, Target>& rhs)
{
    if (lhs.size() != rhs.size()) return false;
    auto plhs = lhs.map(GL_READ_ONLY);
    auto prhs = rhs.map(GL_READ_ONLY);
    auto res = std::equal(plhs, plhs + lhs.size(), prhs);
    lhs.unmap();
    rhs.unmap();
    return res;
}

template<typename T, typename Target>
inline bool operator!=(const BufferObject<T, Target>& lhs,
                       const BufferObject<T, Target>& rhs)
{ return !(lhs == rhs); }

//Alias template para instanciação de um VBO(Vertex Array Buffer); 
template<typename ValueType>
using VBO = BufferObject<ValueType, ArrayBuffer<ValueType>>;
 
}
