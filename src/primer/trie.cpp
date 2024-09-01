#include "primer/trie.h"
#include <stack>
#include <string_view>
#include "common/exception.h"
namespace bustub {

template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  // throw NotImplementedException("Trie::Get is not implemented.");

  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.

  if (root_ == nullptr) {
    return nullptr;
  }

  if (key.empty()) {
    if (root_->is_value_node_) {
      auto x = std::dynamic_pointer_cast<const TrieNodeWithValue<T>>(root_);
      return x->value_.get();
    }
    return nullptr;
  }

  auto cur_node = root_;

  for (auto iter = key.begin(); iter != key.end(); iter++) {
    auto find = cur_node->children_.find(*iter);
    if (find == cur_node->children_.end()) {
      return nullptr;
    }

    if (iter + 1 == key.end()) {
      if (auto x = std::dynamic_pointer_cast<const TrieNodeWithValue<T>>(find->second)) {
        return x->value_.get();
      }
    }
    cur_node = find->second;
  }
  return nullptr;
}

template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  // throw NotImplementedException("Trie::Put is not implemented.");

  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.

  auto value_ptr = std::make_shared<T>(std::move(value));

  if (key.empty() && root_ == nullptr) {
    auto x = std::make_shared<TrieNodeWithValue<T>>(value_ptr);
    return Trie(x);
  }

  if (key.empty()) {
    const auto x = root_->Clone();
    if (x->is_value_node_) {
      auto res = std::make_shared<TrieNodeWithValue<T>>(x->children_, value_ptr);
      return Trie(res);
    }
    auto res = std::make_shared<TrieNodeWithValue<T>>(x->children_, value_ptr);
    return Trie(res);
  }

  std::shared_ptr<TrieNode> cur_node;

  if (root_ == nullptr) {
    cur_node = std::make_shared<TrieNode>();
  } else {
    cur_node = root_->Clone();
  }

  const std::shared_ptr<TrieNode> res = cur_node;
  std::shared_ptr<TrieNode> last = cur_node;

  auto ite = key.begin();
  while (ite != key.end()) {
    if (auto find = cur_node->children_.find(*ite); find != cur_node->children_.end()) {
      auto x = find->second->Clone();
      const auto new_node = std::shared_ptr<TrieNode>(std::move(x));
      last = cur_node;
      cur_node->children_[*ite] = new_node;
      cur_node = new_node;
    } else {
      break;
    }
    ite++;
  }

  if (ite == key.end()) {
    if (!cur_node->is_value_node_) {
      auto new_node = std::make_shared<TrieNodeWithValue<T>>(cur_node->children_, value_ptr);
      last->children_[*(ite - 1)] = new_node;
    } else {
      auto new_node = std::make_shared<TrieNodeWithValue<T>>(cur_node->children_, value_ptr);
      last->children_[*(ite - 1)] = new_node;
    }
    return Trie(res);
  }

  while (ite != key.end()) {
    if (ite == key.end() - 1) {
      auto new_node = std::make_shared<TrieNodeWithValue<T>>(value_ptr);
      cur_node->children_[*ite] = new_node;
      cur_node = new_node;
      return Trie(res);
    }
    const auto new_node = std::make_shared<TrieNode>();
    cur_node->children_[*ite] = new_node;
    cur_node = new_node;

    ite++;
  }
  return Trie(res);
}

auto Trie::Remove(std::string_view key) const -> Trie {
  // throw NotImplementedException("Trie::Remove is not implemented.");

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.

  std::stack<std::pair<char, std::shared_ptr<TrieNode>>> stack;

  if (root_ == nullptr) {
    return Trie(nullptr);
  }

  if (key.empty()) {
    auto x = root_->Clone();
    const auto res = std::make_shared<const TrieNode>(x->children_);

    if (x->is_value_node_ && !x->children_.empty()) {
      return Trie(res);
    }

    if (!x->children_.empty()) {
      return Trie(res);
    }

    return Trie(nullptr);
  }

  std::shared_ptr<TrieNode> cur_node = root_->Clone();
  auto res = cur_node;
  for (char ch : key) {
    if (auto find = cur_node->children_.find(ch); find != cur_node->children_.end()) {
      auto x = find->second->Clone();
      auto new_node = std::shared_ptr<TrieNode>(std::move(x));
      cur_node->children_[ch] = new_node;
      stack.emplace(ch, new_node);
      cur_node = new_node;
    }
  }

  if (stack.top().first != *key.rbegin()) {
    return Trie(res);
  }

  auto delete_char = '\0';
  auto update_char = '\0';

  std::shared_ptr<TrieNode> updated;
  auto [fst, snd] = stack.top();
  if (snd->children_.empty()) {
    delete_char = fst;
  } else if (snd->is_value_node_) {
    update_char = fst;
    updated = std::make_shared<TrieNode>(snd->children_);
  }
  stack.pop();

  while (!stack.empty()) {
    auto [fst, snd] = stack.top();
    stack.pop();
    if (delete_char != '\0') {
      snd->children_.erase(delete_char);
      if (snd->children_.empty() && !snd->is_value_node_) {
        delete_char = fst;
      } else {
        delete_char = '\0';
      }
    } else if (update_char != '\0') {
      snd->children_[update_char] = updated;
      update_char = '\0';
    }
  }

  if (delete_char == *key.begin()) {
    res->children_.erase(delete_char);
    if (res->children_.empty() && !res->is_value_node_) {
      res = nullptr;
    }
  }
  return Trie(res);
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
