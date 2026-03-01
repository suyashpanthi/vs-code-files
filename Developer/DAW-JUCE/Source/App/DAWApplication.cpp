#include "DAWApplication.h"
#include "MainWindow.h"
#include "../UI/DAWLookAndFeel.h"

DAWApplication::DAWApplication() {}
DAWApplication::~DAWApplication() {}

void DAWApplication::initialise (const juce::String&)
{
    lookAndFeel = std::make_unique<DAWLookAndFeel>();
    juce::LookAndFeel::setDefaultLookAndFeel (lookAndFeel.get());
    mainWindow = std::make_unique<MainWindow> (getApplicationName());
}

void DAWApplication::shutdown()
{
    mainWindow = nullptr;
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    lookAndFeel = nullptr;
}

void DAWApplication::systemRequestedQuit()
{
    quit();
}
