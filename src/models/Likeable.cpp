#include "models/Likeable.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <istream>

bool Likeable::toggle_like(int user_id) {
    if (user_id <= 0) {
        throw std::invalid_argument("User ID must be positive");
    }

    const auto found = std::find(liker_ids_.begin(), liker_ids_.end(), user_id);
    if (found != liker_ids_.end()) {
        liker_ids_.erase(found);
        return false;
    }

    liker_ids_.push_back(user_id);
    return true;
}

bool Likeable::remove_like(int user_id) {
    const auto found = std::find(liker_ids_.begin(), liker_ids_.end(), user_id);
    if (found == liker_ids_.end()) {
        return false;
    }

    liker_ids_.erase(found);
    return true;
}

bool Likeable::is_liked_by(int user_id) const {
    return std::find(liker_ids_.begin(), liker_ids_.end(), user_id) != liker_ids_.end();
}

int Likeable::like_count() const {
    return static_cast<int>(liker_ids_.size());
}

void Likeable::write_likes(std::ostream& out) const {
    if (liker_ids_.size() > (std::numeric_limits<std::uint32_t>::max)()) {
        throw std::runtime_error("Too many likes to save");
    }

    const std::uint32_t count = static_cast<std::uint32_t>(liker_ids_.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    if (!out) {
        throw std::runtime_error("Failed to write like count");
    }

    for (int user_id : liker_ids_) {
        if (user_id <= 0 || user_id > (std::numeric_limits<std::int32_t>::max)()) {
            throw std::runtime_error("Invalid user ID in likes");
        }

        const std::int32_t saved_id = static_cast<std::int32_t>(user_id);
        out.write(reinterpret_cast<const char*>(&saved_id), sizeof(saved_id));
        if (!out) {
            throw std::runtime_error("Failed to write a like");
        }
    }
}

void Likeable::read_likes(std::istream& in) {
    std::uint32_t count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in) {
        throw std::runtime_error("Failed to read like count");
    }
    if (count > 1000000) {
        throw std::runtime_error("Like count is too large");
    }

    liker_ids_.clear();
    liker_ids_.reserve(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        std::int32_t user_id = 0;
        in.read(reinterpret_cast<char*>(&user_id), sizeof(user_id));
        if (!in) {
            liker_ids_.clear();
            throw std::runtime_error("Failed to read a like");
        }
        if (user_id <= 0 || is_liked_by(user_id)) {
            liker_ids_.clear();
            throw std::runtime_error("Invalid or duplicate user ID in likes");
        }

        liker_ids_.push_back(static_cast<int>(user_id));
    }
}
