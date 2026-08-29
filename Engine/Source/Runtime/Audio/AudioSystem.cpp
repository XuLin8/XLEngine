#include "xlpch.h"
#include "Runtime/Audio/AudioSystem.h"

// WinMM: PlaySound 从内存 WAV 播放（SND_MEMORY）
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>

#include <cmath>

namespace XLEngine
{
	namespace
	{
		constexpr uint32_t  kSampleRate = 22050;
		constexpr uint16_t  kBitsPerSample = 16;
		constexpr uint16_t  kChannels = 1;

		constexpr float kPi = 3.14159265358979f;

		// 组装标准 16-bit 单声道 PCM RIFF WAV（带 44 字节头）
		void WriteWav(std::vector<uint8_t>& out, const std::vector<int16_t>& samples)
		{
			const uint32_t dataBytes = (uint32_t)samples.size() * sizeof(int16_t);
			const uint32_t byteRate = kSampleRate * kChannels * kBitsPerSample / 8;
			const uint16_t blockAlign = kChannels * kBitsPerSample / 8;

			out.clear();
			out.reserve(44 + dataBytes);
			auto u16 = [&](uint16_t v) { out.insert(out.end(), { (uint8_t)(v & 0xFF), (uint8_t)(v >> 8) }); };
			auto u32 = [&](uint32_t v) { out.insert(out.end(), { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF), (uint8_t)((v >> 16) & 0xFF), (uint8_t)((v >> 24) & 0xFF) }); };

			out.insert(out.end(), { 'R', 'I', 'F', 'F' });
			u32(36 + dataBytes);
			out.insert(out.end(), { 'W', 'A', 'V', 'E' });
			out.insert(out.end(), { 'f', 'm', 't', ' ' });
			u32(16);
			u16(1);                    // PCM
			u16(kChannels);
			u32(kSampleRate);
			u32(byteRate);
			u16(blockAlign);
			u16(kBitsPerSample);
			out.insert(out.end(), { 'd', 'a', 't', 'a' });
			u32(dataBytes);

			const uint8_t* p = reinterpret_cast<const uint8_t*>(samples.data());
			out.insert(out.end(), p, p + dataBytes);
		}
	}

	std::vector<uint8_t> AudioSystem::s_Ding;
	std::vector<uint8_t> AudioSystem::s_Ambient;
	bool AudioSystem::s_AmbientOn = false;

	void AudioSystem::Init()
	{
		// 生成一次环境氛音并启动低音量循环；PlaySound 实例随后按需生成
		SynthAmbient();
		StartAmbient();
	}

	void AudioSystem::Shutdown()
	{
		StopAmbient();
		PlaySoundW(nullptr, nullptr, 0); // 立即停止当前非循环音
		s_Ding.clear();
		s_Ambient.clear();
	}

	void AudioSystem::Play(const std::vector<uint8_t>& wav, bool loop)
	{
		if (wav.empty())
			return;
		DWORD flags = SND_MEMORY | SND_ASYNC | SND_NODEFAULT;
		if (loop)
			flags |= SND_LOOP;
		PlaySoundW(reinterpret_cast<LPCWSTR>(wav.data()), nullptr, flags);
	}

	void AudioSystem::SynthDing(float baseFreq, float duration, float volume)
	{
		const uint32_t n = (uint32_t)(kSampleRate * duration);
		std::vector<int16_t> samples(n);
		for (uint32_t i = 0; i < n; i++)
		{
			const float t = (float)i / kSampleRate;
			const float env = std::exp(-t * 9.0f); // 快速指数衰减
			// 基频 + 两个柔和泛音，接近钟鸣
			float s = std::sin(2.0f * kPi * baseFreq * t)
				+ 0.5f * std::sin(2.0f * kPi * baseFreq * 2.0f * t)
				+ 0.25f * std::sin(2.0f * kPi * baseFreq * 3.0f * t);
			float v = s * env * volume;
			v = std::clamp(v, -1.0f, 1.0f);
			samples[i] = (int16_t)(v * 32000.0f);
		}
		s_Ding.clear();
		WriteWav(s_Ding, samples);
	}

	void AudioSystem::SynthAmbient()
	{
		// 低通化噪声 ≈ 轻柔风声：相邻采样一阶平滑，音量压低
		const uint32_t n = kSampleRate * 2; // 2 秒循环
		uint32_t seed = 0xDEADBEEFu;
		auto rnd = [&]() -> float {
			seed = seed * 1664525u + 1013904223u;
			return (float)(seed >> 16) / 65536.0f; // [0,1)
		};
		std::vector<int16_t> samples(n);
		float prev = 0.0f;
		for (uint32_t i = 0; i < n; i++)
		{
			float white = rnd() * 2.0f - 1.0f;
			prev = prev * 0.90f + white * 0.10f; // 一阶低通 -> 低频暖噪
			float v = prev * 0.10f;              // 底噪音量
			samples[i] = (int16_t)(v * 32000.0f);
		}
		s_Ambient.clear();
		WriteWav(s_Ambient, samples);
	}

	void AudioSystem::PlayCollect()
	{
		SynthDing(880.0f, 0.18f, 0.35f);
		Play(s_Ding, false);
	}

	void AudioSystem::PlayLight()
	{
		SynthDing(523.25f, 0.6f, 0.5f); // C5，长余韵钟鸣
		Play(s_Ding, false);
	}

	void AudioSystem::StartAmbient()
	{
		if (s_AmbientOn || s_Ambient.empty())
			return;
		s_AmbientOn = true;
		Play(s_Ambient, true);
	}

	void AudioSystem::StopAmbient()
	{
		if (!s_AmbientOn)
			return;
		s_AmbientOn = false;
		PlaySoundW(nullptr, nullptr, 0);
	}
}