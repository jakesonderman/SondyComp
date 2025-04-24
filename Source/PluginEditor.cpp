#include "PluginProcessor.h"
#include "PluginEditor.h"

MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor (MyPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Set up the look and feel
    lookAndFeel.setColourScheme({
        juce::Colour(0xFF1E1E1E),  // windowBackground
        juce::Colour(0xFF3E3E3E),  // widgetBackground
        juce::Colour(0xFF4E4E4E),  // menuBackground
        juce::Colour(0xFF5E5E5E),  // outline
        juce::Colour(0xFFAAAAAA),  // defaultText
        juce::Colour(0xFFFFFFFF),  // defaultFill
        juce::Colour(0xFFFF5500),  // highlightedText
        juce::Colour(0xFFFF8800),  // highlightedFill
        juce::Colour(0xFFFF8800)   // menuText
    });
    
    setLookAndFeel(&lookAndFeel);
    
    // Title label
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Sondy Compressor", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    
    // Configure sliders
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        
        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&slider, false);
    };
    
    // Setup each slider with its label
    setupSlider(inputGainSlider, inputGainLabel, "Input Gain");
    setupSlider(outputGainSlider, outputGainLabel, "Output Gain");
    setupSlider(thresholdSlider, thresholdLabel, "Threshold");
    setupSlider(kneeSlider, kneeLabel, "Knee");
    setupSlider(attackTimeSlider, attackTimeLabel, "Attack Time");
    setupSlider(releaseTimeSlider, releaseTimeLabel, "Release Time");
    
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
    addAndMakeVisible(attackEditorLabel);
    attackEditorLabel.setText("Attack Curve", juce::dontSendNotification);
    attackEditorLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(releaseEditorLabel);
    releaseEditorLabel.setText("Release Curve", juce::dontSendNotification);
    releaseEditorLabel.setJustificationType(juce::Justification::centred);
    
    // Set size of the editor
    setSize (800, 600);
    
    // Start the timer to update UI
    startTimerHz(30);
}

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll(lookAndFeel.getCurrentColourScheme().getUIColour(juce::LookAndFeel_V4::ColourScheme::windowBackground));
}

void MyPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    // Title at the top
    titleLabel.setBounds(area.removeFromTop(30));
    
    // Gain reduction meter at the top
    area.removeFromTop(10);
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
    attackEditorLabel.setBounds(attackArea.removeFromTop(20));
    attackWavetableEditor.setBounds(attackArea.reduced(20));
    
    // Release controls
    releaseTimeSlider.setBounds(releaseArea.removeFromTop(100).reduced(20));
    releaseEditorLabel.setBounds(releaseArea.removeFromTop(20));
    releaseWavetableEditor.setBounds(releaseArea.reduced(20));
}

void MyPluginAudioProcessorEditor::timerCallback()
{
    // Update the gain reduction meter
    gainReductionMeter.setGainReduction(processorRef.getCompressor().getCurrentGainReduction());
    gainReductionMeter.setGainReductionHistory(processorRef.getCompressor().getGainReductionHistory());
} 