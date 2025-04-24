#include "PluginProcessor.h"
#include "PluginEditor.h"

MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor (MyPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), 
      processorRef (p),
      pluginBorder(sondyLookAndFeel)
{
    // Set up our look and feel
    setLookAndFeel(&sondyLookAndFeel);
    
    // Add the border component
    addAndMakeVisible(pluginBorder);
    
    // Configure sliders
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        slider.setColour(juce::Slider::textBoxOutlineColourId, sondyLookAndFeel.getThemeColors().border);
        
        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(14.0f, juce::Font::bold));
        label.attachToComponent(&slider, false);
    };
    
    // Setup each slider with its label
    setupSlider(inputGainSlider, inputGainLabel, "INPUT");
    setupSlider(outputGainSlider, outputGainLabel, "OUTPUT");
    setupSlider(thresholdSlider, thresholdLabel, "THRESHOLD");
    setupSlider(kneeSlider, kneeLabel, "KNEE");
    setupSlider(attackTimeSlider, attackTimeLabel, "ATTACK");
    setupSlider(releaseTimeSlider, releaseTimeLabel, "RELEASE");
    
    // Connect sliders to parameters
    auto& parameters = processorRef.getParameters();
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::inputGainId, inputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::outputGainId, outputGainSlider);
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::thresholdId, thresholdSlider);
    kneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::kneeId, kneeSlider);
    attackTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::attackTimeId, attackTimeSlider);
    releaseTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::releaseTimeId, releaseTimeSlider);
    
    // Gain reduction meter
    addAndMakeVisible(gainReductionMeter);
    
    // Wavetable editors
    addAndMakeVisible(attackWavetableEditor);
    attackWavetableEditor.setIsReleaseMode(false);
    attackWavetableEditor.setWavetable(processorRef.getCompressor().getAttackWavetable());
    attackWavetableEditor.setWavetableChangedCallback([this](const std::array<float, 256>& wavetable) {
        processorRef.getCompressor().setAttackWavetable(wavetable);
    });
    
    addAndMakeVisible(releaseWavetableEditor);
    releaseWavetableEditor.setIsReleaseMode(true);
    releaseWavetableEditor.setWavetable(processorRef.getCompressor().getReleaseWavetable());
    releaseWavetableEditor.setWavetableChangedCallback([this](const std::array<float, 256>& wavetable) {
        processorRef.getCompressor().setReleaseWavetable(wavetable);
    });
    
    // Wavetable labels
    auto setupEditorLabel = [this](juce::Label& label, const juce::String& text) {
        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(14.0f, juce::Font::bold));
    };
    
    setupEditorLabel(attackEditorLabel, "ATTACK CURVE");
    setupEditorLabel(releaseEditorLabel, "RELEASE CURVE");
    
    // Set size of the editor
    setSize (800, 600);
    
    // Start the timer to update UI
    startTimerHz(60);
}

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Draw background animation if enabled
    if (enableBackgroundAnimation)
    {
        drawBackgroundAnimation(g);
    }
    
    // Background is also handled by the border component
}

