#include "Compressor.h"
#include <cmath>

Compressor::Compressor()
{
    // Initialize attack wavetable with a linear ramp (0 to 1)
    for (int i = 0; i < attackWavetable.size(); ++i)
    {
        attackWavetable[i] = static_cast<float>(i) / (attackWavetable.size() - 1);
    }
    
    // Initialize release wavetable with an exponential decay (1 to 0)
    for (int i = 0; i < releaseWavetable.size(); ++i)
    {
        float t = static_cast<float>(i) / (releaseWavetable.size() - 1);
        releaseWavetable[i] = 1.0f - t;
    }
    
    // Initialize gain reduction history
    for (auto& value : gainReductionHistory)
    {
        value = 0.0f;
    }
}

void Compressor::prepare(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    // Reset envelope state
    currentEnvelope = 0.0f;
    currentGainReduction = 0.0f;
    attackPhase = 0.0f;
    releasePhase = 0.0f;
    inAttack = false;
    inRelease = false;
}

void Compressor::process(juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    
    // Apply input gain
    buffer.applyGain(juce::Decibels::decibelsToGain(inputGain));
    
    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Find the maximum absolute sample value across all channels
        float maxLevel = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);
            maxLevel = std::max(maxLevel, std::abs(channelData[sample]));
        }
        
        // Convert to dB
        float inputLevelDB = maxLevel > 0.0f ? juce::Decibels::gainToDecibels(maxLevel) : -100.0f;
        
        // Calculate gain reduction and update envelope
        updateEnvelope(inputLevelDB);
        
        // Apply gain reduction to all channels
        float gainFactor = juce::Decibels::decibelsToGain(-currentGainReduction);
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            channelData[sample] *= gainFactor;
        }
        
        // Store gain reduction for visualization
        gainReductionHistory[historyIndex] = currentGainReduction;
        historyIndex = (historyIndex + 1) % gainReductionHistory.size();
    }
    
    // Apply output gain
    buffer.applyGain(juce::Decibels::decibelsToGain(outputGain));
}

float Compressor::calculateGainReduction(float inputLevelDB)
{
    // Fixed ratio of 4:1
    const float ratio = 4.0f;
    
    if (inputLevelDB <= threshold - knee / 2.0f)
    {
        // Below threshold - no gain reduction
        return 0.0f;
    }
    else if (inputLevelDB > threshold + knee / 2.0f)
    {
        // Above threshold + knee - full compression with ratio
        return (inputLevelDB - threshold) * (1.0f - 1.0f / ratio);
    }
    else
    {
        // In the knee region - soft knee compression
        float kneeRatio = (inputLevelDB - (threshold - knee / 2.0f)) / knee;
        float slope = 1.0f - 1.0f / ratio;
        return kneeRatio * kneeRatio * (inputLevelDB - threshold) * slope;
    }
}

void Compressor::updateEnvelope(float inputLevelDB)
{
    // Calculate target gain reduction based on the input level
    float targetGainReduction = calculateGainReduction(inputLevelDB);
    
    // If target is greater than current (more reduction needed) -> attack phase
    if (targetGainReduction > currentGainReduction)
    {
        // Start attack phase
        inAttack = true;
        inRelease = false;
        releasePhase = 0.0f;
        
        // Calculate steps per sample for attack
        float samplesPerAttack = attackTime * sampleRate;
        float attackStep = 1.0f / samplesPerAttack;
        
        // Move along attack curve
        attackPhase += attackStep;
        if (attackPhase > 1.0f)
            attackPhase = 1.0f;
        
        // Get attack curve value
        int wavetableIndex = static_cast<int>(attackPhase * (attackWavetable.size() - 1));
        wavetableIndex = juce::jlimit(0, static_cast<int>(attackWavetable.size()) - 1, wavetableIndex);
        float attackCurveValue = attackWavetable[wavetableIndex];
        
        // Apply attack curve
        currentGainReduction = currentGainReduction + attackCurveValue * (targetGainReduction - currentGainReduction);
        
        // Exit attack if we reached target
        if (attackPhase >= 1.0f)
        {
            currentGainReduction = targetGainReduction;
            inAttack = false;
        }
    }
    // If target is less than current (less reduction needed) -> release phase
    else if (targetGainReduction < currentGainReduction)
    {
        // Start release phase
        inRelease = true;
        inAttack = false;
        attackPhase = 0.0f;
        
        // Calculate steps per sample for release
        float samplesPerRelease = releaseTime * sampleRate;
        float releaseStep = 1.0f / samplesPerRelease;
        
        // Move along release curve
        releasePhase += releaseStep;
        if (releasePhase > 1.0f)
            releasePhase = 1.0f;
        
        // Get release curve value
        int wavetableIndex = static_cast<int>(releasePhase * (releaseWavetable.size() - 1));
        wavetableIndex = juce::jlimit(0, static_cast<int>(releaseWavetable.size()) - 1, wavetableIndex);
        float releaseCurveValue = releaseWavetable[wavetableIndex];
        
        // Apply release curve
        float reduction = currentGainReduction - targetGainReduction;
        currentGainReduction = targetGainReduction + reduction * releaseCurveValue;
        
        // Exit release if we reached target
        if (releasePhase >= 1.0f)
        {
            currentGainReduction = targetGainReduction;
            inRelease = false;
        }
    }
    else
    {
        // No change needed - already at target gain reduction
        currentGainReduction = targetGainReduction;
        inAttack = false;
        inRelease = false;
    }
}

void Compressor::setThreshold(float newThreshold)
{
    threshold = newThreshold;
}

void Compressor::setKnee(float newKnee)
{
    knee = newKnee;
}

void Compressor::setInputGain(float newInputGain)
{
    inputGain = newInputGain;
}

void Compressor::setOutputGain(float newOutputGain)
{
    outputGain = newOutputGain;
}

void Compressor::setAttackTime(float newAttackTimeSeconds)
{
    attackTime = newAttackTimeSeconds;
}

void Compressor::setReleaseTime(float newReleaseTimeSeconds)
{
    releaseTime = newReleaseTimeSeconds;
}

void Compressor::setAttackWavetable(const std::array<float, 256>& wavetable)
{
    attackWavetable = wavetable;
}

void Compressor::setReleaseWavetable(const std::array<float, 256>& wavetable)
{
    releaseWavetable = wavetable;
}

const std::array<float, 256>& Compressor::getAttackWavetable() const
{
    return attackWavetable;
}

const std::array<float, 256>& Compressor::getReleaseWavetable() const
{
    return releaseWavetable;
}

float Compressor::getCurrentGainReduction() const
{
    return currentGainReduction;
}

const std::array<float, 256>& Compressor::getGainReductionHistory() const
{
    return gainReductionHistory;
} 