#pragma once
#include <array>
#include <mutex>
#include <semaphore>

template <class A, int numSlot> class JBUFFER
{
	std::array<A, numSlot> buffer;
	std::mutex m_buf;
	std::size_t posRead{ 0 }, posWrite{ 0 };
	std::counting_semaphore<numSlot> smphEmpty{ numSlot }, smphFull{ 0 };

	void doPushHard(auto&& data) {
		smphEmpty.acquire();
		try {
			std::unique_lock<std::mutex> ul(m_buf);
			buffer[posWrite] = std::forward<decltype(data)>(data);
			posWrite = (posWrite + 1) % numSlot;
		}
		catch (...) {
			smphEmpty.release();
			throw;
		}
		smphFull.release();
	}
	bool doPushSoft(auto&& data) {
		bool success = smphEmpty.try_acquire();
		if (!success) { return 0; }
		try {
			std::unique_lock<std::mutex> ul(m_buf);
			buffer[posWrite] = std::forward<decltype(data)>(data);
			posWrite = (posWrite + 1) % numSlot;
		}
		catch (...) {
			smphEmpty.release();
			throw;
		}
		smphFull.release();
		return 1;
	}

public:
	std::atomic_uintmax_t slotSize{ 0 };

	auto pullHard() {
		auto data = std::optional<A>{};
		smphFull.acquire();
		try {
			std::unique_lock<std::mutex> ul(m_buf);
			data = std::move(buffer[posRead]);
			posRead = (posRead + 1) % numSlot;
		}
		catch (...) {
			smphFull.release();
			throw;
		}
		smphEmpty.release();
		return std::move(*data);
	}
	auto pullSoft() {
		auto data = std::optional<A>{};
		bool success = smphFull.try_acquire();
		if (!success) { return std::move(*data); }
		try {
			std::unique_lock<std::mutex> ul(m_buf);
			data = std::move(buffer[posRead]);
			posRead = (posRead + 1) % numSlot;
		}
		catch (...) {
			smphFull.release();
			throw;
		}
		smphEmpty.release();
		return std::move(*data);
	}
	void pushHard(const A& data) { doPushHard(data); }
	void pushHard(A&& data) { doPushHard(std::move(data)); }
	bool pushSoft(const A& data) { return doPushSoft(data); }
	bool pushSoft(A&& data) { return doPushSoft(std::move(data)); }
};