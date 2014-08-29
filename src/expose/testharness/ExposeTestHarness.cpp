#include "stdafx.h"
#include "ExposeTestHarness.h"
#include "TestStateMachine.h"
#include "ExposeView.h"
#include "uievents/Updatable.h"
#include "osinterface/LeapInput.h"
#include "osinterface/MakesRenderWindowFullScreen.h"
#include "osinterface/OSVirtualScreen.h"
#include <SFML/Window/Event.hpp>

int main(int argc, const char* argv[]) {
  // Make our stuff and start processing:
  AutoConstruct<sf::ContextSettings> contextSettings(0, 0, 16);
  AutoRequired<LeapInput> leapInput;
  AutoRequired<OSVirtualScreen> virtualScreen;
  AutoConstruct<sf::RenderWindow> mw(
    sf::VideoMode(
      (int) virtualScreen->PrimaryScreen().Width(),
      (int) virtualScreen->PrimaryScreen().Height()
    ),
    "Expose Mode Test Harness", sf::Style::None,
    *contextSettings
  );
  AutoRequired<MakesRenderWindowFullScreen>();
  AutoRequired<RenderEngine> renderEngine;

  // Create ExposeView after everything else is set up
  AutoRequired<ExposeView> view;

  // Need to make a state machine
  AutoRequired<TestStateMachine> tss;

  // Begin processing
  AutoGlobalContext()->Initiate();

  // Primary dispatch loop:
  mw->setFramerateLimit(0);
  mw->setVerticalSyncEnabled(true);

  AutoFired<Updatable> upd;
  for(auto then = std::chrono::steady_clock::now();;) {
    // Event dispatch in sf:
    for(sf::Event evt; mw->pollEvent(evt);)
      switch(evt.type) {
        case sf::Event::Closed:
          mw->close();
          AutoCurrentContext()->SignalShutdown(true);
          return 0;
        default:
          break;
    }

    // Broadcast update event to all interested parties:
    auto now = std::chrono::steady_clock::now();
    upd(&Updatable::Tick)(now - then);
    then = now;
  }

  return 0;
}