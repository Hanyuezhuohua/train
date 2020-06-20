#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// #include <iostream>
// #include <cstdio>
// #include <cassert>

// #define debug(...) fprintf(stderr, __VA_ARGS__)

#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"
#include "priority_queue.hpp"

namespace sjtu {
template <class Key, class T, class Compare = std::less<Key>>
class map {
public:
	typedef pair<const Key, T> value_type;
public: // iterator type
	class const_iterator;
	class iterator;
private:
	struct __rbt_node {
		typedef bool which_t;
		enum color_t {
			RED, BLACK
		};

		which_t which; // father's which child
		color_t color;
		__rbt_node *prev, *next;
		__rbt_node *father, *child[2];
		value_type *value; // easy to check whether the node has info

		__rbt_node() : value(nullptr) {}
		~__rbt_node() {
			delete value;
		}

		__rbt_node(__rbt_node *other, __rbt_node *father, __rbt_node *prev, __rbt_node *next) :
			value(new value_type(*other->value)), father(father), prev(prev), next(next),
			which(other->which), color(other->color) {
			child[LEFT] = child[RIGHT] = nullptr;
			prev->next = next->prev = this;
			if (other->child[LEFT] != nullptr)
				child[LEFT] = new __rbt_node(other->child[LEFT], this, prev, this);
			if (other->child[RIGHT] != nullptr)
				child[RIGHT] = new __rbt_node(other->child[RIGHT], this, this, next);
		}
	
		void update_links() {
			next->prev = this;
			prev->next = this;
			if (father != nullptr) father->child[which] = this;
			if (child[LEFT] != nullptr) {
				child[LEFT]->father = this;
				child[LEFT]->which = LEFT;
			}
			if (child[RIGHT] != nullptr) {
				child[RIGHT]->father = this;
				child[RIGHT]->which = RIGHT;
			}
		}
		__rbt_node(const value_type &value, __rbt_node *prev, __rbt_node *next, __rbt_node *father, __rbt_node *left_child, __rbt_node *right_child, which_t which) :
			value(new value_type(value)), prev(prev), next(next), father(father), which(which), color(RED) {
			child[LEFT] = left_child;
			child[RIGHT] = right_child;
			update_links();
		}
		__rbt_node(const Key &key, __rbt_node *prev, __rbt_node *next, __rbt_node *father, __rbt_node *left_child, __rbt_node *right_child, which_t which) :
			value(new value_type(value_type(key, T()))),  prev(prev), next(next), father(father), which(which), color(RED) {
			child[LEFT] = left_child;
			child[RIGHT] = right_child;
			update_links();
		}

		__rbt_node *brother() {
			return father->child[which ^ 1];
		}
	};

private:
	typedef __rbt_node* link_type;
	typedef typename __rbt_node::which_t which_type;
	link_type head, tail, root;
	Compare compare;
	size_t __size;

	static constexpr typename __rbt_node::which_t LEFT = false;
	static constexpr typename __rbt_node::which_t RIGHT = true;
	static constexpr typename __rbt_node::color_t RED = __rbt_node::RED;
	static constexpr typename __rbt_node::color_t BLACK = __rbt_node::BLACK;

	void link_node(link_type a, link_type b) {
		a->next = b;
		b->prev = a;
	}

	void swap_node(link_type a, link_type b) { // for erase
		if (root == a)
			root = b;
		else if (root == b)
			root = a;
		swap(a->prev, b->prev);
		swap(a->next, b->next);
		swap(a->father, b->father);
		swap(a->child[LEFT], b->child[LEFT]);
		swap(a->child[RIGHT], b->child[RIGHT]);
		swap(a->which, b->which);
		swap(a->color, b->color);
		if (a->prev == a) a->prev = b;
		if (a->next == a) a->next = b;
		if (a->father == a) a->father = b;
		if (a->child[LEFT] == a) a->child[LEFT] = b;
		if (a->child[RIGHT] == a) a->child[RIGHT] = b;
		if (b->prev == b) b->prev = a;
		if (b->next == b) b->next = a;
		if (b->father == b) b->father = a;
		if (b->child[LEFT] == b) b->child[LEFT] = a;
		if (b->child[RIGHT] == b) b->child[RIGHT] = a;
		a->update_links();
		b->update_links();
	}

	void rotate(link_type x, which_type which) {
		link_type y = x->child[which ^ 1];
		if (root == x) root = y;
		x->child[which ^ 1] = y->child[which];
		y->father = x->father;
		y->which = x->which;
		x->father = y;
		x->which = which;
		x->update_links();
		y->update_links();
	}

