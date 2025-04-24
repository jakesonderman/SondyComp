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

#include "Compressor.h"

class MyPluginAudioProcessor : public juce::AudioProcessor
{
public:
    MyPluginAudioProcessor();
    ~MyPluginAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Get the compressor for the editor
    Compressor& getCompressor() { return compressor; }
    
    // Parameter Value Tree
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }
    
    // Parameter IDs
    static const juce::String inputGainId;
    static const juce::String outputGainId;
    static const juce::String thresholdId;
    static const juce::String kneeId;
    static const juce::String attackTimeId;
    static const juce::String releaseTimeId;

private:
    // The actual compressor that processes the audio
    Compressor compressor;
    
    // Parameter handling
    juce::AudioProcessorValueTreeState parameters;
    
    // Parameter change handlers
    void updateCompressorSettings();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessor)
};