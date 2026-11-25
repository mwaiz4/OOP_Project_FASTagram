#pragma once
#include "models/Post.h"
#include <string>
#include <vector>
class PollPost : public Post {
public:
    PollPost();
    PollPost(int author_id, const Time& created, const std::string& caption, const std::string& question, const std::vector<std::string>& options);
    PostType type() const override { return PostType::Poll; }
    std::string type_name() const;
    std::string search_text() const override;
    float render_height(float card_width) const;
    void draw(DrawContext& context, const Rectangle& bounds) const override;
    void draw(DrawContext& context, float x, float y, float width) const;
    bool handle_click(float mouse_x, float mouse_y, float x, float y, float width, int viewer_id);
    const std::string& question() const;
    void set_question(const std::string& question);
    int option_count() const;
    std::string option_text(int index) const;
    int vote_count(int index) const;
    int total_votes() const;
    bool vote(int user_id, int option_index);
    int user_vote(int user_id) const;
    bool has_voted(int user_id) const;
    bool is_closed() const;
    void set_closed(bool closed);
    void write(std::ostream& out) const override;
    void read(std::istream& in) override;
private:
    std::string question_;
    std::vector<std::string> options_;
    std::vector<std::vector<int>> option_voters_;
    bool closed_;
};
