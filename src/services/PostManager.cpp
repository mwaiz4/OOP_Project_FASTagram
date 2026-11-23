#include "services/PostManager.h"

#include "services/PostFactory.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

static void delete_posts(std::vector<Post*>& posts) {
    for (Post* post : posts) {
        delete post;
    }
    posts.clear();
}

static bool newest_first(const Post* lhs, const Post* rhs) {
    if (lhs->created() != rhs->created()) {
        return rhs->created() < lhs->created();
    }
    return rhs->id() < lhs->id();
}

static std::string normalize_tag(const std::string& tag) {
    std::string result = tag;
    if (!result.empty() && result[0] == '#') {
        result.erase(0, 1);
    }

    for (char& character : result) {
        character = static_cast<char>(
            std::tolower(static_cast<unsigned char>(character)));
    }
    return result;
}

static bool caption_has_tag(const std::string& caption, const std::string& wanted_tag) {
    if (wanted_tag.empty()) {
        return false;
    }

    for (std::size_t position = 0; position < caption.size(); ++position) {
        if (caption[position] != '#') {
            continue;
        }

        std::size_t end = position + 1;
        while (end < caption.size()) {
            const unsigned char character =
                static_cast<unsigned char>(caption[end]);
            if (!std::isalnum(character) && character != '_') {
                break;
            }
            ++end;
        }

        if (end == position + 1) {
            continue;
        }

        const std::string found_tag =
            normalize_tag(caption.substr(position, end - position));
        if (found_tag == wanted_tag) {
            return true;
        }
        position = end - 1;
    }
    return false;
}

PostManager::PostManager()
    : posts_(), pending_deletes_(), changed_(false) {}

PostManager::~PostManager() {
    delete_posts(posts_);
    delete_posts(pending_deletes_);
}

void PostManager::add(Post* post) {
    if (post == nullptr) {
        throw std::invalid_argument("Cannot add a null post");
    }
    if (find(post->id()) != nullptr) {
        throw std::invalid_argument("A post with this ID already exists");
    }

    posts_.push_back(post);
    changed_ = true;
}

Post* PostManager::find(int post_id) const {
    const auto found = std::find_if(posts_.begin(), posts_.end(),
        [post_id](const Post* post) { return post->id() == post_id; });
    return found == posts_.end() ? nullptr : *found;
}

std::vector<Post*> PostManager::posts_by(int author_id) const {
    std::vector<Post*> result;
    for (Post* post : posts_) {
        if (!post->is_deleted() && post->author_id() == author_id) {
            result.push_back(post);
        }
    }
    std::sort(result.begin(), result.end(), newest_first);
    return result;
}

std::vector<Post*> PostManager::feed_candidates() const {
    std::vector<Post*> result;
    for (Post* post : posts_) {
        if (!post->is_deleted()) {
            result.push_back(post);
        }
    }
    std::sort(result.begin(), result.end(), newest_first);
    return result;
}

std::vector<Post*> PostManager::pinned_notices() const {
    std::vector<Post*> result;
    for (Post* post : posts_) {
        if (!post->is_deleted() && post->type() == PostType::Notice) {
            result.push_back(post);
        }
    }
    std::sort(result.begin(), result.end(), newest_first);
    return result;
}

std::vector<Post*> PostManager::visible_posts(const Account* viewer) const {
    (void)viewer;
    return feed_candidates();
}

std::vector<Post*> PostManager::posts_by_tag(const std::string& tag) const {
    const std::string wanted_tag = normalize_tag(tag);
    std::vector<Post*> result;
    if (wanted_tag.empty()) {
        return result;
    }

    for (Post* post : posts_) {
        if (!post->is_deleted() && caption_has_tag(post->caption(), wanted_tag)) {
            result.push_back(post);
        }
    }
    std::sort(result.begin(), result.end(), newest_first);
    return result;
}

bool PostManager::edit_caption(int post_id, int requester_id,
                               const std::string& new_caption) {
    Post* post = find(post_id);
    if (post == nullptr || post->is_deleted() ||
        post->author_id() != requester_id) {
        return false;
    }

    post->set_caption(new_caption);
    changed_ = true;
    return true;
}

bool PostManager::remove(int post_id, int requester_id, bool is_admin) {
    const auto found = std::find_if(posts_.begin(), posts_.end(),
        [post_id](const Post* post) { return post->id() == post_id; });
    if (found == posts_.end()) {
        return false;
    }

    Post* post = *found;
    if (!is_admin && post->author_id() != requester_id) {
        return false;
    }

    pending_deletes_.push_back(post);
    post->set_deleted(true);
    posts_.erase(found);
    changed_ = true;
    return true;
}

