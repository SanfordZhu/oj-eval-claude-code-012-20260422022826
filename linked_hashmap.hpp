/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */
    
template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;

private:
	// Node structure for hash table and linked list
	struct Node {
		value_type data;
		Node* next_hash;  // next in hash chain
		Node* prev_order; // previous in insertion order
		Node* next_order; // next in insertion order
		size_t hash_val;  // cached hash value

		Node(const value_type& val, size_t h)
			: data(val), next_hash(nullptr), prev_order(nullptr), next_order(nullptr), hash_val(h) {}
	};

	// Hash table bucket
	struct Bucket {
		Node* head;
		Bucket() : head(nullptr) {}
	};

	// Member variables
	Bucket* table;
	size_t table_size;
	size_t num_elements;
	size_t capacity;
	double load_factor;

	// Doubly-linked list for insertion order
	Node* head_order;
	Node* tail_order;

	// Hash function and equality comparator
	Hash hasher;
	Equal key_equal;

	// Helper functions
	size_t get_bucket_index(size_t hash) const {
		return hash % table_size;
	}

	void rehash(size_t new_size) {
		Bucket* new_table = new Bucket[new_size];

		// Reinsert all nodes into new table
		Node* current = head_order;
		while (current) {
			size_t new_index = current->hash_val % new_size;

			// Insert at beginning of chain
			current->next_hash = new_table[new_index].head;
			new_table[new_index].head = current;

			current = current->next_order;
		}

		delete[] table;
		table = new_table;
		table_size = new_size;
		capacity = table_size * load_factor;
	}

	Node* find_node(const Key& key) const {
		size_t hash_val = hasher(key);
		size_t index = get_bucket_index(hash_val);

		Node* current = table[index].head;
		while (current) {
			if (current->hash_val == hash_val && key_equal(current->data.first, key)) {
				return current;
			}
			current = current->next_hash;
		}
		return nullptr;
	}