	void insert_rebalance(link_type target) {
		link_type father = target->father;
		if (father == nullptr) {
			target->color = BLACK;
			return;
		}
		if (father->color == BLACK) return;
		link_type grandfather = father->father;
		link_type uncle = target->father->brother();
		if (uncle == nullptr || uncle->color == BLACK) {
			if (target->which == father->which) {
				father->color = BLACK;
				grandfather->color = RED;
				rotate(grandfather, father->which ^ 1);
			} else {
				target->color = BLACK;
				grandfather->color = RED;
				rotate(father, target->which ^ 1);
				rotate(grandfather, target->which ^ 1);
			}
		} else {
			father->color = uncle->color = BLACK;
			grandfather->color = RED;
			insert_rebalance(grandfather);
		}
	}

	void erase_rebalance(link_type target, bool recursive = false) {
		if (target->color == RED && !recursive) return; // no need to change
		link_type child = target->child[target->child[LEFT] == nullptr];
		if (child != nullptr && child->color == RED && !recursive) {
			child->color = BLACK;
			return;
		}
		if (target == root) { // root is black
			target->color = BLACK;
			return;
		}
		link_type father = target->father;
		// assert(target != root);
		// assert(father != nullptr);
		link_type brother = target->brother();
		// assert(brother != nullptr);
		__rbt_node **cousin = brother->child;

		if (father->color == BLACK && brother->color == BLACK &&
			(cousin[LEFT] == nullptr || cousin[LEFT]->color == BLACK) &&
			(cousin[RIGHT] == nullptr || cousin[RIGHT]->color == BLACK)) {
			brother->color = RED;
			erase_rebalance(father, true);
			return;
		}

		if (brother->color == RED) {
			father->color = RED;
			brother->color = BLACK;
			rotate(father, target->which);
			brother = target->brother();
			cousin = brother->child;
		}

		if (father->color == RED && brother->color == BLACK &&
			(cousin[LEFT] == nullptr || cousin[LEFT]->color == BLACK) &&
			(cousin[RIGHT] == nullptr || cousin[RIGHT]->color == BLACK)) {
			father->color = BLACK;
			brother->color = RED;
			return;
		}

		if (cousin[!target->which] == nullptr || cousin[!target->which]->color == BLACK) {
			cousin[target->which]->color = BLACK;
			brother->color = RED;
			rotate(brother, !(target->which));
			brother = target->brother();
			cousin = brother->child;
		}

		swap(father->color, brother->color);
		cousin[!target->which]->color = BLACK;
		rotate(father, target->which); // make target up
	}

	template <typename U> pair<link_type, bool> __insert(const Key &key, const U &value) {
		if (root == nullptr) {
			link_type new_node = new __rbt_node(value, head, tail, nullptr, nullptr, nullptr, LEFT);
			root = new_node;
			++__size;
			insert_rebalance(new_node);
			return {new_node, true};
		}
		link_type cur = root;
		which_type which;
		while (true) {
			which = compare(cur->value->first, key);
			if (!which && !compare(key, cur->value->first)) return {cur, false};
			if (cur->child[which] == nullptr) break;
			cur = cur->child[which];
		}
		link_type new_node = new __rbt_node(value, which ? cur : cur->prev, which ? cur->next : cur, cur, nullptr, nullptr, which);
		++__size;
		insert_rebalance(new_node);
		return {new_node, true};
	}

	pair<link_type, bool> __insert(const value_type &value) {
		return __insert(value.first, value);
	}
	link_type __insert(const Key &key) {
		return __insert(key, key).first;
	}
	void erase(link_type target) {
		--__size;
		if (target->child[LEFT] != nullptr && target->child[RIGHT] != nullptr) {
			swap_node(target, target->next);
		}
		// debug("erase rebalance start!!!");
		erase_rebalance(target);
		// debug("erase rebalance end!!!");
		link_node(target->prev, target->next);
		link_type child = target->child[target->child[LEFT] == nullptr];
		if (target == root)
			root = child;
		else
			target->father->child[target->which] = child;
		if (child != nullptr) {
			child->father = target->father;
			child->which = target->which;
		}
		delete target;
	}
public:
	map() : head(new __rbt_node), tail(new __rbt_node), root(nullptr), compare(), __size(0) {
		head->next = tail;
		tail->prev = head;
	}
	map(const map &other) : map() {
		if (other.__size == 0) return;
		__size = other.__size;
		root = new __rbt_node(other.root, nullptr, head, tail);
	}
	/**
	 * TODO assignment operator
	 */
	map &operator=(const map &other) {
		if (this == &other) return *this;
		clear();
		__size = other.__size;
		if (__size == 0) return *this;
		root = new __rbt_node(other.root, nullptr, head, tail);
		return *this;
	}
	~map() {
		clear();
		delete head;
		delete tail;
	}

