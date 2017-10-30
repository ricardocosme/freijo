# Freijó
Yet another OpenGL C++ wrapper library because modern OpenGL at C++ world claims modern C++ approach.

## Hello Triangle
```c++
auto program = freijo::program{
    freijo::vertex_shader(vtxSrc).id(),
    freijo::fragment_shader(fragSrc).id(),
};

freijo::VBO<glm::vec3> vertices{
    {-0.5, -0.5, 0},
    {0.5,  -0.5, 0},
    {0.0,   0.5, 0},
};

freijo::VAO vao;
vao.attach(0, vertices);

program.use();
freijo::scoped_vao_bind s(vao); //or without RAII: vao.bind()
glDrawArrays(GL_TRIANGLES, 0, vertices.size());
```
