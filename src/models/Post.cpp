#include "models/Post.h"
#include "models/Comment.h"

#include <algorithm>
#include <cstdint>
#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <vector>
using namespace std;

int Post::next_id_ = 1;

Post::Post()
    : id_(generate_id()),
      author_id_(0),
      created_(),
      caption_(),
      deleted_flag_(false),
      comments_() {}

Post::Post(int author_id, const Time& created, const string& caption)
    : id_(0),
      author_id_(author_id),
      created_(created),
      caption_(caption),
      deleted_flag_(false),
      comments_() {
    if (author_id <= 0) {
        throw invalid_argument("Author ID must be positive");
    }
    id_ = generate_id();
}

Post::~Post() {
    for (Comment* comment : comments_) {
        delete comment;
    }
    comments_.clear();
}

void Post::set_next_id(int value) {
    if (value > next_id_) {
        next_id_ = value;
    }
}

int Post::generate_id() {
    if (next_id_ <= 0 || next_id_ == (numeric_limits<int>::max)()) {
        throw overflow_error("Post ID exhausted");
    }
    return next_id_++;
}

int Post::id() const {
    return id_;
}

int Post::author_id() const {
    return author_id_;
}

const Time& Post::created() const {
    return created_;
}

const string& Post::caption() const {
    return caption_;
}

bool Post::is_deleted() const {
    return deleted_flag_;
}

void Post::set_caption(const string& value) {
    caption_ = value;
}

void Post::set_deleted(bool value) {
    deleted_flag_ = value;
}

void Post::add_comment(Comment* comment) {
    if (comment == nullptr) {
        throw invalid_argument("Comment cannot be null");
    }
    comments_.push_back(comment);
}

bool Post::remove_comment(int target) {
    const auto found = find_if(
        comments_.begin(), comments_.end(),
        [target](const Comment* comment) { return comment->id() == target; });

    if (found != comments_.end()) {
        delete *found;
        comments_.erase(found);
        return true;
    }

    for (Comment* comment : comments_) {
        if (comment->remove_reply(target)) {
            return true;
        }
    }
    return false;
}

Comment* Post::find_comment(int target) const {
    for (Comment* comment : comments_) {
        if (comment->id() == target) {
            return comment;
        }
        if (Comment* nested = comment->find(target)) {
            return nested;
        }
    }
    return nullptr;
}

const vector<Comment*>& Post::comments() const {
    return comments_;
}

void Post::write(ostream& out) const {
    if (id_ <= 0 || author_id_ <= 0) {
        throw runtime_error("Post has invalid IDs");
    }
    if (caption_.size() > 1000000) {
        throw runtime_error("Caption is too large to save");
    }
    if (comments_.size() > 1000000) {
        throw runtime_error("Too many comments to save");
    }

    const int32_t saved_id = static_cast<int32_t>(id_);
    const int32_t saved_author_id = static_cast<int32_t>(author_id_);
    out.write(reinterpret_cast<const char*>(&saved_id), sizeof(saved_id));
    out.write(reinterpret_cast<const char*>(&saved_author_id), sizeof(saved_author_id));
    if (!out) {
        throw runtime_error("Failed to write post IDs");
    }

    created_.write(out);
    const uint32_t length = static_cast<uint32_t>(caption_.size());
    out.write(reinterpret_cast<const char*>(&length), sizeof(length));
    out.write(caption_.data(), static_cast<streamsize>(caption_.size()));

    const uint8_t deleted = deleted_flag_ ? 1 : 0;
    out.write(reinterpret_cast<const char*>(&deleted), sizeof(deleted));
    if (!out) {
        throw runtime_error("Failed to write post fields");
    }

    write_likes(out);
    const uint32_t comment_count = static_cast<uint32_t>(comments_.size());
    out.write(reinterpret_cast<const char*>(&comment_count), sizeof(comment_count));
    if (!out) {
        throw runtime_error("Failed to write comment count");
    }

    for (const Comment* comment : comments_) {
        if (comment == nullptr) {
            throw runtime_error("Post contains a null comment");
        }
        comment->write(out);
    }
}

void Post::read(istream& in) {
    int32_t loaded_id = 0;
    int32_t loaded_author_id = 0;
    in.read(reinterpret_cast<char*>(&loaded_id), sizeof(loaded_id));
    in.read(reinterpret_cast<char*>(&loaded_author_id), sizeof(loaded_author_id));
    if (!in) {
        throw runtime_error("Failed to read post IDs");
    }
    if (loaded_id <= 0 || loaded_author_id <= 0) {
        throw runtime_error("File contains invalid post IDs");
    }

    Time loaded_created;
    loaded_created.read(in);

    uint32_t caption_length = 0;
    in.read(reinterpret_cast<char*>(&caption_length), sizeof(caption_length));
    if (!in || caption_length > 1000000) {
        throw runtime_error("File contains an invalid caption length");
    }

    string loaded_caption(caption_length, '\0');
    if (caption_length > 0) {
        in.read(&loaded_caption[0], static_cast<streamsize>(caption_length));
    }
    if (!in) {
        throw runtime_error("Failed to read post caption");
    }

    uint8_t loaded_deleted = 0;
    in.read(reinterpret_cast<char*>(&loaded_deleted), sizeof(loaded_deleted));
    if (!in || loaded_deleted > 1) {
        throw runtime_error("File contains an invalid deleted flag");
    }

    read_likes(in);

    uint32_t comment_count = 0;
    in.read(reinterpret_cast<char*>(&comment_count), sizeof(comment_count));
    if (!in || comment_count > 1000000) {
        throw runtime_error("File contains an invalid comment count");
    }

    vector<unique_ptr<Comment>> loaded_comments;
    loaded_comments.reserve(comment_count);
    for (uint32_t index = 0; index < comment_count; ++index) {
        unique_ptr<Comment> comment(new Comment());
        comment->read(in);
        loaded_comments.push_back(move(comment));
    }

    if (loaded_id == (numeric_limits<int>::max)()) {
        throw overflow_error("Post ID exhausted");
    }

    // Reserve before deleting current comments, so allocation failure leaves
    // the current comments owned by this Post.
    comments_.reserve(loaded_comments.size());
    for (Comment* comment : comments_) {
        delete comment;
    }
    comments_.clear();

    id_ = loaded_id;
    author_id_ = loaded_author_id;
    created_ = loaded_created;
    caption_.swap(loaded_caption);
    deleted_flag_ = loaded_deleted != 0;

    for (unique_ptr<Comment>& comment : loaded_comments) {
        comments_.push_back(comment.get());
        comment.release();
    }

    set_next_id(id_ + 1);
}

bool operator<(const Post& lhs, const Post& rhs) noexcept {
    if (lhs.created_ < rhs.created_) {
        return true;
    }
    if (rhs.created_ < lhs.created_) {
        return false;
    }
    return lhs.id_ < rhs.id_;
}
