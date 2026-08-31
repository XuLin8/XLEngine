#pragma once

#include <cstdint>
#include <string>

namespace XLEngine
{
	// FNV-1a 64：由字符串生成稳定句柄，保证"路径 -> 句柄"可重复、可持久化。
	[[nodiscard]] inline std::uint64_t AssetHandleHash(const std::string& key) noexcept
	{
		std::uint64_t hash = 1469598103934665603ULL;
		for (char c : key)
		{
			hash ^= static_cast<std::uint8_t>(c);
			hash *= 1099511628211ULL;
		}
		return hash;
	}

	// 轻量资源句柄：本质是一个 64 位 ID（默认 0 表示无效）。
	// 编辑器内用它作为资产在 AssetRegistry 中的稳定身份。
	class AssetHandle
	{
	public:
		AssetHandle() noexcept : m_Handle(0) {}
		AssetHandle(std::uint64_t handle) noexcept : m_Handle(handle) {}
		AssetHandle(const AssetHandle&) noexcept = default;
		AssetHandle& operator=(const AssetHandle&) noexcept = default;

		[[nodiscard]] operator std::uint64_t() const noexcept { return m_Handle; }
		[[nodiscard]] std::uint64_t Get() const noexcept { return m_Handle; }
		[[nodiscard]] bool IsValid() const noexcept { return m_Handle != 0; }

		friend bool operator==(const AssetHandle& a, const AssetHandle& b) noexcept { return a.m_Handle == b.m_Handle; }
		friend bool operator!=(const AssetHandle& a, const AssetHandle& b) noexcept { return a.m_Handle != b.m_Handle; }
		friend bool operator<(const AssetHandle& a, const AssetHandle& b) noexcept { return a.m_Handle < b.m_Handle; }

	private:
		std::uint64_t m_Handle;
	};

	// 让 AssetHandle 可以做 std::unordered_map 的 key
	struct AssetHandleHashFn
	{
		std::size_t operator()(const AssetHandle& h) const noexcept { return static_cast<std::size_t>(h.Get()); }
	};
}