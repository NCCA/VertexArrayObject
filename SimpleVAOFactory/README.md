# Boid
This demo shows how to create a simple Boid shaped object using the built in SimpleVAO from the VAOFactory

In this case we use the ngl built in SimpleVAO which takes one buffer of data and loads it into the VAO.

We then need to set the setVertexAttributePointer for the data. In this case we have

Vertex * 12
Normal * 12

in a contiguous array so we need to do the following offset for the generic vertex attributes
```
m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
m_vao->setVertexAttributePointer(2,3,GL_FLOAT,0,12*3);
```
Where the first sets the offset to 0 in the buffer (first 0 means tightly packed data), and the second is the 12 * (x,y,z) data offset for the next block of data in the buffer.

The set function always pads to float in this demo so we don't need to take this into account.
