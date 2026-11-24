#include "models/Comment.h"

#include <cstdint>
#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <vector>

static void delete_replies(std::vector<Comment*>& replies) {
    for (Comment* reply : replies) {
        delete reply;
    }
    replies.clear();
}

Comment::Comment()
    : id_(0),
      post_id_(0),
      author_id_(0),
      text_(),
      created_(),
      replies_(),
      collapsed_(false) {}

Comment::Comment(int id, int post_id, int author_id,
                 const std::string& text, const Time& created)
    : id_(id),
      post_id_(post_id),
      author_id_(author_id),
      text_(text),
      created_(created),
      replies_(),
      collapsed_(false) {
    if (id <= 0 || post_id <= 0 || author_id <= 0 || text.empty()) {
        throw std::invalid_argument("Comment fields are invalid");
    }
}

Comment::~Comment() {
    delete_replies(replies_);
}

int Comment::id() const {
    return id_;
}

int Comment::post_id() const {
    return post_id_;
}

int Comment::author_id() const {
    return author_id_;
}

const std::string& Comment::text() const {
    return text_;
}

const Time& Comment::created() const {
    return created_;
}

const std::vector<Comment*>& Comment::replies() const {
    return replies_;
}

bool Comment::is_collapsed() const {
    return collapsed_;
}

void Comment::set_collapsed(bool collapsed) {
    collapsed_ = collapsed;
}

void Comment::add_reply(Comment* reply) {
    if (reply == nullptr || reply == this || reply->post_id() != post_id_ ||
        find(reply->id()) != nullptr || reply->find(id_) != nullptr) {
        throw std::invalid_argument("Reply is invalid or would create a cycle");
    }

    replies_.push_back(reply);
}

bool Comment::remove_reply(int reply_id) {
    for (auto current = replies_.begin(); current != replies_.end(); ++current) {
        if ((*current)->id() == reply_id) {
            delete *current;
            replies_.erase(current);
            return true;
        }
    }

    for (Comment* reply : replies_) {
        if (reply->remove_reply(reply_id)) {
            return true;
        }
    }
    return false;
}

Comment* Comment::find(int target_id) const {
    if (id_ == target_id) {
        return const_cast<Comment*>(this);
    }

    for (Comment* reply : replies_) {
        if (Comment* found = reply->find(target_id)) {
            return found;
        }
    }
    return nullptr;
}

void Comment::write(std::ostream& out) const {
    if (id_ <= 0 || post_id_ <= 0 || author_id_ <= 0 || text_.empty() ||
        text_.size() > 1000000 || replies_.size() > 100000) {
        throw std::runtime_error("Comment cannot be saved because its data is invalid");
    }

    if (id_ > (std::numeric_limits<std::int32_t>::max)() ||
        post_id_ > (std::numeric_limits<std::int32_t>::max)() ||
        author_id_ > (std::numeric_limits<std::int32_t>::max)()) {
        throw std::runtime_error("Comment ID does not fit the file format");
    }

    const std::int32_t saved_id = static_cast<std::int32_t>(id_);
    const std::int32_t saved_post_id = static_cast<std::int32_t>(post_id_);
    const std::int32_t saved_author_id = static_cast<std::int32_t>(author_id_);
    out.write(reinterpret_cast<const char*>(&saved_id), sizeof(saved_id));
    out.write(reinterpret_cast<const char*>(&saved_post_id), sizeof(saved_post_id));
    out.write(reinterpret_cast<const char*>(&saved_author_id), sizeof(saved_author_id));

    const std::uint32_t text_length = static_cast<std::uint32_t>(text_.size());
    out.write(reinterpret_cast<const char*>(&text_length), sizeof(text_length));
    out.write(text_.data(), static_cast<std::streamsize>(text_.size()));
    if (!out.good()) {
        throw std::runtime_error("Failed to write comment fields");
    }

    created_.write(out);

    const std::uint8_t collapsed_flag = collapsed_ ? 1 : 0;
    out.write(reinterpret_cast<const char*>(&collapsed_flag), sizeof(collapsed_flag));
    if (!out.good()) {
        throw std::runtime_error("Failed to write comment collapsed state");
    }

    // Likeable exposes its binary helpers to derived classes rather than
    // declaring Likeable::write() and Likeable::read().
    write_likes(out);

    const std::uint32_t reply_count = static_cast<std::uint32_t>(replies_.size());
    out.write(reinterpret_cast<const char*>(&reply_count), sizeof(reply_count));
    if (!out.good()) {
        throw std::runtime_error("Failed to write comment reply count");
    }

    for (const Comment* reply : replies_) {
        if (reply == nullptr) {
            throw std::runtime_error("Comment contains a null reply");
        }
        reply->write(out);
    }

    if (!out.good()) {
        throw std::runtime_error("Failed to write comment");
    }
}

void Comment::read(std::istream& in) {
    delete_replies(replies_);

    std::int32_t loaded_id = 0;
    std::int32_t loaded_post_id = 0;
    std::int32_t loaded_author_id = 0;
    in.read(reinterpret_cast<char*>(&loaded_id), sizeof(loaded_id));
    in.read(reinterpret_cast<char*>(&loaded_post_id), sizeof(loaded_post_id));
    in.read(reinterpret_cast<char*>(&loaded_author_id), sizeof(loaded_author_id));
    if (!in || loaded_id <= 0 || loaded_post_id <= 0 || loaded_author_id <= 0) {
        throw std::runtime_error("File contains invalid comment IDs");
    }

    std::uint32_t text_length = 0;
    in.read(reinterpret_cast<char*>(&text_length), sizeof(text_length));
    if (!in || text_length == 0 || text_length > 1000000) {
        throw std::runtime_error("File contains an invalid comment text length");
    }

    std::string loaded_text(text_length, '\0');
    in.read(&loaded_text[0], static_cast<std::streamsize>(text_length));
    if (!in) {
        throw std::runtime_error("Failed to read comment text");
    }

    Time loaded_created;
    loaded_created.read(in);

    std::uint8_t collapsed_flag = 0;
    in.read(reinterpret_cast<char*>(&collapsed_flag), sizeof(collapsed_flag));
    if (!in || collapsed_flag > 1) {
        throw std::runtime_error("File contains an invalid comment collapsed state");
    }

    read_likes(in);

    std::uint32_t reply_count = 0;
    in.read(reinterpret_cast<char*>(&reply_count), sizeof(reply_count));
    if (!in || reply_count > 100000) {
        throw std::runtime_error("File contains an invalid comment reply count");
    }

    id_ = static_cast<int>(loaded_id);
    post_id_ = static_cast<int>(loaded_post_id);
    author_id_ = static_cast<int>(loaded_author_id);
    text_.swap(loaded_text);
    created_ = loaded_created;
    collapsed_ = collapsed_flag == 1;

    try {
        for (std::uint32_t index = 0; index < reply_count; ++index) {
            std::unique_ptr<Comment> reply(new Comment());
            reply->read(in);
            add_reply(reply.get());
            reply.release();
        }
    } catch (...) {
        delete_replies(replies_);
        throw;
    }
}
