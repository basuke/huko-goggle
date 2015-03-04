#ifndef _NODE_H_
#define _NODE_H_

#include <Arduino.h>
#include <assert.h>


template<typename T, int SIZE>
class Pool {
public:
	Pool() : _size(0) {}

	byte capacity() const { return SIZE; }
	byte size() const { return _size; }

	T* first() { return _size > 0 ? &_nodes[0] : NULL; }

	bool append(T& node) {
		if (_size >= capacity() && !makeRoom()) return false;
		_nodes[_size] = node;
		_size += 1;
		return true;
	}

	T* get(byte index) {
		if (index >= _size) return NULL;
		return &_nodes[index];
	}

	bool remove(T* node) {
		return remove(findIndex(node));
	}

	bool remove(byte index) {
		if (index >= _size) return false;
		shift(index);
		return true;
	}

private:
	T _nodes[SIZE];
	byte _size;

	bool makeRoom() {
		if (_size == 0) return false;

		shift(0);
		return true;
	}

	void shift(byte index) {
		size_t bytes = (_size - index - 1) * sizeof(T);
		if (bytes > 0) {
			::memcpy(&_nodes[index], &_nodes[index + 1], bytes);
		}
		_size -= 1;
	}

	int findIndex(T* node) {
		T *min = _nodes;
		T *max = &_nodes[_size];

		if (node < min || node >= max) return -1;
		::printf("%x, %x, %x  %d\n", min, max, node, (node - min));
		return (node - min);
	}
};


#endif
