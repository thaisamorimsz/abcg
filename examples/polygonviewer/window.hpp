#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onCreate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  GLuint m_vao{};
  GLuint m_vboVertices{};
  GLuint m_vboColors{};
  GLuint m_program{};
  GLuint m_vboPositions{};

  glm::ivec2 m_viewportSize{};

  std::default_random_engine m_randomEngine;

  int sides;

  glm::vec4 m_clearColor{0.906f, 0.910f, 0.918f, 1.00f};

  void createRegularPolygon(int sides);

  // void createIrregularPolygon(int sides);
};

#endif