#include "stdafx.h"
#include "OSWindowMonitorWin.h"
#include "OSWindow.h"
#include "OSWindowEvent.h"

HHOOK hhook;

OSWindowMonitorWin::OSWindowMonitorWin(void)
{
  // Set up a hook so we can snag window creation events:
  SetWindowsHookEx(
    WH_SHELL,
    [] (int code, WPARAM wParam, LPARAM lParam) -> LPARAM {
      return CallNextHookEx(hhook, code, wParam, lParam);
    },
    nullptr,
    0
  );

  // Enumerate all top-level windows that we know about right now:
  ;
}

OSWindowMonitorWin::~OSWindowMonitorWin(void)
{
}

OSWindowMonitor* OSWindowMonitor::New(void) {
  return new OSWindowMonitorWin;
}

void OSWindowMonitorWin::Enumerate(const std::function<void(OSWindow&)>& callback) const {
  std::lock_guard<std::mutex> lk(m_lock);
  for(const auto& knownWindow : m_knownWindows)
    callback(*knownWindow.second);
}