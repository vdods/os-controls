#pragma once

#include "Interactionlayer.h"

class GLShader;

class SpheresLayer : public InteractionLayer {
public:
  SpheresLayer();
  //virtual ~SpheresLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  static const int NUM_SPHERES = 400;
  void ComputePhysics(TimeDelta real_time_delta);

  stdvectorV3f m_Colors;
  stdvectorV3f m_Mono;
  std::vector<float> m_Radius;

  stdvectorV3f m_Pos;
  stdvectorV3f m_Disp;
  stdvectorV3f m_Vel;

  float m_Spring;
  float m_Damp;
  float m_Well;
};
