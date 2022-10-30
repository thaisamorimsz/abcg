#ifndef SIMULATIONDATA_HPP_
#define SIMULATIONDATA_HPP_

#include <bitset>

// TODO - add Zoom in and Zoom Out inputs
enum class Input { Right, Left, Restart, ZoomIn, ZoomOut };
enum class State { SimulationInProgress, SimulationEnded };

struct SimulationData {
  State m_state{State::SimulationInProgress};
  std::bitset<5> m_input; // [Right, Left, Restart, ZoomIn, ZoomOut]
};

#endif