	T & at(const Key &key) {
		iterator it = find(key);
		if (it == end()) throw index_out_of_bound();
		return it->second;
	}
	const T & at(const Key &key) const {
		const_iterator it = find(key);
		if (it == cend()) throw index_out_of_bound();
		return it->second;
	}
	T & operator[](const Key &key) {
		return __insert(key)->value->second;
	}
	const T & operator[](const Key &key) const {
		return at(key);
	}
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
		return iterator(this, head->next);
	}
	const_iterator cbegin() const {
		return const_iterator(this, head->next);
	}
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
		return iterator(this, tail);
	}
	const_iterator cend() const {
		return const_iterator(this, tail);
	}
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
		return __size == 0;
	}
	size_t size() const {
		return __size;
	}
	void clear() {
		__size = 0;
		link_type cur = head->next;
		while (cur != tail) {
			link_type next_node = cur->next;
			delete cur;
			cur = next_node;
		}
		head->next = tail;
		tail->prev = head;
		root = nullptr;
	}
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		auto result = __insert(value);
		return {iterator(this, result.first), result.second};
	}
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if (pos.__map != this || pos == end()) throw invalid_iterator();
		erase(pos.node);
	}
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 * The default method of check the equivalence is !(a < b || b > a)
	 */
	size_t count(const Key &key) const {
		return find(key) == cend() ? 0 : 1;
	}
	iterator find(const Key &key) {
		auto cur = root;
		while (cur != nullptr) {
			if (compare(key, cur->value->first))
				cur = cur->child[LEFT];
			else if (compare(cur->value->first, key))
				cur = cur->child[RIGHT];
			else
				return iterator(this, cur);
		}
		return end();
	}
	const_iterator find(const Key &key) const {
		auto cur = root;
		while (cur != nullptr) {
			if (compare(key, cur->value->first))
				cur = cur->child[LEFT];
			else if (compare(cur->value->first, key))
				cur = cur->child[RIGHT];
			else
				return const_iterator(this, cur);
		}
		return cend();
	}
public:
	class iterator {
		friend class const_iterator;
		friend void map::erase(iterator);
	public:
		typedef pair<const Key, T> value_type;
		typedef value_type * pointer;
		typedef value_type & reference;
		typedef __rbt_node * link_type;
	private:
		map *__map;
		__rbt_node *node;
	public:
		iterator() : __map(nullptr), node(nullptr) {}
		iterator(map *__map, __rbt_node *__node) : __map(__map), node(__node) {}
		iterator(const iterator &other) = default;
		iterator &operator =(const iterator &other) = default;

		operator const_iterator() { // must for transform type 'iterator' to type 'const_iterator'
			return const_iterator(*this);
		}

		const iterator operator++(int) {
			iterator backup(*this);
			operator++();
			return backup;
		}
		iterator & operator++() {
			if (node == nullptr || node == __map->tail) throw invalid_iterator();
			node = node->next;
			return *this;
		}
		const iterator operator--(int) {
			iterator backup(*this);
			operator--();
			return backup;
		}
		iterator & operator--() {
			if (node == nullptr || node == __map->head->next) throw invalid_iterator();
			node = node->prev;
			return *this;
		}
		reference operator*() const {
			return *operator->();
		}
		pointer operator->() const noexcept {
			if (node == nullptr || node == __map->tail) throw invalid_iterator();
			return node->value;
		}

		bool operator==(const iterator &rhs) const {
			return node == rhs.node;
		}
		bool operator==(const const_iterator &rhs) const {
			return node == rhs.node;
		}
		bool operator!=(const iterator &rhs) const {
			return node != rhs.node;
		}
		bool operator!=(const const_iterator &rhs) const {
			return node != rhs.node;
		}
	};
	class const_iterator {
		friend class iterator;
	public:
		typedef const pair<const Key, T> value_type;
		typedef value_type * pointer;
		typedef value_type & reference;
	private:
		const map *__map;
		const __rbt_node *node;
	public:
		const_iterator() : __map(nullptr), node(nullptr) {}
		const_iterator(const map *__map, const __rbt_node *node) : __map(__map), node(node) {}
		const_iterator(const const_iterator &other) = default;
		explicit const_iterator(const iterator &other) : __map(other.__map), node(other.node) {}
		const_iterator &operator=(const const_iterator &other) = default;

		operator iterator() { // for const_iterator to iterator;
			return iterator(*this);
		}

		const const_iterator operator++(int) {
			const_iterator backup(*this);
			operator++();
			return backup;
		}
		const_iterator & operator++() {
			if (node == nullptr || node == __map->tail) throw invalid_iterator();
			node = node->next;
			return *this;
		}
		const const_iterator operator--(int) {
			const_iterator backup(*this);
			operator--();
			return backup;
		}
		const_iterator & operator--() {
			if (node == nullptr || node == __map->head->next) throw invalid_iterator();
			node = node->prev;
			return *this;
		}
		reference operator*() const {
			return *operator->();
		}
		pointer operator->() const noexcept {
			if (node == nullptr || node == __map->tail) throw invalid_iterator();
			return node->value;
		}

		bool operator==(const iterator &rhs) const {
			return node == rhs.node;
		}
		bool operator==(const const_iterator &rhs) const {
			return node == rhs.node;
		}
		bool operator!=(const iterator &rhs) const {
			return node != rhs.node;
		}
		bool operator!=(const const_iterator &rhs) const {
			return node != rhs.node;
		}
	};
	
};

}

#endif
