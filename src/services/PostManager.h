#pragma once

#include "models/Comment.h"
#include "models/Post.h"

#include <string>
#include <vector>

class Account;

class PostManager {
public:
    PostManager();
    ~PostManager();

    // PostManager owns posts added to it and deletes them in its destructor.
    PostManager(const PostManager&) = delete;
    PostManager& operator=(const PostManager&) = delete;

    void add(Post* post);
    Post* find(int post_id) const;
    std::vector<Post*> posts_by(int author_id) const;
    std::vector<Post*> feed_candidates() const;
    std::vector<Post*> pinned_notices() const;
    std::vector<Post*> visible_posts(const Account* viewer) const;
    std::vector<Post*> posts_by_tag(const std::string& tag) const;

    bool edit_caption(int post_id, int requester_id,
                      const std::string& new_caption);
    bool remove(int post_id, int requester_id, bool is_admin);
    bool undo_delete();
    void finalize_pending_deletes();
    void delete_by_user(int user_id);
    void delete_post_cascade(int post_id);

    bool toggle_like(int post_id, int user_id);
    bool add_comment(int post_id, Comment* comment);

    void mark_changed();
    bool is_changed() const;

    void load(const std::string& filepath);
    void save(const std::string& filepath);

private:
    // Active posts and pending posts are both owned by PostManager.
    std::vector<Post*> posts_;
    std::vector<Post*> pending_deletes_;
    bool changed_;
};
