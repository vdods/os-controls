#include "stdafx.h"
#include "ExposeView.h"
#include "ExposeViewWindow.h"
#include "graphics/RenderFrame.h"
#include "SVGPrimitive.h"

ExposeView::ExposeView() :
  m_opacity(0.0f, 0.3f, EasingFunctions::Linear<float>)
{
  
}

ExposeView::~ExposeView() {
  
}

void ExposeView::AutoInit() {
  if(true)
    return;
  
  m_rootNode.NotifyWhenAutowired([this]{
    auto self = shared_from_this();
    m_rootNode->AddChild(self);
    // Add a box as our child
    auto box = std::shared_ptr<SVGPrimitive>(
      new SVGPrimitive(R"svg(<svg  xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"><rect x="10" y="10" height="100" width="100" style="stroke:#ff0000; fill: #0000ff"/></svg>)svg")
    );
    self->AddChild(box);
  });
}

void ExposeView::AnimationUpdate(const RenderFrame& frame) {
  m_opacity.Update(frame.deltaT.count());
  UpdateLayout(frame.deltaT);
}

void ExposeView::Render(const RenderFrame& frame) const {
  for(const auto& renderable : m_renderList)
    renderable->Render(frame);
}

void ExposeView::UpdateLayout(std::chrono::duration<double> dt) {
  for(std::shared_ptr<ExposeViewWindow>& window : m_windows) {
    if(window->m_layoutLocked)
      continue;

    // TODO:  Update the position of the current window
  }
}

std::tuple<double, double> radialCoordsToPoint(double angle, double distance) {
  return std::make_tuple(0.0, 0.0);
}

std::shared_ptr<ExposeViewWindow> ExposeView::NewExposeWindow(OSWindow& osWindow) {
  auto retVal = std::make_shared<ExposeViewWindow>(osWindow);
  m_windows.push_back(retVal);
  return retVal;
}

void ExposeView::StartView() {
  m_opacity.Set(1.0f, 0.3);
}

void ExposeView::CloseView() {
  m_opacity.Set(0.0f, 0.2f);
}

void ExposeView::moveWindowToTop(ExposeViewWindow& window) {
  std::rotate(
    m_renderList.begin(),
    std::find(m_renderList.begin(), m_renderList.end(), &window),
    m_renderList.end()
  );
}