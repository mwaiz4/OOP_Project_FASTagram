#pragma once

#include "core/Time.h"
#include "models/Likeable.h"

#include <iosfwd>
#include <string>
#include <vector>

class Comment : public Likeable {
public:
    Comment();
    Comment(int id, int post_id, int author_id,
            const std::string& text, const Time& created);
    ~Comment() override;

    // Comment owns its replies and deletes them in its destructor.
    Comment(const Comment&) = delete;
    Comment& operator=(const Comment&) = delete;

    int id() const;
    int post_id() const;
    int author_id() const;
    const std::string& text() const;
    const Time& created() const;
    const std::vector<Comment*>& replies() const;

    bool is_collapsed() const;
    void set_collapsed(bool collapsed);

    void add_reply(Comment* reply);
    bool remove_reply(int reply_id);
    Comment* find(int target_id) const;

    void write(std::ostream& out) const;
    void read(std::istream& in);

private:
    int id_;
    int post_id_;
    int author_id_;
    std::string text_;
    Time created_;
    std::vector<Comment*> replies_;
    bool collapsed_;
};
