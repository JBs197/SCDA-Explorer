#pragma once
#include <array>
#include <mutex>
#include <semaphore>

template <class A, int numSlot> class JBUFFER
{
	std::array<A, numSlot> buffer;
	std::mutex m_buf;
	std::size_t posRead{ 0 }, posWrite{ 0 };
	std::counting_semaphore smphEmpty{ numSlot }, smphFull{ 0 };

	void doPush(auto&& data) {
		smphEmpty.acquire();
		std::unique_lock<std::mutex> ul(m_buf);
		buffer[posWrite] = std::forward<decltype(data)>(data);
		posWrite = (posWrite + 1) % numSlot;
		smphFull.release();
	}

public:
	atomic_uintmax_t slotSize{ 0 };

	auto pull() {
		smphFull.acquire();
		std::unique_lock<std::mutex> ul(m_buf);
		auto data = std::move(buffer(posRead));
		posRead = (posRead + 1) % numSlot;
		smphEmpty.release();
		return std::move(*data);
	}
	void push(const A& data) { doPush(data); }
	void push(A&& data) { doPush(std::move(data)); }

};