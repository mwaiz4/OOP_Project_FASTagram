#pragma once
#include "models/Post.h"
#include <string>
class NoticePost : public Post {
public:
    NoticePost();
    NoticePost(int author_id, const Time& created, const std::string& caption, const std::string& title, const std::string& department, bool is_urgent = false, const Time& expires_at = Time(0));
    PostType type() const override { return PostType::Notice; }
    std::string type_name() const;
    std::string search_text() const override;
    float render_height(float card_width) const;
    void draw(DrawContext& context, const Rectangle& bounds) const override;
    void draw(DrawContext& context, float x, float y, float width) const;
    bool handle_click(float mouse_x, float mouse_y, float x, float y, float width, int viewer_id);
    const std::string& title() const; void set_title(const std::string& title);
    const std::string& department() const; void set_department(const std::string& department);
    bool is_urgent() const; void set_urgent(bool urgent);
    const Time& expires_at() const; void set_expires_at(const Time& expires_at);
    bool is_expired() const;
    void write(std::ostream& out) const override;
    void read(std::istream& in) override;
private:
    std::string title_, department_;
    bool is_urgent_;
    Time expires_at_;
};
