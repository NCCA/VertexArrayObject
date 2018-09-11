# SimpleIndexVAO
This demo creates a simple Indexed VAO, the data is stored as

Vec3 Vert -> Vec3 Colour in a simple std::array

The index into this for an icosahedron is stored in another  std::array

These are both loaded to the VAO using the setData method

There is an important lesson on Factories and smart pointers in this demo, originally the code used

```
std::unique_ptr<AbstractVAO> m_vao;
```

To store the VAO and we had to use casts to access the extended (as in not in AbstractVAO methods) such as ```setIndices``` and ```draw(int int)``` this lead to clunky code such as

```
reinterpret_cast<MultiBufferIndexVAO *>( m_vao.get())->setIndices(sizeof(indices),&indices[0], GL_UNSIGNED_SHORT);
reinterpret_cast<MultiBufferIndexVAO *>( m_vao.get())->draw(0,m_index*3);
```

To overcome this we can do something like this

```
std::unique_ptr<MultibufferIndexVAO> m_vao ; // in header
// first we are going to create a multibuffer VAO this is actually an std::unique_ptr<AbstractVAO> as it's part of the factory
auto vao=ngl::VAOFactory::createVAO("multiBufferIndexVAO",GL_TRIANGLES);
// We actually want to store our VAO as a std::unique_ptr<MultibufferIndexVAO> so we need to cast it
// first get a raw pointer to the vao
MultiBufferIndexVAO *tmp = dynamic_cast<MultiBufferIndexVAO*>(vao.get());
// check it worked
if(tmp != nullptr)
{
  // now we release the pointer so we can transfer ownership
  vao.release();
  // and re-create using reset (as this is now a MultibufferIndexVAO *)
  m_vao.reset(tmp);
}

```

This is still a little clunky, so I have added the following into AbstractVAO.h in ngl

```
// based on https://stackoverflow.com/questions/36120424/alternatives-of-static-pointer-cast-for-unique-ptr
// this allows us to cast the AbstractVAO to another type useful for extended VAO's especially see the demo
// https://github.com/NCCA/VertexArrayObject/tree/master/ExtendedVAOFactory for more details
template<typename TO>
std::unique_ptr<TO> vaoFactoryCast (std::unique_ptr<AbstractVAO>&& old)
{
    return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
}

```

so the final code looks like

```
m_vao=ngl::vaoFactoryCast<MultiBufferIndexVAO>(ngl::VAOFactory::createVAO("multiBufferIndexVAO",GL_TRIANGLES));

```

And we can just call

```
m_vao->setIndices(sizeof(indices),&indices[0], GL_UNSIGNED_SHORT);
m_vao->draw(m_index,3);
```

Which is much nicer
