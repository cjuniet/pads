#ifndef _DSAA_H_
#define _DSAA_H_

#include <stdexcept>
#include <functional>
#include <memory>
#include <iostream>

namespace dsaa {

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

  double random()
  {
    return (double) random_integer() / M;
  }

  int random_range(int a, int b)
  {
    return a + (int)((b - a + 1) * random());
  }
};

////////////////////////////////////////////////////////////////////////////////
// 1-2-3 Deterministic Skip List

template<typename Comparable>
class DSL
{
public:
  explicit DSL(const Comparable& inf)
    : infinity(inf)
  {
    bottom = new SkipNode();
    bottom->right = bottom->down = bottom;
    tail = new SkipNode(infinity);
    tail->right = tail;
    header = new SkipNode(infinity, tail, bottom);
  }

  bool contains(const Comparable& x) const
  {
    SkipNode* current = header;
    bottom->element = x;

    for (;;) {
      if (x < current->element) {
        current = current->down;
      } else if (current->element < x) {
        current = current->right;
      } else {
        return current != bottom;
      }
    }
  }

  void insert(const Comparable& x)
  {
    SkipNode* current = header;
    bottom->element = x;

    while (current != bottom) {
      while (current->element < x) {
        current = current->right;
      }

      // if gap size is 3 or at bottom level and must insert, then promote middle element
      if (current->down->right->right->element < current->element) {
        current->right = new SkipNode(current->element, current->right, current->down->right->right);
        current->element = current->down->right->element;
      } else {
        current = current->down;
      }
    }

    // raise height of DSL if necessary
    if (header->right != tail) {
      header = new SkipNode(infinity, tail, header);
    }
  }

private:
  struct SkipNode
  {
    Comparable element;
    SkipNode* right;
    SkipNode* down;

    SkipNode(const Comparable& e = Comparable(), SkipNode* r = 0, SkipNode* d = 0)
      : element(e), right(r), down(d)
    {}
  };

  Comparable infinity;
  SkipNode* header;
  SkipNode* bottom;
  SkipNode* tail;
};

////////////////////////////////////////////////////////////////////////////////
// Binary Search Tree

template<typename T, typename C = std::less<T>, typename A = std::allocator<T> >
class BST
{
public:
  BST() : root(0), count(0) {}
  BST(const BST& rhs) : root(0), count(0) { *this = rhs; }
  ~BST() { clear(); }

  const BST& operator=(const BST& rhs)
  {
    if (this != &rhs) {
      clear();
      deepCopy(root, rhs.root);
    }
    return *this;
  }

  bool empty() const
  {
    return !root;
  }

  size_t size() const
  {
    return count;
  }

  bool contains(const T& t) const
  {
    return contains(t, root);
  }

  const T& findMin() const
  {
    if (empty()) throw std::underflow_error("empty tree");
    return findMin(root)->value;
  }

  const T& findMax() const
  {
    if (empty()) throw std::underflow_error("empty tree");
    return findMax(root)->value;
  }

  void print(std::ostream& os) const
  {
    os << "digraph G {\n";
    print(os, root);
    os << "}\n";
  }

  void insert(const T& t)
  {
    insert(t, root);
  }

  void remove(const T& t)
  {
    remove(t, root);
  }

  void clear()
  {
    clear(root);
    root = 0;
    count = 0;
  }

private:
  struct Node
  {
    Node* left;
    Node* right;
    T value;
  };

  Node* root;
  size_t count;

  C comp;
  A value_alloc;
  typename A::template rebind<Node>::other node_alloc;

private:
  Node* findMin(Node* n) const
  {
    if (n) while (n->left) n = n->left;
    return n;
  }

  Node* findMax(Node* n) const
  {
    if (n) while (n->right) n = n->right;
    return n;
  }

  bool contains(const T& t, Node* n) const
  {
    while (n) {
      if (comp(t, n->value)) {
        n = n->left;
      } else if (comp(n->value, t)) {
        n = n->right;
      } else {
        return true;
      }
    }
    return false;
  }

  void print(std::ostream& os, Node* n) const
  {
    if (n) {
      if (n->left) os << n->value << ":sw -> " << n->left->value << " [color=blue];\n";
      if (n->right) os << n->value << ":se -> " << n->right->value << " [color=red];\n";
      print(os, n->left);
      print(os, n->right);
    }
  }

  void insert(const T& t, Node*& n)
  {
    if (!n) {
      n = node_alloc.allocate(1);
      n->left = n->right = 0;
      n->value = t; // or value_alloc.construct(&n->value, t);
      ++count;
    } else if (comp(t, n->value)) {
      insert(t, n->left);
    } else if (comp(n->value, t)) {
      insert(t, n->right);
    } else {
      n->value = t;
    }
  }

  void remove(const T& t, Node*& n)
  {
    if (!n) {
      return;
    } else if (comp(t, n->value)) {
      remove(t, n->left);
    } else if (comp(n->value, t)) {
      remove(t, n->right);
    } else if (n->left && n->right) {
      // find the smallest node of the right subtree,
      // copy its value and remove it
      n->value = findMin(n->right)->value;
      remove(n->value, n->right);
    } else {
      Node* p = n;
      n = (n->left ? n->left : n->right);
      node_alloc.deallocate(p, 1);
      --count;
    }
  }

