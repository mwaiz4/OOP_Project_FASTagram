#pragma once

#include "core/Time.h"
#include "models/Likeable.h"

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

class Comment;
struct DrawContext;
// Compatible with raylib's later `typedef struct Rectangle { ... } Rectangle;`.
struct Rectangle;

enum class PostType : std::uint8_t {
    Photo = 1,
    Text = 2,
    Carousel = 3,
    Poll = 4,
    Notice = 5,
    LostFound = 6
};

class Post : public Likeable {
public:
    Post();
    Post(int author_id, const Time& created, const std::string& caption);
    virtual ~Post();

    // Post owns its comments and deletes them in its destructor.
    Post(const Post&) = delete;
    Post& operator=(const Post&) = delete;

    static void set_next_id(int next_id);
    static int generate_id();

    int id() const;
    int author_id() const;
    const Time& created() const;
    const std::string& caption() const;
    bool is_deleted() const;

    void set_caption(const std::string& caption);
    void set_deleted(bool deleted);

    void add_comment(Comment* comment);
    bool remove_comment(int comment_id);
    Comment* find_comment(int comment_id) const;
    const std::vector<Comment*>& comments() const;

    virtual PostType type() const = 0;
    virtual void draw(DrawContext& context, const Rectangle& bounds) const = 0;
    virtual std::string search_text() const = 0;

    virtual void write(std::ostream& out) const;
    virtual void read(std::istream& in);

    friend bool operator<(const Post& lhs, const Post& rhs) noexcept;

private:
    int id_;
    int author_id_;
    Time created_;
    std::string caption_;
    bool deleted_flag_;
    std::vector<Comment*> comments_;
    static int next_id_;
};
