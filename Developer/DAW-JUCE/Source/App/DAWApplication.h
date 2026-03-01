#pragma once
#include <JuceHeader.h>

class MainWindow;
class DAWLookAndFeel;

class DAWApplication : public juce::JUCEApplication
{
public:
    DAWApplication();
    ~DAWApplication() override;

    const juce::String getApplicationName() override    { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override           { return false; }

    void initialise (const juce::String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;

private:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<DAWLookAndFeel> lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DAWApplication)
};
