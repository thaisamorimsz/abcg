#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"
#include "simulationdata.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onUpdate() override;
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

  SimulationData m_simulationData;

  glm::ivec2 m_viewportSize{};

  std::default_random_engine m_randomEngine;

  ImFont *m_font{};

  int sides;

  void createRegularPolygon(int sides);

  void restart();

  void checkEndOfTheSimulation();

  // void createIrregularPolygon(int sides);
};

#endif