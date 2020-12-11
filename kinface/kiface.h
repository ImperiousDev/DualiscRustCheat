#pragma once
#include <cstddef>
#include <cstdint>
#include <iostream>
#include "../Iterations/utils.hpp"

namespace kiface
{
	inline unsigned int rust_pid;
	inline HANDLE procHandle;

	template <class T>
	T read(uintptr_t addr)
	{
		T buffer{};

		if (addr < 0xffffff || addr > 0x7fffffff0000 || !rust_pid)
			return buffer;
		ReadProcessMemory(procHandle, reinterpret_cast<LPVOID>(addr), &buffer, sizeof(T), NULL);
		return buffer;
	}

	template <class T>
	bool write(uintptr_t addr, const T& data, bool read_check = true)
	{
		if (!rust_pid)
			return false;

		if (!read<uintptr_t>(addr) && read_check)
			return false;

		return WriteProcessMemory(procHandle, reinterpret_cast<LPVOID>(addr), &data, sizeof(T), NULL);
	}
}