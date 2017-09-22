#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Transformation.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include "MultiBufferIndexVAO.h"
#include <array>


NGLScene::NGLScene()
{

  setTitle("Qt5 SimpleInexVAO created from VAOFactory NGL Demo");
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  m_vao->removeVAO();
}

void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setShape( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}

void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,1,2);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);

  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45,720.0f/576.0f,0.001f,150.0f);

  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // load a frag and vert shaders
  constexpr auto shaderProgram="Colour";
  constexpr auto vertexShader="ColourVertex";
  constexpr auto fragmentShader="ColourFragment";
  // we are creating a shader called Colour
  shader->createShaderProgram(shaderProgram);
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader(vertexShader,ngl::ShaderType::VERTEX);
  shader->attachShader(fragmentShader,ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource(vertexShader,"shaders/ColourVertex.glsl");
  shader->loadShaderSource(fragmentShader,"shaders/ColourFragment.glsl");
  // compile the shaders
  shader->compileShader(vertexShader);
  shader->compileShader(fragmentShader);
  // add them to the program
  shader->attachShaderToProgram(shaderProgram,vertexShader);
  shader->attachShaderToProgram(shaderProgram,fragmentShader);

  // now we have associated this data we can link the shader
  shader->linkProgramObject(shaderProgram);
  // and make it active ready to load values
  (*shader)[shaderProgram]->use();
  // register our new Factory to draw the VAO
  ngl::VAOFactory::registerVAOCreator("multiBufferIndexVAO", MultiBufferIndexVAO::create);
  ngl::VAOFactory::listCreators();

  buildVAO();
  glViewport(0,0,width(),height());
  startTimer(100);
}


void NGLScene::buildVAO()
{
  // data from here http://rbwhitaker.wikidot.com/index-and-vertex-buffers
  std::array<ngl::Vec3,12> verts=
  {
   {
   ngl::Vec3(-0.26286500f, 0.0000000f, 0.42532500f),
   ngl::Vec3(0.26286500f, 0.0000000f, 0.42532500f),
   ngl::Vec3(-0.26286500f, 0.0000000f, -0.42532500f),
   ngl::Vec3(0.26286500f, 0.0000000f, -0.42532500f),
   ngl::Vec3(0.0000000f, 0.42532500f, 0.26286500f),
   ngl::Vec3(0.0000000f, 0.42532500f, -0.26286500f),
   ngl::Vec3(0.0000000f, -0.42532500f, 0.26286500f),
   ngl::Vec3(0.0000000f, -0.42532500f, -0.26286500f),
   ngl::Vec3(0.42532500f, 0.26286500f, 0.0000000f),
   ngl::Vec3(-0.42532500f, 0.26286500f, 0.0000000f),
   ngl::Vec3(0.42532500f, -0.26286500f, 0.0000000f),
   ngl::Vec3(-0.42532500f, -0.26286500f, 0.0000000f)
  }
  };
  std::array<ngl::Vec3,12> colours=
  {
   {
    ngl::Vec3(1.0f,0.0f,0.0f),
    ngl::Vec3(1.0f,0.55f,0.0f),
    ngl::Vec3(1.0f,0.0f,1.0f),
    ngl::Vec3(0.0f,1.0f,0.0f),
    ngl::Vec3(0.0f,0.0f,1.0f),
    ngl::Vec3(0.29f,0.51f,0.0f),
    ngl::Vec3(0.5f,0.0f,0.5f),
    ngl::Vec3(1.0f,1.0f,1.0f),
    ngl::Vec3(0.0f,1.0f,1.0f),
    ngl::Vec3(0.0f,0.0f,0.0f),
    ngl::Vec3(0.12f,0.56f,1.0f),
    ngl::Vec3(0.86f,0.08f,0.24f)
    }
  };
  std::array<GLshort,60> indices=
  {
    {0, 6,1,0,11, 6,1, 4, 0,1, 8, 4,1,10, 8,2, 5, 3,
    2, 9, 5,2,11, 9,3, 7, 2,3,10, 7,4, 8, 5,4, 9, 0,
    5, 8, 3,5, 9, 4,6,10, 1,6,11, 7,7,10, 6,7,11, 2,
    8,10, 3,9,11, 0}
  };
  // create a vao as a series of GL_TRIANGLES
  m_vao.reset(static_cast<MultiBufferIndexVAO *>( ngl::VAOFactory::createVAO("multiBufferIndexVAO",GL_TRIANGLES)) );
  m_vao->bind();

  // in this case we are going to set our data as the vertices above

  m_vao->setData(MultiBufferIndexVAO::VertexData(verts.size()*sizeof(ngl::Vec3),verts[0].m_x));
  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
  m_vao->setData(MultiBufferIndexVAO::VertexData(colours.size()*sizeof(ngl::Vec3),colours[0].m_x));

  m_vao->setVertexAttributePointer(1,3,GL_FLOAT,0,3);
  // as we are storing the abstract we need to get the concrete here to call setIndices, do a quick cast
  m_vao->setIndices(sizeof(indices),&indices[0], GL_UNSIGNED_SHORT);
  // data is 24 bytes apart ( two Vec3's) first index
  // is 0 second is 3 floats into the data set (i.e. vec3 offset)
  m_vao->setNumIndices(indices.size());

 // now unbind
  m_vao->unbind();


}



