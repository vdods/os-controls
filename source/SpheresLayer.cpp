#include "SpheresLayer.h"

#include "Primitives.h"

SpheresLayer::SpheresLayer() :
  m_Pos(NUM_SPHERES),
  m_Disp(NUM_SPHERES, Vector3f::Zero()),
  m_Vel(NUM_SPHERES, Vector3f::Zero()),
  m_Colors(NUM_SPHERES),
  m_Mono(NUM_SPHERES),
  m_Radius(NUM_SPHERES),
  m_Spring(1.0f),
  m_Damp(1.0f),
  m_Well(-1.0f) {
  for (int i = 0; i < NUM_SPHERES; i++) {
    float z = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    float theta = (float)rand() / RAND_MAX * 6.28318530718f;
    float xy = std::sqrt(1.0f - z*z);
    float x = xy*cos(theta);
    float y = xy*sin(theta);

    float r = 0.4f + 0.5f * (float)rand() / RAND_MAX;
    float g = 0.4f + 0.5f * (float)rand() / RAND_MAX;
    float b = 0.4f + 0.5f * (float)rand() / RAND_MAX;
    float dist = 0.55f + (float)rand() / RAND_MAX * 0.2f;
    m_Radius[i] = 0.025f + (float)rand() / RAND_MAX * 0.030f;
    m_Pos[i] = Vector3f(0.0f, 1.7f, -5.0f) + Vector3f(x, y, z)*dist;
    m_Colors[i] << r, g, b;
    m_Mono[i] << 0.3333f*(r+g+b),0.3333f*(r+g+b),0.3333f*(r+g+b);
  }
}

void SpheresLayer::Update(TimeDelta real_time_delta) {
  ComputePhysics(real_time_delta);
}

void SpheresLayer::Render(TimeDelta real_time_delta) const {
  glEnable(GL_BLEND);
  m_Shader->Bind();
  const Vector3f desiredLightPos(0, 10, 10);
  const Vector3f lightPos = desiredLightPos - m_EyePos.cast<float>();
  const int lightPosLoc = m_Shader->LocationOfUniform("lightPosition");
  glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

  for (size_t j = 0; j < NUM_SPHERES; j++) {
    float desaturation = 0.2f / (0.2f + m_Disp[j].squaredNorm());
    Vector3f color = m_Colors[j]*(1.0 - desaturation) + m_Mono[j]*desaturation;

    Sphere sphere;
    sphere.SetRadius(m_Radius[j]);
    sphere.Translation() = (m_Pos[j] + m_Disp[j]).cast<double>();
    sphere.SetDiffuseColor(Color(color.x(), color.y(), color.z(), m_Alpha));
    sphere.SetAmbientFactor(0.3f);
    PrimitiveBase::DrawSceneGraph(sphere, m_Renderer);
  }
  m_Shader->Unbind();
  DrawSkeletonHands();
}

EventHandlerAction SpheresLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  default:
    return EventHandlerAction::PASS_ON;
  }
}

void SpheresLayer::ComputePhysics(TimeDelta real_time_delta) {
  const int num_tips = static_cast<int>(m_Tips.size());
  // std::cout << __LINE__ << ":\t   deltaT = " << (deltaT) << std::endl;
  for (int i = 0; i < NUM_SPHERES; i++) {
    static const float K = 10;
    static const float D = 3;
    static const float A = 0.03;
    static const float AA = 0.0006;

    Vector3f accel = -K*m_Spring*m_Disp[i] -D*m_Damp*m_Vel[i];
    // std::cout << __LINE__ << ":\t     num_tips = " << (num_tips) << std::endl;
    for (int j = 0; j < num_tips; j++) {

      // std::cout << __LINE__ << ":\t       (positions[i] - tips[j]).squaredNorm() = " << ((positions[i] - tips[j]).squaredNorm()) << std::endl;
      const Vector3f diff = m_Tips[j] - (m_Pos[i] + m_Disp[i]);
      float distSq = diff.squaredNorm();
      accel += A*m_Well*diff/(AA + distSq*distSq);
      // accel += A*m_Well*diff/(AA + distSq*diff.norm());
    }

    m_Disp[i] += 0.5*m_Vel[i]*real_time_delta;
    m_Vel[i] += accel*real_time_delta;
    m_Disp[i] += 0.5*m_Vel[i]*real_time_delta;
  }
}