public:
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	private:
		Node* current;
		const linked_hashmap* container;

	public:
		friend class linked_hashmap;
		friend class const_iterator;

		// The following code is written for the C++ type_traits library.
		// Type traits is a C++ feature for describing certain properties of a type.
		// For instance, for an iterator, iterator::value_type is the type that the
		// iterator points to.
		// STL algorithms and containers may use these type_traits (e.g. the following
		// typedef) to work properly.
		// See these websites for more information:
		// https://en.cppreference.com/w/cpp/header/type_traits
		// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
		// About iterator_category: https://en.cppreference.com/w/cpp/iterator
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::bidirectional_iterator_tag;


		iterator() : current(nullptr), container(nullptr) {}
		iterator(Node* node, const linked_hashmap* cont) : current(node), container(cont) {}
		iterator(const iterator &other) : current(other.current), container(other.container) {}

		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			iterator temp = *this;
			if (current) {
				current = current->next_order;
			}
			return temp;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
			if (current) {
				current = current->next_order;
			}
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			iterator temp = *this;
			if (current) {
				current = current->prev_order;
			} else if (container && container->tail_order) {
				// --end() should go to last element
				current = container->tail_order;
			}
			return temp;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
			if (current) {
				current = current->prev_order;
			} else if (container && container->tail_order) {
				// --end() should go to last element
				current = container->tail_order;
			}
			return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
			return current->data;
		}
		bool operator==(const iterator &rhs) const {
			return current == rhs.current;
		}
		bool operator==(const const_iterator &rhs) const {
			return current == rhs.current;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return current != rhs.current;
		}
		bool operator!=(const const_iterator &rhs) const {
			return current != rhs.current;
		}

		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {
			return &(current->data);
		}
	};
 
	class const_iterator {
	private:
		const Node* current;
		const linked_hashmap* container;

	public:
		friend class linked_hashmap;
		friend class iterator;

		using difference_type = std::ptrdiff_t;
		using value_type = const typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::bidirectional_iterator_tag;

		const_iterator() : current(nullptr), container(nullptr) {}
		const_iterator(const Node* node, const linked_hashmap* cont) : current(node), container(cont) {}
		const_iterator(const const_iterator &other) : current(other.current), container(other.container) {}
		const_iterator(const iterator &other) : current(other.current), container(other.container) {}

		// Similar methods as iterator but returning const references
		const_iterator operator++(int) {
			const_iterator temp = *this;
			if (current) {
				current = current->next_order;
			}
			return temp;
		}

		const_iterator & operator++() {
			if (current) {
				current = current->next_order;
			}
			return *this;
		}

		const_iterator operator--(int) {
			const_iterator temp = *this;
			if (current) {
				current = current->prev_order;
			} else if (container && container->tail_order) {
				// --end() should go to last element
				current = container->tail_order;
			}
			return temp;
		}

		const_iterator & operator--() {
			if (current) {
				current = current->prev_order;
			} else if (container && container->tail_order) {
				// --end() should go to last element
				current = container->tail_order;
			}
			return *this;
		}

		const value_type & operator*() const {
			return current->data;
		}

		bool operator==(const const_iterator &rhs) const {
			return current == rhs.current;
		}

		bool operator==(const iterator &rhs) const {
			return current == rhs.current;
		}

		bool operator!=(const const_iterator &rhs) const {
			return current != rhs.current;
		}

		bool operator!=(const iterator &rhs) const {
			return current != rhs.current;
		}

		const value_type* operator->() const noexcept {
			return &(current->data);
		}
	};
 
	/**
	 * TODO two constructors
	 */
	linked_hashmap() : table_size(16), num_elements(0), load_factor(0.75),
	                   head_order(nullptr), tail_order(nullptr) {
		table = new Bucket[table_size];
		capacity = table_size * load_factor;
	}

	linked_hashmap(const linked_hashmap &other) : table_size(other.table_size), num_elements(0),
	                                              load_factor(other.load_factor),
	                                              head_order(nullptr), tail_order(nullptr),
	                                              hasher(other.hasher), key_equal(other.key_equal) {
		table = new Bucket[table_size];
		capacity = table_size * load_factor;
		try {
			for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
				insert(*it);
			}
		} catch (...) {
			clear();
			delete[] table;
			throw;
		}
	}

	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
		if (this == &other) return *this;

		// Clear current content
		clear();
		delete[] table;

		// Copy parameters
		table_size = other.table_size;
		load_factor = other.load_factor;
		hasher = other.hasher;
		key_equal = other.key_equal;

		// Allocate new table
		table = new Bucket[table_size];
		head_order = tail_order = nullptr;
		num_elements = 0;
		capacity = table_size * load_factor;

		// Copy elements
		try {
			for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
				insert(*it);
			}
		} catch (...) {
			clear();
			delete[] table;
			throw;
		}

		return *this;
	}

	/**
	 * TODO Destructors
	 */
	~linked_hashmap() {
		clear();
		delete[] table;
	}
 
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
		Node* node = find_node(key);
		if (!node) throw index_out_of_bound();
		return node->data.second;
	}
	const T & at(const Key &key) const {
		const Node* node = find_node(key);
		if (!node) throw index_out_of_bound();
		return node->data.second;
	}

	/**
	 * TODO
	 * access specified element
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
		Node* node = find_node(key);
		if (node) {
			return node->data.second;
		}

		// Insert new element with default value
		T default_value = T();
		pair<iterator, bool> result = insert(value_type(key, default_value));
		return result.first.current->data.second;
	}

	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
		return at(key);
	}
 
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
		return iterator(head_order, this);
	}
	const_iterator cbegin() const {
		return const_iterator(head_order, this);
	}

	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
		return iterator(nullptr, this);
	}
	const_iterator cend() const {
		return const_iterator(nullptr, this);
	}

	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
		return num_elements == 0;
	}

	/**
	 * returns the number of elements.
	 */
	size_t size() const {
		return num_elements;
	}

	/**
	 * clears the contents
	 */
	void clear() {
		Node* current = head_order;
		while (current) {
			Node* next = current->next_order;
			delete current;
			current = next;
		}
		head_order = tail_order = nullptr;
		num_elements = 0;

		// Reset hash table buckets
		for (size_t i = 0; i < table_size; ++i) {
			table[i].head = nullptr;
		}
	}
 
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion),
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		// Check if key already exists
		Node* existing = find_node(value.first);
		if (existing) {
			return pair<iterator, bool>(iterator(existing, this), false);
		}

		// Check if rehashing is needed
		if (num_elements + 1 > capacity) {
			rehash(table_size * 2);
		}

		// Create new node
		size_t hash_val = hasher(value.first);
		size_t index = get_bucket_index(hash_val);
		Node* new_node = new Node(value, hash_val);

		// Insert into hash table (at beginning of chain)
		new_node->next_hash = table[index].head;
		table[index].head = new_node;

		// Insert into doubly-linked list (at the end)
		if (!head_order) {
			// First element
			head_order = tail_order = new_node;
		} else {
			// Append to the end
			new_node->prev_order = tail_order;
			tail_order->next_order = new_node;
			tail_order = new_node;
		}

		num_elements++;
		return pair<iterator, bool>(iterator(new_node, this), true);
	}
 
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if (pos == end() || pos.container != this) {
			throw invalid_iterator();
		}

		Node* node = pos.current;
		if (!node) throw invalid_iterator();

		// Remove from hash table
		size_t index = get_bucket_index(node->hash_val);
		Node* curr = table[index].head;
		Node* prev = nullptr;

		while (curr) {
			if (curr == node) {
				if (prev) {
					prev->next_hash = curr->next_hash;
				} else {
					table[index].head = curr->next_hash;
				}
				break;
			}
			prev = curr;
			curr = curr->next_hash;
		}

		// Remove from doubly-linked list
		if (node->prev_order) {
			node->prev_order->next_order = node->next_order;
		} else {
			head_order = node->next_order;
		}

		if (node->next_order) {
			node->next_order->prev_order = node->prev_order;
		} else {
			tail_order = node->prev_order;
		}

		delete node;
		num_elements--;
	}

	/**
	 * Returns the number of elements with key
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const {
		return find_node(key) ? 1 : 0;
	}

	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
		Node* node = find_node(key);
		return iterator(node, this);
	}
	const_iterator find(const Key &key) const {
		const Node* node = find_node(key);
		return const_iterator(node, this);
	}
};

}

#endif
