#include "MainWindow.h"
#include "../UI/MainComponent.h"
#include "../UI/Colours.h"

MainWindow::MainWindow (const juce::String& name)
    : DocumentWindow (name,
                      DAWColours::darkBackground,
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setContentOwned (new MainComponent(), true);
    setResizable (true, true);
    centreWithSize (1400, 900);
    setVisible (true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
