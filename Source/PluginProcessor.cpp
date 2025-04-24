#include "PluginProcessor.h"
#include "PluginEditor.h"

// Define static parameter IDs
const juce::String MyPluginAudioProcessor::inputGainId = "input_gain";
const juce::String MyPluginAudioProcessor::outputGainId = "output_gain";
const juce::String MyPluginAudioProcessor::thresholdId = "threshold";
const juce::String MyPluginAudioProcessor::kneeId = "knee";
const juce::String MyPluginAudioProcessor::attackTimeId = "attack_time";
const juce::String MyPluginAudioProcessor::releaseTimeId = "release_time";

MyPluginAudioProcessor::MyPluginAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("SondyComp"), 
      {
          std::make_unique<juce::AudioParameterFloat>(inputGainId, "Input Gain", -24.0f, 24.0f, 0.0f),
          std::make_unique<juce::AudioParameterFloat>(outputGainId, "Output Gain", -24.0f, 24.0f, 0.0f),
          std::make_unique<juce::AudioParameterFloat>(thresholdId, "Threshold", -60.0f, 0.0f, -12.0f),
          std::make_unique<juce::AudioParameterFloat>(kneeId, "Knee", 0.0f, 24.0f, 6.0f),
          std::make_unique<juce::AudioParameterFloat>(attackTimeId, "Attack Time", 0.01f, 1.0f, 0.1f),
          std::make_unique<juce::AudioParameterFloat>(releaseTimeId, "Release Time", 0.01f, 3.0f, 0.3f)
      })
{
    // Initialize compressor with default parameter values
    updateCompressorSettings();
}

MyPluginAudioProcessor::~MyPluginAudioProcessor()
{
}

void MyPluginAudioProcessor::updateCompressorSettings()
{
    compressor.setInputGain(parameters.getRawParameterValue(inputGainId)->load());
    compressor.setOutputGain(parameters.getRawParameterValue(outputGainId)->load());
    compressor.setThreshold(parameters.getRawParameterValue(thresholdId)->load());
    compressor.setKnee(parameters.getRawParameterValue(kneeId)->load());
    compressor.setAttackTime(parameters.getRawParameterValue(attackTimeId)->load());
    compressor.setReleaseTime(parameters.getRawParameterValue(releaseTimeId)->load());
}

const juce::String MyPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MyPluginAudioProcessor::acceptsMidi() const
{
    return true;
}

bool MyPluginAudioProcessor::producesMidi() const
{
    return false;
}

bool MyPluginAudioProcessor::isMidiEffect() const
{
    return false;
}

double MyPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MyPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int MyPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MyPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MyPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void MyPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void MyPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize the compressor with the sample rate
    compressor.prepare(sampleRate, samplesPerBlock);
}

void MyPluginAudioProcessor::releaseResources()
{
    // Nothing to release
}

bool MyPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update parameters before processing
    updateCompressorSettings();
    
    // Process the audio through the compressor
    compressor.process(buffer);
}

bool MyPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MyPluginAudioProcessor::createEditor()
{
    return new MyPluginAudioProcessorEditor (*this);
}

void MyPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save parameters
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MyPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore parameters
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        updateCompressorSettings();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MyPluginAudioProcessor();
}