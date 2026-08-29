#pragma once

#include <cstdint>
#include <vector>

namespace XLEngine
{
	// P1-4 轻量音频系统（WinMM PlaySound，零音频素材、零第三方依赖）。
	// 运行时合成短促的 16-bit PCM「叮铃声」用于玩法反馈（收集光尘 / 点亮灯台），
	// 并提供一个低音量循环环境底噪（氛围用）。均为内存 WAV，退出时统一停止。
	class AudioSystem
	{
	public:
		static void Init();
		static void Shutdown();

		// 收集光尘：明亮短促的高音叮
		static void PlayCollect();
		// 点亮灯台：更响更长、带泛音的和声叮
		static void PlayLight();

		// 环境氛围：低频暖噪循环（音量为底噪，不影响反馈音）
		static void StartAmbient();
		static void StopAmbient();

	private:
		// 生成一段带能量衰减的正弦合成音为内存 WAV
		static void SynthDing(float baseFreq, float duration, float volume);
		static void SynthAmbient();
		static void Play(const std::vector<uint8_t>& wav, bool loop);

		static std::vector<uint8_t> s_Ding;   // 最近一次反馈音（SND_MEMORY 期间须保活）
		static std::vector<uint8_t> s_Ambient; // 环境循环音（须持续保活）
		static bool s_AmbientOn;
	};
}