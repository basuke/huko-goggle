#ifndef _EFFECT_H_
#define _EFFECT_H_

#include <Arduino.h>
#include <assert.h>


typedef enum {
	NoTransition = 0,
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut,
} Transition;


class Effect {
public:
	static double transit(Transition effect, double fraction);
	static double linear(double fraction);
	static double easeIn(double fraction);
	static double easeOut(double fraction);
	static double easeInOut(double fraction);
};


template<class T>
class Node {
public:
	byte next;
	T* data() { return &_data; }

private:
	T _data;
};


#define InvalidNodeIndex 255


template<class T>
class NodePool {
public:
	NodePool(byte size) : _size(size), _first(InvalidNodeIndex), _unused(0) {
		assert(size < 255 && size > 0);

		_nodes = static_cast<Node<T>*>(::malloc(sizeof(T) * size));

		size -= 1;
		for (int i = 0; i < size; i++) {
			_nodes[i].next = i++;
		}
		_nodes[size].next = InvalidNodeIndex;
	}

	~NodePool() {
		::free((void*) _nodes);
	}

	Node<T>* first() {
		return NULL;
	}

	Node<T>* add(T& data);
	void free(Node<T>& node);

private:
	byte _size;
	Node<T> *_nodes;
	byte _first;
	byte _unused;

	Node<T>& operator[](int index) {
		assert(index >= 0 && index < _size);
		return _nodes[index];
	}

	Node<T>* fromIndex(byte index);
	byte toIndex(Node<T>* node);
};


#endif
