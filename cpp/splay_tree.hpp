#ifndef _SPLAY_TREE_HPP_
#define _SPLAY_TREE_HPP_

#include <functional>
#include <memory>
#include <ostream>
#include <stdexcept>

namespace pads {

////////////////////////////////////////////////////////////////////////////////
// Linear Congruential Generator

class lcg
{
  enum { M = 2147483647, A = 48271, Q = M / A, R = M % A };
  int state;

public:
  explicit lcg(int x0 = 1)
  {
    if (x0 < 0) x0 += M;
    state = (x0 ? x0 : 1);
  }

  int random_integer()
  {
    const int tmp = A * (state % Q) - R * (state / Q);
    return (state = (tmp >= 0 ? tmp : tmp + M));
  }

  double random_double()
  {
    return (double) random_integer() / M;
  }

  int random_integer(int a, int b)
  {
    return a + (int)((b - a + 1) * random_double());
  }
};

////////////////////////////////////////////////////////////////////////////////
// Top-Down Splay Tree

template<typename K, typename T>
struct node
{
  node* left;
  node* right;
  K key;
  T value;

  node()
    : key(), value(), left(0), right(0)
  {}

  node(const K& k, const T& t, node* l = 0, node* r = 0)
    : key(k), value(t), left(l), right(r)
  {}
};

///

template<typename K, typename T,
         typename C = std::less<K>,
         typename A = std::allocator<node<K, T> > >
class splay_tree
{
public:
  splay_tree()
  {
    reset_null_node();
    root = null_node;
  }

  splay_tree(const splay_tree& rhs)
  {
    reset_null_node();
    root = null_node;
    *this = rhs;
  }

  splay_tree& operator=(const splay_tree& rhs)
  {
    if (this != &rhs) {
      clear();
      root = clone(rhs.root);
    }
    return *this;
  }

  ~splay_tree()
  {
    clear();
    node_alloc.deallocate(null_node, 1);
  }

public:
  bool empty() const
  {
    return is_null(root);
  }

  const T& find_min()
  {
    if (empty()) throw std::underflow_error("empty tree");
    node_type* n = root;
    while (!is_null(n->left)) n = n->left;
    splay(n->key, root);
    return n->value;
  }

  const T& find_max()
  {
    if (empty()) throw std::underflow_error("empty tree");
    node_type* n = root;
    while (!is_null(n->right)) n = n->right;
    splay(n->key, root);
    return n->value;
  }

  bool contains(const K& k)
  {
    if (empty()) return false;
    splay(k, root);
    return root->key == k;
  }

  T& operator[](const K& k)
  {
    if (!contains(k)) insert(k, T());
    return root->value;
  }

  bool insert(const K& k, const T& t)
  {
    if (is_null(root)) {
      root = get_new_node(k, t);
    } else {
      splay(k, root);
      if (comp(k, root->key)) {
        node_type* n = get_new_node(k, t, root->left, root);
        root->left = null_node;
        root = n;
      } else if (comp(root->key, k)) {
        node_type* n = get_new_node(k, t, root, root->right);
        root->right = null_node;
        root = n;
      } else {
        root->value = t;
        return false;
      }
    }
    return true;
  }

  void remove(const K& k)
  {
    splay(k, root);
    if (root->key != k) return;

    node_type* new_root;
    if (is_null(root->left)) {
      new_root = root->right;
    } else {
      new_root = root->left;
      splay(k, new_root);
      new_root->right = root->right;
    }
    node_alloc.deallocate(root, 1);
    root = new_root;
  }

  void clear()
  {
    while (!empty()) {
      find_max();
      remove(root->key);
    }
  }

  void print(std::ostream& os) const
  {
    os << "digraph G {\n";
    print(os, root);
    os << '}';
  }

private:
  typedef node<K, T> node_type;

  C comp;
  A value_alloc;
  typename A::template rebind<node_type>::other node_alloc;

  node_type* null_node;
  node_type* root;

  bool is_null(const node_type* const n) const
  {
    return n == null_node;
  }

  void reset_null_node()
  {
    null_node = node_alloc.allocate(1);
    node_alloc.construct(null_node, node_type());
    null_node->left = null_node->right = null_node;
  }

  node_type* get_new_node(const K& k, const T& t, node_type* l = 0, node_type* r = 0)
  {
    node_type* n = node_alloc.allocate(1);
    node_alloc.construct(n, node_type(k, t, (l?l:null_node), (r?r:null_node)));
    return n;
  }

private:
  void print(std::ostream& os, node_type* n) const
  {
    if (!is_null(n)) {
      os << n->key << " [label=\"" << n->key << "\\n'" << n->value << "'\"];\n";
      if (!is_null(n->left)) {
        os << n->key << ":sw -> " << n->left->key << " [color=blue];\n";
      }
      if (!is_null(n->right)) {
        os << n->key << ":se -> " << n->right->key << " [color=red];\n";
      }
      print(os, n->left);
      print(os, n->right);
    }
  }

  node_type* clone(node_type* n) const
  {
    if (n == n->left) { // cannot test against null_node...
      return null_node;
    } else {
      node_type* n = get_new_node(n->key, n->value, clone(n->left), clone(n->right));
      return n;
    }
  }

  void left_rotation(node_type*& n)
  {
    node_type* k = n->left;
    n->left = k->right;
    k->right = n;
    n = k;
  }

  void right_rotation(node_type*& n)
  {
    node_type* k = n->right;
    n->right = k->left;
    k->left = n;
    n = k;
  }

  void splay(const K& k, node_type*& n)
  {
    node_type* leftTreeMax;
    node_type* rightTreeMin;
    node_type header;

    header.left = header.right = null_node;
    leftTreeMax = rightTreeMin = &header;

    null_node->key = k; // guarantee a match

    for (;;) {
      if (comp(k, n->key)) {
        if (comp(k, n->left->key)) left_rotation(n);
        if (is_null(n->left)) break;
        // link right
        rightTreeMin->left = n;
        rightTreeMin = n;
        n = n->left;
      } else if (comp(n->key, k)) {
        if (comp(n->right->key, k)) right_rotation(n);
        if (is_null(n->right)) break;
        // link left
        leftTreeMax->right = n;
        leftTreeMax = n;
        n = n->right;
      } else {
        break;
      }
    }

    leftTreeMax->right = n->left;
    rightTreeMin->left = n->right;
    n->left = header.right;
    n->right = header.left;
  }
};

} // namespace pads

template<typename K, typename T, typename C, typename A>
std::ostream& operator<<(std::ostream& os, pads::splay_tree<K, T, C, A>& st) { st.print(os); return os; }

#endif // _SPLAY_TREE_HPP_
