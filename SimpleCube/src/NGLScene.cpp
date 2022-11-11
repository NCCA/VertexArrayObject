#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleIndexVAO.h>
#include <ngl/Transformation.h>
#include <iostream>

NGLScene::NGLScene()
{
  setTitle("Qt5 Simple NGL Demo");
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
  m_vao->removeVAO();
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

constexpr auto ColourShader = "ColourShader";

void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0, 1, 4);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);

  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45, 720.0f / 576.0f, 0.001f, 150);

  // load a frag and vert shaders
  constexpr auto ColourVertex = "ColourVertex";
  constexpr auto ColourFragment = "ColourFragment";
  ngl::ShaderLib::createShaderProgram(ColourShader);

  ngl::ShaderLib::attachShader(ColourVertex, ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader(ColourFragment, ngl::ShaderType::FRAGMENT);
  ngl::ShaderLib::loadShaderSource(ColourVertex, "shaders/ColourVertex.glsl");
  ngl::ShaderLib::loadShaderSource(ColourFragment, "shaders/ColourFragment.glsl");

  ngl::ShaderLib::compileShader(ColourVertex);
  ngl::ShaderLib::compileShader(ColourFragment);
  ngl::ShaderLib::attachShaderToProgram(ColourShader, ColourVertex);
  ngl::ShaderLib::attachShaderToProgram(ColourShader, ColourFragment);

  ngl::ShaderLib::linkProgramObject(ColourShader);

  buildVAO();
}

void NGLScene::buildVAO()
{
  // create a vao as a series of GL_TRIANGLES
  m_vao = ngl::VAOFactory::createVAO(ngl::simpleIndexVAO, GL_TRIANGLES);
  m_vao->bind();

  const static GLubyte indices[] = {
      0, 1, 5, 0, 4, 5, // back
      3, 2, 6, 7, 6, 3, // front
      0, 1, 2, 3, 2, 0, // top
      4, 5, 6, 7, 6, 4, // bottom
      0, 3, 4, 4, 7, 3,
      1, 5, 2, 2, 6, 5};

  GLfloat vertcolour[] = {-1, 1, -1, 1, 0, 0,
                          1, 1, -1, 0, 1, 0,
                          1, 1, 1, 0, 0, 1,
                          -1, 1, 1, 1, 1, 1,
                          -1, -1, -1, 0, 0, 1,
                          1, -1, -1, 0, 1, 0,
                          1, -1, 1, 1, 0, 0,
                          -1, -1, 1, 1, 1, 1};

  // in this case we are going to set our data as the vertices above

  m_vao->setData(ngl::SimpleIndexVAO::VertexData(48 * sizeof(GLfloat), vertcolour[0], sizeof(indices), &indices[0], GL_UNSIGNED_BYTE, GL_STATIC_DRAW));
  // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)
  m_vao->setVertexAttributePointer(0, 3, GL_FLOAT, sizeof(ngl::Vec3) * 2, 0);
  m_vao->setVertexAttributePointer(1, 3, GL_FLOAT, sizeof(ngl::Vec3) * 2, sizeof(ngl::Vec3));
  m_vao->setNumIndices(sizeof(indices));
  // now unbind
  m_vao->unbind();
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Rotation based on the mouse position for our global transform
  ngl::Transformation trans;
  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);

  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  ngl::ShaderLib::use(ColourShader);

  ngl::Mat4 MVP;
  MVP = m_project * m_view * m_mouseGlobalTX;
  ngl::ShaderLib::setUniform("MVP", MVP);
  m_vao->bind();
  m_vao->draw();
  m_vao->unbind();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent(QMouseEvent *_event)
{
// note the method buttons() is the button state when event was called
// that is different from button() which is used to check which button was
// pressed when the mousePress/Release event is generated
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  auto position = _event->position();
#else
  auto position = _event->pos();
#endif
  if (m_win.rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx = position.x() - m_win.origX;
    int diffy = position.y() - m_win.origY;
    m_win.spinXFace += static_cast<int>(0.5f * diffy);
    m_win.spinYFace += static_cast<int>(0.5f * diffx);
    m_win.origX = position.x();
    m_win.origY = position.y();
    update();
  }
  // right mouse translate code
  else if (m_win.translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = static_cast<int>(position.x() - m_win.origXPos);
    int diffY = static_cast<int>(position.y() - m_win.origYPos);
    m_win.origXPos = position.x();
    m_win.origYPos = position.y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent(QMouseEvent *_event)
{
// that method is called when the mouse button is pressed in this case we
// store the value where the maouse was clicked (x,y) and set the Rotate flag to true
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  auto position = _event->position();
#else
  auto position = _event->pos();
#endif
  if (_event->button() == Qt::LeftButton)
  {
    m_win.origX = position.x();
    m_win.origY = position.y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if (_event->button() == Qt::RightButton)
  {
    m_win.origXPos = position.x();
    m_win.origYPos = position.y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent(QMouseEvent *_event)
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

  // check the diff of the wheel position (0 means no change)
  if (_event->angleDelta().x() > 0)
  {
    m_modelPos.m_z += ZOOM;
  }
  else if (_event->angleDelta().x() < 0)
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
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  // turn on wirframe rendering
  case Qt::Key_W:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  // turn off wire frame
  case Qt::Key_S:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
    break;
  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}
