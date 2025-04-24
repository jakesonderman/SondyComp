#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "WavetableEditor.h"
#include "GainReductionMeter.h"
#include "SondyLookAndFeel.h"
#include "PluginBorder.h"

//==============================================================================
class MyPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MyPluginAudioProcessorEditor (MyPluginAudioProcessor&);
    ~MyPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;

private:
    // Reference to our processor
    MyPluginAudioProcessor& processorRef;
    
    // Custom look and feel
    SondyLookAndFeel sondyLookAndFeel;
    PluginBorder pluginBorder;
    
    // Background animation properties
    float animationPhase = 0.0f;
    float animationSpeed = 0.2f;
    bool enableBackgroundAnimation = true;
    
    // Parameter sliders
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::Slider thresholdSlider;
    juce::Slider kneeSlider;
    juce::Slider attackTimeSlider;
    juce::Slider releaseTimeSlider;
    
    // Labels
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;
    juce::Label thresholdLabel;
    juce::Label kneeLabel;
    juce::Label attackTimeLabel;
    juce::Label releaseTimeLabel;
    juce::Label attackEditorLabel;
    juce::Label releaseEditorLabel;
    
    // Wavetable editors
    WavetableEditor attackWavetableEditor;
    WavetableEditor releaseWavetableEditor;
    
    // Gain reduction meter
    GainReductionMeter gainReductionMeter;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseTimeAttachment;
    
    // Draw background animation
    void drawBackgroundAnimation(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessorEditor)
};