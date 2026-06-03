#pragma once

#include <optional>
#include <unordered_map>

template <class L, class R> class BiMap {
    std::unordered_map<L, R> leftToRight;
    std::unordered_map<R, L> rightToLeft;

public:
    BiMap() = default;

    BiMap(std::initializer_list<std::pair<std::optional<L>, std::optional<R>>>
              init) {
        for (const auto& [l, r] : init) {
            insert(l, r);
        }
    }

    R operator[](L left) {
        return leftToRight[left];
    }

    void insert(std::optional<L> left, std::optional<R> right) {
        if (!left.has_value() || !right.has_value()) {
            return;
        }

        leftToRight.insert({ left.value(), right.value() });
        rightToLeft.insert({ right.value(), left.value() });
    }

    R getLeft(L left) {
        return leftToRight[left];
    }

    bool containsRight(R right) {
        return rightToLeft.contains(right);
    }

    L getRight(R right) {
        return rightToLeft[right];
    }
};