  void clear(Node* n)
  {
    if (n) {
      clear(n->left);
      clear(n->right);
      node_alloc.deallocate(n, 1);
      --count;
    }
  }

  void deepCopy(Node*& n, const Node* rhs)
  {
    if (rhs) {
      n = node_alloc.allocate(1);
      n->value = rhs->value; // or value_alloc.construct(&n->value, rhs->value);
      ++count;
      deepCopy(n->left, rhs->left);
      deepCopy(n->right, rhs->right);
    } else {
      n = 0;
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
// Top-Down Splay Tree

template<typename T, typename C = std::less<T>, typename A = std::allocator<T> >
class splay_tree
{
public:
  splay_tree()
  {
    null_node = node_alloc.allocate(1);
    null_node->left = null_node->right = null_node;
    root = null_node;
  }

  splay_tree(const splay_tree& rhs)
  {
    null_node = node_alloc.allocate(1);
    null_node->left = null_node->right = null_node;
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
    node* n = root;
    while (!is_null(n->left)) n = n->left;
    splay(n->value, root);
    return n->value;
  }

  const T& find_max()
  {
    if (empty()) throw std::underflow_error("empty tree");
    node* n = root;
    while (!is_null(n->right)) n = n->right;
    splay(n->value, root);
    return n->value;
  }

  bool contains(const T& t)
  {
    if (empty()) return false;
    splay(t, root);
    return root->value == t;
  }

  void insert(const T& t)
  {
    if (root == null_node) {
      root = node_alloc.allocate(1);
      root->reset(t, null_node, null_node);
    } else {
      splay(t, root);
      if (comp(t, root->value)) {
        node* n = node_alloc.allocate(1);
        n->reset(t, root->left, root);
        root->left = null_node;
        root = n;
      } else if (comp(root->value, t)) {
        node* n = node_alloc.allocate(1);
        n->reset(t, root, root->right);
        root->right = null_node;
        root = n;
      } else {
        root->value = t;
        return;
      }
    }
  }

  void remove(const T& t)
  {
    splay(t, root);
    if (root->value != t) return;

    node* new_root;
    if (is_null(root->left)) {
      new_root = root->right;
    } else {
      new_root = root->left;
      splay(t, new_root);
      new_root->right = root->right;
    }
    node_alloc.deallocate(root, 1);
    root = new_root;
  }

  void clear()
  {
    while (!empty()) {
      find_max();
      remove(root->value);
    }
  }

  void print(std::ostream& os) const
  {
    os << "digraph G {\n";
    print(os, root);
    os << "}\n";
  }

private:
  struct node
  {
    node* left;
    node* right;
    T value;

    node() : value(), left(0), right(0) {}
    node(const T& t, node* l, node* r) : value(t), left(l), right(r) {}
    void reset(const T& t, node* l, node* r) { value = t; left = l; right = r; }
  };

  bool is_null(const node* const n) const { return !n || n == null_node; }

  node* null_node;
  node* root;

  C comp;
  A value_alloc;
  typename A::template rebind<node>::other node_alloc;

private:
  void print(std::ostream& os, node* n) const
  {
    if (!is_null(n)) {
      if (!is_null(n->left)) os << n->value << ":sw -> " << n->left->value << " [color=blue];\n";
      if (!is_null(n->right)) os << n->value << ":se -> " << n->right->value << " [color=red];\n";
      print(os, n->left);
      print(os, n->right);
    }
  }

  node* clone(node* n) const
  {
    if (n == n->left) { // cannot test against null_node...
      return null_node;
    } else {
      node* n = node_alloc.allocate(1);
      n->reset(n->value, clone(n->left), clone(n->right));
      return n;
    }
  }

  void left_rotation(node*& n)
  {
    node* k = n->left;
    n->left = k->right;
    k->right = n;
    n = k;
  }

  void right_rotation(node*& n)
  {
    node* k = n->right;
    n->right = k->left;
    k->left = n;
    n = k;
  }

  void splay(const T& t, node*& n)
  {
    node* leftTreeMax;
    node* rightTreeMin;
    node header;

    header.left = header.right = null_node;
    leftTreeMax = rightTreeMin = &header;

    null_node->value = t; // guarantee a match

    for (;;) {
      if (comp(t, n->value)) {
        if (comp(t, n->left->value)) left_rotation(n);
        if (n->left == null_node) break;
        // link right
        rightTreeMin->left = n;
        rightTreeMin = n;
        n = n->left;
      } else if (comp(n->value, t)) {
        if (comp(n->right->value, t)) right_rotation(n);
        if (n->right == null_node) break;
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

} // namespace dsaa

template<typename T>
std::ostream& operator<<(std::ostream& os, dsaa::BST<T>& bst) { bst.print(os); return os; }

template<typename T>
std::ostream& operator<<(std::ostream& os, dsaa::splay_tree<T>& st) { st.print(os); return os; }

#endif // _DSAA_H_
