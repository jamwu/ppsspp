// Copyright (c) 2022- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include "Common/Log.h"
#include "GPU/Software/Rasterizer.h"

struct BinWaitable;

enum class BinItemType {
	TRIANGLE,
	CLEAR_RECT,
	SPRITE,
	LINE,
	POINT,
};

struct BinCoords {
	int x1;
	int y1;
	int x2;
	int y2;

	bool Invalid() const {
		return x2 < x1 || y2 < y1;
	}

	BinCoords Intersect(const BinCoords &range) const;
};

struct BinItem {
	BinItemType type;
	int stateIndex;
	BinCoords range;
	VertexData v0;
	VertexData v1;
	VertexData v2;
};

template <typename T, size_t N>
struct BinQueue {
	BinQueue() {
		items_ = new T[N];
		Reset();
	}
	~BinQueue() {
		delete [] items_;
	}

	void Reset() {
		head_ = 0;
		tail_ = 0;
		size_ = 0;
	}

	size_t Push(const T &item) {
		_dbg_assert_(size_ < N);
		size_++;

		size_t i = tail_++;
		if (tail_ == N)
			tail_ = 0;
		items_[i] = item;
		return i;
	}

	T Pop() {
		_dbg_assert_(!Empty());
		size_t i = head_++;
		if (head_ == N)
			head_ = 0;
		size_--;
		return items_[i];
	}

	size_t Size() const {
		return size_;
	}

	bool Full() const {
		return size_ == N;
	}

	bool Empty() const {
		return size_ == 0;
	}

	T &operator[](size_t index) {
		return items_[index];
	}

	const T &operator[](size_t index) const {
		return items_[index];
	}

	T *items_ = nullptr;
	size_t head_;
	size_t tail_ ;
	size_t size_;
};

class BinManager {
public:
	BinManager();
	~BinManager();

	void UpdateState();

	const Rasterizer::RasterizerState &State() {
		return states_[stateIndex_];
	}

	void AddTriangle(const VertexData &v0, const VertexData &v1, const VertexData &v2);
	void AddClearRect(const VertexData &v0, const VertexData &v1);
	void AddSprite(const VertexData &v0, const VertexData &v1);
	void AddLine(const VertexData &v0, const VertexData &v1);
	void AddPoint(const VertexData &v0);

	void Drain();
	void Flush();

private:
	BinQueue<Rasterizer::RasterizerState, 32> states_;
	int stateIndex_;
	BinCoords scissor_;
	BinQueue<BinItem, 1024> queue_;
	BinCoords queueRange_;

	int maxTasks_ = 1;
	bool tasksSplit_ = false;
	std::vector<BinCoords> taskRanges_;
	BinWaitable *waitable_ = nullptr;

	BinCoords Scissor(BinCoords range);
	BinCoords Range(const VertexData &v0, const VertexData &v1, const VertexData &v2);
	BinCoords Range(const VertexData &v0, const VertexData &v1);
	BinCoords Range(const VertexData &v0);
	void Expand(const BinCoords &range);
};