void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
  // Rotation based on the mouse position for our global
  // transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_win.spinXFace);
  rotY.rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Colour"]->use();
  m_vao->bind();

  ngl::Transformation t;

  t.setPosition(-1.2f,0.0f,0.0f);
  ngl::Mat4 MVP= m_cam.getVPMatrix()*t.getMatrix()*m_mouseGlobalTX;
  shader->setUniform("MVP",MVP);

  reinterpret_cast<MultiBufferIndexVAO *>( m_vao.get())->draw(0,m_index*3);

  t.setPosition(0.0f,0.0f,0.0f);

   MVP= m_cam.getVPMatrix()*t.getMatrix()*m_mouseGlobalTX;
  shader->setUniform("MVP",MVP);

  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  // draw right one
  m_vao->draw();

  t.setPosition(1.2f,0.0f,0.0f);
  MVP= m_cam.getVPMatrix()*t.getMatrix()*m_mouseGlobalTX;
  shader->setUniform("MVP",MVP);

  reinterpret_cast<MultiBufferIndexVAO *>( m_vao.get())->draw(m_index,3);
  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  m_vao->draw();


  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);



  m_vao->unbind();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent( QMouseEvent* _event )
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if ( m_win.rotate && _event->buttons() == Qt::LeftButton )
  {
    int diffx = _event->x() - m_win.origX;
    int diffy = _event->y() - m_win.origY;
    m_win.spinXFace += static_cast<int>( 0.5f * diffy );
    m_win.spinYFace += static_cast<int>( 0.5f * diffx );
    m_win.origX = _event->x();
    m_win.origY = _event->y();
    update();
  }
  // right mouse translate code
  else if ( m_win.translate && _event->buttons() == Qt::RightButton )
  {
    int diffX      = static_cast<int>( _event->x() - m_win.origXPos );
    int diffY      = static_cast<int>( _event->y() - m_win.origYPos );
    m_win.origXPos = _event->x();
    m_win.origYPos = _event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent( QMouseEvent* _event )
{
  // that method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.origX  = _event->x();
    m_win.origY  = _event->y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if ( _event->button() == Qt::RightButton )
  {
    m_win.origXPos  = _event->x();
    m_win.origYPos  = _event->y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent( QMouseEvent* _event )
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if ( _event->button() == Qt::RightButton )
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent( QWheelEvent* _event )
{

  // check the diff of the wheel position (0 means no change)
  if ( _event->delta() > 0 )
  {
    m_modelPos.m_z += ZOOM;
  }
  else if ( _event->delta() < 0 )
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case Qt::Key_Left : m_index-=3; break;
  case Qt::Key_Right : m_index+=3; break;
  case Qt::Key_Space : m_animate^=true; break;
  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}

void NGLScene::timerEvent(QTimerEvent *)
{
  if(!m_animate) return;
  m_index+=3;
  if(m_index >= 22)
    m_index=0;
  update();
}
