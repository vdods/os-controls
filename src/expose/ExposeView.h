#pragma once
#include "Primitives.h"
#include "graphics/Renderable.h"
#include <autowiring/DispatchQueue.h>
#include <Animation.h>
#include <vector>
#include <tuple>

class ExposeViewWindow;
class ExposeViewEvents;
class OSWindow;
class RenderEngine;
class SVGPrimitive;

/// <summary>
/// Implements expose view
/// </summary>
class ExposeView:
  public std::enable_shared_from_this<ExposeView>,
  public Renderable,
  DispatchQueue
{
public:
  ExposeView(void);
  ~ExposeView(void);
  void AutoInit();
  
private:
  //Root node in the render tree
  Autowired<RenderEngine> m_rootNode;
  
  //Events to send to controller
  AutoFired<ExposeViewEvents> m_exposeViewEvents;

  // Alpha masking value for the entire view
  Animated<float> m_alphaMask;
  
  // All windows currently known to this view:
  std::unordered_set<std::shared_ptr<ExposeViewWindow>> m_windows;

  // Windows represented in order:
  Renderable::ZOrderList m_zorder;

  // Background Overlay Rectangle
  RectanglePrim m_backgroundRect;

private:
  /// <summary>
  /// Evolves the layout by one step
  /// </summary>
  void updateLayout(std::chrono::duration<double> dt);
  
  // Send commend to controller to focus the given window.
  void focusWindow(ExposeViewWindow& windowToFocus);
  
  // Convert a radian angle and a pixel distance to a point.
  // Returns a tuple x,y
  std::tuple<double, double> radialCoordsToPoint(double angle, double distance);

public:
  // RenderEngineNode overrides:
  void AnimationUpdate(const RenderFrame& frame) override;
  void Render(const RenderFrame& frame) const override;


  /// <returns>
  /// True if the ExposeView is presently visible to the user
  /// </returns>
  bool IsVisible(void) const { return 0.001f < m_alphaMask.Current(); }

  /// <summary>
  /// Creates a new ExposeViewWindow for the specified OS window
  /// </summary>
  std::shared_ptr<ExposeViewWindow> NewExposeWindow(OSWindow& osWindow);

  /// <summary>
  /// Removes the specified expose view window from the maintained set
  /// </summary>
  /// <remarks>
  /// This method does not guarantee that the specified ExposeViewWindow is immediately removed.
  /// Actual removal will take place at some point later, depending on what the user is doing and
  /// how long it's going to take any shutdown animations to run.
  ///
  /// Upon return of this call, the specified ExposeViewWindow will not be enumerable from the
  /// ExposeView proper.
  /// </remarks>
  void RemoveExposeWindow(const std::shared_ptr<ExposeViewWindow>& wnd);

  /// <summary>
  /// </summary>
  void StartView();
  void CloseView();

  /// <summary>
  /// Recovers a window from the specified abstract coordinates
  /// </summary>
  std::shared_ptr<ExposeViewWindow> WindowFromPoint(double x, double y) const;
};

