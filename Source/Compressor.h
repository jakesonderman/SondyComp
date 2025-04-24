#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

class Compressor
{
public:
    Compressor();
    ~Compressor() = default;
    
    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    
    // Getters and setters for parameters
    void setThreshold(float newThreshold);
    void setKnee(float newKnee);
    void setInputGain(float newInputGain);
    void setOutputGain(float newOutputGain);
    void setAttackTime(float newAttackTimeSeconds);
    void setReleaseTime(float newReleaseTimeSeconds);
    
    // Wavetable getters and setters
    void setAttackWavetable(const std::array<float, 256>& wavetable);
    void setReleaseWavetable(const std::array<float, 256>& wavetable);
    
    const std::array<float, 256>& getAttackWavetable() const;
    const std::array<float, 256>& getReleaseWavetable() const;
    
    // Get current gain reduction for visualization
    float getCurrentGainReduction() const;
    
    // Get gain reduction history buffer for visualization
    const std::array<float, 256>& getGainReductionHistory() const;
    
private:
    float calculateGainReduction(float inputLevel);
    void updateEnvelope(float inputLevel);
    
    // Parameters
    float threshold = 0.0f;    // dB
    float knee = 0.0f;         // dB
    float inputGain = 0.0f;    // dB
    float outputGain = 0.0f;   // dB
    float attackTime = 0.01f;  // seconds
    float releaseTime = 0.1f;  // seconds
    
    // Envelope follower state
    float currentEnvelope = 0.0f;
    float currentGainReduction = 0.0f;
    
    // Wavetables
    std::array<float, 256> attackWavetable;
    std::array<float, 256> releaseWavetable;
    
    // Attack and release phase trackers
    float attackPhase = 0.0f;
    float releasePhase = 0.0f;
    bool inAttack = false;
    bool inRelease = false;
    
    // For visualization
    std::array<float, 256> gainReductionHistory;
    int historyIndex = 0;
    
    // Sample rate for time calculations
    double sampleRate = 44100.0;
}; 