void MyPluginAudioProcessorEditor::drawBackgroundAnimation(juce::Graphics& g)
{
    const auto& colors = sondyLookAndFeel.getThemeColors();
    
    // Create a subtle animated pattern in the background
    // This will be drawn behind the border
    const int gridSize = 40;
    const float minOpacity = 0.05f;
    const float maxOpacity = 0.12f;
    
    // Draw horizontal lines
    for (int y = -gridSize; y < getHeight() + gridSize; y += gridSize)
    {
        float yPhase = (y + animationPhase * 0.3f) / static_cast<float>(getHeight());
        float opacity = minOpacity + (maxOpacity - minOpacity) * 0.5f * (1.0f + std::sin(yPhase * juce::MathConstants<float>::pi * 8.0f + animationPhase * 0.01f));
        
        g.setColour(colors.accent.withAlpha(opacity));
        
        // Draw a wavy line
        juce::Path path;
        path.startNewSubPath(0, y + std::sin(animationPhase * 0.02f) * 5.0f);
        
        for (int x = 0; x < getWidth(); x += 5)
        {
            float xPhase = static_cast<float>(x) / static_cast<float>(getWidth());
            float offset = std::sin(xPhase * juce::MathConstants<float>::pi * 4.0f + animationPhase * 0.02f) * 3.0f;
            path.lineTo(x, y + offset);
        }
        
        g.strokePath(path, juce::PathStrokeType(0.5f));
    }
    
    // Draw vertical lines
    for (int x = -gridSize; x < getWidth() + gridSize; x += gridSize)
    {
        float xPhase = (x + animationPhase * 0.3f) / static_cast<float>(getWidth());
        float opacity = minOpacity + (maxOpacity - minOpacity) * 0.5f * (1.0f + std::sin(xPhase * juce::MathConstants<float>::pi * 8.0f + animationPhase * 0.01f));
        
        g.setColour(colors.accent.withAlpha(opacity));
        
        // Draw a wavy line
        juce::Path path;
        path.startNewSubPath(x + std::sin(animationPhase * 0.02f) * 5.0f, 0);
        
        for (int y = 0; y < getHeight(); y += 5)
        {
            float yPhase = static_cast<float>(y) / static_cast<float>(getHeight());
            float offset = std::sin(yPhase * juce::MathConstants<float>::pi * 4.0f + animationPhase * 0.02f) * 3.0f;
            path.lineTo(x + offset, y);
        }
        
        g.strokePath(path, juce::PathStrokeType(0.5f));
    }
}

void MyPluginAudioProcessorEditor::resized()
{
    // Update border to match component size
    pluginBorder.setBounds(getLocalBounds());
    
    // Content area with some padding from the border
    auto area = getLocalBounds().reduced(15);
    
    // Allow space for the title bar
    area.removeFromTop(28);
    area.removeFromTop(5); // Extra space after title
    
    // Gain reduction meter at the top
    gainReductionMeter.setBounds(area.removeFromTop(120));
    
    // Main sliders in the middle
    area.removeFromTop(20);
    auto slidersArea = area.removeFromTop(120);
    
    auto sliderWidth = slidersArea.getWidth() / 4;
    
    // Input and threshold on the left
    auto leftArea = slidersArea.removeFromLeft(sliderWidth * 2);
    auto inputArea = leftArea.removeFromLeft(sliderWidth);
    inputGainSlider.setBounds(inputArea.removeFromTop(100).reduced(10));
    
    auto thresholdArea = leftArea;
    thresholdSlider.setBounds(thresholdArea.removeFromTop(100).reduced(10));
    
    // Knee and output on the right
    auto rightArea = slidersArea;
    auto kneeArea = rightArea.removeFromLeft(sliderWidth);
    kneeSlider.setBounds(kneeArea.removeFromTop(100).reduced(10));
    
    auto outputArea = rightArea;
    outputGainSlider.setBounds(outputArea.removeFromTop(100).reduced(10));
    
    // Attack and release controls at the bottom
    area.removeFromTop(20);
    auto bottomArea = area;
    
    auto attackArea = bottomArea.removeFromLeft(bottomArea.getWidth() / 2);
    auto releaseArea = bottomArea;
    
    // Attack controls
    attackTimeSlider.setBounds(attackArea.removeFromTop(100).reduced(20));
    attackEditorLabel.setBounds(attackArea.removeFromTop(25));
    attackWavetableEditor.setBounds(attackArea.reduced(10));
    
    // Release controls
    releaseTimeSlider.setBounds(releaseArea.removeFromTop(100).reduced(20));
    releaseEditorLabel.setBounds(releaseArea.removeFromTop(25));
    releaseWavetableEditor.setBounds(releaseArea.reduced(10));
}

void MyPluginAudioProcessorEditor::timerCallback()
{
    // Update the gain reduction meter
    gainReductionMeter.setGainReduction(processorRef.getCompressor().getCurrentGainReduction());
    gainReductionMeter.setInputLevel(processorRef.getCompressor().getCurrentInputLevel());
    
    // Update background animation
    if (enableBackgroundAnimation)
    {
        animationPhase += animationSpeed;
        if (animationPhase > 1000.0f) // Reset to avoid float precision issues
            animationPhase = 0.0f;
            
        repaint();
    }
} 