# Boid
This demo shows how to create a simple Boid shaped object using the built in MultipleBufferVAO from the VAOFactory

In this case we have to arrays, one of vec3 vertex values for the boids, the second is the normals,

As each buffer is bound when setting the data we must always set the data then the generic vertex attribute values so the code looks like

```
// first buffer
->setData(..)
->setVertexAttributePointer(which, size, 0,0)
// 2nd buffer
->setData(..)
->setVertexAttributePointer(which, size, 0,0)
```
If more than 1 attribute is set in the buffer we just call the appropriate set method whilst the buffer is bound.

In this case we have no way of accessing the buffer or binding it as it is a simple read only, if more complex access is required use this as a basis and register your own extended version in the Factory
