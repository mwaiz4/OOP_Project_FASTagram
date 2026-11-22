#pragma once

#include <iosfwd>
#include <vector>

class Likeable {
public:
    virtual ~Likeable() = default;

    // Returns true when this user is liked after the call.
    bool toggle_like(int user_id);
    bool remove_like(int user_id);

    bool is_liked_by(int user_id) const;
    int like_count() const;

protected:
    void write_likes(std::ostream& out) const;
    void read_likes(std::istream& in);

private:
    std::vector<int> liker_ids_;
};
