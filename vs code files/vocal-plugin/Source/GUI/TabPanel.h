#pragma once
#include <JuceHeader.h>
#include "ProVocalLookAndFeel.h"
#include <vector>
#include <functional>

// Tab switching component for the 4-tab layout
class TabPanel : public juce::Component
{
public:
    TabPanel();

    void addTab(const juce::String& name, juce::Component* content);
    void setActiveTab(int index);
    int getActiveTab() const { return activeTab; }

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;

    std::function<void(int)> onTabChanged;

private:
    struct Tab
    {
        juce::String name;
        juce::Component* content = nullptr;
    };

    std::vector<Tab> tabs;
    int activeTab = 0;

    int getTabWidth() const;
    int getTabAtPosition(int x) const;

    static constexpr int tabHeight = 30;
};
