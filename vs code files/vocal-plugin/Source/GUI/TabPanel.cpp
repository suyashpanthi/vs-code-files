#include "TabPanel.h"

using namespace ProVocalColours;

TabPanel::TabPanel() {}

void TabPanel::addTab(const juce::String& name, juce::Component* content)
{
    tabs.push_back({ name, content });
    if (content != nullptr)
    {
        addAndMakeVisible(content);
        content->setVisible(tabs.size() == 1); // Only first tab visible initially
    }
}

void TabPanel::setActiveTab(int index)
{
    if (index < 0 || index >= static_cast<int>(tabs.size()))
        return;

    activeTab = index;

    for (int i = 0; i < static_cast<int>(tabs.size()); ++i)
    {
        if (tabs[static_cast<size_t>(i)].content != nullptr)
            tabs[static_cast<size_t>(i)].content->setVisible(i == activeTab);
    }

    repaint();
    resized();

    if (onTabChanged)
        onTabChanged(activeTab);
}

void TabPanel::paint(juce::Graphics& g)
{
    // Tab bar background
    g.setColour(sectionBg);
    g.fillRect(0, 0, getWidth(), tabHeight);

    int tabW = getTabWidth();

    for (int i = 0; i < static_cast<int>(tabs.size()); ++i)
    {
        int x = i * tabW;
        auto tabRect = juce::Rectangle<int>(x, 0, tabW, tabHeight);

        if (i == activeTab)
        {
            g.setColour(tabActive);
            g.fillRect(tabRect);

            // Active indicator line
            g.setColour(knob);
            g.fillRect(x, tabHeight - 3, tabW, 3);
        }

        g.setColour(i == activeTab ? text : dimText);
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.drawText(tabs[static_cast<size_t>(i)].name, tabRect, juce::Justification::centred);
    }
}

void TabPanel::resized()
{
    auto contentArea = getLocalBounds().withTrimmedTop(tabHeight);

    for (auto& tab : tabs)
    {
        if (tab.content != nullptr)
            tab.content->setBounds(contentArea);
    }
}

int TabPanel::getTabWidth() const
{
    if (tabs.empty()) return 0;
    return getWidth() / static_cast<int>(tabs.size());
}

int TabPanel::getTabAtPosition(int x) const
{
    int tabW = getTabWidth();
    if (tabW <= 0) return -1;
    int index = x / tabW;
    if (index >= 0 && index < static_cast<int>(tabs.size()))
        return index;
    return -1;
}

void TabPanel::mouseDown(const juce::MouseEvent& e)
{
    if (e.y < tabHeight)
    {
        int tab = getTabAtPosition(e.x);
        if (tab >= 0)
            setActiveTab(tab);
    }
}