bool PostManager::undo_delete() {
    if (pending_deletes_.empty()) {
        return false;
    }

    Post* post = pending_deletes_.back();
    posts_.push_back(post);
    pending_deletes_.pop_back();
    post->set_deleted(false);
    changed_ = true;
    return true;
}

void PostManager::finalize_pending_deletes() {
    delete_posts(pending_deletes_);
}

void PostManager::delete_by_user(int user_id) {
    bool removed_any = false;
    const auto remove_authored_posts = [user_id, &removed_any](
        std::vector<Post*>& posts) {
        auto current = posts.begin();
        while (current != posts.end()) {
            if ((*current)->author_id() == user_id) {
                delete *current;
                current = posts.erase(current);
                removed_any = true;
            } else {
                ++current;
            }
        }
    };

    remove_authored_posts(posts_);
    remove_authored_posts(pending_deletes_);
    if (removed_any) {
        changed_ = true;
    }
}

void PostManager::delete_post_cascade(int post_id) {
    const auto remove_post = [post_id](std::vector<Post*>& posts) {
        const auto found = std::find_if(posts.begin(), posts.end(),
            [post_id](const Post* post) { return post->id() == post_id; });
        if (found == posts.end()) {
            return false;
        }
        delete *found;
        posts.erase(found);
        return true;
    };

    if (remove_post(posts_) || remove_post(pending_deletes_)) {
        changed_ = true;
    }
}

bool PostManager::toggle_like(int post_id, int user_id) {
    Post* post = find(post_id);
    if (post == nullptr || post->is_deleted()) {
        return false;
    }

    post->toggle_like(user_id);
    changed_ = true;
    return true;
}

bool PostManager::add_comment(int post_id, Comment* comment) {
    if (comment == nullptr) {
        throw std::invalid_argument("Cannot add a null comment");
    }

    Post* post = find(post_id);
    if (post == nullptr || post->is_deleted()) {
        delete comment;
        return false;
    }

    post->add_comment(comment);
    changed_ = true;
    return true;
}

void PostManager::mark_changed() {
    changed_ = true;
}

bool PostManager::is_changed() const {
    return changed_;
}

void PostManager::save(const std::string& filepath) {
    if (posts_.size() > 100000) {
        throw std::runtime_error("Too many posts to save");
    }

    std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open post file for writing");
    }

    const char magic[4] = {'F', 'S', 'T', 'P'};
    const std::uint32_t version = 1;
    const std::uint32_t post_count = static_cast<std::uint32_t>(posts_.size());
    out.write(magic, sizeof(magic));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&post_count), sizeof(post_count));
    if (!out) {
        throw std::runtime_error("Failed to write post file header");
    }

    for (const Post* post : posts_) {
        if (post == nullptr) {
            throw std::runtime_error("Post manager contains a null post");
        }
        const std::uint8_t type_tag = static_cast<std::uint8_t>(post->type());
        out.write(reinterpret_cast<const char*>(&type_tag), sizeof(type_tag));
        if (!out) {
            throw std::runtime_error("Failed to write post type tag");
        }
        post->write(out);
    }

    out.close();
    if (!out) {
        throw std::runtime_error("Failed to finish writing post file");
    }
    changed_ = false;
}

void PostManager::load(const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open()) {
        delete_posts(posts_);
        delete_posts(pending_deletes_);
        changed_ = false;
        return;
    }

    char magic[4]{};
    std::uint32_t version = 0;
    std::uint32_t post_count = 0;
    in.read(magic, sizeof(magic));
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    in.read(reinterpret_cast<char*>(&post_count), sizeof(post_count));
    if (!in || std::memcmp(magic, "FSTP", sizeof(magic)) != 0 ||
        version != 1 || post_count > 100000) {
        throw std::runtime_error("Post file header is invalid");
    }

    std::vector<std::unique_ptr<Post>> loaded_posts;
    loaded_posts.reserve(post_count);
    for (std::uint32_t index = 0; index < post_count; ++index) {
        loaded_posts.emplace_back(PostFactory::read_from_stream(in));
    }

    // Allocate capacity before replacing the current data. If allocation fails,
    // the current manager contents are still owned and unchanged.
    posts_.reserve(loaded_posts.size());
    delete_posts(posts_);
    delete_posts(pending_deletes_);
    for (std::unique_ptr<Post>& post : loaded_posts) {
        posts_.push_back(post.release());
    }
    changed_ = false;
}
